/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/SPTCRefereeModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <vxd/dataobjects/VxdID.h>

#include <tracking/vectorTools/B2Vector3.h> // use TVector3 instead?
#include <boost/range/adaptor/reversed.hpp> // for ranged based loops in reversed order

using namespace Belle2;
using namespace std;

REG_MODULE(SPTCReferee) // register the module

SPTCRefereeModule::SPTCRefereeModule() : Module()
{
  setDescription("Module that does some sanity checks on SpacePointTrackCands to prevent some problematic cases to be forwarded to other modules that rely on 'unproblematic' cases (e.g. FilterCalculator). Different checks can be enabled by setting the according flags. Using MC information for the tests can also be switched on/off for tests where MC information can be helpful. NOTE: module is currently under developement!");

  // names
  addParam("sptcName", m_PARAMsptcName, "Container name of the SpacePointTrackCands to be checked (input)", std::string(""));
  addParam("newArrayName", m_PARAMnewArrayName, "Container name of SpacePointTrackCands if 'storeNewArray' is set to true", std::string(""));

  // flags
  addParam("checkSameSensor", m_PARAMcheckSameSensor, "Check if two subsequent SpacePoints are on the same sensor", true);
  addParam("checkMinDistance", m_PARAMcheckMinDistance, "Check if two subsequent SpacePoints are seperated by more than 'minDistance'", true);
  addParam("checkCurling", m_PARAMcheckCurling, "Check the SpacePointTrackCand for curling behaviour and mark it as curling if it does", true);
  addParam("splitCurlers", m_PARAMsplitCurlers, "Split curling SpacePointTrackCands and save the TrackStubs in seperate StoreArrays", false);
  addParam("keepOnlyFirstPart", m_PARAMkeepOnlyFirstPart, "Keep only the first part of a curling SpacePointTrackCand (e.g. when only this is needed)", false);
  addParam("useMCInfo", m_PARAMuseMCInfo, "Set to true if the use of MC information (e.g. from underlying TrueHits) for the checks is wanted, and to false if the checks should all be done with information that can be obtained from SpacePoints directly. NOTE: the tests without MC information have to be developed first!", true);
  addParam("kickSpacePoint", m_PARAMkickSpacePoint, "Set to true if only the 'problematic' SpacePoint shall be kicked and not the whole SpacePointTrackCand", false);
  addParam("storeNewArray", m_PARAMstoreNewArray, "Set to true if the checked SpacePointTrackCands should be stored in a new StoreArray. WARNING: all previously registered relations get lost in this way!", true);

  // other
  addParam("minDistance", m_PARAMminDistance, "Minimal Distance [cm] that two subsequent SpacePoints have to be seperated if 'checkMinDistance' is enabled", double(0));
  std::vector<double> defaultOrigin = { 0., 0., 0. };
  addParam("setOrigin", m_PARAMsetOrigin, "WARNING: still need to find out the units that are used internally! Reset origin to given point. Used for determining the direction of flight of a particle for a given hit. Needs to be reset for e.g. testbeam, where origin is not at (0,0,0)", defaultOrigin);

  // initialize counters (cppcheck)
  initializeCounters();
}

