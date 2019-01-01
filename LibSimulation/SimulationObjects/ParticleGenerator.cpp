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
#include <LibCommon/ParallelHelpers/Scheduler.h>
#include <LibCommon/Utils/JSONHelpers.h>
#include <LibCommon/Utils/NumberHelpers.h>
#include <LibCommon/Logger/Logger.h>

#include <Simulation/Solvers/SolverData/SolverData.h>

#include <Simulation/SimulationObjects/RigidBody.h>
#include <Simulation/SimulationObjects/ParticleGenerator.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace SimulationObjects {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void ParticleGenerator<N, Real_t>::parseGeneratorParameters(const JParams& jParams) {
    JSONHelpers::readBool(jParams, m_bCrashIfNoParticle, "CrashIfNoParticle");
    JSONHelpers::readVector(jParams, m_v0, "InitialVelocity");
    JSONHelpers::readValue(jParams, m_MaterialDensity, "MaterialDensity");
    ////////////////////////////////////////////////////////////////////////////////
    JSONHelpers::readValue(jParams, m_JitterRatio,     "JitterRatio");
    JSONHelpers::readVector(jParams, m_SamplingRatio, "SamplingRatio");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void ParticleGenerator<N, Real_t>::printParameters(Logger& logger) {
    logger.printLog(String("Standard particle generator: ") + this->m_ObjName);
    SimulationObject<N, Real_t>::printParameters(logger);
    logger.printLogIndent(String("Material density: ") + std::to_string(m_MaterialDensity));
    if(m_ParticleMass > TinyReal()) {
        logger.printLogIndent(String("Particle mass: ") + std::to_string(m_ParticleMass));
    }
    logger.printLogIndent(String("Initial velocity: ") + Formatters::toString(m_v0));
    logger.printLogIndent(String("Sampling ratio: ") + Formatters::toString(m_SamplingRatio));
    logger.printLogIndent(String("Jitter ratio: ") + std::to_string(m_JitterRatio));
    logger.newLine();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
UInt ParticleGenerator<N, Real_t>::generateParticles(ParticleSolvers::SolverData<N, Real_t>& solverData,
                                                     StdVT<SharedPtr<RigidBody<N, Real_t>>>& rigidBodies) {
    __NT_REQUIRE(simParams("ParticleRadius").get<Real_t>() > 0);
    StdVT_VecN newPositions = generateParticles(simParams("ParticleRadius").get<Real_t>(), rigidBodies);
    Scheduler::parallel_for(newPositions.size(),
                            [&](size_t p) {
                                const auto& ppos = newPositions[p];
                                __NT_REQUIRE(solverData.gridData().grid.isInsideClampedBoundary(ppos));
                            });
    ////////////////////////////////////////////////////////////////////////////////
    m_ParticleMass = m_MaterialDensity * MathHelpers::pow(Real_t(2.0) * simParams("ParticleRadius").get<Real_t>(), N);
    auto& particleData = solverData.standardParticleData;
    particleData.positions.insert(particleData.positions.end(), newPositions.begin(), newPositions.end());
    particleData.velocities.resize(particleData.velocities.size() + newPositions.size(), m_v0);
    particleData.mass.resize(particleData.mass.size() + newPositions.size(), m_ParticleMass);
    particleData.positions.shrink_to_fit();
    particleData.resize_to_fit();
    ////////////////////////////////////////////////////////////////////////////////
    __NT_REQUIRE(newPositions.size() > 0 || !m_bCrashIfNoParticle);
    return static_cast<UInt>(newPositions.size());
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
typename ParticleGenerator<N, Real_t>::StdVT_VecN
ParticleGenerator<N, Real_t>::generateParticles(Real_t particleRadius, const StdVT<SharedPtr<RigidBody<N, Real_t>>>& rigidBodies) {
    __NT_UNUSED(rigidBodies);
    StdVT_VecN positions;
    if(this->loadParticlesFromFile(positions, particleRadius)) {
        return positions;
    }
    auto spacing = m_SamplingRatio * particleRadius * Real_t(2);
    auto jitter  = m_JitterRatio * particleRadius;
    auto boxMin  = this->m_GeometryObj->getAABBMin();
    auto boxMax  = this->m_GeometryObj->getAABBMax();
    auto pGrid   = NumberHelpers::createGrid<UInt>(boxMin, boxMax, spacing);
    ////////////////////////////////////////////////////////////////////////////////
    positions.reserve(glm::compMul(pGrid));
    ParallelObjects::SpinLock lock;
    Scheduler::parallel_for(pGrid,
                            [&](auto... idx) {
                                auto node = VecX<N, Real_t>(idx...);
                                VecN ppos = boxMin + node * spacing;

                                //                                for(auto& rigidBody : rigidBodies) {
                                //                                    if(rigidBody->signedDistance(ppos) < 0) { // inside boundary
                                //                                        return;
                                //                                    }
                                //                                }
                                if(this->signedDistance(ppos) < 0) { // inside object
                                    lock.lock();
                                    positions.push_back(ppos);
                                    lock.unlock();
                                }
                            });
    ////////////////////////////////////////////////////////////////////////////////
    // jitter positions
    if(jitter > TinyReal()) {
        for(auto& ppos: positions) {
            NumberHelpers::jitter(ppos, jitter);
        }
    }
    ////////////////////////////////////////////////////////////////////////////////
    // save particles to file, if needed
    this->saveParticlesToFile(positions, particleRadius);
    return positions;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
__NT_INSTANTIATE_CLASS_COMMON_DIMENSIONS_AND_TYPES(ParticleGenerator)
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
} // end namespace SimulationObjects
