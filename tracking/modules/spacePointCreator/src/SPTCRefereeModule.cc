/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/SPTCRefereeModule.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/geometry/B2Vector3.h> // use TVector3 instead?
#include <boost/range/adaptor/reversed.hpp> // for ranged based loops in reversed order

using namespace Belle2;
using namespace std;

REG_MODULE(SPTCReferee) // register the module

SPTCRefereeModule::SPTCRefereeModule() : Module()
{
  setDescription("Module that does some sanity checks on SpacePointTrackCands to prevent some problematic cases to be forwarded to other modules that rely on 'unproblematic' cases (e.g. FilterCalculator). Different checks can be enabled by setting the according flags. Using MC information for the tests can also be switched on/off for tests where MC information can be helpful.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // names
  addParam("sptcName", m_PARAMsptcName, "Container name of the SpacePointTrackCands to be checked (input)", std::string(""));
  addParam("newArrayName",
           m_PARAMnewArrayName, "Container name of SpacePointTrackCands if 'storeNewArray' is set to true", std::string(""));
  addParam("curlingSuffix",
           m_PARAMcurlingSuffix,
           "Suffix that will be used to get a name for the StoreArray in which the trackStubs that are obtained by splitting a curling SPTC get stored. NOTE: If 'storeNewArray' is set to true, this will not be used and all output SPTCs will be in the same Array!",
           std::string("_curlParts"));

  // flags
  addParam("checkSameSensor", m_PARAMcheckSameSensor, "Check if two subsequent SpacePoints are on the same sensor", true);
  addParam("checkMinDistance",
           m_PARAMcheckMinDistance, "Check if two subsequent SpacePoints are seperated by more than 'minDistance'", true);
  addParam("checkCurling",
           m_PARAMcheckCurling, "Check the SpacePointTrackCand for curling behaviour and mark it as curling if it does", true);
  addParam("splitCurlers",
           m_PARAMsplitCurlers, "Split curling SpacePointTrackCands and save the TrackStubs in seperate StoreArrays", false);
  addParam("keepOnlyFirstPart",
           m_PARAMkeepOnlyFirstPart, "Keep only the first part of a curling SpacePointTrackCand (e.g. when only this is needed)", false);
  addParam("useMCInfo",
           m_PARAMuseMCInfo,
           "Set to true if the use of MC information (e.g. from underlying TrueHits) for the checks is wanted, and to false if the checks should all be done with information that can be obtained from SpacePoints directly. NOTE: the tests without MC information have to be developed first!",
           true);
  addParam("kickSpacePoint",
           m_PARAMkickSpacePoint, "Set to true if only the 'problematic' SpacePoint shall be kicked and not the whole SpacePointTrackCand",
           false);
  addParam("storeNewArray",
           m_PARAMstoreNewArray,
           "Set to true if the checked SpacePointTrackCands should be stored in a new StoreArray. WARNING: all previously registered relations get lost in this way!",
           true);

  // other
  addParam("minDistance",
           m_PARAMminDistance, "Minimal Distance [cm] that two subsequent SpacePoints have to be seperated if 'checkMinDistance' is enabled",
           double(0));
  std::vector<double> defaultOrigin = { 0., 0., 0. };
  addParam("setOrigin",
           m_PARAMsetOrigin,
           "WARNING: still need to find out the units that are used internally! Reset origin to given point. Used for determining the direction of flight of a particle for a given hit. Needs to be reset for e.g. testbeam, where origin is not at (0,0,0)",
           defaultOrigin);


  addParam("minNumSpacePoints", m_PARAMminNumSpacePoints, "minimum number of space points that a track candidate has to "
           "contain (added later, set to 0 to reproduce old behavior", 0);

  addParam("checkIfFitted", m_PARAMcheckIfFitted, "If true a flag is set in the SpacePointTrackCandidate if any related RecoTrack "
           "with successful track fit is found", m_PARAMcheckIfFitted);

  // initialize counters (cppcheck)
  initializeCounters();
}

// ======================================================================= INITIALIZE =============================================
void SPTCRefereeModule::initialize()
{
  B2INFO("SPTCReferee::initialize(): ------------------------------------------------ ");
  // check if StoreArray of SpacePointTrackCands is her
  StoreArray<SpacePointTrackCand> inputSpacePoints(m_PARAMsptcName);
  inputSpacePoints.isRequired(m_PARAMsptcName);

  // register new StoreArray
  if (m_PARAMstoreNewArray) {
    StoreArray<SpacePointTrackCand> newStoreArray(m_PARAMnewArrayName);
    newStoreArray.registerInDataStore(m_PARAMnewArrayName, DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
    newStoreArray.registerRelationTo(inputSpacePoints, DataStore::c_Event, DataStore::c_DontWriteOut);
  } else {
    m_curlingArrayName = m_PARAMsptcName + m_PARAMcurlingSuffix;
    B2DEBUG(100, "StoreArray name of the curling parts: " << m_curlingArrayName);
    StoreArray<SpacePointTrackCand> newStoreArray(m_curlingArrayName);
    newStoreArray.registerInDataStore(m_curlingArrayName, DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
    newStoreArray.registerRelationTo(inputSpacePoints, DataStore::c_Event, DataStore::c_DontWriteOut);
  }

  // sanity checks on the other parameters
  if (m_PARAMcheckMinDistance) {
    if (m_PARAMminDistance < 0) {
      B2WARNING("minDistance set to value below 0: " << m_PARAMminDistance <<
                ", Taking the absolute value and resetting 'minDistance' to that!");
      m_PARAMminDistance = -m_PARAMminDistance;
    }
  }

  B2DEBUG(100, "Provided Parameters: checkSameSensor - " << m_PARAMcheckSameSensor << ", checkMinDistance - " <<
          m_PARAMcheckMinDistance
          << ", checkCurling - " << m_PARAMcheckCurling << ", splitCurlers - " << m_PARAMsplitCurlers << ", keepOnlyFirstPart - " <<
          m_PARAMkeepOnlyFirstPart << ", useMCInfo - " << m_PARAMuseMCInfo << ", kickSpacePoint - " << m_PARAMkickSpacePoint);
  if (m_PARAMsetOrigin.size() != 3) {
    B2WARNING("CurlingTrackCandSplitter::initialize: Provided origin is not a 3D point! Please provide 3 values (x,y,z). Rejecting user input and setting origin to (0,0,0) for now!");
    m_PARAMsetOrigin.clear();
    m_PARAMsetOrigin.assign(3, 0);
  }
  m_origin.SetXYZ(m_PARAMsetOrigin.at(0), m_PARAMsetOrigin.at(1), m_PARAMsetOrigin.at(2));
  B2DEBUG(10, "Set origin to (x,y,z): (" << m_origin.X() << "," << m_origin.Y() << "," << m_origin.Z() << ")");

  initializeCounters();
}

// ======================================================================== EVENT =================================================
void SPTCRefereeModule::event()
{
  B2DEBUG(10, endl << "Entering event() function" << endl);

  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCtr = eventMetaDataPtr->getEvent();
  B2DEBUG(10, "SpacePoint2TrueHitConnector::event(). Processing event " << eventCtr << " -----------------------");

  StoreArray<SpacePointTrackCand> trackCands(m_PARAMsptcName);
  const int nTCs = trackCands.getEntries();

  m_totalTrackCandCtr += nTCs;

  B2DEBUG(10, "Found " << nTCs << " SpacePointTrackCands in Array " << trackCands.getName() << " for this event");

  for (int iTC = 0; iTC < nTCs; ++iTC) { // loop over all TrackCands
    SpacePointTrackCand* trackCand = trackCands[iTC];
    B2DEBUG(20, "Processing SpacePointTrackCand " << iTC << ": It has " << trackCand->getNHits() << " SpacePoints in it");



    if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 200, PACKAGENAME())) { trackCand->print(); }
    B2DEBUG(50, "refereeStatus of TrackCand before tests: " << trackCand->getRefereeStatus() << " -> " <<
            trackCand->getRefereeStatusString());

    // if all tests will be performed -> add checkedByReferee status to the SPTC, CAUTION: if there are new tests this has to be updated!!!, WARNING: if curling check fails, checkedForCurling will return false but hasRefereeStatus(c_checkedByReferee) will return true after this module!
    if (m_PARAMcheckCurling && m_PARAMcheckMinDistance && m_PARAMcheckSameSensor && m_PARAMcheckIfFitted) {
      trackCand->addRefereeStatus(SpacePointTrackCand::c_checkedByReferee);
    }
    bool allChecksClean = true; // assume that all tests will be passed, change to false if one of them fails
    CheckInfo prevChecksInfo;


    //added check for the number of space points in the track candidate
    if ((int)(trackCand->getNHits()) < m_PARAMminNumSpacePoints) allChecksClean = false;


    // set a flag if a fitted recotrack is found for that trackcand
    if (m_PARAMcheckIfFitted) {
      // take any related recotrack
      RelationVector<RecoTrack> relatedRecoTracks = trackCand->getRelationsTo<RecoTrack>("ALL");
      if (relatedRecoTracks.size() >= 1) {
        // assume that there is only one!
        if (relatedRecoTracks[0]->wasFitSuccessful()) {
          trackCand->addRefereeStatus(SpacePointTrackCand::c_hasFittedRecoTrack);
        } else {
          allChecksClean = false;
          B2DEBUG(1, "Found RecoTrack was not fitted! Will not use this track candidate for training.");
        }
      } else {
        allChecksClean = false;
        B2DEBUG(1, "No related RecoTrack found. Will not use that track candidate for training");
      }
    }


    if (m_PARAMcheckSameSensor) { // check same sensors if desired
      const std::vector<int> sameSensorInds = checkSameSensor(trackCand);
      std::get<0>(prevChecksInfo) = sameSensorInds;
      if (!sameSensorInds.empty()) {
        m_SameSensorCtr++;
        allChecksClean = false;
        if (m_PARAMkickSpacePoint) { std::get<0>(prevChecksInfo) = removeSpacePoints(trackCand, sameSensorInds); } // assign the actually removed indices to the prevChecksInfo
        else { trackCand->addRefereeStatus(SpacePointTrackCand::c_hitsOnSameSensor); } // only add status if the SpacePoints on the same sensors have not been removed!
      } else {
        B2DEBUG(20, "Found no two subsequent SpacePoints on the same sensor for this SpacePointTrackCand (" << iTC << " in Array " <<
                trackCands.getName() << ")");
      }
      trackCand->addRefereeStatus(SpacePointTrackCand::c_checkedSameSensors);
      B2DEBUG(50, "refereeStatus of TrackCand after checkSameSensor " << trackCand->getRefereeStatus() << " -> " <<
              trackCand->getRefereeStatusString());
    }


    if (m_PARAMcheckMinDistance) { // check min distance if desired
      const std::vector<int> lowDistanceInds = checkMinDistance(trackCand, m_PARAMminDistance);
      std::get<1>(prevChecksInfo) = lowDistanceInds;
      if (!lowDistanceInds.empty()) {
        m_minDistanceCtr++;
        allChecksClean = false;
        if (m_PARAMkickSpacePoint) { std::get<1>(prevChecksInfo) = removeSpacePoints(trackCand, lowDistanceInds); } // assign the actually removed indices to the prevChecksInfo
        else { trackCand->addRefereeStatus(SpacePointTrackCand::c_hitsLowDistance); } // only add status if the SpacePoints not far enough apart have not been removed!
      } else {
        B2DEBUG(20, "Found no two subsequent SpacePoints that were closer than " << m_PARAMminDistance <<
                " cm together for this SpacePointTrackCand (" << iTC << " in Array " << trackCands.getName() << ")");
      }
      trackCand->addRefereeStatus(SpacePointTrackCand::c_checkedMinDistance);
      B2DEBUG(30, "refereeStatus of TrackCand after checkMinDistance " << trackCand->getRefereeStatus() << " -> " <<
              trackCand->getRefereeStatusString());
    }

    std::vector<SpacePointTrackCand> curlingTrackStubs; // vector of TrackStubs that shall be saved to another StoreArray
    if (m_PARAMcheckCurling) { // check curling if desired
      // setting the TrackStubIndex to 0 implies that this trackCand has been checked for curling. (If there is something wrong in the curling check this value is reset to -1!)
      trackCand->setTrackStubIndex(0);
      const std::vector<int> curlingSplitInds = checkCurling(trackCand, m_PARAMuseMCInfo);
      if (!curlingSplitInds.empty()) {
        if (!(curlingSplitInds.at(0) == 0 && curlingSplitInds.size() == 1)) {
          // this means essentially that the direction of flight for this SPTC is inwards for all SpacePoints!
          m_curlingTracksCtr++;
          allChecksClean = false;
          if (m_PARAMsplitCurlers) {
            curlingTrackStubs = splitTrackCand(trackCand, curlingSplitInds, m_PARAMkeepOnlyFirstPart, prevChecksInfo, m_PARAMkickSpacePoint);
            if (curlingTrackStubs.empty()) { B2ERROR("The vector returned by splitTrackCand is empty!"); } // safety measure
          }
          // set this to the original SPTC only after splitting to avoid having this status in the trackStubs
          trackCand->addRefereeStatus(SpacePointTrackCand::c_curlingTrack);
        } else {
          B2DEBUG(20, "The only entry in the return vector of checkCurling is 0! The direction of flight is inwards for the whole SPTC!");
          trackCand->setFlightDirection(false);
          m_allInwardsCtr++;
        }
      } else {
        B2DEBUG(20, "SpacePointTrackCand " << trackCand->getArrayIndex() << " is not curling!");
      }
      B2DEBUG(30, "refereeStatus of TrackCand after checkCurling " << trackCand->getRefereeStatus() << " -> " <<
              trackCand->getRefereeStatusString());
    }

    // PROCESSING AFTER CHECKS
    if (allChecksClean) trackCand->addRefereeStatus(SpacePointTrackCand::c_checkedClean);

    B2DEBUG(999, "referee Status of SPTC after referee module: " << trackCand->getRefereeStatus() << " -> " <<
            trackCand->getRefereeStatusString());
    if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 200, PACKAGENAME())) { trackCand->print();}

    // store in appropriate StoreArray
    if (m_PARAMstoreNewArray) {
      StoreArray<SpacePointTrackCand> newArray(m_PARAMnewArrayName);
      if (!trackCand->isCurling()) { copyToNewStoreArray(trackCand, newArray); }
      else {
        for (const SpacePointTrackCand& trackStub : curlingTrackStubs) { addToStoreArray(trackStub, newArray, trackCand); }
      }
    } else {
      StoreArray<SpacePointTrackCand> curlingArray(m_curlingArrayName);
      if (trackCand->isCurling()) {
        for (const SpacePointTrackCand& trackStub : curlingTrackStubs) { addToStoreArray(trackStub, curlingArray, trackCand); }
      }
    }
  }
}

