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

#include <LibCommon/Geometry/GeometryObjectFactory.h>
#include <LibCommon/Logger/Logger.h>
#include <LibCommon/Utils/JSONHelpers.h>
#include <LibCommon/Utils/NumberHelpers.h>
#include <LibCommon/Utils/FileHelpers.h>

#include <LibParticle/ParticleHelpers.h>
#include <LibSimulation/SimulationObjects/SimulationObject.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace SimulationObjects {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
SimulationObject<N, Real_t>::SimulationObject(const JParams& jParams_, const SharedPtr<Logger>& logger_,
                                              ParameterManager& parameterManager_, PropertyManager& propertyManager_) :
    m_Logger(logger_), m_ParameterManager(parameterManager_), m_PropertyManager(propertyManager_) {
    m_ObjID = NumberHelpers::iRand<Int>::rnd();
    while(s_GeneratedObjIDs.find(m_ObjID) != s_GeneratedObjIDs.end()) {
        m_ObjID = NumberHelpers::iRand<Int>::rnd();
    }
    m_ObjName = String("Object_") + std::to_string(m_ObjID);
    ////////////////////////////////////////////////////////////////////////////////
    String geometryType;
    __NT_REQUIRE(JSONHelpers::readValue(jParams_, geometryType, "GeometryType"));
    m_GeometryObj = GeometryObjectFactory::createGeometry<N, Real_t>(geometryType, jParams_);
    __NT_REQUIRE(m_GeometryObj != nullptr);
    parseSimulationObjParameters(jParams_);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void SimulationObject<N, Real_t>::parseSimulationObjParameters(const JParams& jParams) {
    if(jParams.is_null()) {
        return;
    }
    JSONHelpers::readValue(jParams, m_ObjName, "Name");
    JSONHelpers::readBool(jParams, m_bNegativeInside, "NegativeInside");
    ////////////////////////////////////////////////////////////////////////////////
    // file cache parameters
    JSONHelpers::readBool(jParams, m_bUseFileCache,   "UseFileCache");
    if(m_bUseFileCache) {
        JSONHelpers::readValue(jParams, m_ParticleFile, "ParticleFile");
        String pFileType = "BNN";
        if(JSONHelpers::readValue(jParams, pFileType, "ParticleFileType")) {
            if(pFileType == "OBJ" || pFileType == "obj") {
                m_ParticleFileType = ParticleFileType::OBJ;
            } else if(pFileType == "BGEO" || pFileType == "bgeo") {
                m_ParticleFileType = ParticleFileType::BGEO;
            } else if(pFileType == "BNN" || pFileType == "bnn") {
                m_ParticleFileType = ParticleFileType::BNN;
            } else if(pFileType == "BINARY" || pFileType == "binary") {
                m_ParticleFileType = ParticleFileType::BINARY;
            } else {
                __NT_DIE_UNKNOWN_ERROR
            }
        }
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void SimulationObject<N, Real_t>::printParameters(Logger& logger) {
    logger.printLogIndent(String("Geometry: ") + m_GeometryObj->name());
    logger.printLogIndent(String("Negative inside: ") + (m_bNegativeInside ? String("Yes") : String("No")));
    if(!m_bUseFileCache) {
        logger.printLogIndent(String("Use file cache: No"));
    } else {
        logger.printLogIndent(String("Use file cache: Yes"));
        logger.printLogIndent(String("Particle file: ") + m_ParticleFile, 2);
        switch(m_ParticleFileType) {
            case ParticleFileType::OBJ:
                logger.printLogIndent(String("Particle file format: OBJ"),          2);
                break;
            case ParticleFileType::BGEO:
                logger.printLogIndent(String("Particle file format: BGEO"),         2);
                break;
            case ParticleFileType::BNN:
                logger.printLogIndent(String("Particle file format: BananaFormat"), 2);
                break;
            case ParticleFileType::BINARY:
                logger.printLogIndent(String("Particle file format: Binary"),       2);
                break;
            default:;
        }
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool SimulationObject<N, Real_t>::updateObject(ParticleSolvers::SolverData<N, Real_t>& solverData, UInt frame, Real_t frameFraction, Real_t frameDuration) {
    __NT_UNUSED(solverData);
    __NT_UNUSED(frameDuration);
    return m_GeometryObj->updateTransformation(frame, frameFraction);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool SimulationObject<N, Real_t>::loadParticlesFromFile(StdVT_VecN& positions, Real_t particleRadius) {
    if(m_bUseFileCache && !m_ParticleFile.empty() && FileHelpers::fileExisted(m_ParticleFile)) {
        switch(m_ParticleFileType) {
            case ParticleFileType::OBJ:
                return ParticleHelpers::loadParticlesFromObj(m_ParticleFile, positions);
            case ParticleFileType::BGEO:
                return ParticleHelpers::loadParticlesFromBGEO(m_ParticleFile, positions, particleRadius);
            case ParticleFileType::BNN:
                return ParticleHelpers::loadParticlesFromBNN(m_ParticleFile, positions, particleRadius);
            case ParticleFileType::BINARY:
                return ParticleHelpers::loadParticlesFromBinary(m_ParticleFile, positions, particleRadius);
            default:;
                return false;
        }
    }
    return false;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void SimulationObject<N, Real_t>::saveParticlesToFile(const StdVT_VecN& positions, Real_t particleRadius) {
    if(m_bUseFileCache && !m_ParticleFile.empty()) {
        switch(m_ParticleFileType) {
            case ParticleFileType::OBJ:
                ParticleHelpers::saveParticlesToObj(m_ParticleFile, positions);
                break;
            case ParticleFileType::BGEO:
                ParticleHelpers::saveParticlesToBGEO(m_ParticleFile, positions, particleRadius);
                break;
            case ParticleFileType::BNN:
                ParticleHelpers::saveParticlesToBNN(m_ParticleFile, positions, particleRadius);
                break;
            case ParticleFileType::BINARY:
                ParticleHelpers::saveParticlesToBinary(m_ParticleFile, positions, particleRadius);
                break;
            default:;
        }
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
__NT_INSTANTIATE_CLASS_COMMON_DIMENSIONS_AND_TYPES(SimulationObject)
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
} // end namespace SimulationObjects
