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
   * Module that does some sanity checks on SpacePointTrackCands that have been created by conversion
   * from genfit::TrackCands by the TrackFinderMCTruth (or any other for that matter)
   *
   * If set by the parameters the module does:
   * + check if a SPTC is curling (at the moment done by the CurlingTrackCandSplitter) and split into sub tracks if necessary
   * + check if two subsequent SpacePoints of a SPTC are on the same sensor
   * + check if two subsequent SpacePoints of a SPTC are seperated at least by a user defined distance (needed for some filters)
   * + all this checks either by using MC information (where necessary) or with information that can be obtained from the SpacePoint
   *
   * WARNING: Besides of setting different flags, splitting curling SPTCs and removing problematic SpacePoints (if set)
   * the module does nothing! Every decision on how to handle the different outcomes of the different checks is left to the user!
   */
  class SPTCRefereeModule : public Module {

  public:
    SPTCRefereeModule(); /**< Constructor */

    void initialize() override; /**< initialize: check StoreArrays, initialize counters, ... */

    void event() override; /**< event: check SpacePointTrackCands */

    void terminate() override; /**< terminate: print some summary information */

  protected:

    /** typedef for storing the outcome of previously done checks to have them available later.
     * NOTE: declared as tuple in order to be able to add more stuff to it later on, without breaking the code! */
    typedef std::tuple<std::vector<int>, std::vector<int>> CheckInfo;

    // ==================================================== PARAMETERS ============================================================

    /** Name of input container of SpacePointTrackCands */
    std::string m_PARAMsptcName = "";

    /** Name of the output container of SpacePointTrackCands if 'storeNewArray' is set to true */
    std::string m_PARAMnewArrayName = "";

    /** Suffix that will be used to get a name for the StoreArray that holds the trackStubs that were obtained
     * by splitting a curling SPTC. Only needed if 'storeNewArray' is set to false */
    std::string m_PARAMcurlingSuffix = "_curlParts";

    /** parameter for indicating if the check for subsequent SpacePoints being on the same sensor should be done */
    bool m_PARAMcheckSameSensor = true;

    /** parameter for indicating if the check for the minimal distance between two subsequent SpacePoints should be done */
    bool m_PARAMcheckMinDistance = true;

    /** parameter for indicating if the SpacePointTrackCand should be checked for curling */
    bool m_PARAMcheckCurling = true;

    /** parameter for switching on/off the splitting of curling SpacePointTrackCands */
    bool m_PARAMsplitCurlers = false;

    /** parameter for keeping only the first part of a curling SpacePointTrackCand */
    bool m_PARAMkeepOnlyFirstPart = false;

    /** parameter for indicating if MC information should be used or not */
    bool m_PARAMuseMCInfo = true;

    /** parameter for indicating if only the 'problematic' SpacePoint shall be removed from the SPTC or if the whole SPTC shall be kicked */
    bool m_PARAMkickSpacePoint = false;

    /** only keep track candidates which have at least m_PARAMminNumSpacePoints space points **/
    int m_PARAMminNumSpacePoints = 0;

    /** parameter for indicating if all checked SpacePointTrackCands should be stored in a new StoreArray
     * NOTE: by storing the SpacePointTrackCands in a new StoreArray all previously registered relations are lost!) */
    bool m_PARAMstoreNewArray = true;

    /** minimal distance two subsequent SpacePoints have to be seperated */
    double m_PARAMminDistance = 0.;

    /** assumed interaction point from which the SpacePointTrackCands emerge. Needed to determine the direction of flight */
    std::vector<double> m_PARAMsetOrigin = { 0., 0., 0. };

    // ======================================================= INTERNALY USED MEMBERS =============================================

    /** origin used internally. Gets assigned to the values of m_PARAMsetOrigin, after some sanity checks have been done on it */
    B2Vector3F m_origin;

    /** name of the StoreArray in which the trackStubs from a curling SPTC are stored */
    std::string m_curlingArrayName;

    // ========================================== COUNTER VARIABLES ===============================================================

    /** counter for TrackCands with SpacePoints on the same sensor */
    unsigned int m_SameSensorCtr;

    /** counter for TrackCands with SpacePoints not far enough apart */
    unsigned int m_minDistanceCtr;

    /** counter for the total number of TrackCands */
    unsigned int m_totalTrackCandCtr;

    /** counter of kicked SpacePoints */
    unsigned int m_kickedSpacePointsCtr;

    /** counter for tracks that curl */
    unsigned int m_curlingTracksCtr;

    /** counter for the number of track stubs that were registered by this module */
    unsigned int m_regTrackStubsCtr;

    /** counter for the number of SPTCs which have direction of flight inward for all SpacePoints in them */
    unsigned int m_allInwardsCtr;

    /**
     * initialize all counters to 0
     */
    void initializeCounters()
    {
      m_SameSensorCtr = 0;
      m_minDistanceCtr = 0;
      m_totalTrackCandCtr = 0;
      m_kickedSpacePointsCtr = 0;
      m_curlingTracksCtr = 0;
      m_regTrackStubsCtr = 0;
      m_allInwardsCtr = 0;
    }


    // ========================================================== MODULE METHODS ==================================================

    /** Check if two subsequent SpacePoints are on the same sensor.
     * @returns empty vector if it is not the case and the indices of the SpacePoints that are on a sensor if
     * there are SpacePoints on the same sensors
     */
    const std::vector<int> checkSameSensor(Belle2::SpacePointTrackCand* trackCand);

    /** Check if two subsequent SpacePoints are seperated by at least the provided minDistance.
     * @returns empty vector if all SpacePoints are far enough apart, indices of SpacePoints that are not elsewise
     */
    const std::vector<int> checkMinDistance(Belle2::SpacePointTrackCand* trackCand, double minDistance);

    /** Check if the SpacePointTrackCand shows curling behavior.
     * @returns empty vector if it is not the case and the indices where the SpacePointTrackCand can be split into
     * TrackStubs such that each of them is no longer curling
     * If the first entry of the returned vector is 0 the direction of flight is inward for the whole SpacePointTrackCand
     */
    const std::vector<int> checkCurling(Belle2::SpacePointTrackCand* trackCand, bool useMCInfo);

    /** get the directions of Flight for every SpacePoint in the passed vector.
     * @returns true is outgoing, false is ingoing. using MC information can be switched with useMCInfo
     */
    const std::vector<bool> getDirectionsOfFlight(const std::vector<const Belle2::SpacePoint*>& spacePoints, bool useMCInfo);

    /** split a curling SpacePointTrackCand into TrackStubs.
     * @param onlyFirstPart return only the TrackStub that holds the SpacePoint from the first to the first entry of splitIndices (not included in returned SpacePointTrackCand)
     */
    std::vector<Belle2::SpacePointTrackCand>
    splitTrackCand(const Belle2::SpacePointTrackCand* trackCand, const std::vector<int>& splitIndices, bool onlyFirstPart,
                   const CheckInfo& prevChecksInfo, bool removedHits);

    /** get the direction of flight for a SpacePoint by using information from the underlying TrueHit
     * NOTE: this method assumes that there are already registered relations to a TrueHit for each SpacePoint
     * (if there are more only the first in the RelationVector will be used!)
     * @param origin the assumed interaction point
     */
    template <typename TrueHitType>
    bool getDirOfFlightTrueHit(const Belle2::SpacePoint* spacePoint, B2Vector3F origin);

    /** get the directions of flight for a vector of SpacePoints using only information from SpacePoints (i.e. no MC information)
     * NOTE: as the momentum is no property that can be infered from one SpacePoint. the difference of positions between two consecutive SpacePoints is assumed to be the direction of the momentum (i.e. linear approximation)
     * WARNING: using this it is assumed, that the SpacePointTrackCand emerges from the origin (origin is used to calculate the estimate for the momentum of the first SpacePoint)
     */
    std::vector<bool> getDirsOfFlightSpacePoints(const std::vector<const Belle2::SpacePoint*>& spacePoints, B2Vector3F origin);

    /** get the direction of flight provided the global position and momentum of a SpacePoint/TrueHit
     * for the TrueHit the momentum can be obtained from information stored in it. For SpacePoints it has to be guessed somehow
     */
    bool getDirOfFlightPosMom(B2Vector3F position, B2Vector3F momentum, B2Vector3F origin);

    /** copy the SpacePointTrackCand to a new StoreArray and register a relation to the original trackCand
     * @param trackCand pointer to the original SPTC (which is not in newStoreArray befor this call!)
     * @param newStoreArray StoreArray where a copy of trackCand will be stored with a relation to the trackCand in its original StoreArray
     */
    void
    copyToNewStoreArray(const Belle2::SpacePointTrackCand* trackCand, Belle2::StoreArray<Belle2::SpacePointTrackCand> newStoreArray);

    /** register the SpacePointTrackCand (i.e. trackStub) in a new StoreArray and register also a relation to the original SpacePointTrackCand
     * @param trackCand the SPTC that has to be registered in
     * @param storeArray the StoreArray that the trackCand will be added to
     * @param origTrackCand pointer to the SPTC to which a relation from the trackCand will be registered
     */
    void addToStoreArray(const Belle2::SpacePointTrackCand& trackCand, Belle2::StoreArray<Belle2::SpacePointTrackCand> storeArray,
                         const Belle2::SpacePointTrackCand* origTrackCand);

    /** get the checked referee status of a SPTC (i.e. only which checks have been performed, but not the results)
     * at the moment returns the status of trackCand with c_removedHits, c_hitsOnSameSensor and c_hitsLowDistance resetted!
     */
    unsigned short int getCheckStatus(const Belle2::SpacePointTrackCand* trackCand);

    /** remove the SpacePoint passed to this function from the SpacePointTrackCand
     * @return indices of the SpacePoints that are in took the places of the ones that were removed (i.e. if (i,j,k) are passed -> (i,j-1,k-2) is returned)
     * NOTE: trackCand will be altered!
     */
    const std::vector<int> removeSpacePoints(Belle2::SpacePointTrackCand* trackCand, const std::vector<int>& indsToRemove);

    /** function to determine if any of the values in vector V are between the values of P
     * (i.e. any value of V is in [P.first, P.second) ) */
    template<typename T>
    bool vectorHasValueBetween(std::vector<T> V, std::pair<T, T> P)
    {
      return std::find_if(V.begin(), V.end(), [&P](const T & aValue) { return (aValue < P.second && aValue >= P.first);}) != V.end();
    }
  };
}