// ============================================================================= TERMINATE ========================================
void SPTCRefereeModule::terminate()
{
  // TODO: info output more sophisticated
  stringstream summary;
  if (m_PARAMcheckSameSensor) { summary << "Checked for consecutive SpacePoints on same sensor and found " << m_SameSensorCtr << " TrackCands showing this behavior.\n"; }
  if (m_PARAMcheckMinDistance) { summary << "Checked for minimal distance between two consecutive SpacePoints and found " << m_minDistanceCtr << " TrackCands with SpacePoints not far enough apart.\n"; }
  if (m_PARAMkickSpacePoint) { summary << m_kickedSpacePointsCtr << " SpacePoints have been removed from SpacePointTrackCands\n"; }
  if (m_PARAMcheckCurling) { summary << m_curlingTracksCtr << " SPTCs were curling. Registered " << m_regTrackStubsCtr << " track stubs. 'splitCurlers' was set to " << m_PARAMsplitCurlers << ", 'keepOnlyFirstPart' was set to " << m_PARAMkeepOnlyFirstPart << ". There were " << m_allInwardsCtr << " SPTCs that had flight direction 'inward' for all SpacePoints in them"; }
  B2INFO("SPTCRefere::terminate(): Module got " << m_totalTrackCandCtr << " SpacePointTrackCands. " << summary.str());
  if (!m_PARAMuseMCInfo && m_PARAMcheckCurling) { B2WARNING("The curling checking without MC Information is at the moment at a very crude and unsophisticated state. If you have MC information available you should use it to do this check!"); }
}

