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
#include <LibCommon/Utils/NumberHelpers.h>

#include <LibSimulation/Data/StringHash.h>

#include <type_traits>
#include <variant>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class Parameter {
public:
    Parameter() { throw std::exception("This should not be reach!"); }
    virtual ~Parameter() {}

    Parameter(const String& groupName, const String& paramName, const String& paramDesc) :
        m_Group(groupName), m_Name(paramName), m_Description(paramDesc) {}
    ////////////////////////////////////////////////////////////////////////////////
    const auto& group() const { return m_Group; }
    const auto& name() const { return m_Name; }
    const auto& description() const { return m_Description; }
    ////////////////////////////////////////////////////////////////////////////////
    template<class Input> void set(const Input& val) { m_Data = val; }
    template<class Output> Output& get() { assert(std::holds_alternative<Output>(m_Data)); return std::get<Output>(m_Data); }
    template<class Output> const Output& get() const { assert(std::holds_alternative<Output>(m_Data)); return std::get<Output>(m_Data); }
    ////////////////////////////////////////////////////////////////////////////////
    template<class Input> void parseRequiredValue(const JParams& jParams) { __NT_REQUIRE(parseValue<Input>(jParams)); }
    template<class Input> bool parseValue(const JParams& jParams) {
        assert(std::holds_alternative<Input>(m_Data));
        if constexpr (std::is_same_v<Input, bool>) {
            bool& bVal = std::get<bool>(m_Data);
            return JSONHelpers::readBool(jParams, bVal, m_Name);
        } else {
            if constexpr (std::is_same_v<Input, int>|| std::is_same_v<Input, UInt>
                          || std::is_same_v<Input, float>|| std::is_same_v<Input, double>
                          || std::is_same_v<Input, String>) {
                Input& val = std::get<Input>(m_Data);
                return JSONHelpers::readValue(jParams, val, m_Name);
            } else {
                Input& val = std::get<Input>(m_Data);
                return JSONHelpers::readVector(jParams, val, m_Name);
            }
        }
    }

