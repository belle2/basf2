/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>

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
    double flightTimeOfDaughterErr(const Particle* part, const std::vector<double>& daughters);
    double flightDistanceOfDaughterErr(const Particle* part, const std::vector<double>& daughters);
    double flightTimeOfDaughter(const Particle* part, const std::vector<double>& daughters);
    double flightDistanceOfDaughter(const Particle* part, const std::vector<double>& daughters);
    double vertexDistanceOfDaughter(const Particle* part, const std::vector<double>& arguments);
    double vertexDistanceOfDaughterErr(const Particle* part, const std::vector<double>& arguments);
    double vertexDistanceOfDaughterSignificance(const Particle* part, const std::vector<double>& arguments);
    double mcFlightDistanceOfDaughter(const Particle* part, const std::vector<double>& daughters);
    double mcFlightTimeOfDaughter(const Particle* part, const std::vector<double>& daughters);
  }
} //namespace Belle2