// ====================================================================== CHECK SAME SENSORS ======================================
const std::vector<int> SPTCRefereeModule::checkSameSensor(Belle2::SpacePointTrackCand* trackCand)
{
  B2DEBUG(25, "Checking SpacePointTrackCand " << trackCand->getArrayIndex() << " from Array " << trackCand->getArrayName() <<
          " for consecutive SpacePoints on the same sensor");
  std::vector<int> sameSensorInds; // return vector

  std::vector<const SpacePoint*> spacePoints = trackCand->getHits();

  // catch cases where the TC has no space points! (Yes that happens!)
  if (spacePoints.size() == 0) return sameSensorInds;

  VxdID lastSensorId = spacePoints.at(0)->getVxdID();

  for (unsigned int iSp = 1; iSp < spacePoints.size(); ++iSp) {
    VxdID sensorId = spacePoints.at(iSp)->getVxdID();
    B2DEBUG(50, "Checking SpacePoint " << iSp << ". (ArrayIndex " << spacePoints.at(iSp)->getArrayIndex() <<
            ") SensorId of this SpacePoint: " << sensorId << ", SensorId of last SpacePoint: " << lastSensorId);
    if (sensorId == lastSensorId) {
      // push back the index of the first SpacePoint (50:50 chance of getting the right one without further testing) -> retrieving the other index is no big science from this index!!
      sameSensorInds.push_back(iSp - 1);
      B2DEBUG(30, "SpacePoint " << iSp << " and " << iSp - 1 << " are on the same sensor: " << sensorId);
    }
    lastSensorId = sensorId;
  }

  return sameSensorInds;
}

