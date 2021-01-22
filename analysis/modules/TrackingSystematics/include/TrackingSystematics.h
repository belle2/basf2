/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sasha Glazov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/dataobjects/Particle.h>

namespace Belle2 {
  /**
  *This set of module is designed for tracking systematics studies
  *
  */


  /**
   * Tracking efficiency systematics
   */
  class TrackingEfficiencyModule : public Module {
  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    TrackingEfficiencyModule();

    /**
    * Function to be executed at each event
    */
    virtual void event() override;

  private:
    /** input particle lists */
    std::vector<std::string> m_ParticleLists;
    /** fraction of particles to be removed from the particlelist */
    double m_frac;
  }; //TrackingEfficiencyModule


  /**
  * Tracking momentum systematics
  */
  class TrackingMomentumModule : public Module {
  public:
    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    TrackingMomentumModule();

    /**
    * Function to be executed at each event
    */
    virtual void event() override;

  private:
    /** input particle lists */
    std::vector<std::string> m_ParticleLists;
    /** input momentum scale modifier */
    double m_scale;

    /**
     * function to set momentum scaling factor
     */
    void setMomentumScalingFactor(Particle* particle);

  }; // TrackingMomentumModule

}; //namespace

