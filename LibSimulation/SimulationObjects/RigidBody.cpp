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
#include <LibSimulation/SimulationObjects/RigidBody.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace SimulationObjects {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void RigidBody<N, Real_t>::initializeParameters(const JParams& jParams) {
    JSONHelpers::readBool(jParams, m_bIsCollisionObject, "IsCollisionObject");
    if(m_bIsCollisionObject) {
        if(String bcType; JSONHelpers::readValue(jParams, bcType, "BCType")) {
            __NT_REQUIRE(bcType == "Sticky" ||
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
    ////////////////////////////////////////////////////////////////////////////////
    JSONHelpers::readBool(jParams, m_bGenerateParticleInside, "GenerateParticleInside");
    JSONHelpers::readValue(jParams, m_ParticleGenerationThicknessRatio, "ParticleGenerationThicknessRatio");
    JSONHelpers::readValue(jParams, m_ParticleGenerationDensityRatio,   "ParticleGenerationDensityRatio");
    JSONHelpers::readVector(jParams, m_ShiftCenterGeneratedParticles, "ShiftCenterGeneratedParticles");
    ////////////////////////////////////////////////////////////////////////////////
    JSONHelpers::readBool(jParams, m_bConstrainInsideParticles,     "ConstrainInsideParticles");
    JSONHelpers::readBool(jParams, m_bCrashIfNoConstrainedParticle, "CrashIfNoConstrainedParticle");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
UInt RigidBody<N, Real_t>::generateParticles(PropertyGroup& propertyGroup, StdVT<SharedPtr<SimulationObject<N, Real_t>>>& otherObjects,
                                             bool bIgnoreOverlapped /*= false*/) {
    if(!m_bGenerateParticleInside) {
        m_CenterParticles = (this->geometry()->getAABBMin() + this->geometry()->getAABBMax()) * Real_t(0.5);
        return 0;
    }
    ////////////////////////////////////////////////////////////////////////////////
    __NT_REQUIRE(m_GeneratedParticles.size() == 0);
    auto newPositions = generateParticles(otherObjects, bIgnoreOverlapped);
    if(newPositions.size() > 0) {
        auto&  positions = propertyGroup.property<VecN>("Position");
        size_t oldSize   = propertyGroup.size();
        size_t newSize   = propertyGroup.size() + newPositions.size();
        m_RangeGeneratedParticles = Vec2<size_t>(oldSize, newSize);
        propertyGroup.resize(newSize);

        for(size_t p = 0; p < newPositions.size(); ++p) {
            positions[p + oldSize] = newPositions[p];
        }
        m_CenterParticles = ParticleHelpers::getCenter(newPositions) + m_ShiftCenterGeneratedParticles;
        std::swap(m_GeneratedParticles, newPositions);
        return static_cast<UInt>(m_GeneratedParticles.size());
    }
    ////////////////////////////////////////////////////////////////////////////////
    return static_cast<UInt>(m_GeneratedParticles.size());
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
typename RigidBody<N, Real_t>::StdVT_VecN
RigidBody<N, Real_t>::generateParticles(StdVT<SharedPtr<SimulationObject<N, Real_t>>>& otherObjects, bool bIgnoreOverlapped /*= false*/) {
    StdVT_VecN positions;
    if(this->loadParticlesFromFile(positions)) {
        return positions;
    }
    const auto particleRadius     = simParams("ParticleRadius").get<Real_t>();
    auto       thicknessThreshold = m_ParticleGenerationThicknessRatio * particleRadius;
    auto       spacing = particleRadius * m_ParticleGenerationDensityRatio;
    auto       boxMin  = this->m_GeometryObj->getAABBMin();
    auto       boxMax  = this->m_GeometryObj->getAABBMax();
    auto       pGrid   = NumberHelpers::createGrid<UInt>(boxMin, boxMax, spacing);
    ////////////////////////////////////////////////////////////////////////////////
    positions.reserve(glm::compMul(pGrid));
    ParallelObjects::SpinLock lock;
    if(bIgnoreOverlapped) {
        Scheduler::parallel_for(pGrid,
                                [&](auto... idx) {
                                    auto node = VecX<N, Real_t>(idx...);
                                    VecN ppos = boxMin + node * spacing;
                                    if(auto geoPhi = this->signedDistance(ppos);
                                       (geoPhi < -particleRadius) && (geoPhi > -thicknessThreshold)) {
                                        lock.lock();
                                        positions.push_back(ppos);
                                        lock.unlock();
                                    }
                                });
    } else {
        Scheduler::parallel_for(pGrid,
                                [&](auto... idx) {
                                    auto node = VecX<N, Real_t>(idx...);
                                    VecN ppos = boxMin + node * spacing;
                                    for(auto& obj : otherObjects) {
                                        if(obj->objID() != this->objID() && obj->signedDistance(ppos) < 0) {
                                            return;
                                        }
                                    }
                                    if(auto geoPhi = this->signedDistance(ppos);
                                       (geoPhi < -particleRadius) && (geoPhi > -thicknessThreshold)) {
                                        lock.lock();
                                        positions.push_back(ppos);
                                        lock.unlock();
                                    }
                                });
    }
    positions.shrink_to_fit();
    ////////////////////////////////////////////////////////////////////////////////
    // save particles to file, if needed
    this->saveParticlesToFile(positions);
    return positions;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool RigidBody<N, Real_t>::updateObject(UInt frame, Real_t frameFraction, Real_t timestep) {
    if(this->m_GeometryObj->updateTransformation(frame, frameFraction)) {
        tbb::parallel_invoke(
            [&] {
                if(m_bGenerateParticleInside) {
                    updateObjParticles();
                }
            },
            [&] {
                if(m_bHasConstrainedParticles) {
                    updateConstrainParticles(timestep);
                }
            }
            );
        return true;
    } else if(m_bHasConstrainedParticles) {
        resetConstrainedParticles();
    }
    return false;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void RigidBody<N, Real_t>::updateObjParticles() {
    const auto& range        = m_RangeGeneratedParticles; // [start, end)
    const auto& positions_t0 = m_GeneratedParticles;
    auto&       positions    = boundaryParticleData().property<VecN>("Position");
    __NT_REQUIRE(positions_t0.size() + range[0] == range[1] && range[1] <= positions.size());
    Scheduler::parallel_for(positions_t0.size(),
                            [&](size_t p) {
                                positions[p + range[0]] =
                                    this->geometry()->transformAnimation(positions_t0[p] - m_CenterParticles) + m_CenterParticles;
                            });
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
std::pair<bool, StdVT_UInt> RigidBody<N, Real_t>::findConstrainedParticles(PropertyGroup& propertyGroup) {
    if(!m_bConstrainInsideParticles) {
        return { false, {} };
    }
    auto& constrainedIdx         = m_ConstrainedParticleInfo[propertyGroup.hash()].first;
    auto& constrainedPosition_t0 = m_ConstrainedParticleInfo[propertyGroup.hash()].second;
    constrainedIdx.resize(0);
    constrainedPosition_t0.resize(0);
    ////////////////////////////////////////////////////////////////////////////////
    const auto&               positions  = propertyGroup.property<VecN>("Position");
    auto&                     activities = propertyGroup.property<Int8>("Activity");
    ParallelObjects::SpinLock lock;
    Scheduler::parallel_for(static_cast<UInt>(propertyGroup.size()),
                            [&](UInt p) {
                                const auto& ppos = positions[p];
                                if(this->signedDistance(ppos) < 0) {
                                    activities[p] = static_cast<Int8>(Activity::Constrained);
                                    lock.lock();
                                    constrainedIdx.push_back(p);
                                    constrainedPosition_t0.push_back(ppos);
                                    lock.unlock();
                                }
                            });

    ////////////////////////////////////////////////////////////////////////////////
    return { constrainedIdx.size() > 0, StdVT_UInt { static_cast<UInt>(constrainedIdx.size()) } };
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
template<class... Groups>
std::pair<bool, StdVT_UInt> RigidBody<N, Real_t>::findConstrainedParticles(PropertyGroup& propertyGroup, Groups& ... groups) {
    if(!m_bConstrainInsideParticles) {
        return { false, {} };
    }
    auto [bHasConstrainedFirst, nConstrainedFirst]   = findConstrainedParticles(propertyGroup);
    auto [bHasConstrainedOthers, nConstrainedOthers] = findConstrainedParticles(groups...);
    m_bHasConstrainedParticles = bHasConstrainedFirst || bHasConstrainedOthers;
    nConstrainedFirst.insert(nConstrainedFirst.end(), nConstrainedOthers.begin(), nConstrainedOthers.end());
    ////////////////////////////////////////////////////////////////////////////////
    return { m_bHasConstrainedParticles, nConstrainedFirst };
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void RigidBody<N, Real_t>::resetConstrainedParticles() {
    auto resetVelocities = [&](auto& particleIdx, auto& velocities) {
                               if(particleIdx.size() == 0) { return; }
                               for(size_t idx = 0; idx < particleIdx.size(); ++idx) {
                                   auto p = particleIdx[idx];
                                   velocities[p] = VecN(0);
                               }
                           };
    auto resetAngularVelocity = [&](auto& particleIdx, auto& angularVelocities) {
                                    if(particleIdx.size() == 0) { return; }
                                    for(size_t idx = 0; idx < particleIdx.size(); ++idx) {
                                        auto p = particleIdx[idx];
                                        angularVelocities[p] = VecN(0);
                                    }
                                };
    ////////////////////////////////////////////////////////////////////////////////
    for(auto& [groupHash, constrainedInfo] : m_ConstrainedParticleInfo) {
        auto& group = this->m_PropertyManager.group(groupHash);
        resetVelocities(constrainedInfo.first, group.property<VecN>("Velocity"));
        if(group.hasProperty("AngularVelocity")) {
            resetAngularVelocity(constrainedInfo.first, group.property<VecN>("AngularVelocity"));
        }
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void RigidBody<N, Real_t>::updateConstrainParticles(Real_t timestep) {
    __NT_REQUIRE(timestep > 0);
    auto updatePositionsVelocities =
        [&](auto& constrainedInfo, auto& propertyGroup) {
            auto& particleIdx = constrainedInfo.first;
            if(particleIdx.size() == 0) { return; }
            ////////////////////////////////////////////////////////////////////////////////
            auto& positions_t0 = constrainedInfo.second;
            auto& positions    = propertyGroup.property<VecN>("Position");
            auto& velocities   = propertyGroup.property<VecN>("Velocity");
            Scheduler::parallel_for(particleIdx.size(),
                                    [&](size_t idx) {
                                        auto p        = particleIdx[idx];
                                        auto new_pos  = this->geometry()->transformAnimation(positions_t0[idx] - m_CenterParticles) + m_CenterParticles;
                                        velocities[p] = (new_pos - positions[p]) / timestep;
                                        positions[p]  = new_pos;
                                    });
        };

    ////////////////////////////////////////////////////////////////////////////////
    for(auto& [groupHash, constrainedInfo] : m_ConstrainedParticleInfo) {
        auto& group = this->m_PropertyManager.group(groupHash);
        updatePositionsVelocities(constrainedInfo, group);
        ////////////////////////////////////////////////////////////////////////////////
        if constexpr(N == 3) { // only 3D solver can update angular velocity
            auto updateAngularVelocities =
                [&](auto& constrainedInfo, auto& propertyGroup, const auto& q) {
                    auto& particleIdx = constrainedInfo.first;
                    if(particleIdx.size() == 0) { return; }
                    const auto angle = glm::angle(q);
                    // w = angular velocity, set to 0 directly if angle < epsilon (for stability reason)
                    const auto w = (angle < Real_t(1e-8)) ? VecN(0) : glm::axis(q) * (angle / timestep);
                    auto&      angularVelocities = propertyGroup.property<VecN>("AngularVelocity");
                    Scheduler::parallel_for(particleIdx.size(),
                                            [&, w, q](size_t idx) {
                                                auto p = particleIdx[idx];
                                                angularVelocities[p] = w;
                                            });
                };
            auto updateFirstCotangents =
                [&](auto& constrainedInfo, auto& propertyGroup, const auto& q) {
                    auto& particleIdx = constrainedInfo.first;
                    if(particleIdx.size() == 0) { return; }
                    const auto R = glm::mat3_cast(q); // rotation matrix computed from q
                    auto&      firstCotangent = propertyGroup.property<VecN>("FirstCotangent");
                    Scheduler::parallel_for(particleIdx.size(),
                                            [&, q](size_t idx) {
                                                auto p = particleIdx[idx];
                                                firstCotangent[p] = R * firstCotangent[p];
                                            });
                };
            ////////////////////////////////////////////////////////////////////////////////
            bool bHasW = group.hasProperty("AngularVelocity");
            bool bHasFirstCT = group.hasProperty("FirstCotangent");
            if(bHasW || bHasFirstCT) {
                ////////////////////////////////////////////////////////////////////////////////
                // compute the quaternion q that rotates from last orientation to current orientation
                auto prev_q    = glm::quat_cast(this->geometry()->getPrevTransformationMatrix());
                auto current_q = glm::quat_cast(this->geometry()->getTransformationMatrix());
                auto q         = glm::normalize(current_q * glm::inverse(prev_q));
                if(q.w < 0) { q = -q; } // select the shorter rotation
                ////////////////////////////////////////////////////////////////////////////////
                if(bHasW) { updateAngularVelocities(constrainedInfo, group, q); }
                if(bHasFirstCT) { updateFirstCotangents(constrainedInfo, group, q); }
            }
        }
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
VecX<N, Real_t> RigidBody<N, Real_t>::getObjectVelocity(const VecN& ppos, Real_t timestep) {
    if(!this->geometry()->animationTransformed()) {
        return VecN(0);
    } else {
        auto ppos_t0   = this->geometry()->invTransformAnimation(ppos - m_CenterParticles);
        auto last_ppos = VecN(this->geometry()->getPrevAnimationTransformationMatrix() * VecNp1(ppos_t0, 1.0)) + m_CenterParticles;
        return (ppos - last_ppos) / timestep;
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool RigidBody<N, Real_t>::isInside(const VecN& ppos) const {
    return this->m_GeometryObj->isInside(ppos, false);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
Real_t RigidBody<N, Real_t>::signedDistance(const VecN& ppos) const {
    return this->m_GeometryObj->signedDistance(ppos, false);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
VecX<N, Real_t> RigidBody<N, Real_t>::gradSignedDistance(const VecN& ppos, Real_t dxyz /*= Real_t(1e-4)*/) const {
    return this->m_GeometryObj->gradSignedDistance(ppos, false, dxyz);
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
__NT_INSTANTIATE_CLASS_COMMON_DIMENSIONS_AND_TYPES(RigidBody)
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
} // end namespace SimulationObjects
