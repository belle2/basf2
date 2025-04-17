/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * Tracking efficiency systematics module: removes tracks from the lists at random for efficiency studies
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

}; //namespace

