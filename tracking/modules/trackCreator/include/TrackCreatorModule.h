/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <vector>
#include <string>


namespace Belle2 {

  class TrackCreatorModule : public Module {

  public:
    /** Constructor .
     */
    TrackCreatorModule();

    /** Destructor.
     */
    virtual ~TrackCreatorModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    virtual void initialize();

    /** Called when entering a new run.
     */
    virtual void beginRun();

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    virtual void event();

    /** This method is called if the current run ends.
     */
    virtual void endRun();

    /** This method is called at the end of the event processing.
     */
    virtual void terminate();

  private:

    // Input
    std::string m_recoTrackColName;
    std::string m_mcParticleColName;

    // Output
    std::string m_trackColName;
    std::string m_trackFitResultColName;

    std::vector<double> m_beamSpot;
    std::vector<double> m_beamAxis;
    std::vector<int> m_pdgCodes;
  };
}

