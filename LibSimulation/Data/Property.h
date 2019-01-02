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
#include <type_traits>
#include <variant>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class PropertyBase {
public:
    PropertyBase(const String& groupName, const String& name, const String& desc) : m_Group(groupName), m_Name(name), m_Description(desc), m_Flag(0) {}
    virtual ~PropertyBase() = default;
    ////////////////////////////////////////////////////////////////////////////////
    const auto& group() const { return m_Group; }
    const auto& name() const { return m_Name; }
    const auto& description() const { return m_Description; }
    auto& flag() { return m_Flag; }
    ////////////////////////////////////////////////////////////////////////////////
    virtual size_t      size() const        = 0;
    virtual size_t      elementSize() const = 0;
    virtual const char* dataPtr() const     = 0;
    ////////////////////////////////////////////////////////////////////////////////
    virtual void reserve(size_t n)    = 0;
    virtual void resize(size_t n)     = 0;
    virtual void removeAt(size_t idx) = 0;
    virtual void reset() = 0;
protected:
    String m_Group;
    String m_Name;
    String m_Description;
    Int    m_Flag; // variable for storing additional information
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<typename T>
class Property : public PropertyBase {
public:
    Property(const String& groupName, const String& propName, const String& desc) : PropertyBase(groupName, propName, desc), m_bHasDefaultVal(false) {}
    Property(const String& groupName, const String& propName, const String& desc, const T& defaultVal_) :
        PropertyBase(groupName, propName, desc), m_DefaultVal(defaultVal_), m_bHasDefaultVal(true) {}
    ////////////////////////////////////////////////////////////////////////////////
    virtual size_t size() const override { return m_Data.size(); }
    virtual size_t elementSize() const override { return sizeof(T); }
    virtual const char* dataPtr() const override { return reinterpret_cast<const char*>(m_Data.data()); }
    ////////////////////////////////////////////////////////////////////////////////
    virtual void reserve(size_t n) override { m_Data.reserve(n); }
    virtual void resize(size_t n) override { if(m_bHasDefaultVal) { m_Data.resize(n, m_DefaultVal); } else { m_Data.resize(n); } }
    virtual void removeAt(size_t idx) override { assert(idx < m_Data.size()); m_Data.erase(m_Data.begin() + idx); }
    virtual void reset() override { if(m_bHasDefaultVal) { m_Data.assign(size(), m_DefaultVal); } else { m_Data.assign(size(), T()); } }
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
    PropertyGroup() { throw std::exception("This place should not be reached!"); }
    PropertyGroup(const String& name, UInt hash) : m_Name(name), m_Hash(hash), m_DataSize(0) {}
    virtual ~PropertyGroup() {
        for(auto& [prop, propPtr]: m_Properties) {
            delete propPtr;
        }
    }

    const auto& name() const { return m_Name; }
    auto hash() const { return m_Hash; }

    ////////////////////////////////////////////////////////////////////////////////
    template<class T>
    void addProperty(const char* propName, const char* description) {
        __NT_REQUIRE(StringHash::isValidHash(propName) && !hasProperty(propName));
        m_Properties[StringHash::hash(propName)] = new Property<T>(m_Name, propName, description);
    }

    template<class T>
    void addProperty(const char* propName, const char* description, const T& defaultValue) {
        __NT_REQUIRE(StringHash::isValidHash(propName) && !hasProperty(propName));
        m_Properties[StringHash::hash(propName)] = new Property<T>(m_Name, propName, description, defaultValue);
    }

    template<class T>
    void addDiscreteProperty(const char* propName, const char* description, const T& defaultValue) {
        __NT_REQUIRE(StringHash::isValidHash(propName) && !hasDiscreteProperty(propName));
        DiscreteProperty tmp = defaultValue;
        m_DiscreteProperties.emplace(StringHash::hash(propName), std::make_tuple(String(propName), String(description), tmp));
    }

    template<class T>
    void addDiscreteProperty(const char* propName, const char* description) {
        addDiscreteProperty<T>(propName, description, T());
    }

    ////////////////////////////////////////////////////////////////////////////////
    template<class T>
    const Property<T>& property(const char* propName) const {
        assert(hasProperty(propName));
        const auto propPtr = m_Properties.at(StringHash::hash(propName));
        assert(propPtr != nullptr && dynamic_cast<const Property<T>*>(propPtr) != nullptr);
        return static_cast<const Property<T>&>(*propPtr);
    }

    template<class T>
    Property<T>& property(const char* propName) {
        return const_cast<Property<T>&>(static_cast<const PropertyGroup&>(*this).property<T>(propName));
    }

    template<class T>
    const T& discreteProperty(const char* propName) const {
        assert(hasDiscreteProperty(propName));
        const auto& tmp = std::get<2>(m_DiscreteProperties.at(StringHash::hash(propName)));
        assert(std::holds_alternative<T>(tmp)); return std::get<T>(tmp);
    }

    template<class T>
    T& discreteProperty(const char* propName) {
        return const_cast<T&>(static_cast<const PropertyGroup&>(*this).discreteProperty<T>(propName));
    }

    const char* propertyDataPtr(const char* propName) const {
        if(UInt propHash = StringHash::hash(propName); hasProperty(propHash)) {
            return m_Properties.at(propHash)->dataPtr();
        } else {
            return nullptr;
        }
    }

