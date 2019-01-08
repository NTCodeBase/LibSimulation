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
#include <LibSimulation/Forward.h>
#include <LibSimulation/Macros.h>
#include <LibSimulation/ParticleSolvers/GlobalParameters.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace NTCodeBase {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
class ParticleSolverBase {
    ////////////////////////////////////////////////////////////////////////////////
    __NT_TYPE_ALIAS
    ////////////////////////////////////////////////////////////////////////////////
public:
    using RealType = Real_t; // for using in solver factory
    static constexpr Int dimension() { return N; }
    static constexpr bool isFloat() { return std::is_same_v<Real_t, float>; }
    __NT_DECLARE_PARTICLE_SOLVER_ACCESSORS
    ////////////////////////////////////////////////////////////////////////////////
    ParticleSolverBase();
    virtual ~ParticleSolverBase();
    ////////////////////////////////////////////////////////////////////////////////
    virtual JParams loadScene(const String& sceneFile);
    ////////////////////////////////////////////////////////////////////////////////
    void doSimulation();
    void advanceFrame(UInt frame);
    void finalizeSimulation();

protected:
    virtual String getSolverName()        = 0;
    virtual String getSolverDescription() = 0;
    virtual bool   updateSimulationObjects(Real_t timestep);
    virtual void   advanceFrame() = 0;
    ////////////////////////////////////////////////////////////////////////////////
    void setupLogger();
    ////////////////////////////////////////////////////////////////////////////////
    SharedPtr<Logger> m_Logger = nullptr;
    SharedPtr<Logger> m_FallbackConsoleLogger = nullptr;
    ////////////////////////////////////////////////////////////////////////////////
    GlobalParameters<Real_t> m_GlobalParams;
    ////////////////////////////////////////////////////////////////////////////////
    StdVT<SharedPtr<RigidBody<N, Real_t>>>         m_RigidBodies;
    StdVT<SharedPtr<ParticleGenerator<N, Real_t>>> m_ParticleGenerators;
    StdVT<SharedPtr<SimulationObject<N, Real_t>>>  m_SimulationObjects;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
} // end namespace NTCodeBase
