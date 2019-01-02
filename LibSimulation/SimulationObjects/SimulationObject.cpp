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
#include <LibCommon/Utils/FileHelpers.h>
#include <LibCommon/Utils/JSONHelpers.h>
#include <LibCommon/Utils/NumberHelpers.h>

#include <LibParticle/ParticleHelpers.h>
#include <LibSimulation/SimulationObjects/SimulationObject.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace SimulationObjects {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
SimulationObject<N, Real_t>::SimulationObject(const String& desc_, const JParams& jParams_, const SharedPtr<Logger>& logger_,
                                              ParameterManager& parameterManager_, PropertyManager& propertyManager_) :
    m_Description(desc_), m_Logger(logger_), m_ParameterManager(parameterManager_), m_PropertyManager(propertyManager_) {
    ////////////////////////////////////////////////////////////////////////////////
    // internal geometry object
    String geometryType;
    __NT_REQUIRE(JSONHelpers::readValue(jParams_, geometryType, "GeometryType"));
    m_GeometryObj = GeometryObjectFactory::createGeometry<N, Real_t>(geometryType, jParams_);
    __NT_REQUIRE(m_GeometryObj != nullptr);
    ////////////////////////////////////////////////////////////////////////////////
    // other parameters
    initializeParameters(jParams_);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void SimulationObject<N, Real_t>::initializeParameters(const JParams& jParams) {
    if(jParams.is_null()) {
        return;
    }
    ////////////////////////////////////////////////////////////////////////////////
    // object ID and default name
    m_ObjID = NumberHelpers::iRand<UInt>::rnd();
    while(s_GeneratedObjIDs.find(m_ObjID) != s_GeneratedObjIDs.end()) {
        m_ObjID = NumberHelpers::iRand<UInt>::rnd();
    }
    if(!JSONHelpers::readValue(jParams, m_ObjName, "Name")) {
        m_ObjName = String("Object_") + std::to_string(m_ObjID);
    }
    logger().printLog(m_Description + String(": ") + m_ObjName);
    logger().printLogIndent(String("Geometry: ") + m_GeometryObj->name());
    ////////////////////////////////////////////////////////////////////////////////
    // file cache parameters
    JSONHelpers::readBool(jParams, m_bUseFileCache, "UseFileCache");
    if(m_bUseFileCache) {
        JSONHelpers::readValue(jParams, m_ParticleFile, "ParticleFile");
        String pFileType = "BNN";
        if(JSONHelpers::readValue(jParams, pFileType, "FileFormat")) {
            if(pFileType == "OBJ" || pFileType == "obj") {
                m_FileFormat = FileFormat::OBJ;
            } else if(pFileType == "BGEO" || pFileType == "bgeo") {
                m_FileFormat = FileFormat::BGEO;
            } else if(pFileType == "BNN" || pFileType == "bnn") {
                m_FileFormat = FileFormat::BNN;
            } else if(pFileType == "BINARY" || pFileType == "binary") {
                m_FileFormat = FileFormat::BINARY;
            } else {
                __NT_DIE("Unknow file format");
            }
        }
        logger().printLogIndent(String("Use file cache: Yes"));
        logger().printLogIndent(String("Particle file: ") + m_ParticleFile, 2);
        switch(m_FileFormat) {
            case FileFormat::OBJ:
                logger().printLogIndent(String("Particle file format: OBJ"),          2);
                break;
            case FileFormat::BGEO:
                logger().printLogIndent(String("Particle file format: BGEO"),         2);
                break;
            case FileFormat::BNN:
                logger().printLogIndent(String("Particle file format: BananaFormat"), 2);
                break;
            case FileFormat::BINARY:
                logger().printLogIndent(String("Particle file format: Binary"),       2);
                break;
            default:;
        }
    } else {
        logger().printLogIndent(String("Use file cache: No"));
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool SimulationObject<N, Real_t>::updateObject(UInt frame, Real_t frameFraction, Real_t timestep) {
    __NT_UNUSED(timestep);
    return m_GeometryObj->updateTransformation(frame, frameFraction);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool SimulationObject<N, Real_t>::isInside(const VecN& ppos) const {
    return m_GeometryObj->isInside(ppos, true);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
Real_t SimulationObject<N, Real_t>::signedDistance(const VecN& ppos) const {
    return m_GeometryObj->signedDistance(ppos, true);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
VecX<N, Real_t> SimulationObject<N, Real_t>::gradSignedDistance(const VecN& ppos, Real_t dxyz /*= Real_t(1e-4)*/) const {
    return m_GeometryObj->gradSignedDistance(ppos, true, dxyz);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
bool SimulationObject<N, Real_t>::loadParticlesFromFile(StdVT_VecN& positions) {
    if(m_bUseFileCache && !m_ParticleFile.empty() && FileHelpers::fileExisted(m_ParticleFile)) {
        auto particleRadius = simParams("ParticleRadius").get<Real_t>();
        switch(m_FileFormat) {
            case FileFormat::OBJ:
                return ParticleHelpers::loadParticlesFromObj(m_ParticleFile, positions);
            case FileFormat::BGEO:
                return ParticleHelpers::loadParticlesFromBGEO(m_ParticleFile, positions, particleRadius);
            case FileFormat::BNN:
                return ParticleHelpers::loadParticlesFromBNN(m_ParticleFile, positions, particleRadius);
            case FileFormat::BINARY:
                return ParticleHelpers::loadParticlesFromBinary(m_ParticleFile, positions, particleRadius);
            default:;
                return false;
        }
    }
    return false;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
void SimulationObject<N, Real_t>::saveParticlesToFile(const StdVT_VecN& positions) {
    if(m_bUseFileCache && !m_ParticleFile.empty()) {
        auto particleRadius = simParams("ParticleRadius").get<Real_t>();
        switch(m_FileFormat) {
            case FileFormat::OBJ:
                ParticleHelpers::saveParticlesToObj(m_ParticleFile, positions);
                break;
            case FileFormat::BGEO:
                ParticleHelpers::saveParticlesToBGEO(m_ParticleFile, positions, particleRadius);
                break;
            case FileFormat::BNN:
                ParticleHelpers::saveParticlesToBNN(m_ParticleFile, positions, particleRadius);
                break;
            case FileFormat::BINARY:
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