    const char* discretePropertyDataPtr(const char* propName) const {
        if(UInt propHash = StringHash::hash(propName); hasDiscreteProperty(propHash)) {
            return std::visit([&](auto&& arg) { return reinterpret_cast<const char*>(&arg); }, std::get<2>(m_DiscreteProperties.at(propHash)));
        } else {
            return nullptr;
        }
    }

    bool hasProperty(UInt propHash) const { return m_Properties.find(propHash) != m_Properties.cend(); }
    bool hasProperty(const char* propName) const { return hasProperty(StringHash::hash(propName)); }
    bool hasDiscreteProperty(UInt propHash) const { return m_DiscreteProperties.find(propHash) != m_DiscreteProperties.cend(); }
    bool hasDiscreteProperty(const char* propName) const { return hasDiscreteProperty(StringHash::hash(propName)); }
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

    void removeDiscreteProperty(const char* propName) {
        __NT_REQUIRE(StringHash::isValidHash(propName) && hasDiscreteProperty(propName));
        m_DiscreteProperties.erase(StringHash::hash(propName));
    }

    void removeAt(size_t idx) {
        if(m_Properties.size() == 0) { return; }
        for(auto& kv: m_Properties) {
            kv.second->removeAt(idx);
        }
        m_DataSize = m_Properties.begin()->second->size();
    }

private:
    using DiscreteProperty = std::variant<bool, int, UInt, float, double, Vec2i, Vec2ui, Vec2f, Vec3i, Vec3ui, Vec3f, Vec4i, Vec4ui, Vec4d, String>;

    UInt                                                                   m_Hash;
    String                                                                 m_Name;
    size_t                                                                 m_DataSize;
    std::unordered_map<UInt, PropertyBase*>                                m_Properties;
    std::unordered_map<UInt, std::tuple<String, String, DiscreteProperty>> m_DiscreteProperties;
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
        m_PropertyGroups.emplace(hashVal, PropertyGroup(String(groupName), hashVal));
    }

    template<class T>
    void addProperty(const char* groupName, const char* description, const char* propName) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && hasGroup(groupName));
        m_PropertyGroups[StringHash::hash(groupName)].addProperty<T>(propName, description);
    }

    template<class T>
    void addProperty(const char* groupName, const char* propName, const char* description,  const T& defaultValue) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && hasGroup(groupName));
        m_PropertyGroups[StringHash::hash(groupName)].addProperty<T>(propName, description, defaultValue);
    }

    template<class T>
    void addDiscreteProperty(const char* groupName, const char* description, const char* propName) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && hasGroup(groupName));
        m_PropertyGroups[StringHash::hash(groupName)].addDiscreteProperty<T>(propName, description);
    }

    template<class T>
    void addDiscreteProperty(const char* groupName, const char* propName, const char* description,  const T& defaultValue) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && hasGroup(groupName));
        m_PropertyGroups[StringHash::hash(groupName)].addDiscreteProperty<T>(propName, description, defaultValue);
    }

    ////////////////////////////////////////////////////////////////////////////////
    template<class T>
    const Property<T>& property(const char* groupName, const char* propName) const {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && hasGroup(groupName));
        return m_PropertyGroups[StringHash::hash(groupName)].property<T>(propName);
    }

    template<class T>
    Property<T>& property(const char* groupName, const char* propName) {
        return const_cast<Property<T>&>(static_cast<const PropertyManager&>(*this).property<T>(groupName, propName));
    }

    template<class T>
    const T& discreteProperty(const char* groupName, const char* propName) const {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && hasGroup(groupName));
        return m_PropertyGroups[StringHash::hash(groupName)].discreteProperty<T>(propName);
    }

    template<class T>
    T& discreteProperty(const char* groupName, const char* propName) {
        return const_cast<T&>(static_cast<const PropertyManager&>(*this).discreteProperty<T>(groupName, propName));
    }

    ////////////////////////////////////////////////////////////////////////////////
    auto& getAllGroups() { return m_PropertyGroups; }
    const auto& getAllGroups() const { return m_PropertyGroups; }

    auto& group(UInt groupHash) { assert(hasGroup(groupHash)); return m_PropertyGroups.at(groupHash); }
    const auto& group(UInt groupHash) const { assert(hasGroup(groupHash)); return m_PropertyGroups.at(groupHash); }

    auto& group(const char* groupName) { return group(StringHash::hash(groupName)); }
    const auto& group(const char* groupName) const { return group(StringHash::hash(groupName)); }

    void removeGroup(const char* groupName) {
        __NT_REQUIRE(StringHash::isValidHash(groupName) && hasGroup(groupName));
        m_PropertyGroups.erase(StringHash::hash(groupName));
    }

    ////////////////////////////////////////////////////////////////////////////////
    bool hasGroup(UInt groupHash) const { return m_PropertyGroups.find(groupHash) != m_PropertyGroups.end(); }
    bool hasGroup(const char* groupName) const { return hasGroup(StringHash::hash(groupName)); }
    bool hasProperty(const char* groupName, const char* propName) const {
        if(!hasGroup(groupName)) { return false; }
        return group(groupName).hasProperty(propName);
    }

    bool hasDiscreteProperty(const char* groupName, const char* propName) const {
        if(!hasGroup(groupName)) { return false; }
        return group(groupName).hasDiscreteProperty(propName);
    }

private:
    std::unordered_map<UInt, PropertyGroup> m_PropertyGroups;
};
