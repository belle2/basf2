/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <string>
#include <vector>

namespace Belle2 {

  /**
   * Module that does some sanity checks on SpacePointTrackCands that have been created by conversion from genfit::TrackCands by the TrackFinderMCTruth (or any other for that matter)
   *
   * AIMS:
   * + check if a SPTC is curling (at the moment done by the CurlingTrackCandSplitter) and split into sub tracks if necessary
   * + check if two subsequent SpacePoints of a SPTC are seperated at least by a user defined distance (needed for some filters)
   * + check if two subsequent SpacePoints of a SPTC are on the same sensor
   * + do all this checks either by using MC information or with information that can be obtained from the SpacePoint
   *
   * NOTE: currently under developement
   */
  class SPTCRefereeModule : public Module {

  public:
    SPTCRefereeModule(); /**< Constructor */

    virtual void initialize(); /**< initialize: check StoreArrays, initialize counters, ... */

    virtual void event(); /**< event: check SpacePointTrackCands */

    virtual void terminate(); /**< terminate: print some summary information */

  protected:

    // ==================================================== PARAMETERS ===========================================================================

    std::string m_PARAMsptcName; /**< Name of input container of SpacePointTrackCands */

    bool m_PARAMcheckSameSensor; /**< parameter for indicating if the check for subsequent SpacePoints being on the same sensor should be done */

    bool m_PARAMcheckMinDistance; /**< parameter for indicating if the check for the minimal distance between two subsequent SpacePoints should be done */

    bool m_PARAMcheckCurling; /**< parameter for indicating if the SpacePointTrackCand should be checked for curling */

    bool m_PARAMsplitCurlers; /**< parameter for switching on/off the splitting of curling SpacePointTrackCands */

    bool m_PARAMkeepOnlyFirstPart; /**< parameter for keeping only the first part of a curling SpacePointTrackCand */

    bool m_PARAMuseMCInfo; /**< parameter for indicating if MC information should be used or not */

    bool m_PARAMkickSpacePoint; /**< parameter for indicating if only the 'problematic' SpacePoint shall be removed from the SPTC or if the whole SPTC shall be kicked */

    double m_PARAMminDistance; /**< minimal distance two subsequent SpacePoints have to be seperated */

    // ========================================== COUNTER VARIABLES ==================================================================================

    unsigned int m_SameSensorCtr; /**< counter for TrackCands with SpacePoints on the same sensor */

    unsigned int m_minDistanceCtr; /**< counter for TrackCands with SpacePoints not far enough apart */

    unsigned int m_totalTrackCandCtr; /**< counter for the total number of TrackCands */

    /**
     * initialize all counters to 0
     */
    void initializeCounters() {
      m_SameSensorCtr = 0;
      m_minDistanceCtr = 0;
      m_totalTrackCandCtr = 0;
    }


    // ========================================================== MODULE METHODS ========================================================================

    /**
     * Check if two subsequent SpacePoints are on the same sensor. Returns empty vector if it is not the case and the indices of the SpacePoints that are on a sensor if there are SpacePoints on the same sensors
     */
    std::vector<int> checkSameSensor(Belle2::SpacePointTrackCand* trackCand);

    /**
     * Check if two subsequent SpacePoints are seperated by at least the provided minDistance. Return empty vector if all SpacePoints are far enough apart, indices of SpacePoints that are not elsewise
     */
    std::vector<int> checkMinDistance(Belle2::SpacePointTrackCand* trackCand, double minDistance);

  };
}