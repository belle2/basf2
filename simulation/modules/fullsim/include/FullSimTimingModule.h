/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <map>

class TProfile;

class G4Track;
class G4LogicalVolume;
class G4Region;

namespace Belle2 {
  namespace Simulation {
    class StepTiming;
  }

  /**
   * \addtogroup modules
   * @{ FullSimTimingModule @}
   */

  /**
   * Provide more detailed timing info for FullSim module
   *
   * This module extends the FullSim to provide more detailed timing showing
   * how much time was spent in which sub detector.
   */
  class FullSimTimingModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of
     * the module.
     */
    FullSimTimingModule();

    /** No copying */
    FullSimTimingModule(const FullSimTimingModule&) = delete;

    /** No assignment */
    FullSimTimingModule& operator=(const FullSimTimingModule&) = delete;

    /** Setup the timing structure */
    virtual void initialize() override;

    /** Save the simulation time for each region in a profile */
    virtual void event() override;

    /** Save the output */
    virtual void terminate() override;

  private:
    /** store the time it took a Geant4 track for one step inside a volume */
    void processStep(G4Track* track, G4LogicalVolume* volume, double time);

    /** filename of the root file to store the timing profile */
    std::string m_rootFileName;
    /** instance of the StepTiming class which records the step times in geant4 */
    Simulation::StepTiming* m_timing {nullptr};
    /** map containing the amount of time spent in each region */
    std::map<const G4Region*, double> m_regionCache;
    /** iterator pointing to the region of the last step to speed up lookup */
    std::map<const G4Region*, double>::iterator m_lastRegion {m_regionCache.end()};
    /** map between region and and bin indices in the TProfile */
    std::map<const G4Region*, int> m_regionIndices;
    /** profile of the time spent per event in each region */
    TProfile* m_timingProfile {nullptr};
    /** total time spent stepping through the simulation geometry */
    double m_totalTime {0};
    /** stepping time of the current event */
    double m_eventTime {0};
    /** number of events with m_eventTime>0 */
    unsigned int m_eventCount {0};
    /** remember whether we already initialized everything */
    bool m_isInitialized {false};
  };
}
