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
#include <LibSimulation/ParticleSolvers/ParticleSolverBase.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace NTCodeBase {
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
        m_GlobalParameters.parseParameters(jSceneParams["GlobalParameters"]);
        if(globalParams().bSaveFrameData || globalParams().bSaveMemoryState || globalParams().bPrintLog2File) {
            FileHelpers::createFolder(globalParams().dataPath);
            FileHelpers::copyFile(sceneFile, globalParams().dataPath + "/" + FileHelpers::getFileName(sceneFile));
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    // setup logger following global parameters
    {
        setupLogger();
        logger().newLine();
        logger().printLog("Load scene file: " + sceneFile);
        logger().newLine();
        m_GlobalParameters.printParameters(logger());
    }

    ////////////////////////////////////////////////////////////////////////////////
    // read simulation parameters
    __NT_REQUIRE(jSceneParams.find("SimulationParameters") != jSceneParams.end());
    {
        JParams jSimParams = jSceneParams["SimulationParameters"];
        initializeSimulationParameters(jSimParams);

        // todo: rewrite this
        //        m_SolverData.makeReady(m_ParameterManager);
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
void ParticleSolverBase<N, Real_t>::setupLogger() {
    m_Logger = Logger::createLogger(getSolverName(), globalParams().dataPath,
                                    globalParams().bPrintLog2Console,
                                    globalParams().bPrintLog2File,
                                    static_cast<spdlog::level::level_enum>(globalParams().consoleLogLevel),
                                    static_cast<spdlog::level::level_enum>(globalParams().fileLogLevel));
    logger().printTextBox({ getSolverDescription(), String("Build: ") + String(__DATE__) + String(" - ") + String(__TIME__) });
    ////////////////////////////////////////////////////////////////////////////////
    // create a fallback logger if no console logger
    if(!globalParams().bPrintLog2Console) {
        m_FallbackConsoleLogger = Logger::createLogger(getSolverName(), globalParams().dataPath, true, false,
                                                       spdlog::level::level_enum::trace, spdlog::level::level_enum::trace);
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void ParticleSolverBase<N, Real_t>::doSimulation() {
    tbb::task_scheduler_init volatile init(globalParams().nThreads);
    (void)init;
    ////////////////////////////////////////////////////////////////////////////////
    logger().printCenterAligned("Start Simulation", '=');
    ////////////////////////////////////////////////////////////////////////////////
    auto startFrame = (globalParams().startFrame <= 1) ? globalParams().finishedFrame + 1u :
                      MathHelpers::min(globalParams().startFrame, globalParams().finishedFrame + 1u);
    for(auto frame = startFrame; frame <= globalParams().finalFrame; ++frame) {
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
                      Formatters::toSciString(globalParams().frameDuration) +
                      String("(s) (~") + std::to_string(static_cast<int>(round(Real_t(1.0) / globalParams().frameDuration))) +
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
                                                   Formatters::toString(globalParams().finishedFrame - globalParams().startFrame + 1u) +
                                                   String(" (Save frame data: ") + (globalParams().bSaveFrameData ? String("Yes") : String("No")) +
                                                   String(" | Save state: ") + (globalParams().bSaveMemoryState ? String("Yes") : String("No")) +
                                                   (globalParams().bSaveMemoryState ?
                                                    String(" (") + std::to_string(globalParams().nFramesPerState) +
                                                    String(" frames/state)") : String("")) + String(")"));
                                  logger->printLog(String("Data path: ") + globalParams().dataPath);
                                  for(auto& str : strFolderSizeInfo) {
                                      logger->printLog(str);
                                  }
                                  logger->newLine();
                                  logger->printTotalRunTime();
                              };
    ////////////////////////////////////////////////////////////////////////////////
    const auto strFolderSizeInfo = FileHelpers::getFolderSizeInfo(globalParams().dataPath);
    printFinalizingLog(m_Logger, strFolderSizeInfo);
    if(!globalParams().bPrintLog2Console) {
        printFinalizingLog(m_FallbackConsoleLogger, strFolderSizeInfo);
    }
    Logger::flushAll(-1);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool ParticleSolverBase<N, Real_t>::updateSimulationObjects(Real_t timestep) {
    bool bSceneChanged = false;
    if(m_SimulationObjects.size() > 0) {
        for(auto& obj : m_SimulationObjects) {
            bSceneChanged |= obj->updateObject(globalParams().finishedFrame + 1u, /* current frame is 1-based */
                                               globalParams().frameLocalTime / globalParams().frameDuration,
                                               timestep);
        }
    }
    return bSceneChanged;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
__NT_INSTANTIATE_CLASS_COMMON_DIMENSIONS_AND_TYPES(ParticleSolverBase)
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
} // end namespace NTCodeBase