// ========================================================================= CHECK MIN DISTANCE ===================================
const std::vector<int> SPTCRefereeModule::checkMinDistance(Belle2::SpacePointTrackCand* trackCand, double minDistance)
{
  B2DEBUG(25, "Checking the distances between consecutive SpacePoints for SpacePointTrackCand " << trackCand->getArrayIndex() <<
          " from Array " << trackCand->getArrayIndex());
  std::vector<int> lowDistanceInds; // return vector

  std::vector<const SpacePoint*> spacePoints = trackCand->getHits();

  // catch case where the track candidate has no spacepoints
  if (spacePoints.size() == 0) return lowDistanceInds;

  B2Vector3F oldPosition = spacePoints.at(0)->getPosition();

  for (unsigned int iSp = 1; iSp < spacePoints.size(); ++iSp) {
    B2Vector3F position = spacePoints.at(iSp)->getPosition();
    B2Vector3F diffPos = oldPosition - position;
    B2DEBUG(60, "Position of SpacePoint " << iSp << " (ArrayIndex " << spacePoints.at(iSp)->getArrayIndex() << "): (" << position.X() <<
            "," << position.Y() << "," << position.Z() << "), Position of SpacePoint " << iSp - 1 << ": (" << oldPosition.X() << "," <<
            oldPosition.Y() << "," << oldPosition.Z() << ") --> old - new = (" << diffPos.X() << "," << diffPos.Y() << "," << diffPos.Z() <<
            ")");

    if (diffPos.Mag() <= minDistance) {
      B2DEBUG(30, "Position difference is " << diffPos.Mag() <<  " but minDistance is set to " << minDistance << ". SpacePoints: " << iSp
              << " and " << iSp - 1);
      // push back the index of the first SpacePoint (50:50 chance of getting the right one without further testing)
      lowDistanceInds.push_back(iSp);
    }
    oldPosition = position;
  }

  return lowDistanceInds;
}

