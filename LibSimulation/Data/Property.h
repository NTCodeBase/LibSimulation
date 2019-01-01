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
#include <LibSimulation/Data/StringHash.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class PropertyBase {
public:
    PropertyBase(const String& groupName, const String& name_) : m_Group(groupName), m_Name(name_), m_Flag(0) {}
    virtual ~PropertyBase() = default;
    ////////////////////////////////////////////////////////////////////////////////
    const auto& group() const { return m_Group; }
    const auto& name() const { return m_Name; }
    auto& flag() { return m_Flag; }
    ////////////////////////////////////////////////////////////////////////////////
    virtual size_t      size() const        = 0;
    virtual size_t      elementSize() const = 0;
    virtual const char* dataPtr() const     = 0;
    ////////////////////////////////////////////////////////////////////////////////
    virtual void reserve(size_t n)    = 0;
    virtual void resize(size_t n)     = 0;
    virtual bool removeAt(size_t idx) = 0;
protected:
    String m_Group;
    String m_Name;
    Int    m_Flag; // variable for storing additional information
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<typename T>
class Property : public PropertyBase {
public:
    Property(const String& groupName, const String& propName) : PropertyBase(groupName, propName), m_bHasDefaultVal(false) {}
    Property(const String& groupName, const String& propName, const T& defaultVal_) :
        PropertyBase(groupName, propName), m_DefaultVal(defaultVal_), m_bHasDefaultVal(true) {}
    ////////////////////////////////////////////////////////////////////////////////
    virtual size_t size() const override { return m_Data.size(); }
    virtual size_t elementSize() const override { return sizeof(T); }
    virtual const char* dataPtr() const override { return static_cast<const char*>(m_Data.data()); }
    ////////////////////////////////////////////////////////////////////////////////
    virtual void reserve(size_t n) override { m_Data.reserve(n); }
    virtual void resize(size_t n) override { if(m_bHasDefaultVal) { m_Data.resize(n, m_DefaultVal); } else { m_Data.resize(n); } }
    virtual bool removeAt(size_t idx) override { assert(idx < m_Data.size()); m_Data.erase(m_Data.begin() + idx); }
    ////////////////////////////////////////////////////////////////////////////////
    void assign(const T& val) { m_Data.assign(m_Data.size(), val); }
    T& operator[](size_t idx) { return m_Data[idx]; }
    const T& operator[](size_t idx) const { return m_Data[idx]; }
protected:
    StdVT<T> m_Data;
    T        m_DefaultVal;
    bool     m_bHasDefaultVal;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class PropertyGroup {
public:
    PropertyGroup() { __NT_DIE("This place should not be reached!") }
    PropertyGroup(const String& name) : m_Name(name), m_DataSize(0) {}
    virtual ~PropertyGroup() {
        for(auto& [prop, propPtr]: m_Properties) {
            delete propPtr;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    const auto& name() const { return m_Name; }

    template<class T>
    void addProperty(const char* propName) {
        __NT_REQUIRE(StringHash::isValidHash(propName) && !hasProperty(propName));
        m_Properties[StringHash::hash(propName)] = new Property<T>(m_Name, propName);
    }

    template<class T>
    void addProperty(const char* propName, const T& defaultValue) {
        __NT_REQUIRE(StringHash::isValidHash(propName) && !hasProperty(propName));
        m_Properties[StringHash::hash(propName)] = new Property<T>(m_Name, propName, defaultValue);
    }

    template<class T>
    const Property<T>& property(const char* propName) const {
        assert(hasProperty(propName));
        auto propPtr = m_Properties[StringHash::hash(propName)];
        assert(propPtr != nullptr && dynamic_cast<Property<T>*>(propPtr) != nullptr);
        return static_cast<const Property<T>&>(*propPtr);
    }

    template<class T>
    Property<T>& property(const char* propName) {
        return const_cast<Property<T>&>(static_cast<const PropertyGroup&>(*this).property(propName));
    }

    bool hasProperty(const char* propName) const { return m_Properties.find(StringHash::hash(propName)) != m_Properties.cend(); }
    ////////////////////////////////////////////////////////////////////////////////
    const auto& properties() const { return m_Properties; }
    size_t size() const { return m_DataSize; }

    void resize(size_t n) {
        for(auto& kv: m_Properties) {
            kv.second->resize(n);
        }
        m_DataSize = n;
    }

    void reserve(size_t n) {
        for(auto& kv: m_Properties) {
            kv.second->reserve(n);
        }
    }

    void removeProperty(const char* propName) {
        __NT_REQUIRE(StringHash::isValidHash(propName) && hasProperty(propName));
        m_Properties.erase(StringHash::hash(propName));
    }

    void removeAt(size_t idx) {
        if(m_Properties.size() == 0) { return; }
        assert(m_DataSize[hashVal] > idx);
        for(auto& kv: m_Properties) {
            kv.second->removeAt(idx);
        }
        m_DataSize = m_Properties.begin()->second->size();
    }

private:
    String                                    m_Name;
    size_t                                    m_DataSize;
    std::unordered_map<size_t, PropertyBase*> m_Properties;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class PropertyManager {
public:
    PropertyManager() = default;
    ////////////////////////////////////////////////////////////////////////////////
    /**
     * \brief Group must be added before adding properties of that group
     */
    void addGroup(const char* groupName) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && !hasGroup(groupName));
        auto hashVal = StringHash::hash(groupName);
        m_PropertyGroups[hashVal] = PropertyGroup(String(groupName));
    }

    template<class T>
    void addProperty(const char* groupName, const char* propName) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && hasGroup(groupName));
        m_PropertyGroups[StringHash::hash(groupName)].addProperty(propName);
    }

    ////////////////////////////////////////////////////////////////////////////////
    template<class T>
    void addProperty(const char* groupName, const char* propName,  const T& defaultValue) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && hasGroup(groupName));
        m_PropertyGroups[StringHash::hash(groupName)].addProperty(propName, defaultValue);
    }

    template<class T>
    const Property<T>& property(const char* groupName, const char* propName) const {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && hasGroup(groupName));
        return m_PropertyGroups[StringHash::hash(groupName)].property(propName);
    }

    template<class T>
    Property<T>& property(const char* groupName, const char* propName) {
        return const_cast<Property<T>&>(static_cast<const PropertyManager&>(*this).property(groupName, propName));
    }

    ////////////////////////////////////////////////////////////////////////////////
    auto& getAllGroups() { return m_PropertyGroups; }
    const auto& getAllGroups() const { return m_PropertyGroups; }

    auto& group(size_t group) { assert(hasGroup(group)); return m_PropertyGroups.at(group); }
    const auto& group(size_t group) const { assert(hasGroup(group)); return m_PropertyGroups.at(group); }

    auto& group(const char* groupName) { return group(StringHash::hash(groupName)); }
    const auto& group(const char* groupName) const { return group(StringHash::hash(groupName)); }

    void removeGroup(const char* groupName) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && hasGroup(groupName));
        m_PropertyGroups.erase(StringHash::hash(groupName));
    }

    ////////////////////////////////////////////////////////////////////////////////
    bool hasGroup(size_t group) const { return m_PropertyGroups.find(group) != m_PropertyGroups.end(); }
    bool hasGroup(const char* groupName) const { return hasGroup(StringHash::hash(groupName)); }
    bool hasProperty(const char* groupName, const char* propName) const {
        if(!hasGroup(groupName)) { return false; }
        return group(groupName).hasProperty(propName);
    }

private:
    std::unordered_map<size_t, PropertyGroup> m_PropertyGroups;
};
