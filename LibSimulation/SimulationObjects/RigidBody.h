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
#include <unordered_map>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace SimulationObjects {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
class RigidBody : public SimulationObject<N, Real_t> {
    ////////////////////////////////////////////////////////////////////////////////
    __NT_TYPE_ALIAS __NT_DECLARE_PARTICLE_SOLVER_ACCESSORS
    ////////////////////////////////////////////////////////////////////////////////
public:
    RigidBody() = delete;
    RigidBody(const String& desc_, const JParams& jParams_, const SharedPtr<Logger>& logger_,
              ParameterManager& parameterManager_, PropertyManager& propertyManager_) :
        SimulationObject<N, Real_t>(desc_, jParams_, logger_, parameterManager_, propertyManager_) { initializeParameters(jParams_); }
    ////////////////////////////////////////////////////////////////////////////////
    virtual void initializeParameters(const JParams& jParams) override;
    virtual bool updateObject(UInt frame, Real_t frameFraction, Real_t timestep) override;
    virtual UInt generateParticles(PropertyGroup& propertyGroup, StdVT<SharedPtr<SimulationObject<N, Real_t>>>& otherObjects,
                                   bool bIgnoreOverlapped = false) override;
    ////////////////////////////////////////////////////////////////////////////////
    virtual bool            isInside(const VecN& ppos) const override;
    virtual Real_t          signedDistance(const VecN& ppos) const override;
    virtual VecX<N, Real_t> gradSignedDistance(const VecN& ppos, Real_t dxyz = Real_t(1e-4)) const override;
    bool isCollisionObject() const { return m_bIsCollisionObject; }
    ////////////////////////////////////////////////////////////////////////////////
    std::pair<bool, StdVT_UInt>                           findConstrainedParticles(PropertyGroup& propertyGroup);
    template<class... Groups> std::pair<bool, StdVT_UInt> findConstrainedParticles(PropertyGroup& propertyGroup, Groups& ... groups);
    ////////////////////////////////////////////////////////////////////////////////
    bool resolveCollision(VecN& ppos, VecN& pvel, Real_t timestep);                   // return true if pvel has been modified
    bool resolveCollisionVelocityOnly(const VecN& ppos, VecN& pvel, Real_t timestep); // return true if pvel has been modified

protected:
    StdVT_VecN generateParticles(StdVT<SharedPtr<SimulationObject<N, Real_t>>>& otherObjects, bool bIgnoreOverlapped = false);
    ////////////////////////////////////////////////////////////////////////////////

    void updateObjParticles();
    void resetConstrainedParticles();
    void updateConstrainParticles(Real_t timestep);
    ////////////////////////////////////////////////////////////////////////////////
    VecN getObjectVelocity(const VecN& ppos, Real_t timestep);
    ////////////////////////////////////////////////////////////////////////////////
    bool resolveCollision_StickyBC(VecN& ppos, VecN& pvel, Real_t timestep);
    bool resolveCollision_SlipBC(VecN& ppos, VecN& pvel, Real_t timestep);
    bool resolveCollision_SeparateBC(VecN& ppos, VecN& pvel, Real_t timestep);
    ////////////////////////////////////////////////////////////////////////////////
    bool resolveCollisionVelocityOnly_StickyBC(const VecN& ppos, VecN& pvel, Real_t timestep);
    bool resolveCollisionVelocityOnly_SlipBC(const VecN& ppos, VecN& pvel, Real_t timestep);
    bool resolveCollisionVelocityOnly_SeparateBC(const VecN& ppos, VecN& pvel, Real_t timestep);
    ////////////////////////////////////////////////////////////////////////////////
    bool              m_bIsCollisionObject = true;
    BoundaryCondition m_BoundaryCondition  = BoundaryCondition::Slip;
    Real_t            m_BoundaryFriction   = Real_t(0);

    bool   m_bGenerateParticleInside          = true;
    Real_t m_ParticleGenerationThicknessRatio = HugeReal();
    Real_t m_ParticleGenerationDensityRatio   = Real_t(1.5);
    ////////////////////////////////////////////////////////////////////////////////
    // record generated particles
    StdVT<VecN>  m_GeneratedParticles;
    VecN         m_CenterParticles;
    Vec2<size_t> m_RangeGeneratedParticles; // range [start, end) of generated particle indices
    VecN         m_ShiftCenterGeneratedParticles = VecN(0);
    ////////////////////////////////////////////////////////////////////////////////
    // contrained particle parameters
    bool m_bConstrainInsideParticles     = false;
    bool m_bCrashIfNoConstrainedParticle = false;
    bool m_bHasConstrainedParticles      = false;
    // contrained particle data
    std::unordered_map<UInt, std::pair<StdVT_UInt, StdVT_VecN>> m_ConstrainedParticleInfo;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
} // end namespace SimulationObjects