// ============================================================= REMOVE SPACEPOINTS ===============================================
const std::vector<int>
SPTCRefereeModule::removeSpacePoints(Belle2::SpacePointTrackCand* trackCand, const std::vector<int>& indsToRemove)
{
  std::vector<int> removedInds; // return vector
  try {
    unsigned int nInds = indsToRemove.size();
    B2DEBUG(50, "Got " << nInds << " indices to remove from SPTC " << trackCand->getArrayIndex());

    int nRemoved = 0;
    for (int index : boost::adaptors::reverse(indsToRemove)) { // reverse iteration as trackCand gets 'resized' with every remove
      B2DEBUG(999, "Removing " << nRemoved + 1 << " from " << nInds << ". index = " << index); // +1 only for better readability
      trackCand->removeSpacePoint(index);
      nRemoved++;
      m_kickedSpacePointsCtr++;
      B2DEBUG(50, "Removed SpacePoint " << index << " from SPTC " << trackCand->getArrayIndex());
      // NOTE: this way if a removed SpacePoint is "at the edge" between two trackStubs the status will be assigned to the second of those!
      removedInds.push_back(index - (nInds - nRemoved));
    }
    trackCand->addRefereeStatus(SpacePointTrackCand::c_removedHits);
  } catch (SpacePointTrackCand::SPTCIndexOutOfBounds& anE) {
    B2WARNING("Caught an Exception while trying to remove a SpacePoint from a SpacePointTrackCand: " << anE.what());
  }

  return removedInds;
}
// =========================================================== CHECK CURLING ======================================================
const std::vector<int> SPTCRefereeModule::checkCurling(Belle2::SpacePointTrackCand* trackCand, bool useMCInfo)
{
  std::vector<int> splitInds; // return vector

  //catch cases where there are no space points in the trackCand!
  if (trackCand->getHits().size() == 0) return splitInds;

  // Only do curling checking if useMCInfo is false, OR if useMCInfo is true if the SPTCs SpacePoints have been checked for a relation to TrueHits!
  if (!m_PARAMuseMCInfo || trackCand->hasRefereeStatus(SpacePointTrackCand::c_checkedTrueHits)) {

    std::string mcInfoStr = useMCInfo ? std::string("with") : std::string("without");
    B2DEBUG(25, "Checking SpacePointTrackCand " << trackCand->getArrayIndex() << " from Array " << trackCand->getArrayName() <<
            " for curling behavior " << mcInfoStr << " MC Information");

    // get the SpacePoints of the TrackCand
    const std::vector<const SpacePoint*>& tcSpacePoints = trackCand->getHits();
    B2DEBUG(50, "SPTC has " << tcSpacePoints.size() << " SpacePoints");

    // get the directions of flight for every SpacePoint
    const std::vector<bool> dirsOfFlight = getDirectionsOfFlight(tcSpacePoints, useMCInfo);

    //   if(trackCand->getNHits() != dirsOfFlight.size()) B2FATAL("did not get a direction of flight for every SpacePoint"); // should not /cannot happen

    // loop over all entries of dirsOfFlight and compare them pair-wise. If they change -> add Index to splitInds.
    if (!dirsOfFlight.at(0)) {
      // if the direction of flight is inwards for the first hit, push_back 0 -> make information accessible from outside this function
      splitInds.push_back(0);
      B2DEBUG(999, "Direction of flight was inwards for first SpacePoint of this SPTC");
    }
    // DEBUG output
    B2DEBUG(999, "Direction of flight is " << dirsOfFlight.at(0) << " for SpacePoint " << 0 << " of this SPTC");
    for (unsigned int i = 1; i < dirsOfFlight.size(); ++i) {
      B2DEBUG(999, "Direction of flight is " << dirsOfFlight.at(i) << " for SpacePoint " << i << " of this SPTC");
      if (dirsOfFlight.at(i) ^ dirsOfFlight.at(i - 1)) {
        splitInds.push_back(i); // NOTE: using the bitoperator for XOR here to determine if the bools differ!
        B2DEBUG(999, "Direction of flight has changed from SpacePoint " << i - 1 << " to " << i << ".");
      }
    } // END DEBUG output
  } else {
    B2ERROR("'useMCInfo' is set to true, but SpacePoints of SPTC have not been checked for relations to TrueHits! Not Checking this SPTC for curling!");
    trackCand->setTrackStubIndex(-1); // reset to not being checked for curling
  }
  return splitInds;
}