// ======================================================================= INITIALIZE ========================================================================
void SPTCRefereeModule::initialize()
{
  B2INFO("SPTCReferee::initialize(): ------------------------------------------------ ")
  // check if StoreArray of SpacePointTrackCands is her
  StoreArray<SpacePointTrackCand> inputSpacePoints(m_PARAMsptcName);
  inputSpacePoints.isRequired(m_PARAMsptcName);

  // register new StoreArray
  if (m_PARAMstoreNewArray) {
    StoreArray<SpacePointTrackCand> newStoreArray(m_PARAMnewArrayName);
    newStoreArray.registerInDataStore(m_PARAMnewArrayName, DataStore::c_DontWriteOut);
    newStoreArray.registerRelationTo(inputSpacePoints, DataStore::c_Event, DataStore::c_DontWriteOut);
  }

  // sanity checks on the other parameters
  if (m_PARAMcheckMinDistance) {
    if (m_PARAMminDistance < 0) {
      B2WARNING("minDistance set to value below 0: " << m_PARAMminDistance << ", Taking the absolute value and resetting 'minDistance' to that!")
      m_PARAMminDistance = -m_PARAMminDistance;
    }
  }

  B2DEBUG(1, "Provided Parameters: checkSameSensor - " << m_PARAMcheckSameSensor << ", checkMinDistance - " << m_PARAMcheckMinDistance << ", checkCurling - " << m_PARAMcheckCurling << ", splitCurlers - " << m_PARAMsplitCurlers << ", keepOnlyFirstPart - " << m_PARAMkeepOnlyFirstPart << ", useMCInfo - " << m_PARAMuseMCInfo << ", kickSpacePoint - " << m_PARAMkickSpacePoint)

  if (m_PARAMsetOrigin.size() != 3) {
    B2WARNING("CurlingTrackCandSplitter::initialize: Provided origin is not a 3D point! Please provide 3 values (x,y,z). Rejecting user input and setting origin to (0,0,0) for now!");
    m_PARAMsetOrigin.clear();
    m_PARAMsetOrigin.assign(3, 0);
  }
  m_origin.SetXYZ(m_PARAMsetOrigin.at(0), m_PARAMsetOrigin.at(1), m_PARAMsetOrigin.at(2));
  B2DEBUG(10, "Set origin to (x,y,z): (" << m_origin.X() << "," << m_origin.Y() << "," << m_origin.Z() << ")");

  initializeCounters();
}

