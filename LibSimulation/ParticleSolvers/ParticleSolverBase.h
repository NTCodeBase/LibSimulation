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
#include <LibSimulation/Data/Parameter.h>
#include <LibSimulation/Data/Property.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace ParticleSolvers {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
class ParticleSolverBase {
    ////////////////////////////////////////////////////////////////////////////////
    __NT_TYPE_ALIAS __NT_DECLARE_PARTICLE_SOLVER_ACCESSORS
    ////////////////////////////////////////////////////////////////////////////////
public:
    using SolverRealType = Real_t;
    ParticleSolverBase();
    virtual ~ParticleSolverBase();
    static constexpr Int dimension() noexcept { return N; }
    ////////////////////////////////////////////////////////////////////////////////
    virtual JParams loadScene(const String& sceneFile);
    ////////////////////////////////////////////////////////////////////////////////
    void doSimulation();
    void advanceFrame(UInt frame);
    void finalizeSimulation();

protected:
    virtual String getSolverName()                  = 0;
    virtual String getSolverDescription()           = 0;
    virtual void   initializeSimulationProperties() = 0;
    virtual void   initializeSimulationParameters(const JParams& jParams);
    virtual void   initializeIntegrationObjects(const JParams& jParams) = 0;
    virtual void   createParticleGenerators(const JParams& jParams)     = 0;
    virtual void   createRigidBodyObjects(const JParams& jParams);
    virtual bool   updateSimulationObjects();
    virtual void   generateParticles();
    virtual void   advanceFrame() = 0;
    ////////////////////////////////////////////////////////////////////////////////
    void setupLogger();
    void initializeGlobalParameters(const JParams& jParams);
    void printGlobalParameters();
    ////////////////////////////////////////////////////////////////////////////////
    ParameterManager m_ParameterManager;
    PropertyManager  m_PropertyManager;
    ////////////////////////////////////////////////////////////////////////////////
    SharedPtr<Logger> m_Logger = nullptr;
    SharedPtr<Logger> m_FallbackConsoleLogger = nullptr;
    ////////////////////////////////////////////////////////////////////////////////
    StdVT<SharedPtr<SimulationObjects::RigidBody<N, Real_t>>>         m_RigidBodies;
    StdVT<SharedPtr<SimulationObjects::ParticleGenerator<N, Real_t>>> m_ParticleGenerators;
    StdVT<SharedPtr<SimulationObjects::SimulationObject<N, Real_t>>>  m_SimulationObjects;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
} // end namespace ParticleSolvers
