/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/CurlingTrackCandSplitterModule.h>

// DataStore Stuff
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

// Clusters and TrueHits
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>

// SpacePoint related stuff
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

using namespace std;
using namespace Belle2;

REG_MODULE(CurlingTrackCandSplitter)

// COULDDO: somehow retrieve the names under which Clusters and TrueHits are stored in the StoreArray (possible?) Or make them a parameter of the module. Otherwise it can happen, that, if more than one StoreArray of XYZCluster is present, all of them get searched, which might be unintended (or even undefined behaviour)

CurlingTrackCandSplitterModule::CurlingTrackCandSplitterModule()
{
  setDescription("Module for checking SpacePointTrackCands for curling behaviour and (if wanted) splitting them into SpacePointTrackCands that no longer show curling behaviour");

  addParam("splitCurlers", m_PARAMsplitCurlers, "Split curling SpacePointTrackCands into non-curling SpacePointTrackCands and store them", true);
  addParam("nTrackStubs", m_PARAMnTrackStubs, "Maximum number of SpacePointTrackCand Stubs to be created from a curling SpacePointTrackCand. Set to 0 if you want all possible TrackCand Stubs", 0);

  addParam("SpacePointTCName", m_PARAMsptcName, "Collection name of the SpacePointTrackCands to be analyzed for curling behaviour", std::string(""));
  addParam("curlingFirstOutName", m_PARAMcurlingOutFirstName, "Collection name under which the first outgoing part of a curling TrackCand will be stored in the StoreArray. The first part of a curling Track has its origin at the interaction point.", std::string(""));
  addParam("curlingAllInName", m_PARAMcurlingAllInName, "Collection name under which all ingoing parts of a curling TrackCand will be stored in the StoreArray", std::string(""));
  addParam("curlingRestOutName", m_PARAMcurlingOutRestName, "Collection name under which all but the first outgoing parts of a curling TrackCand will be stored in the StoreArray", std::string(""));
  addParam("completeCurlerName", m_PARAMcompleteCurlerName, "Collection name under which all parts of a curling TrackCand will be stored in the StoreArray together. NOTE: only if this parameter is set to a non-empty string a complete (but splitted) curling TrackCand will be stored!", std::string(""));

  // WARNING TODO: find out the units that are used internally!!!
  std::vector<double> defaultOrigin = { 0., 0., 0. };
  addParam("setOrigin", m_PARAMsetOrigin, "WARNING: still need to find out the units that are used internally! Reset origin to given point. Used for determining the direction of flight of a particle for a given hit. Needs to be reset for e.g. testbeam, where origin is not at (0,0,0)", defaultOrigin);
}

