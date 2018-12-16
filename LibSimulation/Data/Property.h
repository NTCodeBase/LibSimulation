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

#include <LibCommon/CommonSetup.h>
#include <unordered_map>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class PropertyBase {
public:
    PropertyBase(const String& groupName, const String& propName) : m_Group(groupName), m_PropertyName(propName) {}
    virtual ~PropertyBase() {}
    const String& groupName() const { return m_Group; }
    const String& propertyName() const { return m_PropName; }
    ////////////////////////////////////////////////////////////////////////////////
    virtual void reserve(size_t n)    = 0;
    virtual void resize(size_t n)     = 0;
    virtual void push_back()          = 0;
    virtual bool removeAt(size_t idx) = 0;
protected:
    String m_Group;
    String m_PropertyName;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<typename T>
class Property : public PropertyBase {
public:
    Property(const String& groupName, const String& propName) : PropertyBase(groupName, propName), m_bHasDefaultVal(false) {}
    Property(const String& groupName, const String& propName, const T& defaultVal_) : PropertyBase(groupName, propName),
        m_DefaultVal(defaultVal_), m_bHasDefaultVal(true) {}
    ////////////////////////////////////////////////////////////////////////////////
    virtual void reserve(size_t n) { m_Data.reserve(n); }
    virtual void resize(size_t n) { if(m_bHasDefaultVal) { m_Data.resize(n, m_DefaultVal); } else { m_Data.resize(n); } }
    virtual void push_back() { if(m_bHasDefaultVal) { m_Data.push_back(m_DefaultVal); } else { m_Data.push_back(T()); } }
    virtual bool removeAt(size_t idx) { assert(idx < m_Data.size()); m_Data.erase(m_Data.begin() + idx); }
    ////////////////////////////////////////////////////////////////////////////////
    size_t size() const { return m_Data.size(); }
    ////////////////////////////////////////////////////////////////////////////////
    StdVT<T>& data() { return m_Data; }
    const StdVT<T>& data() const { return m_Data; }
    ////////////////////////////////////////////////////////////////////////////////
    T* dataPtr() { return m_Data.data(); }
    const T* dataPtr() const { return m_Data.data(); }
    ////////////////////////////////////////////////////////////////////////////////
    T& operator[](size_t idx) { return m_Data[idx]; }
    const T& operator[](size_t idx) const { return m_Data[idx]; }
protected:
    StdVT<T> m_Data;
    T        m_DefaultVal;
    bool     m_bHasDefaultVal;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class PropertyManager {
public:
    PropertyManager() = default;
    virtual ~PropertyManager() {
        for(auto& [groupName, groupProps]: m_Properties) {
            for(auto& [propName, prop]: groupProps) {
                delete prop;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    /**
     * \brief Group must be added before adding properties of that group
     */
    void addGroup(const char* groupName) {
        __NT_REQUIRE(isValidHash(groupName) && !hasGroup(groupName));
        auto hashVal = strHash(groupName);
        m_Properties[hashVal] = {};
        m_GroupNames[hashVal] = String(groupName);
    }

    ////////////////////////////////////////////////////////////////////////////////
    template<class T>
    void addProperty(const char* groupName, const char* propName) {
        __NT_REQUIRE(isValidHash(groupName) && isValidHash(propName) && hasGroup(groupName));
        m_Properties[strHash(groupName)][strHash(propName)] = new Property<T>(groupName, propName);
    }

    template<class T>
    void addProperty(const char* groupName, const char* propName,  const T& defaultValue) {
        __NT_REQUIRE(isValidHash(groupName) && isValidHash(propName) && hasGroup(groupName));
        m_Properties[strHash(groupName)][strHash(propName)] = new Property<T>(groupName, propName, defaultValue);
    }

    template<class T>
    Property<T>* property(const char* groupName, const char* propName) {
        assert(hasProperty(groupName, propName));
        auto propPtr = m_Properties[strHash(groupName)][strHash(propName)]; assert(propPtr != nullptr && dynamic_cast<Property<T>*>(propPtr) != nullptr);
        return static_cast<Property<T>*>(propPtr);
    }

    template<class T>
    const Property<T>* property(const char* groupName, const char* propName) const {
        assert(hasProperty(groupName, propName));
        const auto propPtr = m_Properties[strHash(groupName)][strHash(propName)]; assert(propPtr != nullptr && dynamic_cast<Property<T>*>(propPtr) != nullptr);
        return static_cast<const Property<T>*>(propPtr);
    }

    ////////////////////////////////////////////////////////////////////////////////
    size_g getGroupDataSize(const char* groupName) const {
        return m_GroupDataSizes[strHash(groupName)];
    }

    void resizeGroupData(const char* groupName, size_t size) {
        assert(hasGroup(groupName));
        auto  hashVal    = strHash(groupName);
        auto& groupProps = m_Properties[hashVal];
        for(auto& kv: groupProps) {
            kv.second->resize(size);
        }
        m_GroupDataSizes[hashVal] = size;
    }

    void reserveGroupData(const char* groupName, size_t size) {
        assert(hasGroup(groupName));
        auto& groupProps = m_Properties[strHash(groupName)];
        for(auto& kv: groupProps) {
            kv.second->reserve(size);
        }
    }

    void removeGroup(const char* groupName) {
        __NT_REQUIRE(isValidHash(groupName) && hasGroup(groupName));
        m_Properties.erase(strHash(groupName));
    }

    void removeProperty(const char* groupName, const char* propName) {
        __NT_REQUIRE(isValidHash(groupName) && isValidHash(propName) && hasProperty(groupName, propName));
        auto& groupProps = getGroupProperties(groupName);
        groupProps.erase(strHash(propName));
    }

    void removeGroupElementAt(const char* groupName, size_t idx) {
        assert(isValidHash(groupName) && hasGroup(groupName));
        auto hashVal = strHash(groupName);
        assert(m_GroupDataSizes[hashVal] > idx);
        auto& groupProps = getGroupProperties(hashVal);
        for(auto& kv: groupProps) {
            kv.second->removeAt(idx);
        }
        m_GroupDataSizes[hashVal] = groupProps.begin()->size();
    }

    ////////////////////////////////////////////////////////////////////////////////
    bool hasGroup(const char* groupName) const {
        return m_Properties.find(strHash(groupName)) != m_Properties.end();
    }

    bool hasProperty(const char* groupName, const char* propName) const {
        if(!hasGroup(groupName)) { return false; }
        const auto& groupProps = getGroupProperties(groupName);
        return groupProps.find(strHash(propName)) != groupProps.cend();
    }

    const auto& getAllProperties() const { return m_Properties; }
    const auto& getGroupProperties(const char* groupName) const { return m_Properties[strHash(groupName)]; }
    const auto& getGroupProperties(size_t group) const { return m_Properties[group]; }
    auto getGroupNameWithProperties(const char* groupName) const {
        auto hashVal = strHash(groupName);
        return std::pair<String, const std::unordered_map<size_t, PropertyBase*>&>(m_GroupNames[hashVal], m_Properties[hashVal]);
    }

private:
    constexpr size_t strHash(const char* str) {
        size_t d = 5381;
        size_t i = 0;
        while(str[i] != '\0') {
            d = d * 33 + str[i++];
        }
        return d;
    }

    bool isValidHash(const char* cstr) {
        const auto str     = String(str);
        const auto hashVal = strHash(cstr);
        if(m_HashedValues.find(str) != m_HashedValues.end() && m_HashedValues[str] != hashVal) {
            return false;
        }
        m_HashedValues[str] = hashVal;
        return true;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // helper variable to verify the validity of perfect string hashing
    std::unordered_map<String, size_t> m_HashedValues;

    ////////////////////////////////////////////////////////////////////////////////
    // store group data
    std::unordered_map<size_t, String> m_GroupNames;
    std::unordered_map<size_t, size_t> m_GroupDataSizes;

    // store properties, index by hashes of groups then by hashes of property names
    std::unordered_map<size_t, std::unordered_map<size_t, PropertyBase*>> m_Properties;
};
