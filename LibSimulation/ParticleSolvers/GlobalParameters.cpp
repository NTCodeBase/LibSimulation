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

#include <LibCommon/Utils/JSONHelpers.h>
#include <LibCommon/Logger/Logger.h>
#include <LibSimulation/ParticleSolvers/GlobalParameters.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace NTCodeBase {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<class Real_t>
void GlobalParameters<Real_t>::parseParameters(const JParams& jParams) {
    JSONHelpers::readBool(jParams, bAutoStart, "AutoStart");
    JSONHelpers::readValue(jParams, nThreads, "NThreads");

    ////////////////////////////////////////////////////////////////////////////////
    // frame and time parameters
    JSONHelpers::readValue(jParams, frameDuration,  "FrameDuration");
    JSONHelpers::readValue(jParams, startFrame,     "StartFrame");
    JSONHelpers::readValue(jParams, finalFrame,     "FinalFrame");
    JSONHelpers::readValue(jParams, nPhaseInFrames, "NPhaseInFrames");
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // data IO parameters
    JSONHelpers::readValue(jParams, dataPath, "DataPath");
    if(String format; JSONHelpers::readValue(jParams, format, "OutputFormat")) {
        NT_REQUIRE(format == "OBJ" || format == "BGEO" || format == "BGEO_GZ" || format == "BNN" || format == "Binary");
        if(format == "OBJ") {
            outputFormat = FileFormat::OBJ;
        } else if(format == "BGEO") {
            outputFormat = FileFormat::BGEO;
        } else if(format == "BGEO_GZ") {
            outputFormat = FileFormat::BGEO_GZ;
        } else if(format == "BNN") {
            outputFormat = FileFormat::BNN;
        } else {
            outputFormat = FileFormat::BINARY;
        }
    }
    JSONHelpers::readBool(jParams, bLoadMemoryState,   "LoadMemoryState");
    JSONHelpers::readBool(jParams, bSaveMemoryState,   "SaveMemoryState");
    JSONHelpers::readBool(jParams, bSaveFrameData,     "SaveFrameData");
    JSONHelpers::readBool(jParams, bClearOldFrameData, "ClearOldFrameData");
    JSONHelpers::readBool(jParams, bClearAllOldData,   "ClearAllOldData");
    JSONHelpers::readValue(jParams, nFramesPerState, "FramePerState");
    JSONHelpers::readVector(jParams, saveDataList, "OptionalSavingData");
    ////////////////////////////////////////////////////////////////////////////////

    JSONHelpers::readBool(jParams, bPrintLog2Console, "PrintLogToConsole");
    JSONHelpers::readBool(jParams, bPrintLog2File,    "PrintLogToFile");
    JSONHelpers::readValue(jParams, consoleLogLevel, "ConsoleLogLevel");
    JSONHelpers::readValue(jParams, fileLogLevel,    "FileLogLevel");
    ////////////////////////////////////////////////////////////////////////////////
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<class Real_t>
void GlobalParameters<Real_t>::printParameters(Logger& logger) {
    logger.printLog(String("Global parameters:"));
    logger.printLogIndent(String("Number of working threads: ") + (nThreads > 0 ? std::to_string(nThreads) : String("Automatic")));

    ////////////////////////////////////////////////////////////////////////////////
    // frame and time parameters
    logger.printLogIndent(String("Frame duration: ") + Formatters::toSciString(frameDuration) +
                          String(" (~") + std::to_string(static_cast<int>(round(1.0_f / frameDuration))) + String(" fps)"));
    logger.printLogIndent(String("Start frame: ") + std::to_string(startFrame));
    logger.printLogIndent(String("Phase in frames: ") + std::to_string(nPhaseInFrames));
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // data IO parameters
    if(bSaveMemoryState || bSaveFrameData || bPrintLog2File) {
        logger.printLogIndent("Data path: " + dataPath);
        if(outputFormat == FileFormat::OBJ) {
            logger.printLogIndent("Output format: OBJ");
        } else if(outputFormat == FileFormat::BGEO) {
            logger.printLogIndent("Output format: Bgeo");
        } else if(outputFormat == FileFormat::BNN) {
            logger.printLogIndent("Output format: BNN");
        } else {
            logger.printLogIndent("Output format: Binary");
        }
    }
    logger.printLogIndent(String("Load saved memory state: ") + Formatters::toString(bLoadMemoryState));
    logger.printLogIndent(String("Save memory state: ") + Formatters::toString(bSaveMemoryState));
    logger.printLogIndentIf(bSaveMemoryState, String("Frames/state: ") + std::to_string(nFramesPerState), 2);
    logger.printLogIndent(String("Save simulation data each frame: ") + Formatters::toString(bSaveFrameData));
    if(bSaveFrameData && saveDataList.size() > 0) {
        String str; for(const auto& s : saveDataList) {
            str += s; str += String(", ");
        }
        str.erase(str.find_last_of(","), str.size()); // remove last ',' character
        logger.printLogIndent(String("Save data: ") + str, 2);
    }
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // logging parameters
    logger.printLogIndent(String("Log to file: ") + Formatters::toString(bPrintLog2File));
    logger.printLogIndent(String("Log to console: ") + Formatters::toString(bPrintLog2Console));
    ////////////////////////////////////////////////////////////////////////////////

    logger.newLine();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<class Real_t>
Real_t GlobalParameters<Real_t>::systemTime() const {
    return frameDuration * static_cast<Real_t>(finishedFrame) + frameLocalTime;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<class Real_t>
bool GlobalParameters<Real_t>::saveData(const String& dataName) const {
    return (std::find(saveDataList.begin(), saveDataList.end(), dataName) != saveDataList.end());
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
NT_INSTANTIATE_STRUCT_COMMON_TYPES(GlobalParameters)
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
} // end namespace NTCodeBase
