
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
// Own include
#include <analysis/VariableManager/Manager.h>


// dataobjects
#include <analysis/dataobjects/Particle.h>

namespace Belle2 {
  class Particle;

  namespace Variable {
    double flightTime(const Particle* part);
    double flightDistance(const Particle* part);
    double flightTimeErr(const Particle* part);
    double flightDistanceErr(const Particle* part);
    double vertexDistance(const Particle* part);
    double vertexDistanceErr(const Particle* part);
    double vertexDistanceSignificance(const Particle* part);
    double mcFlightDistance(const Particle* part);
    double mcFlightTime(const Particle* part);
    Manager::FunctionPtr flightTimeOfDaughterErr(const std::vector<std::string>& arguments);
    Manager::FunctionPtr flightDistanceOfDaughterErr(const std::vector<std::string>& arguments);
    Manager::FunctionPtr flightTimeOfDaughter(const std::vector<std::string>& arguments);
    Manager::FunctionPtr flightDistanceOfDaughter(const std::vector<std::string>& arguments);
    Manager::FunctionPtr vertexDistanceOfDaughter(const std::vector<std::string>& arguments);
    Manager::FunctionPtr vertexDistanceOfDaughterErr(const std::vector<std::string>& arguments);
    Manager::FunctionPtr vertexDistanceOfDaughterSignificance(const std::vector<std::string>& arguments);
    Manager::FunctionPtr mcFlightDistanceOfDaughter(const std::vector<std::string>& arguments);
    Manager::FunctionPtr mcFlightTimeOfDaughter(const std::vector<std::string>& arguments);
  }
} //namespace Belle2
