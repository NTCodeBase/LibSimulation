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

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#define __NT_DECLARE_PARTICLE_SOLVER_ACCESSORS                                                                                             \
    ParameterManager & paramManager() { return this->m_ParameterManager; }                                                                 \
    Parameter& globalParams(const char* paramName) { return paramManager().parameter("GlobalParameters", paramName); }                     \
    Parameter& simParams(const char* paramName) { return paramManager().parameter("SimulationParameters", paramName); }                    \
    PropertyManager& propertyManager() { return this->m_PropertyManager; }                                                                 \
    Logger& logger() { assert(this->m_Logger != nullptr); return *this->m_Logger; }                                                        \
                                                                                                                                           \
    const ParameterManager& paramManager() const { return this->m_ParameterManager; }                                                      \
    const Parameter&        globalParams(const char* paramName) const { return paramManager().parameter("GlobalParameters", paramName); }  \
    const Parameter&        simParams(const char* paramName) const { return paramManager().parameter("SimulationParameters", paramName); } \
    const PropertyManager&  propertyManager() const { return this->m_PropertyManager; }                                                    \
    const Logger&           logger() const { assert(this->m_Logger != nullptr); return *this->m_Logger; }
