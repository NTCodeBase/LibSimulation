//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//    .--------------------------------------------------.
//    |  This file is part of NTCodeBase                 |
//    |  Created 2018 by NT (https://ttnghia.github.io)  |
//    '--------------------------------------------------'
//                            \o/
//                             |
//                            / |
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#pragma once

#include <LibCommon/Utils/JSONHelpers.h>
#include <LibSimulation/Data/StringHash.h>
#include <type_traits>
#include <variant>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class Parameter {
public:
    Parameter() { throw std::exception("This should not be reach!"); }
    virtual ~Parameter() {}

    Parameter(const String& groupName, const String& paramName, const String& paramDesc) :
        m_Group(groupName), m_ParamName(paramName), m_ParamDescription(paramDesc) {}
    ////////////////////////////////////////////////////////////////////////////////
    const auto& groupName() const { return m_Group; }
    const auto& paramName() const { return m_ParamName; }
    const auto& description() const { return m_ParamDescription; }
    ////////////////////////////////////////////////////////////////////////////////
    template<class Input> void set(const Input& val) { m_Data = val; }
    template<class Output> Output& get() { assert(std::holds_alternative<Output>(m_Data)); return std::get<Output>(m_Data); }
    template<class Output> Output get() const { assert(std::holds_alternative<Output>(m_Data)); return std::get<Output>(m_Data); }
    ////////////////////////////////////////////////////////////////////////////////
    template<class Input> void parseRequiredValue(const JParams& jParams) { __NT_REQUIRE(parseValue<Input>(jParams)); }
    template<class Input> bool parseValue(const JParams& jParams) {
        assert(std::holds_alternative<Input>(m_Data));
        if constexpr (std::is_same_v<Input, bool>) {
            bool& bVal = std::get<bool>(m_Data);
            return JSONHelpers::readBool(jParams, bVal, m_ParamName);
        } else {
            if constexpr (std::is_same_v<Input, int>|| std::is_same_v<Input, UInt>
                          || std::is_same_v<Input, float>|| std::is_same_v<Input, double>
                          || std::is_same_v<Input, String>) {
                Input& val = std::get<Input>(m_Data);
                return JSONHelpers::readValue(jParams, val, m_ParamName);
            } else {
                Input& val = std::get<Input>(m_Data);
                return JSONHelpers::readVector(jParams, val, m_ParamName);
            }
        }
    }

private:
    using ParameterData = std::variant<bool, int, UInt, float, double, Vec3i, Vec3ui, Vec3f, Vec4i, Vec4ui, Vec4d, String>;
    ParameterData m_Data;

    String m_Group;
    String m_ParamName;
    String m_ParamDescription;
};
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class ParameterManager {
public:
    ParameterManager()          = default;
    virtual ~ParameterManager() = default;

    ////////////////////////////////////////////////////////////////////////////////
    /**
     * \brief Group must be added before adding parameters of that group
     */
    void addGroup(const char* groupName, const char* groupDesc) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && !hasGroup(groupName));
        auto hashVal = StringHash::hash(groupName);
        m_Parameters[hashVal] = {};
        m_GroupInfo[hashVal]  = std::pair(groupName, groupDesc);
    }

    ////////////////////////////////////////////////////////////////////////////////
    template<class Input>
    Parameter& addParameter(const char* groupName, const char* paramName, const char* description, const Input& defaultValue) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && StringHash::isValidHash(paramName) && hasGroup(groupName));
        auto [iter, bSuccess] = m_Parameters[StringHash::hash(groupName)].emplace(StringHash::hash(paramName), Parameter(groupName, paramName, description));
        __NT_REQUIRE(bSuccess);
        iter->second.set<Input>(defaultValue);
        return iter->second;
    }

    template<class Input>
    Parameter& addParameter(const char* groupName, const char* paramName, const char* description, const Input& defaultValue,
                            const JParams& jParams, bool bRequiredInptParam = false) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && StringHash::isValidHash(paramName) && hasGroup(groupName));
        auto [iter, bSuccess] = m_Parameters[StringHash::hash(groupName)].emplace(StringHash::hash(paramName), Parameter(groupName, paramName, description));
        __NT_REQUIRE(bSuccess);
        auto& param = iter->second;

        param.set<Input>(defaultValue);
        if(bRequiredInptParam) {
            param.parseRequiredValue<Input>(jParams);
        } else {
            param.parseValue<Input>(jParams);
        }
        return param;
    }

    Parameter& parameter(const char* groupName, const char* paramName) {
        assert(hasParmeter(groupName, paramName));
        return m_Parameters.at(StringHash::hash(groupName)).at(StringHash::hash(paramName));
    }

    const Parameter& parameter(const char* groupName, const char* paramName) const {
        assert(hasParmeter(groupName, paramName));
        return m_Parameters.at(StringHash::hash(groupName)).at(StringHash::hash(paramName));
    }

    ////////////////////////////////////////////////////////////////////////////////
    const auto& getAllParameters() const { return m_Parameters; }
    const auto& getGroupParameters(const char* groupName) const { assert(hasGroup(groupName)); return m_Parameters.at(StringHash::hash(groupName)); }
    const auto& getGroupParameters(size_t group) const { assert(hasGroup(group)); return m_Parameters.at(group); }

    auto getGroupNameWithParams(const char* groupName) const {
        auto group = StringHash::hash(groupName); assert(hasGroup(group));
        return std::pair<String, const std::unordered_map<size_t, Parameter>&>(m_GroupInfo.at(group).first, m_Parameters.at(group));
    }

    void removeGroup(const char* groupName) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && hasGroup(groupName));
        m_Parameters.erase(StringHash::hash(groupName));
    }

    void removeParameter(const char* groupName, const char* paramName) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && StringHash::isValidHash(paramName) && hasParmeter(groupName, paramName));
        auto& groupParams = m_Parameters[StringHash::hash(groupName)];
        groupParams.erase(StringHash::hash(paramName));
    }

    ////////////////////////////////////////////////////////////////////////////////
    bool hasGroup(size_t group) const { return m_Parameters.find(group) != m_Parameters.end(); }
    bool hasGroup(const char* groupName) const { return hasGroup(StringHash::hash(groupName)); }
    bool hasParmeter(const char* groupName, const char* paramName) const {
        if(!hasGroup(groupName)) { return false; }
        const auto& groupParams = getGroupParameters(groupName);
        return groupParams.find(StringHash::hash(paramName)) != groupParams.cend();
    }

private:
    ////////////////////////////////////////////////////////////////////////////////
    // store group data
    std::unordered_map<size_t, std::pair<String, String>> m_GroupInfo;

    // store properties, index by hashes of groups then by hashes of property names
    std::unordered_map<size_t, std::unordered_map<size_t, Parameter>> m_Parameters;
};
