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

#include <LibCommon/Geometry/GeometryObjects.h>
#include <LibCommon/LinearAlgebra/LinaHelpers.h>
#include <LibCommon/Utils/JSONHelpers.h>
#include <LibCommon/Utils/NumberHelpers.h>

#include <LibParticle/ParticleHelpers.h>
#include <LibParticle/ParticleSerialization.h>

#include <LibSimulation/Enums.h>
#include <LibSimulation/ParticleSolvers/ParticleDataBase.h>
#include <LibSimulation/SimulationObjects/RigidBody.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace NTCodeBase {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void RigidBody<N, Real_t>::initializeParameters(const JParams& jParams) {
    JSONHelpers::readBool(jParams, m_bIsCollisionObject, "IsCollisionObject");
    logger().printLogIndent(String("Collision object: ") + Formatters::toString(m_bIsCollisionObject));
    if(m_bIsCollisionObject) {
        if(String bcType; JSONHelpers::readValue(jParams, bcType, "BCType")) {
            NT_REQUIRE(bcType == "Sticky" ||
                       bcType == "Slip" ||
                       bcType == "Separate");
            if(bcType == "Sticky") {
                m_BoundaryCondition = BoundaryCondition::Sticky;
                logger().printLogIndent(String("Boundary condition type: Sticky"));
            } else if(bcType == "Slip") {
                m_BoundaryCondition = BoundaryCondition::Slip;
                logger().printLogIndent(String("Boundary condition type: Slip"));
            } else {
                m_BoundaryCondition = BoundaryCondition::Separate;
                logger().printLogIndent(String("Boundary condition type: Separate"));
            }
        }
        JSONHelpers::readValue(jParams, m_BoundaryFriction, "BoundaryFriction");
        logger().printLogIndent(String("Friction: ") + std::to_string(m_BoundaryFriction));
        logger().newLine();
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void RigidBody<N, Real_t>::updateObjParticles(StdVT_VecN& positions) {
    const auto& range        = this->m_RangeGeneratedParticles; // [start, end)
    const auto& positions_t0 = this->m_GeneratedParticles;
    NT_REQUIRE(positions_t0.size() + range[0] == range[1] && range[1] <= positions.size());
    ParallelExec::run(positions_t0.size(),
                      [&](size_t p) {
                          positions[p + range[0]] =
                              this->geometry()->transformAnimation(positions_t0[p] - this->m_CenterParticles) + this->m_CenterParticles;
                      });
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
UInt RigidBody<N, Real_t>::generateParticles(ParticleDataBase<N, Real_t>& particleData) {
    if(!this->m_GenParticleParams.bEnabled) {
        this->m_CenterParticles = (this->geometry()->getAABBMin() + this->geometry()->getAABBMax()) * Real_t(0.5);
        return 0;
    }
    ////////////////////////////////////////////////////////////////////////////////
    NT_REQUIRE(this->m_GeneratedParticles.size() == 0);
    auto newPositions = this->generateParticleInside();
    if(newPositions.size() > 0) {
        size_t oldSize = particleData.positions.size();
        size_t nGen    = newPositions.size();
        size_t newSize = oldSize + nGen;
        this->m_RangeGeneratedParticles = Vec2<size_t>(oldSize, newSize);
        particleData.positions.insert(particleData.positions.end(), newPositions.begin(), newPositions.end());
        particleData.resize_to_fit();
        this->m_CenterParticles = ParticleHelpers::getCenter(newPositions) + this->m_ShiftCenterGeneratedParticles;
        std::swap(this->m_GeneratedParticles, newPositions);
        return static_cast<UInt>(this->m_GeneratedParticles.size());
    }
    ////////////////////////////////////////////////////////////////////////////////
    return static_cast<UInt>(this->m_GeneratedParticles.size());
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
VecX<N, Real_t> RigidBody<N, Real_t>::getObjectVelocity(const VecN& ppos, Real_t timestep) {
    if(!this->geometry()->animationTransformed()) {
        return VecN(0);
    } else {
        auto ppos_t0   = this->geometry()->invTransformAnimation(ppos - this->m_CenterParticles);
        auto last_ppos = VecN(this->geometry()->getPrevAnimationTransformationMatrix() * VecNp1(ppos_t0, 1.0)) + this->m_CenterParticles;
        return (ppos - last_ppos) / timestep;
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool RigidBody<N, Real_t>::resolveCollision(VecN& ppos, VecN& pvel, Real_t timestep) {
    // do not collide with particles if the object is not a collision object
    if(!m_bIsCollisionObject) {
        return false;
    }
    ////////////////////////////////////////////////////////////////////////////////
    switch(m_BoundaryCondition) {
        case BoundaryCondition::Sticky:
            return resolveCollision_StickyBC(ppos, pvel, timestep);
        case BoundaryCondition::Separate:
            return resolveCollision_SeparateBC(ppos, pvel, timestep);
        case BoundaryCondition::Slip:
            return resolveCollision_SlipBC(ppos, pvel, timestep);
        default:;
    }
    return false;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool RigidBody<N, Real_t>::resolveCollisionVelocityOnly(const VecN& ppos, VecN& pvel, Real_t timestep) {
    // do not collide with particles if the object is not a rigid body object
    if(!m_bIsCollisionObject) {
        return false;
    }
    ////////////////////////////////////////////////////////////////////////////////
    switch(m_BoundaryCondition) {
        case BoundaryCondition::Sticky:
            return resolveCollisionVelocityOnly_StickyBC(ppos, pvel, timestep);
        case BoundaryCondition::Slip:
            return resolveCollisionVelocityOnly_SlipBC(ppos, pvel, timestep);
        case BoundaryCondition::Separate:
            return resolveCollisionVelocityOnly_SeparateBC(ppos, pvel, timestep);
        default:;
    }
    return false;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool RigidBody<N, Real_t>::resolveCollision_StickyBC(VecN& ppos, VecN& pvel, Real_t timestep) {
    if(const auto phiVal = this->signedDistance(ppos); phiVal < 0) {
        auto n = this->gradSignedDistance(ppos);
        if(auto n_l2 = glm::length2(n); n_l2 > Real_t(1e-20)) {
            n    /= std::sqrt(n_l2);
            ppos -= phiVal * n;
        }
        pvel = getObjectVelocity(ppos, timestep);
        return true;
    }
    return false;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool RigidBody<N, Real_t>::resolveCollisionVelocityOnly_StickyBC(const VecN& ppos, VecN& pvel, Real_t timestep) {
    if(const auto phiVal = this->signedDistance(ppos); phiVal < 0) {
        if(!this->geometry()->animationTransformed()) {
            pvel = VecN(0);
        } else {
            pvel = getObjectVelocity(ppos, timestep);
        }
        return true;
    }
    return false;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool RigidBody<N, Real_t>::resolveCollision_SlipBC(VecN& ppos, VecN& pvel, Real_t timestep) {
    if(const auto phiVal = this->signedDistance(ppos); phiVal < 0) {
        auto n = this->gradSignedDistance(ppos);
        if(auto n_l2 = glm::length2(n); n_l2 > Real_t(1e-20)) {
            n    /= std::sqrt(n_l2);
            ppos -= phiVal * n;
        }
        const auto vdn = glm::dot(pvel, n);
        pvel -= n * vdn;
        if(m_BoundaryFriction > 0 && vdn < 0) {
            const auto v_l  = glm::length(pvel);
            const auto vdnf = -vdn * m_BoundaryFriction;
            if(vdnf < v_l) {
                pvel -= pvel / v_l * vdnf;
            } else {
                pvel = VecN(0);
            }
        }
        pvel += getObjectVelocity(ppos, timestep);
        return true;
    }
    return false;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool RigidBody<N, Real_t>::resolveCollisionVelocityOnly_SlipBC(const VecN& ppos, VecN& pvel, Real_t timestep) {
    if(const auto phiVal = this->signedDistance(ppos); phiVal < 0) {
        auto n = this->gradSignedDistance(ppos);
        if(auto n_l2 = glm::length2(n); n_l2 > Real_t(1e-20)) {
            n /= std::sqrt(n_l2);
        }
        const auto vdn = glm::dot(pvel, n);
        pvel -= n * vdn;
        if(m_BoundaryFriction > 0 && vdn < 0) {
            const auto v_l  = glm::length(pvel);
            const auto vdnf = -vdn * m_BoundaryFriction;
            if(vdnf < v_l) {
                pvel -= pvel / v_l * vdnf;
            } else {
                pvel = VecN(0);
            }
        }
        pvel += getObjectVelocity(ppos, timestep);
        return true;
    }
    return false;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool RigidBody<N, Real_t>::resolveCollision_SeparateBC(VecN& ppos, VecN& pvel, Real_t timestep) {
    if(const auto phiVal = this->signedDistance(ppos); phiVal < 0) {
        auto n = this->gradSignedDistance(ppos);
        if(auto n_l2 = glm::length2(n); n_l2 > Real_t(1e-20)) {
            n /= std::sqrt(n_l2);
        }
        const auto vdn = glm::dot(pvel, n);
        if(vdn < 0) {
            ppos -= phiVal * n;
            pvel -= n * vdn;
            if(m_BoundaryFriction > 0) {
                const auto v_l  = glm::length(pvel);
                const auto vdnf = -vdn * m_BoundaryFriction;
                if(vdnf < v_l) {
                    pvel -= pvel / v_l * vdnf;
                } else {
                    pvel = VecN(0);
                }
            }
            pvel += getObjectVelocity(ppos, timestep);
            return true;
        }
    }
    return false;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool RigidBody<N, Real_t>::resolveCollisionVelocityOnly_SeparateBC(const VecN& ppos, VecN& pvel, Real_t timestep) {
    if(const auto phiVal = this->signedDistance(ppos); phiVal < 0) {
        auto n = this->gradSignedDistance(ppos);
        if(auto n_l2 = glm::length2(n); n_l2 > Real_t(1e-20)) {
            n /= std::sqrt(n_l2);
        }
        const auto vdn = glm::dot(pvel, n);
        if(vdn < 0) {
            pvel -= n * vdn;
            if(m_BoundaryFriction > 0) {
                const auto v_l  = glm::length(pvel);
                const auto vdnf = -vdn * m_BoundaryFriction;
                if(vdnf < v_l) {
                    pvel -= pvel / v_l * vdnf;
                } else {
                    pvel = VecN(0);
                }
            }
            pvel += getObjectVelocity(ppos, timestep);
            return true;
        }
    }
    return false;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
NT_INSTANTIATE_CLASS_COMMON_DIMENSIONS_AND_TYPES(RigidBody)
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
} // end namespace NTCodeBase