// ================================================= INITIALIZE =========================================================
void CurlingTrackCandSplitterModule::initialize()
{
  initializeCounters();
  B2INFO("CurlingTrackCandSplitter ----------------------------- initialize() -------------------------------------");

  // check if all necessary StoreArrays are present
  StoreArray<SpacePointTrackCand> spacePointTCs(m_PARAMsptcName);
  spacePointTCs.required(m_PARAMsptcName);

  // count all empty input parameter strings, and issue a warning if more than one is empty (COULDDO: B2FATAL instead of warning)
  int emptyCtr = 0;
  if (m_PARAMcurlingOutFirstName.empty()) { emptyCtr++; }
  if (m_PARAMcurlingAllInName.empty()) { emptyCtr++; }
  if (m_PARAMcurlingOutRestName.empty()) { emptyCtr++; }

  if (emptyCtr > 1) {
    B2WARNING("CurlingTrackCandSplitter::initialize: More than one of your input strings for the collection names is empty. This can lead to undeterministic behaviour since two or more collections will be stored under the same name!");
  }

  // register new StoreArrays, and relation to original TrackCand
  StoreArray<SpacePointTrackCand> curlingFirstOuts(m_PARAMcurlingOutFirstName);
  curlingFirstOuts.registerPersistent(m_PARAMcurlingOutFirstName);
  curlingFirstOuts.registerRelationTo(spacePointTCs);

  StoreArray<SpacePointTrackCand> curlingAllIns(m_PARAMcurlingAllInName);
  curlingAllIns.registerPersistent(m_PARAMcurlingAllInName);
  curlingAllIns.registerRelationTo(spacePointTCs);

  StoreArray<SpacePointTrackCand> curlingRestOuts(m_PARAMcurlingOutRestName);
  curlingRestOuts.registerPersistent(m_PARAMcurlingOutRestName);
  curlingRestOuts.registerRelationTo(spacePointTCs);

  // have to do this here, because in event() I do not want to check every time if this string is empty or not and act accordingly. If I register this with an empty string here, I can use it with an empty string in event() and only store stuff into it, when it is actually named with a non-empty string
  StoreArray<SpacePointTrackCand> curlingCompletes(m_PARAMcompleteCurlerName);
  curlingCompletes.registerPersistent(m_PARAMcompleteCurlerName);
  curlingCompletes.registerRelationTo(spacePointTCs);

  if (!m_PARAMcompleteCurlerName.empty()) {
    m_saveCompleteCurler = true;
    B2DEBUG(1, "You put in " << m_PARAMcompleteCurlerName << " as collection name for complete curling TrackCands. Complete curling TrackCands will hence be stored.");
  } else {
    B2DEBUG(1, "You did not put in any under which complete curling TrackCands should be stored, hence curling TrackCands will only be stored in parts.");
    m_saveCompleteCurler = false;
  }

  // check value for nTrackStubs and reset if necessary
  if (m_PARAMnTrackStubs < 0) {
    B2WARNING("CurlingTrackCandSplitter::initialize> Value of nTrackStubs is below 0: nTrackStubs = " << m_PARAMnTrackStubs << ". Resetting this value to 0 now! This means that all parts of curling TrackCands will be stored.")
    m_PARAMnTrackStubs = 0;
  } else { B2DEBUG(1, "Entered value for nTrackStubs = " << m_PARAMnTrackStubs); }

  B2DEBUG(1, "Entered Value for splitCurlers: " << m_PARAMsplitCurlers);

  if (m_PARAMsetOrigin.size() != 3) {
    B2WARNING("CurlingTrackCandSplitter::initialize: Provided origin is not a 3D point! Please provide 3 values (x,y,z). Rejecting user input and setting origin to (0,0,0) for now!");
    m_PARAMsetOrigin.clear();
    m_PARAMsetOrigin.assign(3, 0);
  }
  m_origin.SetXYZ(m_PARAMsetOrigin.at(0), m_PARAMsetOrigin.at(1), m_PARAMsetOrigin.at(2));
  B2DEBUG(10, "Set origin to (x,y,z): (" << m_origin.X() << "," << m_origin.Y() << "," << m_origin.Z() << ")");
}

