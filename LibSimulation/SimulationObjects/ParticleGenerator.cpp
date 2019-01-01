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

#include <LibSimulation/SimulationObjects/ParticleGenerator.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace SimulationObjects {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void ParticleGenerator<N, Real_t>::initializeParameters(const JParams& jParams) {
    __NT_REQUIRE(JSONHelpers::readValue(jParams, m_MaterialDensity, "MaterialDensity"));
    m_ParticleMass = m_MaterialDensity * MathHelpers::pow(Real_t(2.0) * simParams("ParticleRadius").get<Real_t>(), N);
    logger().printLogIndent(String("Material density: ") + std::to_string(m_MaterialDensity));
    logger().printLogIndent(String("Particle mass: ") + std::to_string(m_ParticleMass));
    ////////////////////////////////////////////////////////////////////////////////
    JSONHelpers::readValue(jParams, m_JitterRatio, "JitterRatio");
    JSONHelpers::readVector(jParams, m_SamplingRatio, "SamplingRatio");
    logger().printLogIndent(String("Jitter ratio: ") + std::to_string(m_JitterRatio));
    logger().printLogIndent(String("Sampling ratio: ") + Formatters::toString(m_SamplingRatio));
    ////////////////////////////////////////////////////////////////////////////////
    JSONHelpers::readVector(jParams, m_v0, "InitialVelocity");
    logger().printLogIndent(String("Initial velocity: ") + Formatters::toString(m_v0));
    logger().newLine();
    ////////////////////////////////////////////////////////////////////////////////
    JSONHelpers::readBool(jParams, m_bCrashIfNoParticle, "CrashIfNoParticle");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
UInt ParticleGenerator<N, Real_t>::generateParticles(PropertyGroup& propertyGroup, StdVT<SharedPtr<SimulationObject<N, Real_t>>>& otherObjects,
                                                     bool bIgnoreOverlapped /*= false*/) {
    auto newPositions = generateParticles(otherObjects, bIgnoreOverlapped);
    __NT_REQUIRE(newPositions.size() > 0 || !m_bCrashIfNoParticle);
    ////////////////////////////////////////////////////////////////////////////////
    size_t oldSize = propertyGroup.size();
    propertyGroup.resize(oldSize + newPositions.size());
    auto& positions  = propertyGroup.property<VecN>("Position");
    auto& velocities = propertyGroup.property<VecN>("Velocity");
    auto& masses     = propertyGroup.property<Real_t>("Mass");
    for(size_t p = 0; p < newPositions.size(); ++p) {
        positions[p + oldSize]  = newPositions[p];
        velocities[p + oldSize] = m_v0;
        masses[p + oldSize]     = m_ParticleMass;
    }
    ////////////////////////////////////////////////////////////////////////////////
    return static_cast<UInt>(newPositions.size());
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
typename ParticleGenerator<N, Real_t>::StdVT_VecN
ParticleGenerator<N, Real_t>::generateParticles(StdVT<SharedPtr<SimulationObject<N, Real_t>>>& otherObjects,
                                                bool bIgnoreOverlapped /*= false*/) {
    StdVT_VecN positions;
    if(this->loadParticlesFromFile(positions)) {
        return positions;
    }
    const auto particleRadius = simParams("ParticleRadius").get<Real_t>();
    const auto spacing        = m_SamplingRatio * particleRadius * Real_t(2);
    const auto boxMin         = this->m_GeometryObj->getAABBMin();
    const auto boxMax         = this->m_GeometryObj->getAABBMax();
    const auto pGrid          = NumberHelpers::createGrid<UInt>(boxMin, boxMax, spacing);
    ////////////////////////////////////////////////////////////////////////////////
    positions.reserve(glm::compMul(pGrid));
    ParallelObjects::SpinLock lock;
    if(bIgnoreOverlapped) {
        Scheduler::parallel_for(pGrid,
                                [&](auto... idx) {
                                    auto node = VecX<N, Real_t>(idx...);
                                    VecN ppos = boxMin + node * spacing;
                                    if(this->signedDistance(ppos) < 0) { // inside object
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
                                    if(this->signedDistance(ppos) < 0) { // inside object
                                        lock.lock();
                                        positions.push_back(ppos);
                                        lock.unlock();
                                    }
                                });
    }
    ////////////////////////////////////////////////////////////////////////////////
    // jitter positions
    const auto jitter = m_JitterRatio * particleRadius;
    if(jitter > TinyReal()) {
        for(auto& ppos: positions) {
            NumberHelpers::jitter(ppos, jitter);
        }
    }
    ////////////////////////////////////////////////////////////////////////////////
    // save particles to file, if needed
    this->saveParticlesToFile(positions);
    return positions;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
__NT_INSTANTIATE_CLASS_COMMON_DIMENSIONS_AND_TYPES(ParticleGenerator)
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
} // end namespace SimulationObjects
