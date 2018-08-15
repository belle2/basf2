/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2018 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Luigi Li Gioi, Sviatoslav Bilokin                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once
// Own include
#include <analysis/VariableManager/Manager.h>


// dataobjects
#include <analysis/dataobjects/Particle.h>

namespace Belle2 {
  class Particle;

  namespace Variable {
    // Generated vertex information
    double particleMCDistance(const Particle* part);

    double particleMCRho(const Particle* part);

    double particleMCProductionX(const Particle* part);
    double particleMCProductionY(const Particle* part);
    double particleMCProductionZ(const Particle* part);
    // Production vertex position
    double particleProductionX(const Particle* part);
    double particleProductionY(const Particle* part);
    double particleProductionZ(const Particle* part);
    // Production vertex covariance matrix
    double particleProductionCovXX(const Particle* part);
    double particleProductionCovXY(const Particle* part);
    double particleProductionCovXZ(const Particle* part);
    double particleProductionCovYX(const Particle* part);
    double particleProductionCovYY(const Particle* part);
    double particleProductionCovYZ(const Particle* part);
    double particleProductionCovZX(const Particle* part);
    double particleProductionCovZY(const Particle* part);
    double particleProductionCovZZ(const Particle* part);
  }
} //namespace Belle2