// ============================================================ SPLIT CURLING TRACK CAND ==========================================
std::vector<Belle2::SpacePointTrackCand>
SPTCRefereeModule::splitTrackCand(const Belle2::SpacePointTrackCand* trackCand, const std::vector<int>& splitIndices,
                                  bool onlyFirstPart, const CheckInfo& prevChecksInfo, bool removedHits)
{
  std::vector<SpacePointTrackCand> trackStubs; // return vector

  B2DEBUG(25, "Splitting SpacePointTrackCand " << trackCand->getArrayIndex() << " from Array " << trackCand->getArrayName() <<
          ": number of entries in splitIndices " << splitIndices.size());
  //   int trackStub = 0;
  bool dirOfFlight = splitIndices.at(0) != 0; // if first entry is zero the direction of flight is false (= ingoing)

  B2DEBUG(999, "first entry of passed vector<int> is " << splitIndices.at(0) << " --> direction of flight is " << dirOfFlight);
  // if the first entry of splitIndices is zero the first TrackStub is from 0 to second entry instead of from 0 to first entry
  int firstLast = dirOfFlight ? splitIndices.at(0) : splitIndices.at(1);
  std::vector<std::pair<int, int> >
  rangeIndices; // .first is starting, .second is final index for each TrackStub. Store them in vector to be able to easily loop over them
  rangeIndices.push_back(std::make_pair(0, firstLast));

  if (!onlyFirstPart) { // if more than the first part is desired push_back the other ranges too
    unsigned int iStart = dirOfFlight ? 1 : 2;
    for (unsigned int i = iStart; i < splitIndices.size(); ++i) {
      rangeIndices.push_back(std::make_pair(splitIndices.at(i - 1), splitIndices.at(i)));
    }
    // last TrackStub is from last split index to end of TrackCand
    rangeIndices.push_back(std::make_pair(splitIndices.at(splitIndices.size() - 1), trackCand->getNHits()));
  }
  B2DEBUG(30, "There will be " << rangeIndices.size() << " TrackStubs created for this TrackCand. (size of the passed splitIndices: "
          << splitIndices.size() << ", onlyFirstPart " << onlyFirstPart);

  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 999, PACKAGENAME())) {
    stringstream dbOutput;
    dbOutput << "The indices that will be used for splitting the SPTC: ";
    for (auto entry : rangeIndices) { dbOutput << "[" << entry.first << "," << entry.second << ") "; }
    B2DEBUG(999, dbOutput.str());
  }

  // loop over all entries in range indices and create a SpacePointTrackCand from it
  for (unsigned int iTs = 0; iTs < rangeIndices.size(); ++iTs) {
    int firstInd = rangeIndices.at(iTs).first;
    int lastInd = rangeIndices.at(iTs).second;

    unsigned short int refStatus = getCheckStatus(trackCand);

    B2DEBUG(250, "Trying to create TrackStub from SPTC " << trackCand->getArrayIndex() << " with indices [" << firstInd << "," <<
            lastInd << ")");
    // encapsulate in try block to catch indices out of range
    try {
      const std::vector<const SpacePoint*> spacePoints = trackCand->getHitsInRange(firstInd, lastInd);
      const std::vector<double> sortingParams = trackCand->getSortingParametersInRange(firstInd, lastInd);

      // create new TrackCand
      SpacePointTrackCand trackStub(spacePoints, trackCand->getPdgCode(), trackCand->getChargeSeed(), trackCand->getMcTrackID());
      trackStub.setSortingParameters(sortingParams);

      // set the state seed and the cov seed only for the first trackStub of the TrackCand
      if (iTs < 1) {
        trackStub.set6DSeed(trackCand->getStateSeed());
        trackStub.setCovSeed(trackCand->getCovSeed());
      }

      // set the direction of flight and flip it afterwards, because next trackCand hs changed direction of flight
      trackStub.setFlightDirection(dirOfFlight);
      dirOfFlight = !dirOfFlight;

      // trackStub index starts at 1 for curling SPTCs. NOTE: this might be subject to chagnes with the new bitfield in SpacePointTrackCand
      trackStub.setTrackStubIndex(iTs + 1);

      // determine and set the referee status of this trackStub based upon the information from the previous tests
      const std::vector<int>& sameSensInds = std::get<0>(prevChecksInfo);
      const std::vector<int>& lowDistInds = std::get<0>(prevChecksInfo);
      bool hasSameSens = vectorHasValueBetween(sameSensInds, rangeIndices.at(iTs));
      bool hasLowDist = vectorHasValueBetween(lowDistInds, rangeIndices.at(iTs));
      if ((hasSameSens || hasLowDist) && removedHits) refStatus += SpacePointTrackCand::c_removedHits;
      if (hasSameSens && !removedHits) refStatus += SpacePointTrackCand::c_hitsOnSameSensor;
      if (hasLowDist && !removedHits) refStatus += SpacePointTrackCand::c_hitsLowDistance;

      trackStub.setRefereeStatus(refStatus);
      B2DEBUG(999, "Set TrackStubIndex " << iTs + 1 << " and refereeStatus " << trackStub.getRefereeStatus() <<
              " for this trackStub (refStatus string: " << trackStub.getRefereeStatusString());

      trackStubs.push_back(trackStub);
      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 499, PACKAGENAME())) { trackStub.print(); }
    } catch (SpacePointTrackCand::SPTCIndexOutOfBounds& anE) {
      B2WARNING("Caught an exception while trying to split a curling SpacePointTrackCand: " << anE.what() <<
                " This trackStub will not be created!");
    }
  }

  return trackStubs;
}

