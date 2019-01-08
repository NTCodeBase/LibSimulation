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
namespace NTCodeBase {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
class RigidBody : public SimulationObject<N, Real_t> {
    ////////////////////////////////////////////////////////////////////////////////
    __NT_TYPE_ALIAS __NT_DECLARE_LOGGER_ACCESSORS
    ////////////////////////////////////////////////////////////////////////////////
public:
    RigidBody() = delete;
    RigidBody(const JParams& jParams_, const SharedPtr<Logger>& logger_, Real_t particleRadius) :
        SimulationObject<N, Real_t>("Rigid body", jParams_, logger_, particleRadius) { initializeParameters(jParams_); }
    ////////////////////////////////////////////////////////////////////////////////
    virtual void initializeParameters(const JParams& jParams) override;
    ////////////////////////////////////////////////////////////////////////////////
    bool isCollisionObject() const { return m_bIsCollisionObject; }
    bool resolveCollision(VecN& ppos, VecN& pvel, Real_t timestep);                   // return true if pvel has been modified
    bool resolveCollisionVelocityOnly(const VecN& ppos, VecN& pvel, Real_t timestep); // return true if pvel has been modified
    void updateObjParticles(StdVT_VecN& positions);
    UInt generateParticles(ParticleDataBase<N, Real_t>& particleData, StdVT<SharedPtr<SimulationObject<N, Real_t>>>& otherObjects);

protected:
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
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
} // end namespace NTCodeBase
