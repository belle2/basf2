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
    double particleMCX(const Particle* part);
    double particleMCY(const Particle* part);
    double particleMCZ(const Particle* part);
    double particleMCDistance(const Particle* part);

    double particleMCRho(const Particle* part);

    double particleMCProductionX(const Particle* part);
    double particleMCProductionY(const Particle* part);
    double particleMCProductionZ(const Particle* part);
    /**
     * return position in x relative to interaction point
     */
    double particleDX(const Particle* part);

    /**
     * return position in y relative to interaction point
     */
    double particleDY(const Particle* part);

    /**
     * return position in z relative to interaction point
     */
    double particleDZ(const Particle* part);

    /**
     * return uncertainty in x relative to interaction point
     */
    double particleDXUncertainty(const Particle* part);

    /**
     * return uncertainty in y relative to interaction point
     */
    double particleDYUncertainty(const Particle* part);

    /**
     * return uncertainty in z relative to interaction point
     */
    double particleDZUncertainty(const Particle* part);
    /**
     * return transverse distance relative to interaction point
     */
    double particleDRho(const Particle* part);

    /**
     * return vertex azimuthal angle
     */
    double particleDPhi(const Particle* part);

    /**
     * return vertex polar angle
     */
    double particleDCosTheta(const Particle* part);
    /**
     * return distance relative to interaction point
     */
    double particleDistance(const Particle* part);

    /**
     * return significance of distance relative to interaction point
     * (distance relative to interaction point)/ ( error on distance measurement )
     */
    double particleDistanceSignificance(const Particle* part);
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
