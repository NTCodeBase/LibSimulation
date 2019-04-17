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
#include <LibCommon/Logger/Logger.h>
#include <LibCommon/Utils/JSONHelpers.h>
#include <LibCommon/Utils/NumberHelpers.h>

#include <LibParticle/ParticleHelpers.h>
#include <LibSimulation/SimulationObjects/ParticleGenerator.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace NTCodeBase {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void ParticleGenerator<N, Real_t>::initializeParameters(const JParams& jParams) {
    __NT_REQUIRE(JSONHelpers::readValue(jParams, m_MaterialDensity, "MaterialDensity"));
    m_ParticleMass = m_MaterialDensity * MathHelpers::pow(Real_t(2.0) * this->m_ParticleRadius, N);
    logger().printLogIndent(String("Material density: ") + std::to_string(m_MaterialDensity));
    logger().printLogIndent(String("Particle mass: ") + std::to_string(m_ParticleMass));
    ////////////////////////////////////////////////////////////////////////////////
    JSONHelpers::readVector(jParams, m_v0, "InitialVelocity");
    logger().printLogIndent(String("Initial velocity: ") + Formatters::toString(m_v0));
    logger().newLine();
    ////////////////////////////////////////////////////////////////////////////////
    JSONHelpers::readBool(jParams, m_bCrashIfNoParticle, "CrashIfNoParticle");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
UInt ParticleGenerator<N, Real_t>::generateParticles(ParticleDataBase<N, Real_t>& particleData) {
    if(!this->m_GenParticleParams.bEnabled) {
        this->m_CenterParticles = (this->geometry()->getAABBMin() + this->geometry()->getAABBMax()) * Real_t(0.5);
        return 0;
    }
    ////////////////////////////////////////////////////////////////////////////////
    StdVT_VecN newPositions;
    if(this->m_GeneratedParticles.size() > 0) {
        newPositions = this->m_GeneratedParticles;
    } else {
        newPositions = this->generateParticleInside();
    }
    size_t nGen = newPositions.size();
    __NT_REQUIRE(nGen > 0 || !this->m_bCrashIfNoParticle);
    if(nGen > 0) {
        size_t oldSize = particleData.positions.size();
        size_t newSize = oldSize + nGen;
        this->m_RangeGeneratedParticles = Vec2<size_t>(oldSize, newSize);
        particleData.positions.insert(particleData.positions.end(), newPositions.begin(), newPositions.end());
        particleData.velocities.resize(newSize, this->m_v0);
        particleData.masses.resize(newSize, this->m_ParticleMass);
        particleData.resize_to_fit();
        this->m_CenterParticles = ParticleHelpers::getCenter(newPositions) + this->m_ShiftCenterGeneratedParticles;
        std::swap(this->m_GeneratedParticles, newPositions);
        return static_cast<UInt>(this->m_GeneratedParticles.size());
    }
    ////////////////////////////////////////////////////////////////////////////////
    return static_cast<UInt>(nGen);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
__NT_INSTANTIATE_CLASS_COMMON_DIMENSIONS_AND_TYPES(ParticleGenerator)
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
} // end namespace NTCodeBase
