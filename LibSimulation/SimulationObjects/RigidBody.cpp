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

#include <LibSimulation/SimulationObjects/RigidBody.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace SimulationObjects {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void RigidBody<N, Real_t>::parseBoundaryParameters(const JParams& jParams) {
    if(String bcType; JSONHelpers::readValue(jParams, bcType, "BCType")) {
        __NT_REQUIRE(bcType == "Sticky" ||
                     bcType == "Slip" ||
                     bcType == "Separate");
        if(bcType == "Sticky") {
            m_BoundaryConditionType = BoundaryConditionType::Sticky;
        } else if(bcType == "Slip") {
            m_BoundaryConditionType = BoundaryConditionType::Slip;
        } else {
            m_BoundaryConditionType = BoundaryConditionType::Separate;
        }
    }
    JSONHelpers::readValue(jParams, m_BoundaryFriction, "BoundaryFriction");
    JSONHelpers::readBool(jParams, m_bIsCollisionObject,     "IsCollisionObject");
    ////////////////////////////////////////////////////////////////////////////////
    JSONHelpers::readBool(jParams, m_bGenerateGhostParticle, "GenerateGhostParticle");
    JSONHelpers::readValue(jParams, m_ParticleGenerationThicknessRatio, "ParticleGenerationThicknessRatio");
    JSONHelpers::readValue(jParams, m_ParticleGenerationDensityRatio,   "ParticleGenerationDensityRatio");
    JSONHelpers::readVector(jParams, m_ShiftCenterGeneratedParticles, "ShiftCenterGeneratedParticles");
    ////////////////////////////////////////////////////////////////////////////////
    JSONHelpers::readBool(jParams, m_bConstrainInsideParticles,     "ConstrainInsideParticles");
    JSONHelpers::readBool(jParams, m_bCrashIfNoConstrainedParticle, "CrashIfNoConstrainedParticle");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void RigidBody<N, Real_t>::printParameters(Logger& logger) {
    logger.printLog(String("Rigid body object: ") + this->m_ObjName);
    SimulationObject<N, Real_t>::printParameters(logger);

    if(m_bIsCollisionObject) {
        if(m_BoundaryConditionType == BoundaryConditionType::Sticky) {
            logger.printLogIndent(String("Boundary condition type: Sticky"));
        } else {
            if(m_BoundaryConditionType == BoundaryConditionType::Slip) {
                logger.printLogIndent(String("Boundary condition type: Slip"));
            } else {
                logger.printLogIndent(String("Boundary condition type: Separate"));
            }
            logger.printLogIndent(String("Friction: ") + std::to_string(m_BoundaryFriction));
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    // todo: print generated ghost particles, and constrain inside particles

    logger.newLine();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
UInt RigidBody<N, Real_t>::generateParticles(StdVT<SharedPtr<RigidBody<N, Real_t>>>& rigidBodies) {
    __NT_UNUSED(rigidBodies);
    __NT_REQUIRE(m_GeneratedParticles.size() == 0);
    if(!m_bGenerateGhostParticle) {
        m_CenterGeneratedParticles = (this->geometry()->getAABBMin() + this->geometry()->getAABBMax()) * Real_t(0.5);
        return 0;
    }
    auto&      particleData = solverData.ghostParticleData;
    StdVT_VecN positions;
    ////////////////////////////////////////////////////////////////////////////////
    // load particles from cache, if spcified
    if(this->loadParticlesFromFile(positions, simParams("ParticleRadius").get<Real_t>())) {
        m_RangeGeneratedGhostParticles = Vec2<size_t>(particleData.positions.size(), particleData.positions.size() + positions.size());
        particleData.positions.insert(particleData.positions.end(), positions.begin(), positions.end());
        particleData.resize_to_fit();
        m_CenterGeneratedParticles = ParticleHelpers::getCenter(positions) + m_ShiftCenterGeneratedParticles;
        std::swap(m_GeneratedParticles, positions);
        return static_cast<UInt>(m_GeneratedParticles.size());
    }
    ////////////////////////////////////////////////////////////////////////////////
    auto                                                                                                                   thicknessThreshold = m_ParticleGenerationThicknessRatio < Real_t(1e10) ?
                                                                                       m_ParticleGenerationThicknessRatio* simParams("ParticleRadius").get<Real_t>() : HugeReal();
    auto                                                                                                                   spacing = simParams("ParticleRadius").get<Real_t>() * m_ParticleGenerationDensityRatio;
    auto                                                                                                                   boxMin  = this->m_GeometryObj->getAABBMin();
    auto                                                                                                                   boxMax  = this->m_GeometryObj->getAABBMax();
    auto                                                                                                                   pGrid   = NumberHelpers::createGrid<UInt>(boxMin, boxMax, spacing);
    ////////////////////////////////////////////////////////////////////////////////
    positions.reserve(glm::compMul(pGrid));
    ParallelObjects::SpinLock lock;

    Scheduler::parallel_for(pGrid,
                            [&](auto... idx) {
                                auto node = VecX<N, Real_t>(idx...);
                                VecN ppos = boxMin + node * spacing;
                                if(auto geoPhi = this->signedDistance(ppos);
                                   (geoPhi < -simParams("ParticleRadius").get<Real_t>()) && (geoPhi > -thicknessThreshold)) {
                                    lock.lock();
                                    positions.push_back(ppos);
                                    lock.unlock();
                                }
                            });
    m_RangeGeneratedGhostParticles = Vec2<size_t>(particleData.positions.size(), particleData.positions.size() + positions.size());
    particleData.positions.insert(particleData.positions.end(), positions.begin(), positions.end());
    particleData.resize_to_fit();
    ////////////////////////////////////////////////////////////////////////////////
    // save particles to file, if needed
    this->saveParticlesToFile(positions, simParams("ParticleRadius").get<Real_t>());
    ////////////////////////////////////////////////////////////////////////////////
    positions.shrink_to_fit();
    m_CenterGeneratedParticles = ParticleHelpers::getCenter(positions) + m_ShiftCenterGeneratedParticles;
    std::swap(m_GeneratedParticles, positions);
    return static_cast<UInt>(m_GeneratedParticles.size());
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool RigidBody<N, Real_t>::updateObject(UInt frame, Real_t frameFraction, Real_t frameDuration) {
    if(this->m_GeometryObj->updateTransformation(frame, frameFraction)) {
        tbb::parallel_invoke(
            [&] {
                if(m_bGenerateGhostParticle) {
                    updateGhostParticles(solverData);
                }
            },
            [&] {
                if(m_bHasConstrainedParticles) {
                    updateConstrainParticles(solverData, frame, frameFraction, frameDuration);
                }
            }
            );
        return true;
    } else if(m_bHasConstrainedParticles) {
        resetConstrainedParticles(solverData);
    }
    return false;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void RigidBody<N, Real_t>::updateGhostParticles() {
    const auto& range        = m_RangeGeneratedGhostParticles; // [start, end)
    const auto& positions_t0 = m_GeneratedParticles;
    __NT_REQUIRE(positions_t0.size() + range[0] == range[1] && range[1] <= solverData.ghostParticleData.positions.size());
    Scheduler::parallel_for(positions_t0.size(),
                            [&](size_t p) {
                                solverData.ghostParticleData.positions[p + range[0]] =
                                    this->geometry()->transformAnimation(positions_t0[p] - m_CenterGeneratedParticles) + m_CenterGeneratedParticles;
                            });
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void RigidBody<N, Real_t>::resetConstrainedParticles() {
    auto resetVelocities = [&](auto& particleIdx, auto& particleData) {
                               if(particleIdx.size() == 0) { return; }
                               for(size_t idx = 0; idx < particleIdx.size(); ++idx) {
                                   auto p = particleIdx[idx];
                                   particleData.velocities[p] = VecN(0);
                               }
                           };
    auto resetAngularVelocity = [&](auto& particleIdx, auto& particleData) {
                                    if(particleIdx.size() == 0) { return; }
                                    for(size_t idx = 0; idx < particleIdx.size(); ++idx) {
                                        auto p = particleIdx[idx];
                                        particleData.angularVelocities[p] = VecN(0);
                                    }
                                };
    ////////////////////////////////////////////////////////////////////////////////
    resetVelocities(m_ConstrainedStandardParticlesIdx, solverData.standardParticleData);
    resetVelocities(m_ConstrainedVertexParticlesIdx,   solverData.vertexParticleData);
    resetVelocities(m_ConstrainedQuadParticlesIdx,     solverData.quadParticleData);
    resetAngularVelocity(m_ConstrainedQuadParticlesIdx, solverData.quadParticleData);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void RigidBody<N, Real_t>::updateConstrainParticles(UInt frame, Real_t frameFraction, Real_t frameDuration) {
    auto updatePositionsVelocities = [&](auto& particleIdx, auto& positions_t0, auto& particleData, auto dT) {
                                         assert(dT > 0);
                                         if(particleIdx.size() == 0) { return; }
                                         Scheduler::parallel_for(particleIdx.size(),
                                                                 [&](size_t idx) {
                                                                     auto p = particleIdx[idx];
                                                                     updatePositionVelocity(positions_t0[idx], particleData.positions[p], particleData.velocities[p], dT);
                                                                 });
                                     };
    auto updateQuadParticles = [&](auto& quadParticleIdx, auto& quadParticleData, auto& vertexParticleData, auto dT) {
                                   assert(dT > 0);
                                   if(quadParticleIdx.size() == 0) { return; }
                                   ////////////////////////////////////////////////////////////////////////////////
                                   // compute the quaternion q that rotates from last orientation to current orientation
                                   auto prev_q    = glm::quat_cast(this->geometry()->getPrevTransformationMatrix());
                                   auto current_q = glm::quat_cast(this->geometry()->getTransformationMatrix());
                                   auto q         = glm::normalize(current_q * glm::inverse(prev_q));
                                   if(q.w < 0) { q = -q; } // select the shorter rotation
                                   auto angle = glm::angle(q);
                                   // w = angular velocity, set to 0 directly if angle < epsilon (for stability reason)
                                   auto w = (angle < Real_t(1e-8)) ? VecN(0) : glm::axis(q) * (angle / dT);
                                   ////////////////////////////////////////////////////////////////////////////////
                                   Scheduler::parallel_for(quadParticleIdx.size(),
                                                           [&, w, q](size_t i) {
                                                               auto p = quadParticleIdx[i];
                                                               const auto& parent = quadParticleData.parentID[p];
                                                               quadParticleData.positions[p] = (vertexParticleData.positions[parent[0]] +
                                                                                                vertexParticleData.positions[parent[1]]) * Real_t(0.5);
                                                               quadParticleData.velocities[p] = (vertexParticleData.velocities[parent[0]] +
                                                                                                 vertexParticleData.velocities[parent[1]]) * Real_t(0.5);

                                                               quadParticleData.angularVelocities[p] = w;
                                                               auto constrained_q = glm::normalize(q * LinaHelpers::vec4ToQuat(quadParticleData.localFrame[p]));
                                                               quadParticleData.localFrame[p] = Vec4r(constrained_q.x, constrained_q.y, constrained_q.z, constrained_q.w);
                                                           });
                               };
    ////////////////////////////////////////////////////////////////////////////////
    auto currentTime = frameDuration * (static_cast<Real_t>(frame) + frameFraction);
    auto diffTime = currentTime - m_LastUpdateTime; assert(diffTime > 0);
    m_LastUpdateTime = currentTime;
    updatePositionsVelocities(m_ConstrainedStandardParticlesIdx, m_ConstrainedStandardParticles_t0, solverData.standardParticleData, diffTime);
    updatePositionsVelocities(m_ConstrainedVertexParticlesIdx,   m_ConstrainedVertexParticles_t0,   solverData.vertexParticleData,   diffTime);
    updateQuadParticles(m_ConstrainedQuadParticlesIdx, solverData.quadParticleData, solverData.vertexParticleData, diffTime);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
inline void RigidBody<N, Real_t>::updatePositionVelocity(VecN& ppos_t0, VecN& ppos, VecN& pvel, Real_t timestep) {
    auto new_pos = this->geometry()->transformAnimation(ppos_t0 - m_CenterGeneratedParticles) + m_CenterGeneratedParticles;
    pvel = (new_pos - ppos) / timestep;
    ppos = new_pos;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
inline VecX<N, Real_t> RigidBody<N, Real_t>::getObjectVelocity(const VecN& ppos, Real_t timestep) {
    if(!this->geometry()->animationTransformed()) {
        return VecN(0);
    } else {
        auto ppos_t0   = this->geometry()->invTransformAnimation(ppos - m_CenterGeneratedParticles);
        auto last_ppos = VecN(this->geometry()->getPrevAnimationTransformationMatrix() * VecNp1(ppos_t0, 1.0)) + m_CenterGeneratedParticles;
        return (ppos - last_ppos) / timestep;
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool RigidBody<N, Real_t>::isInside(const VecN& ppos) const {
    return m_GeometryObj->isInside(ppos, false);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
Real_t RigidBody<N, Real_t>::signedDistance(const VecN& ppos) const {
    return m_GeometryObj->signedDistance(ppos, false);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
VecX<N, Real_t> RigidBody<N, Real_t>::gradSignedDistance(const VecN& ppos, Real_t dxyz /*= Real_t(1e-4)*/) const {
    return m_GeometryObj->gradSignedDistance(ppos, false, dxyz);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool RigidBody<N, Real_t>::findConstrainedParticles() {
    if(!m_bConstrainInsideParticles) {
        return false;
    }
    m_ConstrainedStandardParticlesIdx.resize(0);
    m_ConstrainedVertexParticlesIdx.resize(0);
    m_ConstrainedQuadParticlesIdx.resize(0);

    m_ConstrainedStandardParticles_t0.resize(0);
    m_ConstrainedVertexParticles_t0.resize(0);
    m_ConstrainedQuadParticles_t0.resize(0);
    m_ConstrainedQuadParticleLocalFrames_t0.resize(0);
    ////////////////////////////////////////////////////////////////////////////////
    auto&                     standardParticleData = solverData.standardParticleData;
    auto&                     vertexParticleData   = solverData.vertexParticleData;
    auto&                     quadParticleData     = solverData.quadParticleData;
    ParallelObjects::SpinLock lock;
    Scheduler::parallel_for(standardParticleData.nParticles(),
                            [&](UInt p) {
                                const auto& ppos = standardParticleData.positions[p];
                                if(this->signedDistance(ppos) < 0) {
                                    standardParticleData.activity[p] = static_cast<Int8>(Activity::Constrained);
                                    lock.lock();
                                    m_ConstrainedStandardParticlesIdx.push_back(p);
                                    m_ConstrainedStandardParticles_t0.push_back(ppos);
                                    lock.unlock();
                                }
                            });

    Scheduler::parallel_for(vertexParticleData.nParticles(),
                            [&](UInt p) {
                                const auto& ppos = vertexParticleData.positions[p];
                                auto ppos2       = vertexParticleData.positions[p];
                                if(this->signedDistance(ppos2) < 0) {
                                    vertexParticleData.activity[p] = static_cast<Int8>(Activity::Constrained);
                                    lock.lock();
                                    m_ConstrainedVertexParticlesIdx.push_back(p);
                                    m_ConstrainedVertexParticles_t0.push_back(ppos);
                                    lock.unlock();
                                }
                            });

    Scheduler::parallel_for(quadParticleData.nParticles(),
                            [&](UInt p) {
                                const auto& ppos = quadParticleData.positions[p];
                                auto ppos2       = quadParticleData.positions[p];
                                if(this->signedDistance(ppos2) < 0) {
                                    const auto& parent = quadParticleData.parentID[p];
                                    if(vertexParticleData.activity[parent[0]] == static_cast<Int8>(Activity::Constrained) &&
                                       vertexParticleData.activity[parent[1]] == static_cast<Int8>(Activity::Constrained)) {
                                        quadParticleData.activity[p] = static_cast<Int8>(Activity::Constrained);
                                        lock.lock();
                                        m_ConstrainedQuadParticlesIdx.push_back(p);
                                        m_ConstrainedQuadParticles_t0.push_back(ppos);
                                        m_ConstrainedQuadParticleLocalFrames_t0.push_back(quadParticleData.localFrame[p]);
                                        lock.unlock();
                                    }
                                }
                            });

    m_bHasConstrainedParticles = m_ConstrainedStandardParticlesIdx.size() > 0 ||
                                 m_ConstrainedVertexParticlesIdx.size() > 0 ||
                                 m_ConstrainedQuadParticlesIdx.size() > 0;
    __NT_REQUIRE(!m_bCrashIfNoConstrainedParticle || m_bHasConstrainedParticles);

    if(m_bHasConstrainedParticles) {
        m_ConstrainedStandardParticlesIdx.shrink_to_fit();
        m_ConstrainedVertexParticlesIdx.shrink_to_fit();
        m_ConstrainedQuadParticlesIdx.shrink_to_fit();

        m_ConstrainedStandardParticles_t0.shrink_to_fit();
        m_ConstrainedVertexParticles_t0.shrink_to_fit();
        m_ConstrainedQuadParticles_t0.shrink_to_fit();
        m_ConstrainedQuadParticleLocalFrames_t0.shrink_to_fit();
    }
    return m_bHasConstrainedParticles;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool RigidBody<N, Real_t>::resolveCollision(VecN& ppos, VecN& pvel, Real_t timestep) {
    // do not collide with particles if the object is not a collision object
    if(!m_bIsCollisionObject) {
        return false;
    }
    ////////////////////////////////////////////////////////////////////////////////
    switch(m_BoundaryConditionType) {
        case BoundaryConditionType::Sticky:
            return resolveCollision_StickyBC(ppos, pvel, timestep);
        case BoundaryConditionType::Separate:
            return resolveCollision_SeparateBC(ppos, pvel, timestep);
        case BoundaryConditionType::Slip:
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
    switch(m_BoundaryConditionType) {
        case BoundaryConditionType::Sticky:
            return resolveCollisionVelocityOnly_StickyBC(ppos, pvel, timestep);
        case BoundaryConditionType::Slip:
            return resolveCollisionVelocityOnly_SlipBC(ppos, pvel, timestep);
        case BoundaryConditionType::Separate:
            return resolveCollisionVelocityOnly_SeparateBC(ppos, pvel, timestep);
        default:;
    }
    return false;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
inline bool RigidBody<N, Real_t>::resolveCollision_StickyBC(VecN& ppos, VecN& pvel, Real_t timestep) {
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
inline bool RigidBody<N, Real_t>::resolveCollisionVelocityOnly_StickyBC(const VecN& ppos, VecN& pvel, Real_t timestep) {
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
inline bool RigidBody<N, Real_t>::resolveCollision_SlipBC(VecN& ppos, VecN& pvel, Real_t timestep) {
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
inline bool RigidBody<N, Real_t>::resolveCollisionVelocityOnly_SlipBC(const VecN& ppos, VecN& pvel, Real_t timestep) {
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
inline bool RigidBody<N, Real_t>::resolveCollision_SeparateBC(VecN& ppos, VecN& pvel, Real_t timestep) {
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
inline bool RigidBody<N, Real_t>::resolveCollisionVelocityOnly_SeparateBC(const VecN& ppos, VecN& pvel, Real_t timestep) {
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