// ========================================================= GET DIRECTIONS OF FLIGHT =============================================
const std::vector<bool>
SPTCRefereeModule::getDirectionsOfFlight(const std::vector<const Belle2::SpacePoint*>& spacePoints, bool useMCInfo)
{
  std::vector<bool> dirsOfFlight; // return vector

  if (useMCInfo) {
    try {
      for (const SpacePoint* spacePoint : spacePoints) {  // loop over all SpacePoints
        if (spacePoint->getType() == VXD::SensorInfoBase::PXD) {
          dirsOfFlight.push_back(getDirOfFlightTrueHit<PXDTrueHit>(spacePoint, m_origin));
        } else if (spacePoint->getType() == VXD::SensorInfoBase::SVD) {
          dirsOfFlight.push_back(getDirOfFlightTrueHit<SVDTrueHit>(spacePoint, m_origin));
        } else throw
          SpacePointTrackCand::UnsupportedDetType(); // NOTE: should never happen, because SpacePointTrackCand can only handle PXD and SVD at the moment!
      }
    } catch (SpacePointTrackCand::UnsupportedDetType& anE) {
      B2FATAL("Caught a fatal exception while checking if a SpacePointTrackCand curls: " <<
              anE.what()); // FATAL because if this happens this needs some time to implement and it affects more than only this module!
    }
  } else {
    dirsOfFlight = getDirsOfFlightSpacePoints(spacePoints, m_origin);
  }

  return dirsOfFlight;
}