// ======================================================================== EVENT =============================================================================
void SPTCRefereeModule::event()
{
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

    unsigned short int refereeStatus = 0; // collect the referee status throughout this loop and set it at the end -> COULDDO: (probably less error prone, use addRefereeStatus instead to add each status seperately)
    bool removedTrueHits = false; // to avoid adding this more than once, via the bitfield (would lead to wrong results!)

    if (m_PARAMcheckSameSensor) { // check same sensors if desired
      refereeStatus += SpacePointTrackCand::c_checkedSameSensors;
      const std::vector<int> sameSonsorInds = checkSameSensor(trackCand);
      if (!sameSonsorInds.empty()) {
        m_SameSensorCtr++;
        if (m_PARAMkickSpacePoint) {
          try {
            for (int index : boost::adaptors::reverse(sameSonsorInds)) { // have to start from highest index, as the vector in the trackCand is resized in this step!
              trackCand->removeSpacePoint(index);
              m_kickedSpacePointsCtr++;
            }
            removedTrueHits = true;
          } catch (SpacePointTrackCand::SPTCIndexOutOfBounds& anE) {
            B2WARNING("Caught an Exception while trying to remove a SpacePoint from a SpacePointTrackCand: " << anE.what());
          }
        } else { refereeStatus += SpacePointTrackCand::c_hitsOnSameSensor; } // only add status if the SpacePoints on the same sensors have not been removed!
      } else {
        B2DEBUG(20, "Found no two subsequent SpacePoints on the same sensor for this SpacePointTrackCand (" << iTC << " in Array " << trackCands.getName() << ")")
      }
      B2DEBUG(30, "refereeStatus after checkSameSensor " << refereeStatus);
    }

    if (m_PARAMcheckMinDistance) { // check min distance if desired
      refereeStatus += SpacePointTrackCand::c_checkedMinDistance;
      const std::vector<int> lowDistanceInds = checkMinDistance(trackCand, m_PARAMminDistance);
      if (!lowDistanceInds.empty()) {
        m_minDistanceCtr++;
        if (m_PARAMkickSpacePoint) {
          try {
            for (int index : boost::adaptors::reverse(lowDistanceInds)) { // have to start from highest index, as the vector in the trackCand is resized in this step!
              trackCand->removeSpacePoint(index);
              m_kickedSpacePointsCtr++;
            }
            removedTrueHits = true;
          } catch (SpacePointTrackCand::SPTCIndexOutOfBounds& anE) {
            B2WARNING("Caught an Exception while trying to remove a SpacePoint from a SpacePointTrackCand: " << anE.what());
          }
        } else { refereeStatus += SpacePointTrackCand::c_hitsLowDistance; } // only add status if the SpacePoints not far enough apart have not been removed!
      } else {
        B2DEBUG(20, "Found no two subsequent SpacePoints that were closer than " << m_PARAMminDistance << " cm together for this SpacePointTrackCand (" << iTC << " in Array " << trackCands.getName() << ")")
      }
      B2DEBUG(30, "refereeStatus after checkMinDistance " << refereeStatus);
    }


    // momentary solution!! this will be changed (encapsulated in /////// )
    std::vector<SpacePointTrackCand> curlingTrackStubs; // vector of TrackStubs that shall be saved to another StoreArray
    if (m_PARAMcheckCurling) { // check curling if desired
      // Only do curling checking if useMCInfo is false, OR if useMCInfo is true if the SPTCs SpacePoints have been checked for a relation to TrueHits!
      if (!m_PARAMuseMCInfo || trackCand->hasRefereeStatus(SpacePointTrackCand::c_checkedTrueHits)) {
        const std::vector<int> curlingSplitInds = checkCurling(trackCand, m_PARAMuseMCInfo);
        if (!curlingSplitInds.empty()) {
          refereeStatus += SpacePointTrackCand::c_curlingTrack;
          m_curlingTracksCtr++;
          if (m_PARAMsplitCurlers) {
            // another sanity check // TODO TODO TODO: check why this is even neccessary!
            if (!(curlingSplitInds.at(0) == 0 && curlingSplitInds.size() == 1)) {
              const std::vector<SpacePointTrackCand> trackStubs = splitTrackCand(trackCand, curlingSplitInds, m_PARAMkeepOnlyFirstPart);
              if (!trackStubs.empty()) {
                ////////////////////////////////////////////////////
                for (auto trackStub : trackStubs) {
                  curlingTrackStubs.push_back(trackStub);
                }
                ///////////////////////////////////////////////////
              } else { B2ERROR("The vector returned by splitTrackCand is empty!") } // safety measure
            } else { B2ERROR("The only entry in the return vector of checkCurling is 0! Cannot Split this SPTC!") }
          }
        } else {
          B2DEBUG(20, "SpacePointTrackCand " << trackCand->getArrayIndex() << " is not curling!");
        }
      } else {
        B2WARNING("'useMCInfo' is set to true, but SpacePoints of SPTC have not been checked for relations to TrueHits! Not Checking this SPTC for curling!")
      }
    }

    // assign the referee status to the SpacePointTrackCand
    if (removedTrueHits) refereeStatus += SpacePointTrackCand::c_removedHits;
    if (m_PARAMcheckMinDistance && m_PARAMcheckSameSensor && m_PARAMcheckCurling) refereeStatus += SpacePointTrackCand::c_checkedByReferee; // CAUTION: if there are new tests implemented this has to be updated!!!
    B2DEBUG(20, "referee Status that will be assigned to trackCand " << refereeStatus);
    trackCand->addRefereeStatus(refereeStatus);
    if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 200, PACKAGENAME())) { trackCand->print(); }

    //////////////////////////////////////////
    for (auto trackStub : curlingTrackStubs) { trackStub.addRefereeStatus(refereeStatus); }
    //////////////////////////////////////////

    if (m_PARAMstoreNewArray) {
      StoreArray<SpacePointTrackCand> newArray(m_PARAMnewArrayName);
      if (trackCand->checkedForCurling() && !trackCand->isCurling()) {
        SpacePointTrackCand* newTC = newArray.appendNew(*trackCand);
        newTC->addRelationTo(trackCand);
        B2DEBUG(20, "Added new SPTC to StoreArray " << newTC->getArrayName() << " and registered relation to SPTC " << trackCand->getArrayIndex());
      } else {
        for (auto trackStub : curlingTrackStubs) {
          SpacePointTrackCand* newTC = newArray.appendNew(trackStub);
          newTC->addRelationTo(trackCand);
          B2DEBUG(20, "Added new TrackStub to StoreArray " << newTC->getArrayName() << " and registered relation to SPTC " << trackCand->getArrayIndex())
          m_regTrackStubsCtr++;
        }
      }
    }
  }
}

