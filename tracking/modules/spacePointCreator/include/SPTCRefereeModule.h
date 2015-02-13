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
   *
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

    std::vector<double> m_PARAMsetOrigin; /**< assumed interaction point from which the SpacePointTrackCands emerge. Needed to determine the direction of flight */

    // ======================================================= INTERNALY USED MEMBERS ===============================================================

    B2Vector3F m_origin; /**< origin used internally. Gets assigned to the values of m_PARAMsetOrigin, after some sanity checks have been done on it */

    // ========================================== COUNTER VARIABLES ==================================================================================

    unsigned int m_SameSensorCtr; /**< counter for TrackCands with SpacePoints on the same sensor */

    unsigned int m_minDistanceCtr; /**< counter for TrackCands with SpacePoints not far enough apart */

    unsigned int m_totalTrackCandCtr; /**< counter for the total number of TrackCands */

    unsigned int m_kickedSpacePointsCtr; /**< counter of kicked SpacePoints */

    unsigned int m_curlingTracksCtr; /**< counter for tracks that curl */
    /**
     * initialize all counters to 0
     */
    void initializeCounters() {
      m_SameSensorCtr = 0;
      m_minDistanceCtr = 0;
      m_totalTrackCandCtr = 0;
      m_kickedSpacePointsCtr = 0;
      m_curlingTracksCtr = 0;
    }


    // ========================================================== MODULE METHODS ========================================================================

    /**
     * Check if two subsequent SpacePoints are on the same sensor. Returns empty vector if it is not the case and the indices of the SpacePoints that are on a sensor if there are SpacePoints on the same sensors
     */
    const std::vector<int> checkSameSensor(Belle2::SpacePointTrackCand* trackCand);

    /**
     * Check if two subsequent SpacePoints are seperated by at least the provided minDistance. Return empty vector if all SpacePoints are far enough apart, indices of SpacePoints that are not elsewise
     */
    const std::vector<int> checkMinDistance(Belle2::SpacePointTrackCand* trackCand, double minDistance);

    /**
     * Check if the SpacePointTrackCand shows curling behavior. Returns empty vector if it is not the case and the indices where the SpacePointTrackCand can be split into TrackStubs such that each of them is no longer curling
     */
    const std::vector<int> checkCurling(Belle2::SpacePointTrackCand* trackCand, bool useMCInfo);

    /**
     * get the directions of Flight for every SpacePoint in the passed vector. true is outgoing, false is ingoing. using MC information can be switched with useMCInfo
     */
    const std::vector<bool> getDirectionsOfFlight(const std::vector<const Belle2::SpacePoint*>& spacePoints, bool useMCInfo);

    /**
     * get the direction of flight for a SpacePoint by using information from the underlying TrueHit
     * @param origin the assumed interaction point
     * NOTE: this method assumes that there are already registered relations to a (if there are more only the first in the RelationVector will be used!) TrueHit for each SpacePoint
     */
    template <typename TrueHitType>
    bool getDirOfFlightTrueHit(const Belle2::SpacePoint* spacePoint, B2Vector3F origin);

    /**
     * get the directions of flight for a vector of SpacePoints using only information from SpacePoints (i.e. no MC information)
     * NOTE: as the momentum is no property that can be infered from one SpacePoint. the difference of positions between two consecutive SpacePoints is assumed to be the direction of the momentum (i.e. linear approximation)
     * WARNING: using this it is assumed, that the SpacePointTrackCand emerges from the origin (origin is used to calculate the estimate for the momentum of the first SpacePoint)
     */
    std::vector<bool> getDirsOfFlightSpacePoints(const std::vector<const Belle2::SpacePoint*>& spacePoints, B2Vector3F origin);

    /**
     * get the direction of flight provided the global position and momentum of a SpacePoint/TrueHit
     * for the TrueHit the momentum can be obtained from information stored in it. For SpacePoints it has to be guessed somehow
     */
    bool getDirOfFlightPosMom(B2Vector3F position, B2Vector3F momentum, B2Vector3F origin);
  };
}