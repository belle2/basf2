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
    /**
     * return x decay vertex coordinate of matched generated particle
     */
    double particleMCX(const Particle* part);
    /**
     * return y decay vertex coordinate of matched generated particle
     */
    double particleMCY(const Particle* part);
    /**
     * return z decay vertex coordinate of matched generated particle
     */
    double particleMCZ(const Particle* part);
    /**
     * return distance to interaction point of matched generated particle decay vertex
     */
    double particleMCDistance(const Particle* part);

    /**
     * return transverse distance to interaction point of matched generated particle decay vertex
     */
    double particleMCRho(const Particle* part);

    /**
     * return x production vertex coordinate of matched generated particle
     */
    double particleMCProductionX(const Particle* part);
    /**
     * return y production vertex coordinate of matched generated particle
     */
    double particleMCProductionY(const Particle* part);
    /**
     * return z production vertex coordinate of matched generated particle
     */
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
    /**
     * return x component of particle production vertex
     */
    double particleProductionX(const Particle* part);
    /**
     * return y component of particle production vertex
     */
    double particleProductionY(const Particle* part);
    /**
     * return z component of particle production vertex
     */
    double particleProductionZ(const Particle* part);
    /**
     * returns an element of a production vertex covariance matrix
     */
    Manager::FunctionPtr particleProductionCovElement(const std::vector<std::string>& arguments);
    /**
     * return x component uncertainty of particle production vertex
     */
    double particleProductionXErr(const Particle* part);
    /**
     * return y component uncertainty of particle production vertex
     */
    double particleProductionYErr(const Particle* part);
    /**
     * return z component uncertainty of particle production vertex
     */
    double particleProductionZErr(const Particle* part);
  }
} //namespace Belle2
