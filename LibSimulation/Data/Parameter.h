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

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<class Real_t>
class Parameter {
public:
    Parameter(const String& groupName, const String& paramName, const String& paramDesc = nullptr) :
        m_Group(groupName), m_ParamName(paramName), m_ParamDescription(paramDesc) {}
    virtual ~Parameter() = default;
    const auto& groupName() const { return m_Group; }
    const auto& paramName() const { return m_ParamName; }
    const auto& description() const { return m_ParamDescription; }
    ////////////////////////////////////////////////////////////////////////////////
    template<class Output> Output get() const { static_assert(false, "This line should not be reached!"); }

    template<> bool get<bool>() const { return v_bool; }
    template<> int get<int>() const { return v_int; }
    template<> UInt get<UInt>() const { return v_uint; }
    template<> Real_t get<Real_t>() const { return v_real; }

    template<> Vec3i get<Vec3i>() const { return v3_int; }
    template<> Vec3ui get<Vec3ui>() const { return v3_uint; }
    template<> Vec3<Real_t> get<Vec3<Real_t>>() const { return v3_real; }

    template<> Vec4i get<Vec4i>() const { return v4_int; }
    template<> Vec4ui get<Vec4ui>() const { return v4_uint; }
    template<> Vec4<Real_t> get<Vec4<Real_t>>() const { return v4_real; }
    ////////////////////////////////////////////////////////////////////////////////
    template<class Input> void set(const Input&) { static_assert(false, "This line should not be reached!"); }

    template<> void set<bool>(const bool& val) { v_bool = val; }
    template<> void set<int>(const int& val) { v_int = val; }
    template<> void set<UInt>(const UInt& val) { v_uint = val; }
    template<> void set<Real_t>(const Real_t& val) { v_real = val; }

    template<> void set<Vec3i>(const Vec3i& val) { v3_int = val; }
    template<> void set<Vec3ui>(const Vec3ui& val) { v3_uint = val; }
    template<> void set<Vec3<Real_t>>(const Vec3<Real_t>& val) { v3_real = val; }

    template<> void set<Vec4i>(const Vec4i& val) { v4_int = val; }
    template<> void set<Vec4ui>(const Vec4ui& val) { v4_uint = val; }
    template<> void set<Vec4<Real_t>>(const Vec4<Real_t>& val) { v4_real = val; }
    ////////////////////////////////////////////////////////////////////////////////
    template<class Input> bool parseValue(const JParams&) { static_assert(false, "This line should not be reached!"); }

    template<> bool parseValue<bool>(const JParams& jParams) { return JSONHelpers::readBool(jParams, v_bool, m_ParamName); }
    template<> bool parseValue<int>(const JParams& jParams) { return JSONHelpers::readValue(jParams, v_int, m_ParamName); }
    template<> bool parseValue<UInt>(const JParams& jParams) { return JSONHelpers::readValue(jParams, v_uint, m_ParamName); }
    template<> bool parseValue<Real_t>(const JParams& jParams) { return JSONHelpers::readValue(jParams, v_real, m_ParamName); }

    template<> bool parseValue<Vec3i>(const JParams&) { return JSONHelpers::readVector(jParams, v3_int, m_ParamName); }
    template<> bool parseValue<Vec3ui>(const JParams&) { return JSONHelpers::readVector(jParams, v3_uint, m_ParamName); }
    template<> bool parseValue<Vec3<Real_t>>(const JParams&) { return JSONHelpers::readVector(jParams, v3_real, m_ParamName); }

    template<> bool parseValue<Vec4i>(const JParams&) { return JSONHelpers::readVector(jParams, v4_int, m_ParamName); }
    template<> bool parseValue<Vec4ui>(const JParams&) { return JSONHelpers::readVector(jParams, v4_uint, m_ParamName); }
    template<> bool parseValue<Vec4<Real_t>>(const JParams&) { return JSONHelpers::readVector(jParams, v4_real, m_ParamName); }

private:
    union  {
        bool   v_bool;
        int    v_int;
        UInt   v_uint;
        Real_t v_real;

        Vec3i        v3_int;
        Vec3ui       v3_uint;
        Vec3<Real_t> v3_real;

        Vec4i        v4_int;
        Vec4ui       v4_uint;
        Vec4<Real_t> v4_real;
    };

    String m_Group;
    String m_ParamName;
    String m_ParamDescription;
};
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<class Real_t>
class ParameterManager {
public:
    ParameterManager()          = default;
    virtual ~ParameterManager() = default;

    ////////////////////////////////////////////////////////////////////////////////
    /**
     * \brief Group must be added before adding parameters of that group
     */
    void addGroup(const char* groupName) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && !hasGroup(groupName));
        auto hashVal = StringHash::hash(groupName);
        m_Parameters[hashVal] = {};
        m_GroupNames[hashVal] = String(groupName);
    }

    ////////////////////////////////////////////////////////////////////////////////
    void addParameter(const char* groupName, const char* paramName, const char* description) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && StringHash::isValidHash(paramName) && hasGroup(groupName));
        m_Parameters[StringHash::hash(groupName)][StringHash::hash(paramName)] = Parameter<Real_t>(groupName, paramName, description);
    }

    template<class Input>
    void addParameter(const char* groupName, const char* paramName, const char* description, const Input& defaultValue) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && StringHash::isValidHash(paramName) && hasGroup(groupName));
        auto param = Parameter<Real_t>(groupName, paramName, description);
        param.set<Input>(defaultValue);
        m_Parameters[StringHash::hash(groupName)][StringHash::hash(paramName)] = std::move(param);
    }

    auto& parameter(const char* groupName, const char* paramName) {
        assert(hasParmeter(groupName, paramName));
        return m_Parameters.at(StringHash::hash(groupName)).at(StringHash::hash(paramName));
    }

    const auto& parameter(const char* groupName, const char* paramName) const {
        assert(hasParmeter(groupName, paramName));
        return m_Parameters.at(StringHash::hash(groupName)).at(StringHash::hash(paramName));
    }

    ////////////////////////////////////////////////////////////////////////////////
    const auto& getAllParameters() const { return m_Parameters; }
    const auto& getGroupParameters(const char* groupName) const { assert(hasGroup(groupName)); return m_Parameters.at(StringHash::hash(groupName)); }
    const auto& getGroupParameters(size_t group) const { assert(hasGroup(group)); return m_Parameters.at(group); }

    auto getGroupNameWithParams(const char* groupName) const {
        auto group = StringHash::hash(groupName); assert(hasGroup(group));
        return std::pair<String, const std::unordered_map<size_t, Parameter<Real_t>>&>(m_GroupNames.at(group), m_Parameters.at(group));
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
    std::unordered_map<size_t, String> m_GroupNames;

    // store properties, index by hashes of groups then by hashes of property names
    std::unordered_map<size_t, std::unordered_map<size_t, Parameter<Real_t>>> m_Parameters;
};
