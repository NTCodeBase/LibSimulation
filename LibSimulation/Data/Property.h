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
    PropertyBase(const String& groupName, const String& propName) : m_Group(groupName), m_PropertyName(propName) {}
    virtual ~PropertyBase() = default;
    const auto& groupName() const { return m_Group; }
    const auto& propertyName() const { return m_PropertyName; }
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
    String m_PropertyName;
    Int    m_Flag; // variable for storing additional information
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<typename T>
class Property : public PropertyBase {
public:
    Property(const String& groupName, const String& propName) : PropertyBase(groupName, propName), m_bHasDefaultVal(false) {}
    Property(const String& groupName, const String& propName, const T& defaultVal_) : PropertyBase(groupName, propName),
        m_DefaultVal(defaultVal_), m_bHasDefaultVal(true) {}
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
    void push_back(const StdVT<T>& vals) { m_Data.insert(m_Data.end(), vals.begin(), vals.end()); }
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
            for(auto& [propName, propPtr]: groupProps) {
                delete propPtr;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    /**
     * \brief Group must be added before adding properties of that group
     */
    void addGroup(const char* groupName) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && !hasGroup(groupName));
        auto hashVal = StringHash::hash(groupName);
        m_Properties[hashVal] = {};
        m_GroupNames[hashVal] = String(groupName);
    }

    ////////////////////////////////////////////////////////////////////////////////
    template<class T>
    void addProperty(const char* groupName, const char* propName) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && StringHash::isValidHash(propName) && hasGroup(groupName) && !hasProperty(groupName, propName));
        m_Properties[StringHash::hash(groupName)][StringHash::hash(propName)] = new Property<T>(groupName, propName);
    }

    template<class T>
    void addProperty(const char* groupName, const char* propName,  const T& defaultValue) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && StringHash::isValidHash(propName) && hasGroup(groupName) && !hasProperty(groupName, propName));
        m_Properties[StringHash::hash(groupName)][StringHash::hash(propName)] = new Property<T>(groupName, propName, defaultValue);
    }

    template<class T>
    Property<T>* property(const char* groupName, const char* propName) {
        assert(hasProperty(groupName, propName));
        auto propPtr = m_Properties[StringHash::hash(groupName)][StringHash::hash(propName)]; assert(propPtr != nullptr && dynamic_cast<Property<T>*>(propPtr) != nullptr);
        return static_cast<Property<T>*>(propPtr);
    }

    template<class T>
    const Property<T>* property(const char* groupName, const char* propName) const {
        assert(hasProperty(groupName, propName));
        const auto propPtr = m_Properties[StringHash::hash(groupName)][StringHash::hash(propName)]; assert(propPtr != nullptr && dynamic_cast<Property<T>*>(propPtr) != nullptr);
        return static_cast<const Property<T>*>(propPtr);
    }

    ////////////////////////////////////////////////////////////////////////////////
    const auto& getAllProperties() const { return m_Properties; }
    const auto& getGroupProperties(const char* groupName) const { assert(hasGroup(groupName)); return m_Properties.at(StringHash::hash(groupName)); }
    const auto& getGroupProperties(size_t group) const { assert(hasGroup(group)); return m_Properties.at(group); }

    auto getGroupNameWithProperties(const char* groupName) const {
        auto group = StringHash::hash(groupName); assert(hasGroup(group));
        return std::pair<String, const std::unordered_map<size_t, PropertyBase*>&>(m_GroupNames.at(group), m_Properties.at(group));
    }

    size_t getGroupDataSize(const char* groupName) const {
        assert(hasGroup(groupName));
        return m_GroupDataSizes.at(StringHash::hash(groupName));
    }

    void resizeGroupData(const char* groupName, size_t size) {
        assert(hasGroup(groupName));
        auto  hashVal    = StringHash::hash(groupName);
        auto& groupProps = m_Properties[hashVal];
        for(auto& kv: groupProps) {
            kv.second->resize(size);
        }
        m_GroupDataSizes[hashVal] = size;
    }

    void reserveGroupData(const char* groupName, size_t size) {
        assert(hasGroup(groupName));
        auto& groupProps = m_Properties[StringHash::hash(groupName)];
        for(auto& kv: groupProps) {
            kv.second->reserve(size);
        }
    }

    void removeGroup(const char* groupName) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && hasGroup(groupName));
        m_Properties.erase(StringHash::hash(groupName));
    }

    void removeProperty(const char* groupName, const char* propName) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && StringHash::isValidHash(propName) && hasProperty(groupName, propName));
        auto& groupProps = m_Properties[StringHash::hash(groupName)];
        groupProps.erase(StringHash::hash(propName));
    }

    void removeGroupElementAt(const char* groupName, size_t idx) {
        assert(StringHash::isValidHash(groupName) && hasGroup(groupName));
        auto hashVal = StringHash::hash(groupName);
        assert(m_GroupDataSizes[hashVal] > idx);
        const auto& groupProps = getGroupProperties(hashVal);
        for(auto& kv: groupProps) {
            kv.second->removeAt(idx);
        }
        m_GroupDataSizes[hashVal] = groupProps.begin()->second->size();
    }

    ////////////////////////////////////////////////////////////////////////////////
    bool hasGroup(size_t group) const { return m_Properties.find(group) != m_Properties.end(); }
    bool hasGroup(const char* groupName) const { return hasGroup(StringHash::hash(groupName)); }
    bool hasProperty(const char* groupName, const char* propName) const {
        if(!hasGroup(groupName)) { return false; }
        const auto& groupProps = getGroupProperties(groupName);
        return groupProps.find(StringHash::hash(propName)) != groupProps.cend();
    }

private:
    ////////////////////////////////////////////////////////////////////////////////
    // store group data
    std::unordered_map<size_t, String> m_GroupNames;
    std::unordered_map<size_t, size_t> m_GroupDataSizes;

    // store properties, index by hashes of groups then by hashes of property names
    std::unordered_map<size_t, std::unordered_map<size_t, PropertyBase*>> m_Properties;
};