// ================================================ GET DIRECTION OF FLIGHT FROM TRUEHIT ==========================================
template <typename TrueHitType>
bool SPTCRefereeModule::getDirOfFlightTrueHit(const Belle2::SpacePoint* spacePoint, B2Vector3F origin)
{
  TrueHitType* trueHit = spacePoint->template getRelatedTo<TrueHitType>("ALL"); // COULDDO: search only certain arrays

  if (trueHit == NULL) { B2ERROR("Found no TrueHit to SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName()); }

  // get SensorId - needed for transforming local to global coordinates
  VxdID vxdID = trueHit->getSensorID();

  const VXD::SensorInfoBase& sensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(vxdID);
  B2Vector3F position = sensorInfoBase.pointToGlobal(B2Vector3F(trueHit->getU(), trueHit->getV(), 0)); // global position
  B2Vector3F momentum = sensorInfoBase.vectorToGlobal(trueHit->getMomentum()); // global momentum

  B2DEBUG(150, "Getting the direction of flight for SpacePoint " << spacePoint->getArrayIndex() << ", related to TrueHit " <<
          trueHit->getArrayIndex() << ". Both are on Sensor " << vxdID << ". (TrueHit) Position: (" << position.x() << "," << position.y() <<
          "," << position.z() << "), (TrueHit) Momentum: (" << momentum.x() << "," << momentum.y() << "," << momentum.z() << ")");

  return getDirOfFlightPosMom(position, momentum, origin);
}

// ==================================================== GET DIRECTION OF FLIGHT FROM SPACEPOINT ===================================
std::vector<bool>
SPTCRefereeModule::getDirsOfFlightSpacePoints(const std::vector<const Belle2::SpacePoint*>& spacePoints, B2Vector3F origin)
{
  std::vector<bool> dirsOfFlight; // return vector

  B2Vector3F oldPosition = origin; // assumption: first position is origin
  for (unsigned int iSP = 0; iSP < spacePoints.size(); ++iSP) {
    B2Vector3F position = spacePoints.at(iSP)->getPosition();
    // estimate momentum by linearizing between old position and new position -> WARNING: not a very good estimate!!!
    B2Vector3F momentumEst = position - oldPosition;
    B2DEBUG(150, "Getting the direction of flight for SpacePoint " << spacePoints.at(iSP)->getArrayIndex() << ". Position: (" <<
            position.x() << "," << position.y() << "," << position.z() << "), estimated momentum: (" << momentumEst.x() << "," <<
            momentumEst.y() << "," << momentumEst.z() << ")");
    dirsOfFlight.push_back(getDirOfFlightPosMom(position, momentumEst, origin));
    oldPosition = position; // reassign for next round
  }

  return dirsOfFlight;
}

// =============================================== GET DIRECTION OF FLIGHT FROM POSITION AND MOMENTUM =============================
bool SPTCRefereeModule::getDirOfFlightPosMom(B2Vector3F position, B2Vector3F momentum, B2Vector3F origin)
{
  // calculate the positon relative to the set origin, and add the momentum to the position to get the direction of flight
  B2Vector3F originToHit = position - origin;
  B2Vector3F momentumAtHit = originToHit + momentum;

  B2DEBUG(250, "Position relative to origin: (" << originToHit.x() << "," << originToHit.y() << "," << originToHit.z() <<
          "). Momentum (origin set to position of hit relative to origin): (" << momentumAtHit.x() << "," << momentumAtHit.y() << "," <<
          momentumAtHit.z() << ").");

  // get the radial components (resp. the square values) in cylindrical coordinates and compare them to make a decision on the direction of flight
  float hitRadComp = originToHit.Perp2(); // using perp2 because possibly faster and only interested in ratio
  float hitMomRadComp = momentumAtHit.Perp2();
  B2DEBUG(250, "squared radial component of hit coordinates: " << hitRadComp <<
          ", squared radial component of tip of momentum vector with its origin set to hit position: " << hitMomRadComp);

  if (hitMomRadComp < hitRadComp) {
    B2DEBUG(100, "Direction of flight is inwards for this hit");
    return false;
  } else {
    B2DEBUG(100, "Direction of flight is outwards for this hit");
    return true;
  }
}

// ============================================ COPY TO NEW STORE ARRAY ===========================================================
void SPTCRefereeModule::copyToNewStoreArray(const Belle2::SpacePointTrackCand* trackCand,
                                            Belle2::StoreArray<Belle2::SpacePointTrackCand> newStoreArray)
{
  SpacePointTrackCand* newTC = newStoreArray.appendNew(*trackCand);
  newTC->addRelationTo(trackCand);
  B2DEBUG(20, "Added new SPTC to StoreArray " << newStoreArray.getName() << " and registered relation to SPTC " <<
          trackCand->getArrayIndex() << " from Array " << trackCand->getArrayName());
}

// =================================================== ADD TO STORE ARRAY =========================================================
void SPTCRefereeModule::addToStoreArray(const Belle2::SpacePointTrackCand& trackCand,
                                        Belle2::StoreArray<Belle2::SpacePointTrackCand> storeArray,
                                        const Belle2::SpacePointTrackCand* origTrackCand)
{
  SpacePointTrackCand* newTC = storeArray.appendNew(trackCand);
  newTC->addRelationTo(origTrackCand);
  B2DEBUG(20, "Added new SPTC to StoreArray " << storeArray.getName() << " and registered relation to SPTC " <<
          origTrackCand->getArrayIndex() << " from Array " << origTrackCand->getArrayName());
  m_regTrackStubsCtr++;
}

// ======================================================== GET CHECK STATUS ======================================================
unsigned short int SPTCRefereeModule::getCheckStatus(const Belle2::SpacePointTrackCand* trackCand)
{
  unsigned short int status = trackCand->getRefereeStatus();
  if (trackCand->hasRefereeStatus(SpacePointTrackCand::c_hitsLowDistance)) status -= SpacePointTrackCand::c_hitsLowDistance;
  if (trackCand->hasRefereeStatus(SpacePointTrackCand::c_hitsOnSameSensor)) status -= SpacePointTrackCand::c_hitsOnSameSensor;
  if (trackCand->hasRefereeStatus(SpacePointTrackCand::c_removedHits)) status -= SpacePointTrackCand::c_removedHits;
  return status;
}