private:
    String m_Group;
    String m_Name;
    String m_Description;

    using ParameterData = std::variant<bool, int, UInt, float, double, Vec2i, Vec2ui, Vec2f, Vec3i, Vec3ui, Vec3f, Vec4i, Vec4ui, Vec4d, String>;
    ParameterData m_Data;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class ParameterGroup {
public:
    ParameterGroup(const String& name, const String& desc) : m_Name(name), m_Description(desc) {}
    const auto& name() const { return m_Name; }
    ////////////////////////////////////////////////////////////////////////////////
    template<class Input>
    Parameter& addParameter(const char* paramName, const char* description, const Input& defaultValue) {
        __NT_REQUIRE(StringHash::isValidHash(paramName) && !hasParameter(paramName));
        auto [iter, bSuccess] = m_ParameterGroups.emplace(StringHash::hash(paramName), Parameter(m_Name, paramName, description));
        __NT_REQUIRE(bSuccess);
        auto& param = iter->second;
        param.set<Input>(defaultValue);
        return param;
    }

    template<class Input>
    Parameter& addParameter(const char* paramName, const char* description, const Input& defaultValue, const JParams& jParams,
                            bool bRequiredInput = false) {
        __NT_REQUIRE(StringHash::isValidHash(paramName) && !hasParameter(paramName));
        auto [iter, bSuccess] = m_ParameterGroups.emplace(StringHash::hash(paramName), Parameter(m_Name, paramName, description));
        __NT_REQUIRE(bSuccess);
        auto& param = iter->second;
        param.set<Input>(defaultValue);
        if(bRequiredInput) {
            param.parseRequiredValue<Input>(jParams);
        } else {
            param.parseValue<Input>(jParams);
        }
        return param;
    }

    void removeParameter(const char* paramName) {
        __NT_REQUIRE(StringHash::isValidHash(paramName) && hasParameter(paramName));
        m_ParameterGroups.erase(StringHash::hash(paramName));
    }

    ////////////////////////////////////////////////////////////////////////////////
    bool hasParameter(const char* paramName) const { return m_ParameterGroups.find(StringHash::hash(paramName)) != m_ParameterGroups.cend(); }
    Parameter& parameter(const char* paramName) { assert(hasParameter(paramName)); return m_ParameterGroups.at(StringHash::hash(paramName)); }
    const Parameter& parameter(const char* paramName) const { assert(hasParameter(paramName)); return m_ParameterGroups.at(StringHash::hash(paramName)); }

private:
    String                              m_Name;
    String                              m_Description;
    std::unordered_map<UInt, Parameter> m_ParameterGroups;
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
        m_ParameterGroups.emplace(hashVal, ParameterGroup(String(groupName), String(groupDesc)));
    }

    UInt addAnonymousGroup(const char* groupDesc) {
        auto groupHash = NumberHelpers::iRand<UInt>::rnd();
        while(hasGroup(groupHash)) {
            groupHash = NumberHelpers::iRand<UInt>::rnd();
        }
        m_ParameterGroups.emplace(groupHash, ParameterGroup(std::to_string(groupHash), String(groupDesc)));
        return groupHash;
    }

    ////////////////////////////////////////////////////////////////////////////////
    template<class Input>
    Parameter& addParameter(UInt groupHash, const char* paramName, const char* description, const Input& defaultValue) {
        __NT_REQUIRE(hasGroup(groupHash));
        return m_ParameterGroups.at(groupHash).addParameter(paramName, description, defaultValue);
    }

    template<class Input>
    Parameter& addParameter(UInt groupHash, const char* paramName, const char* description, const Input& defaultValue, const JParams& jParams,
                            bool bRequiredInput = false) {
        __NT_REQUIRE(hasGroup(groupHash));
        return m_ParameterGroups.at(groupHash).addParameter(paramName, description, defaultValue, jParams, bRequiredInput);
    }

    template<class Input, class... Args>
    Parameter& addParameter(const char* groupName, Args&& ... args) {
        __NT_REQUIRE(StringHash::isValidHash(groupName));
        return addParameter<Input>(StringHash::hash(groupName), std::forward<Args>(args)...);
    }

    ////////////////////////////////////////////////////////////////////////////////
    Parameter& parameter(UInt groupHash, const char* paramName) {
        assert(hasGroup(groupHash));
        return m_ParameterGroups.at(groupHash).parameter(paramName);
    }

    const Parameter& parameter(UInt groupHash, const char* paramName) const {
        assert(hasGroup(groupHash));
        return m_ParameterGroups.at(groupHash).parameter(paramName);
    }

    Parameter& parameter(const char* groupName, const char* paramName) { return parameter(StringHash::hash(groupName), paramName); }
    const Parameter& parameter(const char* groupName, const char* paramName) const { return parameter(StringHash::hash(groupName), paramName); }
    ////////////////////////////////////////////////////////////////////////////////
    auto& getAllGroups() { return m_ParameterGroups; }
    const auto& getAllGroups() const { return m_ParameterGroups; }

    auto& group(UInt groupHash) { assert(hasGroup(groupHash)); return m_ParameterGroups.at(groupHash); }
    const auto& group(UInt groupHash) const { assert(hasGroup(groupHash)); return m_ParameterGroups.at(groupHash); }

    auto& group(const char* groupName) { return group(StringHash::hash(groupName)); }
    const auto& group(const char* groupName) const { return group(StringHash::hash(groupName)); }

    void removeGroup(const char* groupName) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && hasGroup(groupName));
        m_ParameterGroups.erase(StringHash::hash(groupName));
    }

    ////////////////////////////////////////////////////////////////////////////////
    bool hasGroup(UInt groupHash) const { return m_ParameterGroups.find(groupHash) != m_ParameterGroups.end(); }
    bool hasGroup(const char* groupName) const { return hasGroup(StringHash::hash(groupName)); }
    bool hasParmeter(UInt groupHash, const char* paramName) const {
        if(!hasGroup(groupHash)) { return false; }
        return group(groupHash).hasParameter(paramName);
    }

    bool hasParmeter(const char* groupName, const char* paramName) const { return hasParmeter(StringHash::hash(groupName), paramName); }

private:
    std::unordered_map<UInt, ParameterGroup> m_ParameterGroups;
};