// ============================================================================= TERMINATE =====================================================================
void SPTCRefereeModule::terminate()
{
  // TODO: info output more sophisticated
  stringstream summary;
  if (m_PARAMcheckSameSensor) { summary << "Checked for consecutive SpacePoints on same sensor and found " << m_SameSensorCtr << " TrackCands showing this behavior."; }
  if (m_PARAMcheckMinDistance) { summary << "Checked for minimal distance between two consecutive SpacePoints and found " << m_minDistanceCtr << " TrackCands with SpacePoints not far enough apart."; }
  if (m_PARAMkickSpacePoint) { summary << m_kickedSpacePointsCtr << " SpacePoints have been removed from SpacePointTrackCands\n"; }
  if (m_PARAMcheckCurling) { summary << m_curlingTracksCtr << " SPTCs were curling. Registered " << m_regTrackStubsCtr << " track stubs. 'splitCurlers' was set to " << m_PARAMsplitCurlers << "."; }
  B2INFO("SPTCRefere::terminate(): Module got " << m_totalTrackCandCtr << " SpacePointTrackCands. " << summary.str())
  if (!m_PARAMuseMCInfo && m_PARAMcheckCurling) { B2WARNING("The curling checking without MC Information is at the moment at a very crude and unsophisticated state. If you have MC information available you should use it to do this check!"); }
}

// ====================================================================== CHECK SAME SENSORS ====================================================================
const std::vector<int> SPTCRefereeModule::checkSameSensor(Belle2::SpacePointTrackCand* trackCand)
{
  B2DEBUG(25, "Checking SpacePointTrackCand " << trackCand->getArrayIndex() << " from Array " << trackCand->getArrayName() << " for consecutive SpacePoints on the same sensor")
  std::vector<int> sameSensorInds; // return vector

  std::vector<const SpacePoint*> spacePoints = trackCand->getHits();
  VxdID lastSensorId = spacePoints.at(0)->getVxdID();

  for (unsigned int iSp = 1; iSp < spacePoints.size(); ++iSp) {
    VxdID sensorId = spacePoints.at(iSp)->getVxdID();
    B2DEBUG(50, "Checking SpacePoint " << iSp << ". SensorId of this SpacePoint: " << sensorId << ", SensorId of last SpacePoint: " << lastSensorId)
    if (sensorId == lastSensorId) {
      sameSensorInds.push_back(iSp - 1); // push back the index of the first SpacePoint (50:50 chance of getting the right one without further testing) -> retrieving the other index is no big science from this index!!
      B2DEBUG(30, "SpacePoint " << iSp << " and " << iSp - 1 << " are on the same sensor: " << sensorId)
    }
    lastSensorId = sensorId;
  }

  return sameSensorInds;
}

