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
#include <LibSimulation/SimulationObjects/SimulationObject.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace SimulationObjects {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
class RigidBody : public SimulationObject<N, Real_t> {
    ////////////////////////////////////////////////////////////////////////////////
    __NT_TYPE_ALIAS
    const Parameter& simParams(const char* paramName) const { return this->m_ParameterManager.parameter("SimulationParameters", paramName); }
    ////////////////////////////////////////////////////////////////////////////////
public:
    RigidBody() = delete;
    RigidBody(const JParams& jParams_, ParameterManager& parameterManager_, PropertyManager& propertyManager_) :
        SimulationObject<N, Real_t>(jParams_, parameterManager_, propertyManager_) { parseBoundaryParameters(jParams_); }
    ////////////////////////////////////////////////////////////////////////////////
    virtual void printParameters(Logger& logger) override;
    virtual UInt generateParticles(ParticleSolvers::SolverData<N, Real_t>& solverData,
                                   StdVT<SharedPtr<RigidBody<N, Real_t>>>& rigidBodies) override;
    virtual bool updateObject(ParticleSolvers::SolverData<N, Real_t>& solverData, UInt frame, Real_t frameFraction, Real_t frameDuration) override;
    ////////////////////////////////////////////////////////////////////////////////
    bool isCollisionObject() const { return m_bIsCollisionObject; }
    bool findConstrainedParticles(ParticleSolvers::SolverData<N, Real_t>& solverData);
    auto getNConstrainedParticles() const { return std::make_tuple(m_ConstrainedStandardParticlesIdx.size(), m_ConstrainedVertexParticlesIdx.size(), m_ConstrainedQuadParticlesIdx.size()); }
    bool resolveCollision(VecN& ppos, VecN& pvel, Real_t timestep);                   // return true if pvel has been modified
    bool resolveCollisionVelocityOnly(const VecN& ppos, VecN& pvel, Real_t timestep); // return true if pvel has been modified

protected:
    void parseBoundaryParameters(const JParams& jParams);
    ////////////////////////////////////////////////////////////////////////////////
    void updateGhostParticles(ParticleSolvers::SolverData<N, Real_t>& solverData);
    void resetConstrainedParticles(ParticleSolvers::SolverData<N, Real_t>& solverData);
    void updateConstrainParticles(ParticleSolvers::SolverData<N, Real_t>& solverData, UInt frame, Real_t frameFraction, Real_t frameDuration);
    ////////////////////////////////////////////////////////////////////////////////
    inline void updatePositionVelocity(VecN& ppos_t0, VecN& ppos, VecN& pvel, Real_t timestep);
    inline VecN getObjectVelocity(const VecN& ppos, Real_t timestep);
    ////////////////////////////////////////////////////////////////////////////////
    inline bool resolveCollision_StickyBC(VecN& ppos, VecN& pvel, Real_t timestep);
    inline bool resolveCollision_SlipBC(VecN& ppos, VecN& pvel, Real_t timestep);
    inline bool resolveCollision_SeparateBC(VecN& ppos, VecN& pvel, Real_t timestep);
    ////////////////////////////////////////////////////////////////////////////////
    inline bool resolveCollisionVelocityOnly_StickyBC(const VecN& ppos, VecN& pvel, Real_t timestep);
    inline bool resolveCollisionVelocityOnly_SlipBC(const VecN& ppos, VecN& pvel, Real_t timestep);
    inline bool resolveCollisionVelocityOnly_SeparateBC(const VecN& ppos, VecN& pvel, Real_t timestep);
    ////////////////////////////////////////////////////////////////////////////////
    bool                  m_bIsCollisionObject    = true;
    BoundaryConditionType m_BoundaryConditionType = BoundaryConditionType::Slip;
    Real_t                m_BoundaryFriction      = Real_t(0);

    bool   m_bGenerateGhostParticle           = true;
    Real_t m_ParticleGenerationThicknessRatio = HugeReal();
    Real_t m_ParticleGenerationDensityRatio   = Real_t(1.5);
    ////////////////////////////////////////////////////////////////////////////////
    // record generated particles
    StdVT<VecN>  m_GeneratedParticles;
    VecN         m_CenterGeneratedParticles;
    Vec2<size_t> m_RangeGeneratedGhostParticles; // range [start, end) of generated ghost particle indices
    VecN         m_ShiftCenterGeneratedParticles = VecN(0);
    ////////////////////////////////////////////////////////////////////////////////
    // contrained particles
    bool m_bConstrainInsideParticles     = false;
    bool m_bCrashIfNoConstrainedParticle = false;
    bool m_bHasConstrainedParticles      = false;

    StdVT<UInt> m_ConstrainedStandardParticlesIdx;
    StdVT<UInt> m_ConstrainedVertexParticlesIdx;
    StdVT<UInt> m_ConstrainedQuadParticlesIdx;

    StdVT<VecN>  m_ConstrainedStandardParticles_t0;
    StdVT<VecN>  m_ConstrainedVertexParticles_t0;
    StdVT<VecN>  m_ConstrainedQuadParticles_t0;
    StdVT<Vec4r> m_ConstrainedQuadParticleLocalFrames_t0;
    ////////////////////////////////////////////////////////////////////////////////
    // system time at last scene update
    Real_t m_LastUpdateTime = 0;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
} // end namespace SimulationObjects