// =================================================== EVENT ============================================================
void CurlingTrackCandSplitterModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(10, "CurlingTrackCandSplitter::event(). -------------- Processing event " << eventCounter << " ----------------");

  // StoreArrays that will be used for storing
  StoreArray<SpacePointTrackCand> outgoingFirstTCs(m_PARAMcurlingOutFirstName);
  StoreArray<SpacePointTrackCand> ingoingAllTCs(m_PARAMcurlingAllInName);
  StoreArray<SpacePointTrackCand> outgoingRestTCs(m_PARAMcurlingOutRestName);
  StoreArray<SpacePointTrackCand> completeCurlingTCs(m_PARAMcompleteCurlerName);


  StoreArray<SpacePointTrackCand> spacePointTCs(m_PARAMsptcName);
  int nTCs = spacePointTCs.getEntries();

  B2DEBUG(15, "Found " << nTCs << " in StoreArray " << spacePointTCs.getName() << " for this event");

  for (int iTC = 0; iTC < nTCs; ++iTC) {
    SpacePointTrackCand* spacePointTC = spacePointTCs[iTC];
    m_spacePointTCCtr++;

    B2DEBUG(15, "=========================== Processing SpacePointTrackCand " << iTC << " ===============================");
    try {
      const std::vector<int> splittingIndices = checkTrackCandForCurling(*spacePointTC);

      if (splittingIndices.empty()) {
        B2DEBUG(15, "This SpacePointTrackCand shows no curling behaviour");
      } else {
        B2DEBUG(15, "This SpacePointTrackCand shows curling behaviour");
        if (!m_PARAMsplitCurlers) {
          B2DEBUG(15, "This SpacePointTrackCand could be split into " << splittingIndices.size() + 1 << " but will not, because splitCurlers is set to false");
          continue; // should jump to enclosing for-loop!!! (process the next SpacePointTrackCand)
        }

        // get the TrackCand Stubs
        std::vector<SpacePointTrackCand> trackStubs = splitCurlingTrackCand(*spacePointTC, m_PARAMnTrackStubs, splittingIndices);

        for (auto trackStub : trackStubs) {
          if (m_saveCompleteCurler) {
            SpacePointTrackCand* newSPTC = completeCurlingTCs.appendNew(trackStub);
            newSPTC->addRelationTo(spacePointTC);
          }
          if (!trackStub.isOutgoing()) {
            SpacePointTrackCand* newSPTC = ingoingAllTCs.appendNew(trackStub);
            newSPTC->addRelationTo(spacePointTC);
          }

        }
      }
    } catch (FoundNoTrueHit& anE) {
      B2WARNING("Caught an exception during checking for curling behaviour: " << anE.what() << " This TrackCandidate cannot be checked for curling behaviour")
      m_noDecisionPossibleCtr++;
    } catch (FoundNoCluster& anE) {
      B2WARNING("Caught an exception during checking for curling behaviour: " << anE.what() << " This TrackCandidate cannot be checked for curling behaviour")
      m_noDecisionPossibleCtr++;
    } catch (TrueHitsNotMatching& anE) {
      B2WARNING("Caught an exception during checking for curling behaviour: " << anE.what() << " This TrackCandidate cannot be checked for curling behaviour")
      m_noDecisionPossibleCtr++;
    } catch (SpacePointTrackCand::UnsupportedDetType& anE) {
      B2WARNING("Caught an exception during checking for curling behaviour: " << anE.what() << " This TrackCandidate cannot be checked for curling behaviour")
      m_noDecisionPossibleCtr++;
    } catch (SpacePoint::InvalidNumberOfClusters& anE) {
      B2WARNING("Caught an exception during checking for curling behaviour: " << anE.what() << " This TrackCandidate cannot be checked for curling behaviour")
      m_noDecisionPossibleCtr++;
    }

  }
}

// =================================================== TERMINATE ========================================================
void CurlingTrackCandSplitterModule::terminate()
{
  // TODO
}