// ========================================================================= CHECK MIN DISTANCE =================================================================
const std::vector<int> SPTCRefereeModule::checkMinDistance(Belle2::SpacePointTrackCand* trackCand, double minDistance)
{
  B2DEBUG(25, "Checking the distances between consecutive SpacePoints for SpacePointTrackCand " << trackCand->getArrayIndex() << " from Array " << trackCand->getArrayIndex())
  std::vector<int> lowDistanceInds; // return vector

  std::vector<const SpacePoint*> spacePoints = trackCand->getHits();
  B2Vector3F oldPosition = spacePoints.at(0)->getPosition();

  for (unsigned int iSp = 1; iSp < spacePoints.size(); ++iSp) {
    B2Vector3F position = spacePoints.at(iSp)->getPosition();
    B2Vector3F diffPos = oldPosition - position;
    B2DEBUG(60, "Position of SpacePoint " << iSp << ": (" << position.X() << "," << position.Y() << "," << position.Z() << "), Position of SpacePoint " << iSp - 1 << ": (" << oldPosition.X() << "," << oldPosition.Y() << "," << oldPosition.Z() << ") --> old - new = (" << diffPos.X() << "," << diffPos.Y() << "," << diffPos.Z() <<  ")")

    if (diffPos.Mag() <= minDistance) {
      B2DEBUG(30, "Position difference is " << diffPos.Mag() <<  " but minDistance is set to " << minDistance << ". SpacePoints: " << iSp << " and " << iSp - 1)
      lowDistanceInds.push_back(iSp); // push back the index of the first SpacePoint (50:50 chance of getting the right one without further testing)
    }
    oldPosition = position;
  }

  return lowDistanceInds;
}

// ============================================================================= CHECK CURLING ======================================================================
const std::vector<int> SPTCRefereeModule::checkCurling(Belle2::SpacePointTrackCand* trackCand, bool useMCInfo)
{
  std::vector<int> splitInds; // return vector

  std::string mcInfoStr = useMCInfo ? std::string("with") : std::string("without");
  B2DEBUG(25, "Checking SpacePointTrackCand " << trackCand->getArrayIndex() << " from Array " << trackCand->getArrayName() << " for curling behavior " << mcInfoStr << " MC Information");

  // get the SpacePoints of the TrackCand
  const std::vector<const SpacePoint*>& tcSpacePoints = trackCand->getHits();
  B2DEBUG(50, "SPTC has " << tcSpacePoints.size() << " SpacePoints");

  const std::vector<bool> dirsOfFlight = getDirectionsOfFlight(tcSpacePoints, useMCInfo); // get the directions of flight for every SpacePoint

//   if(trackCand->getNHits() != dirsOfFlight.size()) B2FATAL("did not get a direction of flight for every SpacePoint") // should not /cannot happen

  // loop over all entries of dirsOfFlight and compare them pair-wise. If they change -> add Index to splitInds.
  if (!dirsOfFlight.at(0)) splitInds.push_back(0); // if the direction of flight is inwards for the first hit, push_back 0 -> make information accessible from outside this function
  for (unsigned int i = 1; i < dirsOfFlight.size(); ++i) {
    if (dirsOfFlight.at(i) ^ dirsOfFlight.at(i - 1)) splitInds.push_back(i); // NOTE: using the bitoperator for XOR here to determine if the bools differ!
  }
  return splitInds;
}

