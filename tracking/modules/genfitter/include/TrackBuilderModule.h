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

#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/HitPatternCDC.h>

// FIXME 2015-05-26 In order to allow friend declaration of
// hit-pattern builders.  Should be removed once the
// TrackBuilderModule is used always and the code to build tracks is
// removed from the GenFitterModule.
#include <tracking/modules/genfitter/GenFitterModule.h>

#include <vector>
#include <string>

namespace genfit {
  class Track;
}

namespace Belle2 {

  class TrackBuilderModule : public Module {

  public:
    /** Constructor .
     */
    TrackBuilderModule();

    /** Destructor.
     */
    virtual ~TrackBuilderModule();

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

    std::string m_gfTrackCandsColName;               /**< genfit::TrackCandidates collection name */
    std::string m_mcParticlesColName;                /**< MCParticles collection name */
    std::string m_gfTracksColName;                   /**< genfit::Tracks collection name */

    std::vector<double> m_beamSpot;                  /**< point on line to which tracks will be extrapolated */

    // FIXME 2015-05-26 These two functions are to remain static as
    // long as they are also used by the GenFitterModule.
    friend void GenFitterModule::event();
    static HitPatternCDC getHitPatternCDC(const genfit::Track&); /**< returns HitPatternCDC of the Track */
    static HitPatternVXD getHitPatternVXD(const genfit::Track&); /**< returns the HitPatternVXD of the Track*/
  };
}

