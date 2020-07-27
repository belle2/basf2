
/*******************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                          *
 * Copyright(C) 2018 - Belle II Collaboration                                  *
 *                                                                             *
 * Author: The Belle II Collaboration                                          *
 * Contributors: Giulia Casarosa, Sviatoslav Bilokin                           *
 *                                                                             *
 * This software is provided "as is" without any warranty.                     *
 ******************************************************************************/

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
