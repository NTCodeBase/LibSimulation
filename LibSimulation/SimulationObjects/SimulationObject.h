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
#include <LibCommon/Geometry/GeometryObjects.h>

#include <LibSimulation/Constants.h>
#include <LibSimulation/Macros.h>
#include <LibSimulation/Data/Parameter.h>
#include <LibSimulation/Data/Property.h>

#include <unordered_set>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace SimulationObjects {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
class SimulationObject {
    ////////////////////////////////////////////////////////////////////////////////
    __NT_TYPE_ALIAS __NT_DECLARE_PARTICLE_SOLVER_ACCESSORS
    using GeometryPtr = SharedPtr<GeometryObjects::GeometryObject<N, Real_t>>;
    ////////////////////////////////////////////////////////////////////////////////
public:
    SimulationObject() = delete;
    SimulationObject(const String& desc_, const JParams& jParams_, const SharedPtr<Logger>& logger_,
                     ParameterManager& parameterManager_, PropertyManager& propertyManager_);
    ////////////////////////////////////////////////////////////////////////////////
    // to remove
    auto objID() const { return m_ObjID; }
    auto& name() { return m_ObjName; }
    auto& geometry() { return m_GeometryObj; }
    const auto& name() const { return m_ObjName; }
    const auto& geometry() const { return m_GeometryObj; }
    ////////////////////////////////////////////////////////////////////////////////
    //    virtual void initializeProperties() = 0;
    virtual void initializeParameters(const JParams& jParams);
    virtual bool updateObject(UInt frame, Real_t frameFraction, Real_t timestep);
    virtual UInt generateParticles(PropertyGroup& propertyGroup, StdVT<SharedPtr<SimulationObject<N, Real_t>>>& otherObjects,
                                   bool bIgnoreOverlapped = false) = 0;
    ////////////////////////////////////////////////////////////////////////////////
    virtual bool            isInside(const VecN& ppos) const;
    virtual Real_t          signedDistance(const VecN& ppos) const;
    virtual VecX<N, Real_t> gradSignedDistance(const VecN& ppos, Real_t dxyz = Real_t(1e-4)) const;

protected:
    bool loadParticlesFromFile(StdVT_VecN& positions);
    void saveParticlesToFile(const StdVT_VecN& positions);
    ////////////////////////////////////////////////////////////////////////////////
    SharedPtr<Logger> m_Logger;
    ParameterManager& m_ParameterManager;
    PropertyManager&  m_PropertyManager;
    ////////////////////////////////////////////////////////////////////////////////
    // id and name of the object
    static inline std::unordered_set<UInt> s_GeneratedObjIDs {};
    UInt                                   m_ObjID;
    String                                 m_ObjName;
    String                                 m_Description;
    ////////////////////////////////////////////////////////////////////////////////
    // internal geometry object
    GeometryPtr m_GeometryObj = nullptr;
    ////////////////////////////////////////////////////////////////////////////////
    // particle file cache parameters
    String     m_ParticleFile  = String("");
    FileFormat m_FileFormat    = FileFormat::BNN;
    bool       m_bUseFileCache = false;
    ////////////////////////////////////////////////////////////////////////////////
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
} // end namespace SimulationObjects