// ============================================================ SPLIT CURLING TRACK CAND ====================================================================
const std::vector<Belle2::SpacePointTrackCand>
SPTCRefereeModule::splitTrackCand(const Belle2::SpacePointTrackCand* trackCand, const std::vector<int>& splitIndices, bool onlyFirstPart)
{
  std::vector<SpacePointTrackCand> trackStubs; // return vector

  B2DEBUG(25, "Splitting SpacePointTrackCand " << trackCand->getArrayIndex() << " from Array " << trackCand->getArrayName() << " number of entries in splitIndices " << splitIndices.size())
//   int trackStub = 0;
  bool dirOfFlight = splitIndices.at(0) != 0; // if first entry is zero the direction of flight is false (= ingoing)

  for (int entry : splitIndices) { B2DEBUG(100, entry) }

  int firstLast = dirOfFlight ? splitIndices.at(0) : splitIndices.at(1); // if the first entry of splitIndices is zero the first TrackStub is from 0 to second entry instead of from 0 to first entry
  std::vector<std::pair<int, int> > rangeIndices; // .first is starting, .second is final index for each TrackStub. Store them in vector to be able to easily loop over them
  rangeIndices.push_back(std::make_pair(0, firstLast));

  if (!onlyFirstPart) { // if more than the first part is desired push_back the other ranges too
    unsigned int iStart = dirOfFlight ? 1 : 2;
    for (unsigned int i = iStart; i < splitIndices.size(); ++i) {
      rangeIndices.push_back(std::make_pair(splitIndices.at(i - 1), splitIndices.at(i)));
    }
  }

  B2DEBUG(30, "There will be " << rangeIndices.size() << " TrackStubs created for this TrackCand. (size of the passes splitIndices: " << splitIndices.size() << ", onlyFirstPart " << onlyFirstPart);


  // loop over all entries in range indices and create a SpacePointTrackCand from it
  for (unsigned int iTs = 0; iTs < rangeIndices.size(); ++iTs) {
    int firstInd = rangeIndices.at(iTs).first;
    int lastInd = rangeIndices.at(iTs).second;

    // encapsulate in try block to catch indices out of range
    try {
      const std::vector<const SpacePoint*> spacePoints = trackCand->getHitsInRange(firstInd, lastInd);
      const std::vector<double> sortingParams = trackCand->getSortingParametersInRange(firstInd, lastInd);

      SpacePointTrackCand trackStub(spacePoints, trackCand->getPdgCode(), trackCand->getChargeSeed(), trackCand->getMcTrackID()); // create new TrackCand
      trackStub.setSortingParameters(sortingParams);

      // set the state seed and the cov seed only for the first trackStub of the TrackCand
      if (iTs < 1) {
        trackStub.set6DSeed(trackCand->getStateSeed());
        trackStub.setCovSeed(trackCand->getCovSeed());
      }

      // set the direction of flight and flip it afterwards, because next trackCand hs changed direction of flight
      trackStub.setFlightDirection(dirOfFlight);
      dirOfFlight = !dirOfFlight;

      trackStub.setTrackStubIndex(iTs + 1); // trackStub index starts at 1 for curling SPTCs. NOTE: this might be subject to chagnes with the new bitfield in SpacePointTrackCand

      trackStubs.push_back(trackStub);
    } catch (SpacePointTrackCand::SPTCIndexOutOfBounds& anE) {
      B2WARNING("Caught an exception while trying to split a curling SpacePointTrackCand: " << anE.what() << " This trackStub will not be created!");
    }
  }

  return trackStubs;
}

// ========================================================= GET DIRECTIONS OF FLIGHT =================================================================================
const std::vector<bool> SPTCRefereeModule::getDirectionsOfFlight(const std::vector<const Belle2::SpacePoint*>& spacePoints, bool useMCInfo)
{
  std::vector<bool> dirsOfFlight; // return vector

  if (useMCInfo) {
    try {
      for (const SpacePoint * spacePoint : spacePoints) { // loop over all SpacePoints
        if (spacePoint->getType() == VXD::SensorInfoBase::PXD) { dirsOfFlight.push_back(getDirOfFlightTrueHit<PXDTrueHit>(spacePoint, m_origin)); }
        else if (spacePoint->getType() == VXD::SensorInfoBase::SVD) { dirsOfFlight.push_back(getDirOfFlightTrueHit<SVDTrueHit>(spacePoint, m_origin)); }
        else throw SpacePointTrackCand::UnsupportedDetType(); // NOTE: should never happen, because SpacePointTrackCand can only handle PXD and SVD at the moment!
      }
    } catch (SpacePointTrackCand::UnsupportedDetType& anE) {
      B2FATAL("Caught a fatal exception while checking if a SpacePointTrackCand curls: " << anE.what()) // FATAL because if this happens this needs some time to implement and it affects more than only this module!
    }
  } else {
    dirsOfFlight = getDirsOfFlightSpacePoints(spacePoints, m_origin);
  }

  return dirsOfFlight;
}

