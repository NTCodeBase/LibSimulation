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
namespace NTCodeBase {
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<Int N, class Real_t>
SimulationObject<N, Real_t>::SimulationObject(const String& desc_, const JParams& jParams_, const SharedPtr<Logger>& logger_, Real_t particleRadius_) :
    m_Description(desc_), m_Logger(logger_), m_ParticleRadius(particleRadius_) {
    ////////////////////////////////////////////////////////////////////////////////
    // internal geometry object
    String geometryType;
    __NT_REQUIRE(JSONHelpers::readValue(jParams_, geometryType, "GeometryType"));
    m_GeometryObj = GeometryObjectFactory<N, Real_t>::createGeometry(geometryType, jParams_);
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
    do {
        m_ObjID = NumberHelpers::iRand<UInt>::rnd();
    } while(s_GeneratedObjIDs.find(m_ObjID) != s_GeneratedObjIDs.end());
    s_GeneratedObjIDs.insert(m_ObjID);
    ////////////////////////////////////////////////////////////////////////////////
    if(!JSONHelpers::readValue(jParams, m_ObjName, "Name")) {
        m_ObjName = String("Object_") + std::to_string(m_ObjID);
    }
    logger().printLog(m_Description + String(": ") + m_ObjName);
    logger().printLogIndent(String("Geometry: ") + m_GeometryObj->geometryName());
    ////////////////////////////////////////////////////////////////////////////////
    // internal particle generation
    if(jParams.find("ParticleGeneration") != jParams.end()) {
        auto jGen = jParams["ParticleGeneration"];
        __NT_REQUIRE(JSONHelpers::readBool(jGen, m_GenParticleParams.bEnabled, "Enable"));
        JSONHelpers::readValue(jGen, m_GenParticleParams.jitterRatio, "JitterRatio");
        JSONHelpers::readVector(jGen, m_GenParticleParams.samplingRatio, "SamplingRatio");
        JSONHelpers::readValue(jGen, m_GenParticleParams.thicknessRatio, "ThicknessRatio");
        JSONHelpers::readVector(jGen, m_GenParticleParams.shiftCenter, "ShiftCenter");

        logger().printLogIndent(String("Generate particle inside: ") + Formatters::toString(m_GenParticleParams.bEnabled));
        logger().printLogIndent(String("Jitter ratio (if applicable): ") + std::to_string(m_GenParticleParams.jitterRatio), 2);
        logger().printLogIndent(String("Sampling ratio (if applicable): ") + Formatters::toString(m_GenParticleParams.samplingRatio), 2);
        logger().printLogIndent(String("Thickenss ratio (if applicable): ") + Formatters::toString(m_GenParticleParams.thicknessRatio), 2);
        logger().printLogIndent(String("Shift center (if applicable): ") + Formatters::toString(m_GenParticleParams.shiftCenter), 2);
    }
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
typename SimulationObject<N, Real_t>::StdVT_VecN
SimulationObject<N, Real_t>::generateParticleInside() {
    StdVT_VecN positions;
    if(this->loadParticlesFromFile(positions)) {
        return positions;
    }
    auto thicknessThreshold = m_GenParticleParams.thicknessRatio * m_ParticleRadius;
    auto spacing = m_ParticleRadius * Real_t(2) * m_GenParticleParams.samplingRatio;
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
                                if(auto geoPhi = this->signedDistance(ppos);
                                   (geoPhi < -m_ParticleRadius) && (geoPhi > -thicknessThreshold)) {
                                    lock.lock();
                                    positions.push_back(ppos);
                                    lock.unlock();
                                }
                            });
    positions.shrink_to_fit();
    ////////////////////////////////////////////////////////////////////////////////
    // jitter positions
    if(const auto jitter = m_GenParticleParams.jitterRatio * m_ParticleRadius; jitter > TinyReal()) {
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
template<Int N, class Real_t>
bool SimulationObject<N, Real_t>::loadParticlesFromFile(StdVT_VecN& positions) {
    if(m_bUseFileCache && !m_ParticleFile.empty() && FileHelpers::fileExisted(m_ParticleFile)) {
        switch(m_FileFormat) {
            case FileFormat::OBJ:
                return ParticleHelpers::loadParticlesFromObj(m_ParticleFile, positions);
            case FileFormat::BGEO:
                return ParticleHelpers::loadParticlesFromBGEO(m_ParticleFile, positions, m_ParticleRadius);
            case FileFormat::BNN:
                return ParticleHelpers::loadParticlesFromBNN(m_ParticleFile, positions, m_ParticleRadius);
            case FileFormat::BINARY:
                return ParticleHelpers::loadParticlesFromBinary(m_ParticleFile, positions, m_ParticleRadius);
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
        switch(m_FileFormat) {
            case FileFormat::OBJ:
                ParticleHelpers::saveParticlesToObj(m_ParticleFile, positions);
                break;
            case FileFormat::BGEO:
                ParticleHelpers::saveParticlesToBGEO(m_ParticleFile, positions, m_ParticleRadius);
                break;
            case FileFormat::BNN:
                ParticleHelpers::saveParticlesToBNN(m_ParticleFile, positions, m_ParticleRadius);
                break;
            case FileFormat::BINARY:
                ParticleHelpers::saveParticlesToBinary(m_ParticleFile, positions, m_ParticleRadius);
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
} // end namespace NTCodeBase
