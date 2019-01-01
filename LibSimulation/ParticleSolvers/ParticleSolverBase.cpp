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
#include <LibCommon/Utils/JSONHelpers.h>

#include <LibParticle/ParticleSerialization.h>
#include <LibSimulation/SimulationObjects/RigidBody.h>
#include <LibSimulation/SimulationObjects/ParticleGenerator.h>
#include <Simulation/HairGenerators/StrandGenerator.h>
#include <Simulation/HairGenerators/HairOnBall.h>
#include <Simulation/HairGenerators/HairOnMesh.h>

#include <Simulation/Solvers/SolverData/DataManager.h>
#include <Simulation/Solvers/ParticleSolverBase.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace ParticleSolvers {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
ParticleSolverBase<N, Real_t>::ParticleSolverBase() {
    static_assert(std::is_floating_point<Real_t>::value);
}

template<Int N, class Real_t>
ParticleSolverBase<N, Real_t>::~ParticleSolverBase() {
    Logger::removeLogger(m_Logger);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
JParams ParticleSolverBase<N, Real_t>::loadScene(const String& sceneFile) {
    std::ifstream inputFile(sceneFile);
    if(!inputFile.is_open()) {
        std::cerr << "Cannot open scene file: " << sceneFile << std::endl;
        return JParams();
    }
    auto jSceneParams = JParams::parse(inputFile);

    ////////////////////////////////////////////////////////////////////////////////
    // read global parameters
    __NT_REQUIRE(jSceneParams.find("GlobalParameters") != jSceneParams.end());
    {
        initializeGlobalParameters(jSceneParams["GlobalParameters"]);
        if(globalParams("SaveFrameData").get<bool>()
           || globalParams("SaveMemoryState").get<bool>()
           || globalParams("LogToFile").get<bool>()) {
            FileHelpers::createFolder(globalParams("DataPath").get<String>());
            FileHelpers::copyFile(sceneFile, globalParams("DataPath").get<String>() + "/" + FileHelpers::getFileName(sceneFile));
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    // setup logger following global parameters
    {
        setupLogger();
        logger().newLine();
        logger().printLog("Load scene file: " + sceneFile);
        logger().newLine();
        printGlobalParameters();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // read simulation parameters
    __NT_REQUIRE(jSceneParams.find("SimulationParameters") != jSceneParams.end());
    {
        JParams jSimParams = jSceneParams["SimulationParameters"];
        initializeSimulationParameters(jSimParams);
        m_SolverData.makeReady(m_ParameterManager);
        ////////////////////////////////////////////////////////////////////////////////
        initializeIntegrationObjects(jSimParams);
        logger().separatorLine(1);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // and generate scene objects
    //    createRigidBodyObjects(jSceneParams);
    //    createParticleGenerators(jSceneParams);
    //    generateParticles();
    ////////////////////////////////////////////////////////////////////////////////
    return jSceneParams;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void ParticleSolverBase<N, Real_t>::initializeGlobalParameters(const JParams& jParams) {
    paramManager().addGroup("GlobalParameters", "Global parameters");

    ////////////////////////////////////////////////////////////////////////////////
    // misc parameters
    paramManager().addParameter<bool>("GlobalParameters", "AutoStart", "Start simulation immediately", false, jParams);
    paramManager().addParameter<int>("GlobalParameters", "NThreads", "Number of working threads", -1, jParams);
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    paramManager().addParameter<Real_t>("GlobalParameters", "SystemTime", "Evolution time of the entire system", Real_t(0));
    paramManager().addParameter<Real_t>("GlobalParameters", "FrameLocalTime", "Evolution time within the current frame", Real_t(0));
    paramManager().addParameter<UInt>("GlobalParameters", "FinishedFrame", "Number of finished frames", 0u);

    paramManager().addParameter<Real_t>("GlobalParameters", "FrameDuration", "Frame duration", Real_t(1.0 / 30.0), jParams);
    paramManager().addParameter<UInt>("GlobalParameters", "StartFrame", "Start frame", 1u, jParams);
    paramManager().addParameter<UInt>("GlobalParameters", "FinalFrame", "Final frame", 1u, jParams);
    paramManager().addParameter<UInt>("GlobalParameters", "NPhaseInFrames", "Number of phase-in frames", 0u, jParams);
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // logging parameters
    paramManager().addParameter<bool>("GlobalParameters", "LogToConsole", "Log to console", true, jParams);
    paramManager().addParameter<bool>("GlobalParameters", "LogToFile", "Log to file", true, jParams);
    paramManager().addParameter<int>("GlobalParameters", "ConsoleLogLevel", "Console log level", 0, jParams);
    paramManager().addParameter<int>("GlobalParameters", "FileLogLevel", "File log level", 0, jParams);
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // data IO parameters
    paramManager().addParameter<String>("GlobalParameters", "DataPath", "Output path", String("./Output"), jParams);
    paramManager().addParameter<bool>("GlobalParameters", "LoadMemoryState", "Load saved memory state", false, jParams);
    paramManager().addParameter<bool>("GlobalParameters", "SaveMemoryState", "Save memory state", false, jParams);
    paramManager().addParameter<bool>("GlobalParameters", "SaveFrameData", "Save simulation data each frame", false, jParams);
    paramManager().addParameter<bool>("GlobalParameters", "ClearOldFrameData", "Clear old data", false, jParams);
    paramManager().addParameter<bool>("GlobalParameters", "ClearAllOldData", "Clear all old data", false, jParams);
    paramManager().addParameter<UInt>("GlobalParameters", "NFramePerState", "Number of frames that are skipped for each memory state", 1u, jParams);
    ////////////////////////////////////////////////////////////////////////////////
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void ParticleSolverBase<N, Real_t>::initializeSimulationParameters(const JParams& jParams) {
    paramManager().addGroup("SimulationParameters", "Global parameters");
    logger().printLog(String("Simulation parameters:"));
    ////////////////////////////////////////////////////////////////////////////////
    // time step size
    const auto& prMinTimestep = paramManager().addParameter<Real_t>("SimulationParameters", "MinTimestep", "Minimum allowed time step", Real_t(1.0e-6), jParams);
    const auto& prMaxTimestep = paramManager().addParameter<Real_t>("SimulationParameters", "MaxTimestep", "Maximum allowed time step", Real_t(1.0 / 30.0), jParams);
    const auto& prCFL         = paramManager().addParameter<Real_t>("SimulationParameters", "CFLFactor", "CFLFactor", Real_t(1.0), jParams);
    logger().printLogIndent(prMinTimestep.description() + String(": ") + Formatters::toSciString(prMinTimestep.get<Real_t>()));
    logger().printLogIndent(prMaxTimestep.description() + String(": ") + Formatters::toSciString(prMaxTimestep.get<Real_t>()));
    logger().printLogIndent(prCFL.description() + String(": ") + std::to_string(prCFL.get<Real_t>()));
    logger().newLine();
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // simulation domain
    auto& prBMin = paramManager().addParameter<VecN>("SimulationParameters", "DomainBMin", "DomainBMin", VecN(-1));
    auto& prBMax = paramManager().addParameter<VecN>("SimulationParameters", "DomainBMax", "DomainBMin", VecN(1));
    __NT_REQUIRE(jParams.find("DomainBox") != jParams.end());
    {
        JParams jBoxParams = jParams["DomainBox"];
        jBoxParams["GeometryType"] = String("Box");

        auto obj = std::make_shared<SimulationObjects::RigidBody<N, Real_t>>(jBoxParams, m_ParameterManager, m_PropertyManager);
        obj->name() = String("DomainBox");
        obj->isNegativeInside() = false; // must be false for the simulation domain box
        m_RigidBodies.push_back(obj);

        auto box = std::dynamic_pointer_cast<GeometryObjects::BoxObject<N, Real_t>>(obj->geometry());

        prBMin.set<VecN>(box->getTransformedBoxMin());
        prBMax.set<VecN>(box->getTransformedBoxMax());
    }
    logger().printLogIndent(String("Domain box: ") + Formatters::toString(prBMin.get<VecN>()) + " -> " + Formatters::toString(prBMax.get<VecN>()));
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // particle parameters
    const auto& prParticleRadius = paramManager().addParameter<Real_t>("SimulationParameters", "ParticleRadius", "Particle radius", Real_t(0), jParams, true);
    paramManager().addParameter<Real_t>("SimulationParameters", "ParticleRadiusSqr", "Particle radius squared", MathHelpers::sqr(prParticleRadius.get<Real_t>()));
    logger().printLogIndent(prParticleRadius.description() + String(": ") + std::to_string(prParticleRadius.get<Real_t>()));
    logger().newLine();
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // gravity
    const auto& prGravity = paramManager().addParameter<VecN>("SimulationParameters", "Gravity", "Gravity", VecN(0), jParams);
    logger().printLogIndent(prGravity.description() + String(": ") + Formatters::toString(prGravity.get<VecN>()));
    logger().newLine();
    ////////////////////////////////////////////////////////////////////////////////
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void ParticleSolverBase<N, Real_t>::printGlobalParameters() {
    ////////////////////////////////////////////////////////////////////////////////
    logger().printLog(String("Global parameters:"));

    ////////////////////////////////////////////////////////////////////////////////
    // misc parameters
    const auto& prNThreads = globalParams("NThreads");
    logger().printLogIndent(prNThreads.description() + String(": ") + (prNThreads.get<Int>() > 0 ? std::to_string(prNThreads.get<Int>()) : String("Automatic")));
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    const auto& prFrameDuration = globalParams("FrameDuration");
    const auto& prStartFrame    = globalParams("StartFrame");
    const auto& prFinalFrame    = globalParams("FinalFrame");
    const auto& prNPhaseInFrame = globalParams("NPhaseInFrames");

    logger().printLogIndent(prFrameDuration.description() + String(": ") + Formatters::toSciString(prFrameDuration.get<Real_t>()) +
                            String(" (~") + std::to_string(static_cast<int>(std::round(1.0_f / prFrameDuration.get<Real_t>()))) + String(" fps)"));
    logger().printLogIndent(prStartFrame.description() + String(": ") + std::to_string(prStartFrame.get<UInt>()));
    logger().printLogIndent(prFinalFrame.description() + String(": ") + std::to_string(prFinalFrame.get<UInt>()));
    logger().printLogIndent(prNPhaseInFrame.description() + String(": ") + std::to_string(prNPhaseInFrame.get<UInt>()));
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // data IO parameters
    const auto& prLogToFile         = globalParams("LogToFile");
    const auto& prDataPath          = globalParams("DataPath");
    const auto& prLoadMemoryState   = globalParams("LoadMemoryState");
    const auto& prSaveMemoryState   = globalParams("SaveMemoryState");
    const auto& prSaveFrameData     = globalParams("SaveFrameData");
    const auto& prClearOldFrameData = globalParams("ClearOldFrameData");
    const auto& prClearAllOldData   = globalParams("ClearAllOldData");
    const auto& prFramePerState     = globalParams("NFramePerState");

    logger().printLogIndentIf(prSaveMemoryState.get<bool>() || prSaveFrameData.get<bool>() || prLogToFile.get<bool>(), ("Data path: ") + prDataPath.get<String>());
    logger().printLogIndent(prLoadMemoryState.description() + String(": ") + (prLoadMemoryState.get<bool>() ? String("Yes") : String("No")));
    logger().printLogIndent(prSaveMemoryState.description() + String(": ") + (prSaveMemoryState.get<bool>() ? String("Yes") : String("No")));
    logger().printLogIndentIf(prSaveMemoryState.get<bool>(), prFramePerState.description() + String(": ") + std::to_string(prFramePerState.get<UInt>()), 2);
    logger().printLogIndent(prSaveFrameData.description() + String(": ") + (prSaveFrameData.get<bool>() ? String("Yes") : String("No")));
    logger().printLogIndent(prClearOldFrameData.description() + String(": ") + (prClearOldFrameData.get<bool>() ? String("Yes") : String("No")));
    logger().printLogIndent(prClearAllOldData.description() + String(": ") + (prClearAllOldData.get<bool>() ? String("Yes") : String("No")));
    logger().newLine();
    ////////////////////////////////////////////////////////////////////////////////
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void ParticleSolverBase<N, Real_t>::setupLogger() {
    m_Logger = Logger::createLogger(getSolverName(), globalParams("DataPath").get<String>(),
                                    globalParams("LogToConsole").get<bool>(),
                                    globalParams("LogToFile").get<bool>(),
                                    static_cast<spdlog::level::level_enum>(globalParams("ConsoleLogLevel").get<Int>()),
                                    static_cast<spdlog::level::level_enum>(globalParams("FileLogLevel").get<Int>()));
    logger().printTextBox({ getSolverDescription(), String("Build: ") + String(__DATE__) + String(" - ") + String(__TIME__) });
    ////////////////////////////////////////////////////////////////////////////////
    // create a fallback logger if no console logger
    if(!globalParams("LogToConsole").get<bool>()) {
        m_FallbackConsoleLogger = Logger::createLogger(getSolverName(), globalParams("DataPath").get<String>(), true, false,
                                                       spdlog::level::level_enum::trace, spdlog::level::level_enum::trace);
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void ParticleSolverBase<N, Real_t>::doSimulation() {
    tbb::task_scheduler_init volatile init(globalParams("NThreads").get<Int>());
    (void)init;
    ////////////////////////////////////////////////////////////////////////////////
    logger().printCenterAligned("Start Simulation", '=');
    ////////////////////////////////////////////////////////////////////////////////
    auto startFrame = (globalParams("StartFrame").get<UInt>() <= 1) ?
                      globalParams("FinishedFrame").get<UInt>() + 1u :
                      MathHelpers::min(globalParams("StartFrame").get<UInt>(), globalParams("FinishedFrame").get<UInt>() + 1u);
    for(auto frame = startFrame; frame <= globalParams("FinalFrame").get<UInt>(); ++frame) {
        advanceFrame(frame);
    }
    finalizeSimulation();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void ParticleSolverBase<N, Real_t>::advanceFrame(UInt frame) {
    logger().newLine();
    logger().printCenterAligned(String("Frame ") + Formatters::toString(frame), '=');
    logger().newLine();
    ////////////////////////////////////////////////////////////////////////////////
    Timer timer;
    timer.tick();
    advanceFrame();
    logger().newLine();
    logger().printLog(String("Frame #") + std::to_string(frame) + String(" finished | Frame duration: ") +
                      Formatters::toSciString(globalParams("FrameDuration").get<Real_t>()) +
                      String("(s) (~") + std::to_string(static_cast<int>(round(Real_t(1.0) / globalParams("FrameDuration").get<Real_t>()))) +
                      String(" fps) | Total computation time: ") + timer.getRunTime());
    logger().printMemoryUsage();
    logger().newLine();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void ParticleSolverBase<N, Real_t>::finalizeSimulation() {
    auto printFinalizingLog = [&](const auto& logger, const auto& strFolderSizeInfo) {
                                  logger->newLine();
                                  logger->printCenterAligned(String("Simulation finished"), '+');
                                  logger->printLog(String("Total frames: ") +
                                                   Formatters::toString(globalParams("FinishedFrame").get<UInt>() - globalParams("StartFrame").get<UInt>() + 1u) +
                                                   String(" (Save frame data: ") + (globalParams("SaveFrameData").get<bool>() ? String("Yes") : String("No")) +
                                                   String(" | Save state: ") + (globalParams("SaveMemoryState").get<bool>() ? String("Yes") : String("No")) +
                                                   (globalParams("SaveMemoryState").get<bool>() ?
                                                    String(" (") + std::to_string(globalParams("NFramePerState").get<UInt>()) +
                                                    String(" frames/state)") : String("")) + String(")"));
                                  logger->printLog(String("Data path: ") + globalParams("DataPath").get<String>());
                                  for(auto& str : strFolderSizeInfo) {
                                      logger->printLog(str);
                                  }
                                  logger->newLine();
                                  logger->printTotalRunTime();
                              };
    ////////////////////////////////////////////////////////////////////////////////
    const auto strFolderSizeInfo = FileHelpers::getFolderSizeInfo(globalParams("DataPath").get<String>());
    printFinalizingLog(m_Logger, strFolderSizeInfo);
    if(!globalParams("LogToConsole").get<bool>()) {
        printFinalizingLog(m_FallbackConsoleLogger, strFolderSizeInfo);
    }
    Logger::flushAll(-1);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void ParticleSolverBase<N, Real_t>::createRigidBodyObjects(const JParams& jParams) {
    __NT_REQUIRE(m_RigidBodies.size() == 1);
    Timer timer;
    m_RigidBodies.front()->printParameters(logger());
    if constexpr(N == 2) { // for 2D only: generate the ghost particles for the entire domain box
        timer.tick();
        auto nGen = m_RigidBodies.front()->generateParticles(solverData(), m_RigidBodies);
        timer.tock();
        if(nGen > 0) {
            logger().printLog(String("Generated ") + Formatters::toString(nGen) + String(" particles by rigid body object: ") + m_RigidBodies.front()->name() +
                              String(" (") + timer.getRunTime() + String(")"));
        }
        m_RigidBodies.front()->printParameters(logger());
    }
    ////////////////////////////////////////////////////////////////////////////////
    if(jParams.find("RigidBodies") != jParams.end()) {
        for(auto& jObj : jParams["RigidBodies"]) {
            auto obj = std::make_shared<SimulationObjects::RigidBody<N, Real_t>>(jObj, m_ParameterManager, m_PropertyManager);
            timer.tick();
            auto nGen = obj->generateParticles(solverData(), m_RigidBodies);
            timer.tock();
            if(nGen > 0) {
                logger().printLog(String("Generated ") + Formatters::toString(nGen) + String(" particles by rigid body object: ") + obj->name() +
                                  String(" (") + timer.getRunTime() + String(")"));
            }
            obj->printParameters(logger());
            m_RigidBodies.emplace_back(std::move(obj));
        }
    }
    ////////////////////////////////////////////////////////////////////////////////
    for(const auto& obj : m_RigidBodies) {
        m_SimulationObjects.push_back(obj);
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool ParticleSolverBase<N, Real_t>::updateSimulationObjects() {
    bool bSceneChanged = false;
    if(m_SimulationObjects.size() > 0) {
        for(auto& obj : m_SimulationObjects) {
            bSceneChanged |= obj->updateObject(solverData(), globalParams("FinishedFrame").get<UInt>() + 1u, /* current frame is 1-based */
                                               globalParams("FrameLocalTime").get<Real_t>() / globalParams("FrameDuration").get<Real_t>(),
                                               globalParams("FrameDuration").get<Real_t>());
        }
    }
    return bSceneChanged;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void ParticleSolverBase<N, Real_t>::generateParticles() {
    Timer timer;
    for(auto& generator : m_ParticleGenerators) {
        timer.tick();
        auto nGen = generator->generateParticles(solverData(), m_RigidBodies);
        timer.tock();
        if(nGen == 0) {
            logger().printWarning(String("Particles generator '") + generator->name() + String("' could not generate any particles!"));
        } else {
            logger().printLog(String("Generated ") + Formatters::toString(nGen) + String(" particles by standard particle generator: ") + generator->name() +
                              String(" (") + timer.getRunTime() + String(")"));
        }
        generator->printParameters(logger());
    }
    //    for(auto& generator : m_StrandGenerators) {
    //        timer.tick();
    //        auto nGen = generator->generateParticles(solverParams(), solverData(), m_RigidBodies);
    //        timer.tock();
    //        if(nGen == 0) {
    //            logger().printWarning(String("Strand generator '") + generator->name() + String("' could not generate any particles!"));
    //        } else {
    //            logger().printLog(String("Generated ") + Formatters::toString(nGen) + String(" particles by strand generator: ") + generator->name() +
    //                              String(" (") + timer.getRunTime() + String(")"));
    //        }
    //        generator->printParameters(logger());
    //    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
__NT_INSTANTIATE_CLASS_COMMON_DIMENSIONS_AND_TYPES(ParticleSolverBase)
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
} // end namespace ParticleSolvers
