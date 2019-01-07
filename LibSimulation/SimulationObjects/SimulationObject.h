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
#include <LibSimulation/Forward.h>
#include <LibSimulation/Enums.h>

#include <unordered_set>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace NTCodeBase {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
class SimulationObject {
    ////////////////////////////////////////////////////////////////////////////////
    __NT_TYPE_ALIAS
    using GeometryPtr = SharedPtr<GeometryObject<N, Real_t>>;
    Logger& logger() { assert(this->m_Logger != nullptr); return *this->m_Logger; }
    const Logger& logger() const { assert(this->m_Logger != nullptr); return *this->m_Logger; }
    ////////////////////////////////////////////////////////////////////////////////
public:
    SimulationObject() = delete;
    SimulationObject(const String& desc_, const JParams& jParams_, const SharedPtr<Logger>& logger_, Real_t particleRadius);
    ////////////////////////////////////////////////////////////////////////////////
    // to remove
    auto objID() const { return m_ObjID; }
    auto& name() { return m_ObjName; }
    auto& geometry() { return m_GeometryObj; }
    auto& negativeInside() { return m_bNegativeInside; }
    const auto& name() const { return m_ObjName; }
    const auto& geometry() const { return m_GeometryObj; }
    auto negativeInside() const { return m_bNegativeInside; }
    ////////////////////////////////////////////////////////////////////////////////
    virtual bool updateObject(UInt frame, Real_t frameFraction, Real_t timestep);
    ////////////////////////////////////////////////////////////////////////////////
    bool   isInside(const VecN& ppos) const { return m_GeometryObj->isInside(ppos, m_bNegativeInside); }
    Real_t signedDistance(const VecN& ppos) const { return m_GeometryObj->signedDistance(ppos, m_bNegativeInside); }
    VecN   gradSignedDistance(const VecN& ppos, Real_t dxyz = Real_t(1e-4)) const { return m_GeometryObj->gradSignedDistance(ppos, m_bNegativeInside, dxyz); }

protected:
    virtual void initializeParameters(const JParams& jParams);
    StdVT_VecN   generateParticleInside(StdVT<SharedPtr<SimulationObject<N, Real_t>>>& otherObjects, bool bIgnoreOverlapped = false);
    bool         loadParticlesFromFile(StdVT_VecN& positions);
    void         saveParticlesToFile(const StdVT_VecN& positions);
    ////////////////////////////////////////////////////////////////////////////////
    SharedPtr<Logger> m_Logger;
    ////////////////////////////////////////////////////////////////////////////////
    // id and name of the object
    static inline std::unordered_set<UInt> s_GeneratedObjIDs {};
    UInt   m_ObjID;
    String m_ObjName;
    String m_Description;
    ////////////////////////////////////////////////////////////////////////////////
    // internal geometry object
    GeometryPtr m_GeometryObj     = nullptr;
    bool        m_bNegativeInside = true;
    Real_t      m_ParticleRadius  = 0;
    ////////////////////////////////////////////////////////////////////////////////
    // internal particle generation
    StdVT<VecN>  m_GeneratedParticles;
    VecN         m_CenterParticles;
    Vec2<size_t> m_RangeGeneratedParticles; // range [start, end) of generated particle indices
    VecN         m_ShiftCenterGeneratedParticles = VecN(0);
    struct {
        bool   bGenerateParticle = false;
        Real_t thicknessRatio    = HugeReal();
        Real_t jitterRatio       = Real_t(0);
        VecN   samplingRatio     = VecN(1.0);
        VecN   shiftCenter       = VecN(1.0);
    } m_GenParticleParams;
    ////////////////////////////////////////////////////////////////////////////////
    // particle file cache parameters
    String     m_ParticleFile  = String("");
    FileFormat m_FileFormat    = FileFormat::BNN;
    bool       m_bUseFileCache = false;
    ////////////////////////////////////////////////////////////////////////////////
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
} // end namespace NTCodeBase
