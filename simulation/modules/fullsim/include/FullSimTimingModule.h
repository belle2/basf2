/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef SIMULATION_MODULES_FULLSIM_FULLSIMTIMINGMODULE_H
#define SIMULATION_MODULES_FULLSIM_FULLSIMTIMINGMODULE_H

#include <framework/core/Module.h>
#include <simulation/kernel/StepTiming.h>
#include <map>

class TProfile;

namespace Belle2 {
  /**
   * \addtogroup modules
   * @{ FullSimTimingModule @}
   */

  /**
   * Provide more detailled timing info for FullSim module
   *
   * This module extends the FullSim to provide more detailed timing showing
   * how much time was spent in which sub detector
   *
   */
  class FullSimTimingModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of
     * the module.
     */
    FullSimTimingModule();

    /** Setup the timing structure */
    virtual void initialize();

    virtual void event();

    /** Save the output */
    virtual void terminate();

  private:
    void processStep(G4Track* track, G4LogicalVolume* volume, double time);

    std::string m_rootFileName;
    Simulation::StepTiming* m_timing {nullptr};
    std::map<const G4Region*, double> m_regionCache;
    std::map<const G4Region*, double>::iterator m_lastRegion {m_regionCache.end()};
    std::map<const G4Region*, int> m_regionIndices;
    TProfile* m_timingProfile {nullptr};

    double m_totalTime {0};
    double m_eventTime {0};
    unsigned int m_eventCount {0};
    bool m_isInitialized {false};
  };
}

#endif // SIMULATION_MODULES_FULLSIM_FULLSIMTIMINGMODULE_H
