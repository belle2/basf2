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

  // flags
  addParam("checkSameSensor", m_PARAMcheckSameSensor, "Check if two subsequent SpacePoints are on the same sensor", true);
  addParam("checkMinDistance", m_PARAMcheckMinDistance, "Check if two subsequent SpacePoints are seperated by more than 'minDistance'", true);
  addParam("checkCurling", m_PARAMcheckCurling, "Check the SpacePointTrackCand for curling behaviour and mark it as curling if it does", true);
  addParam("splitCurlers", m_PARAMsplitCurlers, "Split curling SpacePointTrackCands and save the TrackStubs in seperate StoreArrays", false);
  addParam("keepOnlyFirstPart", m_PARAMkeepOnlyFirstPart, "Keep only the first part of a curling SpacePointTrackCand (e.g. when only this is needed)", false);
  addParam("useMCInfo", m_PARAMuseMCInfo, "Set to true if the use of MC information (e.g. from underlying TrueHits) for the checks is wanted, and to false if the checks should all be done with information that can be obtained from SpacePoints directly. NOTE: the tests without MC information have to be developed first!", true);
  addParam("kickSpacePoint", m_PARAMkickSpacePoint, "Set to true if only the 'problematic' SpacePoint shall be kicked and not the whole SpacePointTrackCand", false);

  // other
  addParam("minDistance", m_PARAMminDistance, "Minimal Distance [cm] that two subsequent SpacePoints have to be seperated if 'checkMinDistance' is enabled", double(0));
}

// ======================================================================= INITIALIZE ========================================================================
void SPTCRefereeModule::initialize()
{
  B2INFO("SPTCReferee::initialize(): ------------------------------------------------ ")
  // check if StoreArray of SpacePointTrackCands is her
  StoreArray<SpacePointTrackCand>::required(m_PARAMsptcName);

  // sanity checks on the other parameters
  if (m_PARAMcheckMinDistance) {
    if (m_PARAMminDistance < 0) {
      B2WARNING("minDistance set to value below 0: " << m_PARAMminDistance << ", Taking the absolute value and resetting 'minDistance' to that!")
      m_PARAMminDistance = -m_PARAMminDistance;
    }
  }

  B2DEBUG(1, "Provided Parameters: checkSameSensor - " << m_PARAMcheckSameSensor << ", checkMinDistance - " << m_PARAMcheckMinDistance << ", checkCurling - " << m_PARAMcheckCurling << ", splitCurlers - " << m_PARAMsplitCurlers << ", keepOnlyFirstPart - " << m_PARAMkeepOnlyFirstPart << ", useMCInfo - " << m_PARAMuseMCInfo << ", kickSpacePoint - " << m_PARAMkickSpacePoint)

  if (m_PARAMcheckCurling || m_PARAMsplitCurlers || m_PARAMkeepOnlyFirstPart) {
    B2WARNING("Curling Checking is not yet implemented! 'checkCurling', 'splitCurlers' and 'keepOnlyFirstPart' have no impact on the behavior at the moment!")
  }
  if (m_PARAMuseMCInfo) {
    B2WARNING("'useMCInfo' is set to true, but there is not yet a feature that could use MC information!")
  }
  if (m_PARAMkickSpacePoint) {
    B2WARNING("'kickSpacePoint' is set to true, but the module kicks the whole SpacePointTrackCand at the moment if one of the checks fails!")
  }

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
        refereeStatus += SpacePointTrackCand::c_hitsOnSameSensor;
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
        }
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
        refereeStatus += SpacePointTrackCand::c_hitsLowDistance;
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
        }
      } else {
        B2DEBUG(20, "Found no two subsequent SpacePoints that were closer than " << m_PARAMminDistance << " cm together for this SpacePointTrackCand (" << iTC << " in Array " << trackCands.getName() << ")")
      }
      B2DEBUG(30, "refereeStatus after checkMinDistance " << refereeStatus);
    }

    // assign the referee status to the SpacePointTrackCand
    if (removedTrueHits) refereeStatus += SpacePointTrackCand::c_removedHits;
    if (m_PARAMcheckMinDistance && m_PARAMcheckSameSensor) refereeStatus += SpacePointTrackCand::c_checkedByReferee; // CAUTION: if there are new tests implemented this has to be updated!!!
    B2DEBUG(20, "referee Status that will be assigned to trackCand " << refereeStatus);
    trackCand->setRefereeStatus(refereeStatus);
    if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 200, PACKAGENAME())) { trackCand->print(); }
  }
}

// ============================================================================= TERMINATE =====================================================================
void SPTCRefereeModule::terminate()
{
  stringstream summary;
  if (m_PARAMcheckSameSensor) { summary << "Checked for consecutive SpacePoints on same sensor and found " << m_SameSensorCtr << " TrackCands showing this behavior."; }
  if (m_PARAMcheckMinDistance) { summary << "Checked for minimal distance between two consecutive SpacePoints and found " << m_minDistanceCtr << " TrackCands with SpacePoints not far enough apart."; }
  if (m_PARAMkickSpacePoint) { summary << m_kickedSpacePointsCtr << " SpacePoints have been removed from SpacePointTrackCands\n"; }
  B2INFO("SPTCRefere::terminate(): Module got " << m_totalTrackCandCtr << " SpacePointTrackCands. " << summary.str())
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