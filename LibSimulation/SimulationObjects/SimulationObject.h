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
#include <LibSimulation/Data/Parameter.h>
#include <LibSimulation/Data/Property.h>

#include <unordered_set>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace SimulationObjects {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
class SimulationObject {
    ////////////////////////////////////////////////////////////////////////////////
    __NT_TYPE_ALIAS
    using GeometryPtr = SharedPtr<GeometryObjects::GeometryObject<N, Real_t>>;
    ////////////////////////////////////////////////////////////////////////////////
public:
    SimulationObject() = delete;
    SimulationObject(const JParams& jParams_, const SharedPtr<Logger>& logger_, ParameterManager& parameterManager_, PropertyManager& propertyManager_);
    ////////////////////////////////////////////////////////////////////////////////
    // to remove
    auto& name() { return m_ObjName; }
    auto& geometry() { return m_GeometryObj; }
    ////////////////////////////////////////////////////////////////////////////////
    auto signedDistance(const VecN& ppos) const { return m_GeometryObj->signedDistance(ppos, m_bNegativeInside); }
    auto gradSignedDistance(const VecN& ppos, Real_t dxyz = Real_t(1e-4)) const { return m_GeometryObj->gradSignedDistance(ppos, m_bNegativeInside, dxyz); }
    auto isInside(const VecN& ppos) const { return m_GeometryObj->isInside(ppos, m_bNegativeInside); }
    ////////////////////////////////////////////////////////////////////////////////
    virtual void initializeParameters(const JParams& jParams);
    virtual void initializeProperties() = 0;
    virtual UInt generateParticles(StdVT<SharedPtr<SimulationObject<N, Real_t>>>& otherObjects, bool bIgnoreOverlapped = false) = 0;
    ////////////////////////////////////////////////////////////////////////////////
    bool updateObject(UInt frame, Real_t frameFraction, Real_t frameDuration);

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
    ////////////////////////////////////////////////////////////////////////////////
    // internal geometry object
    GeometryPtr m_GeometryObj     = nullptr;
    bool        m_bNegativeInside = true;
    ////////////////////////////////////////////////////////////////////////////////
    // particle file cache parameters
    bool       m_bUseFileCache    = false;
    String     m_ParticleFile     = String("");
    FileFormat m_ParticleFileType = FileFormat::BNN;
    ////////////////////////////////////////////////////////////////////////////////
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
} // end namespace SimulationObjects