// ============================================== CHECK FOR CURLING ======================================================
const std::vector<int> CurlingTrackCandSplitterModule::checkTrackCandForCurling(const Belle2::SpacePointTrackCand& SPTrackCand)
{
  const std::vector<const Belle2::SpacePoint*>& tcSpacePoints = SPTrackCand.getHits();
  unsigned int nHits = SPTrackCand.getNHits();

  std::vector<int> returnVector; // fill this vector with indices, if no indices can be found, leave it empty

  std::pair<bool, bool> directions; // only store the last two directions to decide if it has changed or not. .first is always last hit, .second is present hit.
  directions.first = true; // assume that the track points outwards from the interaction point

  for (unsigned int iHit = 0; iHit < nHits; ++iHit) {
    const SpacePoint* spacePoint = tcSpacePoints[iHit];
    auto detType = spacePoint->getType();

    // get global position and momentum for every spacePoint in the SpacePointTrackCand
    std::pair<TVector3, TVector3> hitGlobalPosMom;

    if (detType == VXD::SensorInfoBase::PXD) {
      // first get PXDCluster, from that get TrueHit
      PXDCluster* pxdCluster = spacePoint->getRelatedTo<PXDCluster>("ALL"); // COULDDO: search only certain Cluster Arrays -> get name somehow
      // CAUTION: only looking for one TrueHit here, but there could actually be more of them 'molded' into one Cluster
      PXDTrueHit* pxdTrueHit = pxdCluster->getRelatedTo<PXDTrueHit>("ALL"); // COULDDO: search only certain PXDTrueHit arrays -> new parameter for module

      if (pxdTrueHit == NULL) {
        B2WARNING("Found no PXDTrueHit for PXDCluster " << pxdCluster->getArrayIndex() << " from Array " << pxdCluster->getArrayName() << ". This PXDCluster is related with SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName());
        throw FoundNoTrueHit();
      }

      B2DEBUG(100, "Now getting global position and momentum for PXDCluster " << pxdCluster->getArrayIndex() << " from Array " << pxdCluster->getArrayName());
      hitGlobalPosMom = getGlobalPositionAndMomentum(pxdTrueHit);

    } else if (detType == VXD::SensorInfoBase::SVD) {
      // get all related SVDClusters and do some sanity checks, before getting the SVDTrueHits and then using them to get global position and momentum
      RelationVector<SVDCluster> svdClusters = spacePoint->getRelationsTo<SVDCluster>("ALL"); // COULDDO: search only certain Cluster Arrays -> get name somehow
      if (svdClusters.size() > 2) { throw SpacePoint::InvalidNumberOfClusters(); } // should never throw, since this check should already be done in SpacePoint creation!
      if (svdClusters.size() == 0) {
        B2WARNING("Found no related clusters for SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << ". With no Cluster no information if a track is curling or not can be obtained");
        throw FoundNoCluster(); // this should also never happen, as the vice versa way is used above to get to the SpacePoints in the first place!!
      } else {
        // collect the TrueHits (maximum 2), if there is more than one compare them, to see if both Clusters point to the same TrueHit
        std::vector<SVDTrueHit*> svdTrueHits;
        for (const SVDCluster & aCluster : svdClusters) {
          // CAUTION: there can be more than one TrueHit for a given Cluster!!!
          SVDTrueHit* svdTrueHit = aCluster.getRelatedTo<SVDTrueHit>("ALL"); // COULDDO: search only certain SVDTrueHit arrays -> new parameter for module
          if (svdTrueHit == NULL) {
            B2WARNING("Found no SVDTrueHit for SVDCluster " << aCluster.getArrayIndex() << " from Array " << aCluster.getArrayName() << ". This SVDCluster is related with SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName());
            throw FoundNoTrueHit();
          }
          B2DEBUG(100, "Found TrueHit for SVDCluster " << aCluster.getArrayIndex() << " from Array " << aCluster.getArrayName())
          svdTrueHits.push_back(svdTrueHit);
        }
        // if there were 2 clusters, check if they are both related to the same SVDTrueHit, if not throw. Else get position and momentum for this SpacePoint
        if (svdTrueHits.size() > 1) {
          B2DEBUG(150, "Now checking if TrueHits of Clusters belonging to SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << " are equal.")
          if (svdTrueHits[0] != svdTrueHits[1]) {
            B2WARNING("TrueHits of SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << " are not matching: TrueHit 1 has Index " << svdTrueHits[0]->getArrayIndex() << ", TrueHit 2 has Index " << svdTrueHits[1]->getArrayIndex());

            // Only do these calculations if the debug Level is set to 150 or higher
            if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 150, PACKAGENAME())) {   // comparison with true is uneccessary?
              B2DEBUG(150, "There are " << svdTrueHits.size() << " TrueHits related to SpacePoint " << spacePoint->getArrayIndex())

              // get position and momentum for every TrueHit and print it, to compare them later
              stringstream positions;
              stringstream momenta;
              for (unsigned int i = 0; i < svdTrueHits.size(); ++i) {
                auto posMom = getGlobalPositionAndMomentum(svdTrueHits[i]);
                positions << posMom.first.X() << " " << posMom.first.Y() << " " << posMom.first.Z() << " ";
                momenta << posMom.second.X() << " " << posMom.second.Y() << " " << posMom.second.Z() << " ";
              }

              // now print out with easy to grep pattern
              B2DEBUG(150, "MISMATCHING_TRUEHITS_POSITIONS: " << positions.str());
              B2DEBUG(150, "MISMATCHING_TRUEHITS_MOMENTA:" << momenta.str());
            }
            throw TrueHitsNotMatching();
          }
        }

        B2DEBUG(100, "Now getting global position and momentum for SVDCluster " << svdClusters[0]->getArrayIndex() << " from Array " << svdClusters[0]->getArrayName());
        hitGlobalPosMom = getGlobalPositionAndMomentum(svdTrueHits[0]);
      }
    } else { // this should never be reached, because it should be caught in the creation of the SpacePointTrackCand which is passed to this function!
      throw SpacePointTrackCand::UnsupportedDetType();
    }

    // get the direction of flight for the present SpacePoint
    directions.second = getDirectionOfFlight(hitGlobalPosMom, m_origin);

    // check if the directions have changed since the last hit, if so, add the number of the SpacePoint (inside the SpacePointTrackCand) to the returnVector
    if (directions.first != directions.second) {
      B2DEBUG(75, "The direction of flight has changed for SpacePoint " << iHit << " in SpacePointTrackCand. The StoreArray index of this SpacePoint is " << spacePoint->getArrayIndex() << " in " << spacePoint->getArrayName());
      returnVector.push_back(iHit);
    }
    // assign old value to .first, for next comparison
    directions.first = directions.second;
  }
  return returnVector;
}

// ======================================= GET GLOBAL POSITION AND MOMENTUM ============================================================
template<class TrueHit>
std::pair<const TVector3, const TVector3> CurlingTrackCandSplitterModule::getGlobalPositionAndMomentum(TrueHit* aTrueHit)
{
  // get sensor stuff (needed for pointToGlobal)
  VxdID aVxdId = aTrueHit->getSensorID();

  B2DEBUG(100, "Getting global position and momentum vectors for TrueHit " << aTrueHit->getArrayIndex() << " from Array " << aTrueHit->getArrayName() << ". This hit has VxdID " << aVxdId);

  const VXD::GeoCache& geometry = VXD::GeoCache::getInstance();
  const VXD::SensorInfoBase& sensorInfoBase = geometry.getSensorInfo(aVxdId);

  // get position
  TVector3 hitLocal = TVector3(aTrueHit->getU(), aTrueHit->getV(), 0);
  TVector3 hitGlobal = sensorInfoBase.pointToGlobal(hitLocal); // should work like this, since local coordinates are only 2D
  B2DEBUG(100, "Local position of hit is (" << hitLocal.X() << "," << hitLocal.Y() << "," << hitLocal.Z() << "), Global position of hit is (" << hitGlobal.X() << "," << hitGlobal.Y() << "," << hitGlobal.Z() << ")");

  // get momentum
  TVector3 pGlobal = sensorInfoBase.vectorToGlobal(aTrueHit->getMomentum());
  B2DEBUG(100, "Global momentum of hit is (" << pGlobal.X() << "," << pGlobal.Y() << "," << pGlobal.Z() << ")");

  return std::make_pair(hitGlobal, pGlobal);
}

// ======================================= GET DIRECTION OF FLIGHT ======================================================================
bool CurlingTrackCandSplitterModule::getDirectionOfFlight(const std::pair<const TVector3, const TVector3>& hitPosAndMom, const TVector3 origin)
{
  TVector3 originToHit = hitPosAndMom.first - origin;
  TVector3 momentumAtHit = hitPosAndMom.second + originToHit;

  B2DEBUG(100, "Position of hit relative to origin is (" << originToHit.X() << "," << originToHit.Y() << "," << originToHit.Z() << "). Momentum relative to hit (relative to origin) (" << momentumAtHit.X() << "," << momentumAtHit.Y() << "," << momentumAtHit.Z() << ")");

  // cylindrical coordinates (?) -> use TVector3.Perp() to get the radial component of a given vector
  // for spherical coordinates -> use TVector3.Mag() for the same purposes
  double hitRadComp = originToHit.Perp(); // radial component of hit coordinates
  double hitMomRadComp = momentumAtHit.Perp(); // radial component of the tip of the momentum vector, when its origin would be the hit position (as it is only the direction of the momentum that matters here, units are completely ignored)

  if (hitMomRadComp < hitRadComp) {
    B2DEBUG(100, "Direction of flight is inwards for this hit");
    return false;
  } else {
    B2DEBUG(100, "Direction of flight is outwards for this hit");
    return true;
  }
}

// ================================================ SPLIT CURLING TRACK CAND =========================================================
const std::vector<Belle2::SpacePointTrackCand>
CurlingTrackCandSplitterModule::splitCurlingTrackCand(const Belle2::SpacePointTrackCand& SPTrackCand, int NTracklets, const std::vector<int>& splitIndices)
{
  std::vector<SpacePointTrackCand> spacePointTCs;

  std::vector<std::pair<int, int> > rangeIndices; // store pairs of Indices indicating the first and the last index of a TrackStub inside a SpacePointTrackCand

  int firstIndex = 0; // first 'first index' is 0
  for (int index : splitIndices) {
    rangeIndices.push_back({firstIndex, index});
    firstIndex = index + 1; // next first index is last index + 1
  }
  rangeIndices.push_back({firstIndex, SPTrackCand.getNHits() - 1}); // the last TrackStub contains all hits from the last splitIndex to the end of the SpacePointTrackCand

  // CAUTION: simply assuming that the first part of the Track is outgoing, although this should be true (at least for TrackCands from MCTrackFinderTruth) it is not guaranteed to be!
  bool outgoing = true;

  // return NTracklets (user defined) at most
  for (unsigned iTr = 0; iTr < uint(NTracklets) && iTr < rangeIndices.size(); ++iTr) {
    int lastInd = rangeIndices[iTr].second;
    int firstInd = rangeIndices[iTr].first;

    B2DEBUG(75, "Creating Track Stub " << iTr << " of " << splitIndices.size() << " possible Track Stub for this SpacePointTrackCand. The indices for this Tracklet are (first,last): (" << firstInd << "," << lastInd << "). This SpacePointTrackCand contains " << SPTrackCand.getNHits() << " SpacePoints.");

    const std::vector<const SpacePoint*> trackletSpacePoints = SPTrackCand.getHitsInRange(firstInd, lastInd);
    const std::vector<double> trackletSortingParams = SPTrackCand.getSortingParametersInRange(firstInd, lastInd);

    SpacePointTrackCand newSPTrackCand = SpacePointTrackCand(trackletSpacePoints, SPTrackCand.getPdgCode(), SPTrackCand.getChargeSeed(), SPTrackCand.getMcTrackID());
    newSPTrackCand.setSortingParameters(trackletSortingParams);

    // TODO: set state seed and cov seed for all but the first tracklets (first is just the seed of the original TrackCand)
    if (iTr < 1) {
      newSPTrackCand.set6DSeed(SPTrackCand.getStateSeed());
      newSPTrackCand.setCovSeed(SPTrackCand.getCovSeed());
    }

    // set direction of flight and flip it for the next track stub (track is split where the direction of flight changes so this SHOULD not introduce any errors)
    newSPTrackCand.setFlightDirection(outgoing);
    outgoing = !outgoing;

    spacePointTCs.push_back(newSPTrackCand);
  }

  return spacePointTCs;
}


// ====================================================== INITIALIZE COUNTERS =======================================================
void CurlingTrackCandSplitterModule::initializeCounters()
{
  m_createdTrackStubsCtr = 0;
  m_curlingTCCtr = 0;
  m_noDecisionPossibleCtr = 0;
  m_spacePointTCCtr = 0;
}