// ================================================ GET DIRECTION OF FLIGHT FROM TRUEHIT =================================================================================
template <typename TrueHitType>
bool SPTCRefereeModule::getDirOfFlightTrueHit(const Belle2::SpacePoint* spacePoint, B2Vector3F origin)
{
  TrueHitType* trueHit = spacePoint->template getRelatedTo<TrueHitType>("ALL"); // COULDDO: search only certain arrays

  if (trueHit == NULL) { B2ERROR("Found no TrueHit to SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName()) }

  // get SensorId - needed for transforming local to global coordinates
  VxdID vxdID = trueHit->getSensorID();

  const VXD::SensorInfoBase& sensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(vxdID); // get the sensorInfoBase to use pointToGlobal and VectorToGlobal
  B2Vector3F position = sensorInfoBase.pointToGlobal(B2Vector3F(trueHit->getU(), trueHit->getV(), 0)); // global position
  B2Vector3F momentum = sensorInfoBase.vectorToGlobal(trueHit->getMomentum()); // global momentum

  B2DEBUG(150, "Getting the direction of flight for SpacePoint " << spacePoint->getArrayIndex() << ", related to TrueHit " << trueHit->getArrayIndex() << ". Both are on Sensor " << vxdID << ". (TrueHit) Position: (" << position.x() << "," << position.y() << "," << position.z() << "), (TrueHit) Momentum: (" << momentum.x() << "," << momentum.y() << "," << momentum.z() << ")");

  return getDirOfFlightPosMom(position, momentum, origin);
}

// ==================================================== GET DIRECTION OF FLIGHT FROM SPACEPOINT ==============================================================================
std::vector<bool> SPTCRefereeModule::getDirsOfFlightSpacePoints(const std::vector<const Belle2::SpacePoint*>& spacePoints, B2Vector3F origin)
{
  std::vector<bool> dirsOfFlight; // return vector

  B2Vector3F oldPosition = origin; // assumption: first position is origin
  for (unsigned int iSP = 0; iSP < spacePoints.size(); ++iSP) {
    B2Vector3F position = spacePoints.at(iSP)->getPosition();
    B2Vector3F momentumEst = position - oldPosition; // estimate momentum by linearizing between old position and new position -> WARNING: not a very good estimate!!!
    B2DEBUG(150, "Getting the direction of flight for SpacePoint " << spacePoints.at(iSP)->getArrayIndex() << ". Position: (" << position.x() << "," << position.y() << "," << position.z() << "), estimated momentum: (" << momentumEst.x() << "," << momentumEst.y() << "," << momentumEst.z() << ")");
    dirsOfFlight.push_back(getDirOfFlightPosMom(position, momentumEst, origin));
    oldPosition = position; // reassign for next round
  }

  return dirsOfFlight;
}

// ============================================================ GET DIRECTION OF FLIGHT FROM POSITION AND MOMENTUM ==========================================================
bool SPTCRefereeModule::getDirOfFlightPosMom(B2Vector3F position, B2Vector3F momentum, B2Vector3F origin)
{
  // calculate the positon relative to the set origin, and add the momentum to the position to get the direction of flight
  B2Vector3F originToHit = position - origin;
  B2Vector3F momentumAtHit = originToHit + momentum;

  B2DEBUG(250, "Position relative to origin: (" << originToHit.x() << "," << originToHit.y() << "," << originToHit.z() << "). Momentum (origin set to position of hit relative to origin): (" << momentumAtHit.x() << "," << momentumAtHit.y() << "," << momentumAtHit.z() << ").");

  // get the radial components (resp. the square values) in cylindrical coordinates and compare them to make a decision on the direction of flight
  float hitRadComp = originToHit.Perp2(); // using perp2 because possibly faster and only interested in ratio
  float hitMomRadComp = momentumAtHit.Perp2();
  B2DEBUG(250, "squared radial component of hit coordinates: " << hitRadComp << ", squared radial component of tip of momentum vector with its origin set to hit position: " << hitMomRadComp);

  if (hitMomRadComp < hitRadComp) {
    B2DEBUG(100, "Direction of flight is inwards for this hit");
    return false;
  } else {
    B2DEBUG(100, "Direction of flight is outwards for this hit");
    return true;
  }
}