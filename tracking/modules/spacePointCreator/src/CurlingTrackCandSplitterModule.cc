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

#include <boost/format.hpp>

#include <algorithm> // sort & unique

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

  addParam("positionAnalysis", m_PARAMpositionAnalysis, "Set to true to investigate the positions of SpacePoints and TrueHits and write them to a ROOT file", false);

  std::vector<std::string> defaultRootFName;
  defaultRootFName.push_back("PositionResiduals");
  defaultRootFName.push_back("RECREATE");

  addParam("rootFileName", m_PARAMrootFileName, "Filename and write-mode ('RECREATE' or 'UPDATE'). If given more than 2 strings this module will cause termination", defaultRootFName);

  initializeCounters(); // NOTE: they get initialized in initialize again!!
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

  if (m_PARAMpositionAnalysis) {
    // check if there are two entries and if the second value is either UPDATE or RECREATE
    if (m_PARAMrootFileName.size() != 2 || (m_PARAMrootFileName[1] != "UPDATE" && m_PARAMrootFileName[1] != "RECREATE")) {
      string output;
      for (string entry : m_PARAMrootFileName) { output += "'" + entry + "' "; }
      B2FATAL("CurlingTrackCandSplitter::initialize() : rootFileName is set wrong: entries are: " << output)
    }
    // create ROOT file
    m_PARAMrootFileName[0] += ".root";
    m_rootFilePtr = new TFile(m_PARAMrootFileName[0].c_str(), m_PARAMrootFileName[1].c_str());
    m_treePtr = new TTree("m_treePtr", "aTree");

    // link everything to the according variables
    for (int layer = 0; layer < c_nPlanes; ++layer) {
      string layerString = (boost::format("%1%") % (layer + 1)).str(); // layer numbering starts at 1 this way (plus cppcheck complains about division by zero otherwise)

      string name = "SpacePointXGlobal_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootSpacePointXGlobals.at(layer));
      name = "SpacePointYGlobal_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootSpacePointYGlobals.at(layer));
      name = "SpacePointZGlobal_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootSpacePointZGlobals.at(layer));

      name = "SpacePointULocal_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootSpacePointULocals.at(layer));
      name = "SpacePointVlocal_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootSpacePointVLocals.at(layer));

      name = "TrueHitXGlobal_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootTrueHitXGlobals.at(layer));
      name = "TrueHitYGlobal_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootTrueHitYGlobals.at(layer));
      name = "TrueHitZGlobal_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootTrueHitZGlobals.at(layer));

      name = "TrueHitULocal_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootTrueHitULocals.at(layer));
      name = "TrueHitXVLocal_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootTrueHitVLocals.at(layer));

      name = "PosResidualsXGlobal_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootPosResidueXGlobal.at(layer));
      name = "PosResidualsYGlobal_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootPosResidueYGlobal.at(layer));
      name = "PosResidualsZGlobal_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootPosResidueZGlobal.at(layer));

      name = "PosResidualsULocal_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootPosResidueULocal.at(layer));
      name = "PosResidualsVLocal_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootPosResidueVLocal.at(layer));


      name = "LocalPositionResiduals_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootLocalPosResiduals.at(layer));
      name = "GlobalPositionResiduals_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootGlobalPosResiduals.at(layer));

      name = "MisMatchPosDistance_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootMisMatchPosDistance.at(layer));
      name = "MisMatchPosX_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootMisMatchPosX.at(layer));
      name = "MisMatchPosY_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootMisMatchPosY.at(layer));
      name = "MisMatchPosZ_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootMisMatchPosZ.at(layer));

      name = "MisMatchPosU_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootMisMatchPosU.at(layer));
      name = "MisMatchPosV_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootMisMatchPosV.at(layer));

      name = "MisMatchMomX_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootMisMatchMomX.at(layer));
      name = "MisMatchMomY_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootMisMatchMomY.at(layer));
      name = "MisMatchMomZ_" + layerString;
      m_treePtr->Branch(name.c_str(), &m_rootMisMatchMomZ.at(layer));
    }
  } else {
    m_rootFilePtr = NULL;
    m_treePtr = NULL;
  }
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

  RootVariables rootVariables;

  for (int iTC = 0; iTC < nTCs; ++iTC) {
    SpacePointTrackCand* spacePointTC = spacePointTCs[iTC];
    m_spacePointTCCtr++;

    B2DEBUG(15, "=========================== Processing SpacePointTrackCand " << iTC << " ===============================");
    try {
      const std::vector<int> splittingIndices = checkTrackCandForCurling(*spacePointTC, rootVariables);

      if (splittingIndices.empty()) {
        B2DEBUG(15, "This SpacePointTrackCand shows no curling behaviour");
        spacePointTC->setTrackStubIndex(0); // set TrackStubIndex to 0 (indicates, that this TrackCandidate shows no curling behaviour)
      } else {
        B2DEBUG(15, "This SpacePointTrackCand shows curling behaviour");
        if (!m_PARAMsplitCurlers) {
          B2DEBUG(15, "This SpacePointTrackCand could be split into " << splittingIndices.size() + 1 << " but will not, because splitCurlers is set to false");
          continue; // should jump to enclosing for-loop!!! (process the next SpacePointTrackCand)
        }
        m_curlingTCCtr++;
        // get the TrackCand Stubs
        std::vector<SpacePointTrackCand> trackStubs = splitCurlingTrackCand(*spacePointTC, m_PARAMnTrackStubs, splittingIndices);

        // add the stubs to the appropriate StoreArray
        for (SpacePointTrackCand trackStub : trackStubs) {
          m_createdTrackStubsCtr++;
          if (m_saveCompleteCurler) {
            SpacePointTrackCand* newSPTC = completeCurlingTCs.appendNew(trackStub);
            newSPTC->addRelationTo(spacePointTC);
            B2DEBUG(25, "Added SpacePointTrackCand " << newSPTC->getArrayIndex() << " to StoreArray " << newSPTC->getArrayName());
          }
          if (!trackStub.isOutgoing()) {
            SpacePointTrackCand* newSPTC = ingoingAllTCs.appendNew(trackStub);
            newSPTC->addRelationTo(spacePointTC);
            B2DEBUG(25, "Added SpacePointTrackCand " << newSPTC->getArrayIndex() << " to StoreArray " << newSPTC->getArrayName());
          } else { // if not ingoing differentiate between first part and all of the rest
            if (trackStub.getTrackStubIndex() > 1) {
              SpacePointTrackCand* newSPTC = outgoingRestTCs.appendNew(trackStub);
              newSPTC->addRelationTo(spacePointTC);
              B2DEBUG(25, "Added SpacePointTrackCand " << newSPTC->getArrayIndex() << " to StoreArray " << newSPTC->getArrayName());
            } else {
              SpacePointTrackCand* newSPTC = outgoingFirstTCs.appendNew(trackStub);
              newSPTC->addRelationTo(spacePointTC);
              B2DEBUG(25, "Added SpacePointTrackCand " << newSPTC->getArrayIndex() << " to StoreArray " << newSPTC->getArrayName());
            }
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
  // only write to root once per event
  if (m_PARAMpositionAnalysis) { writeToRoot(rootVariables); }
}

// =================================================== TERMINATE ========================================================
void CurlingTrackCandSplitterModule::terminate()
{
  B2INFO("CurlingTrackCandSplitter::terminate(): checked " << m_spacePointTCCtr << " SpacePointTrackCands for curling behaviour. " << m_curlingTCCtr << " of them were curling and " << m_createdTrackStubsCtr << " TrackStubs were created. In " << m_noDecisionPossibleCtr << " cases no decision could be made.");

  // do ROOT file stuff
  if (m_treePtr != NULL) {
    m_rootFilePtr->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module
    m_treePtr->Write();
    m_rootFilePtr->Close();
  }
}

// ============================================== CHECK FOR CURLING ======================================================
const std::vector<int> CurlingTrackCandSplitterModule::checkTrackCandForCurling(const Belle2::SpacePointTrackCand& SPTrackCand, RootVariables& rootVariables)
{
  const std::vector<const Belle2::SpacePoint*>& tcSpacePoints = SPTrackCand.getHits();
  unsigned int nHits = SPTrackCand.getNHits();

  std::vector<int> returnVector; // fill this vector with indices, if no indices can be found, leave it empty

  std::pair<bool, bool> directions; // only store the last two directions to decide if it has changed or not. .first is always last hit, .second is present hit.
  directions.first = true; // assume that the track points outwards from the interaction point at first. NOTE: this assumption is not dangerous here (it is in other places because it does not have to be the case). The information on the direction of flight for the first hit is stored in the returnVector itself. If the first entry is 0 (this means that the trackCand first 'pointed' towards the interaction point)

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

      // if position analysis is set to true, print to root file
      if (m_PARAMpositionAnalysis) { getValuesForRoot(spacePoint, pxdTrueHit, rootVariables); }

    } else if (detType == VXD::SensorInfoBase::SVD) {
      // get all related SVDClusters and do some sanity checks, before getting the SVDTrueHits and then using them to get global position and momentum
      RelationVector<SVDCluster> svdClusters = spacePoint->getRelationsTo<SVDCluster>("ALL"); // COULDDO: search only certain Cluster Arrays -> get name somehow
      if (svdClusters.size() > 2) { throw SpacePoint::InvalidNumberOfClusters(); } // should never throw, since this check should already be done in SpacePoint creation!
      if (svdClusters.size() == 0) {
        B2WARNING("Found no related clusters for SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << ". With no Cluster no information if a track is curling or not can be obtained");
        throw FoundNoCluster(); // this should also never happen, as the vice versa way is used above to get to the SpacePoints in the first place!!
      } else {
        // collect the TrueHits, if there is more than one compare them, to see if both Clusters point to the same TrueHit
        // WARNING there can be more! more than one TrueHit can be 'hidden' in one Cluster!!!
        // TODO: look at this again, this seems not to work properly at the moment!!!
        std::vector<const SVDTrueHit*> svdTrueHits;
        for (const SVDCluster & aCluster : svdClusters) {
          // CAUTION: there can be more than one TrueHit for a given Cluster!!!
          RelationVector<SVDTrueHit> relTrueHits = aCluster.getRelationsTo<SVDTrueHit>("ALL"); // COULDDO: search only certain SVDTrueHit arrays -> new parameter for module
          if (relTrueHits.size() == 0) {
            B2WARNING("Found no SVDTrueHit for SVDCluster " << aCluster.getArrayIndex() << " from Array " << aCluster.getArrayName() << ". This SVDCluster is related with SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName());
            throw FoundNoTrueHit();
          }

          B2DEBUG(100, "Found " << relTrueHits.size() << " TrueHits for SVDCluster " << aCluster.getArrayIndex() << " from Array " << aCluster.getArrayName())
          for (unsigned int i = 0; i < relTrueHits.size(); ++i) { svdTrueHits.push_back(relTrueHits[i]); }
        }

        // if there is only one cluster related to the SpacePoint simply check if one (or more TrueHits are present). Additionally checking the size for svdTrueHits again is not necessary here, because if there was only one Cluster and no TrueHits were found this part is never reached!
        // WARNING: It is not guaranteed that this actually leads to a valid relation between SpacePoint and TrueHit!!
        if (svdClusters.size() == 1) {
          stringstream inds;
          for (const SVDTrueHit * trueHit : svdTrueHits) { inds << trueHit->getArrayIndex() << ", "; }
          B2DEBUG(150, "Found only one Cluster related to SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << ". To this Cluster " << svdTrueHits.size() << " related TrueHits were found. Indices: " << inds.str());
          continue; // start over with next SpacePoint
        }
        // if there are 2 Clusters, there have to be at least 2 TrueHits (else above part would have thrown)
        if (svdTrueHits.size() > 1) {
          B2DEBUG(150, "Found " << svdTrueHits.size() << " related to Clusters related to SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << ". Now checking if they are compatible")

          //
          std::sort(svdTrueHits.begin(), svdTrueHits.end());
          unsigned int oldSize = svdTrueHits.size();
          auto newEnd = std::unique(svdTrueHits.begin(), svdTrueHits.end());
          svdTrueHits.resize(std::distance(svdTrueHits.begin(), newEnd));

          // WARNING if there are one (or more) matching TrueHits only the first TrueHit is used for comparison!!!
          if (svdTrueHits.size() == oldSize) {
            stringstream trueHitInds;
            for (const SVDTrueHit * trueHit : svdTrueHits) { trueHitInds << trueHit->getArrayIndex() << ", "; }
            B2WARNING("There is no overlapping TrueHit for SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << ". The Indices of the TrueHits are: " << trueHitInds.str());

            // Only do these calculations if output to root is is enabled
            if (m_PARAMpositionAnalysis) {
              std::vector<TVector3> globalPositions;
              std::vector<TVector3> globalMomenta;
              // collect all values
              for (unsigned int i = 0; i < svdTrueHits.size(); ++i) {
                auto posMom = getGlobalPositionAndMomentum(svdTrueHits[i]);
                globalPositions.push_back(posMom.first);
                globalMomenta.push_back(posMom.second);
              }
              int layer = svdTrueHits[0]->getSensorID().getLayerNumber() - 1; // layer numbering starts at 1, indexing of array at 0
              // do the calculations (starting from one because of comparison of two elements in each run through loop)
              for (unsigned int i = 1; i < globalPositions.size(); ++i) {
                rootVariables.MisMatchPosResiduals.at(layer).push_back((globalPositions[i] - globalPositions[i - 1]).Mag());

                rootVariables.MisMatchPosX.at(layer).push_back((globalPositions[i] - globalPositions[i - 1]).X());
                rootVariables.MisMatchPosY.at(layer).push_back((globalPositions[i] - globalPositions[i - 1]).Y());
                rootVariables.MisMatchPosZ.at(layer).push_back((globalPositions[i] - globalPositions[i - 1]).Z());

                rootVariables.MisMatchPosU.at(layer).push_back((svdTrueHits[i]->getU() - svdTrueHits[i - 1]->getU()));
                rootVariables.MisMatchPosV.at(layer).push_back((svdTrueHits[i]->getV() - svdTrueHits[i - 1]->getV()));

                TVector3 momDiff = globalMomenta[i] - globalMomenta[i - 1];
                rootVariables.MisMatchMomX.at(layer).push_back(momDiff.X());
                rootVariables.MisMatchMomY.at(layer).push_back(momDiff.Y());
                rootVariables.MisMatchMomZ.at(layer).push_back(momDiff.Z());
              }
            }
            throw TrueHitsNotMatching();
          }
        }

        B2DEBUG(100, "Now getting global position and momentum for SVDCluster " << svdClusters[0]->getArrayIndex() << " from Array " << svdClusters[0]->getArrayName());
        hitGlobalPosMom = getGlobalPositionAndMomentum(svdTrueHits[0]);

        // if position analysis is set to true, print to root file
        if (m_PARAMpositionAnalysis) { getValuesForRoot(spacePoint, svdTrueHits[0], rootVariables); }
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
  double hitMomRadComp = momentumAtHit.Perp(); // radial component of the tip of the momentum vector, when its origin would be the hit position (as it is only the direction of the momentum that matters here, units are completely ignored -> COULDDO: use the unit() method from TVector3

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

  // NOTE: if the first index of splitIndices is zero, this means that the direction of flight for the first SpacePoint (i.e. TrueHit) of the TrackCand was not outgoing.
  // WARNING: This is only true as long as this behaviour is not changed in checkTrackCandForCurling(...), as there the assumption is made that the direction of flight of the first hit is outgoing, and only if it is not 0 is the first entry of the returnVector of the latter.
  bool outgoing = splitIndices[0] != 0;

  // return NTracklets (user defined) at most, or if NTracklets is 0, return all
  // if NTracklets is 0 set it to the appropriate size for the following for-loop
  if (NTracklets < 1) { NTracklets = rangeIndices.size(); }
  for (unsigned iTr = 0; iTr < rangeIndices.size() && iTr < uint(NTracklets); ++iTr) {

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

    // if the TrackCandidate curls this index starts at 1, if it is a curling TrackCand
    newSPTrackCand.setTrackStubIndex(iTr + 1);

    spacePointTCs.push_back(newSPTrackCand);
  }

  return spacePointTCs;
}

// ======================================================= GET ROOT VALUES =========================================================
template <class TrueHit>
void CurlingTrackCandSplitterModule::getValuesForRoot(const Belle2::SpacePoint* spacePoint, const TrueHit* trueHit, RootVariables& rootVariables)
{
  B2DEBUG(100, "Getting positions of SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << " and TrueHit " << trueHit->getArrayIndex() << " from Array " << trueHit->getArrayName());

  // get positions from SpacePoint and add them to rootVariables
  const TVector3& spacePointGlobal = spacePoint->getPosition();
  std::pair<double, double> spacePointUV = getUV(spacePoint);
  const TVector3 spacePointLocal = TVector3(spacePointUV.first, spacePointUV.second, 0);

  // get VxdIDs of spacePoint and trueHit (and their according layer numbers for storing the information in the appropriate arrays)
  VxdID spacePointVxdId = spacePoint->getVxdID();
  VxdID trueHitVxdId = trueHit->getSensorID();

  // Layer numbering starts at 1 not at 0 so deduce layer by one to access array
  int spLayer = spacePointVxdId.getLayerNumber() - 1;
  int thLayer = trueHitVxdId.getLayerNumber() - 1;

  rootVariables.SpacePointXGlobal.at(spLayer).push_back(spacePoint->X());
  rootVariables.SpacePointYGlobal.at(spLayer).push_back(spacePoint->Y());
  rootVariables.SpacePointZGlobal.at(spLayer).push_back(spacePoint->Z());

  rootVariables.SpacePointULocal.at(spLayer).push_back(spacePointUV.first);
  rootVariables.SpacePointVLocal.at(spLayer).push_back(spacePointUV.second);

  B2DEBUG(200, "Global (x,y,z)/Local (U,V) positions of SpacePoint: (" << spacePointGlobal.X() << "," << spacePointGlobal.Y() << "," << spacePointGlobal.Z() << ")/(" << spacePointLocal.X() << "," << spacePointLocal.Y() << ")");

  // get positions from TrueHit

  const VXD::GeoCache& geometry = VXD::GeoCache::getInstance();
  const VXD::SensorInfoBase& sensorInfoBase = geometry.getSensorInfo(trueHitVxdId);

  const TVector3 trueHitLocal = TVector3(trueHit->getU(), trueHit->getV(), 0);
  const TVector3 trueHitGlobal = sensorInfoBase.pointToGlobal(trueHitLocal);

  rootVariables.TrueHitXGlobal.at(thLayer).push_back(spacePoint->X());
  rootVariables.TrueHitYGlobal.at(thLayer).push_back(spacePoint->Y());
  rootVariables.TrueHitZGlobal.at(thLayer).push_back(spacePoint->Z());

  rootVariables.TrueHitULocal.at(thLayer).push_back(trueHit->getU());
  rootVariables.TrueHitVLocal.at(thLayer).push_back(trueHit->getV());

  B2DEBUG(200, "Global (x,y,z)/Local (U,V) positions of TrueHit: (" << trueHitGlobal.X() << "," << trueHitGlobal.Y() << "," << trueHitGlobal.Z() << ")/(" << trueHitLocal.X() << "," << trueHitLocal.Y() << ")");

  B2DEBUG(200, "This leads to position differences global/local: " << (spacePointGlobal - trueHitGlobal).Mag() << "/" << (spacePointLocal - trueHitLocal).Mag());

  // calculate position differences (& check if both are on the same layer, but should be)
  if (spLayer == thLayer) {
    rootVariables.PosResiduesGlobal.at(spLayer).push_back((spacePointGlobal - trueHitGlobal).Mag());
    rootVariables.PosResiduesLocal.at(spLayer).push_back((spacePointLocal - trueHitLocal).Mag());

    rootVariables.PosResidueXGlobal.at(spLayer).push_back((spacePointGlobal - trueHitGlobal).X());
    rootVariables.PosResidueYGlobal.at(spLayer).push_back((spacePointGlobal - trueHitGlobal).Y());
    rootVariables.PosResidueZGlobal.at(spLayer).push_back((spacePointGlobal - trueHitGlobal).Z());

    rootVariables.PosResidueULocal.at(spLayer).push_back((spacePointUV.first - trueHit->getU()));
    rootVariables.PosResidueVLocal.at(spLayer).push_back((spacePointUV.second - trueHit->getV()));

  }
}

// =================================== WRITE TO ROOT ===============================================================================
void CurlingTrackCandSplitterModule::writeToRoot(RootVariables& rootVariables)
{
  m_rootGlobalPosResiduals = rootVariables.PosResiduesGlobal;
  m_rootLocalPosResiduals = rootVariables.PosResiduesLocal;

  m_rootSpacePointULocals = rootVariables.SpacePointULocal;
  m_rootSpacePointVLocals = rootVariables.SpacePointVLocal;

  m_rootSpacePointXGlobals = rootVariables.SpacePointXGlobal;
  m_rootSpacePointYGlobals = rootVariables.SpacePointYGlobal;
  m_rootSpacePointZGlobals = rootVariables.SpacePointZGlobal;

  m_rootTrueHitULocals = rootVariables.TrueHitULocal;
  m_rootTrueHitVLocals = rootVariables.TrueHitVLocal;

  m_rootTrueHitXGlobals = rootVariables.TrueHitXGlobal;
  m_rootTrueHitYGlobals = rootVariables.TrueHitYGlobal;
  m_rootTrueHitZGlobals = rootVariables.TrueHitZGlobal;

  m_rootPosResidueXGlobal = rootVariables.PosResidueXGlobal;
  m_rootPosResidueYGlobal = rootVariables.PosResidueYGlobal;
  m_rootPosResidueZGlobal = rootVariables.PosResidueZGlobal;
  m_rootPosResidueULocal = rootVariables.PosResidueULocal;
  m_rootPosResidueVLocal = rootVariables.PosResidueVLocal;

  m_rootMisMatchPosDistance = rootVariables.MisMatchPosResiduals;
  m_rootMisMatchPosX = rootVariables.MisMatchPosX;
  m_rootMisMatchPosY = rootVariables.MisMatchPosY;
  m_rootMisMatchPosZ = rootVariables.MisMatchPosZ;

  m_rootMisMatchPosU = rootVariables.MisMatchPosU;
  m_rootMisMatchPosV = rootVariables.MisMatchPosV;

  m_rootMisMatchMomX = rootVariables.MisMatchMomX;
  m_rootMisMatchMomY = rootVariables.MisMatchMomY;
  m_rootMisMatchMomZ = rootVariables.MisMatchMomZ;

  m_treePtr->Fill();
}

// ========================================================= GET U AND V ============================================================
std::pair<double, double> CurlingTrackCandSplitterModule::getUV(const Belle2::SpacePoint* spacePoint)
{
  auto detType = spacePoint->getType();
  if (detType == VXD::SensorInfoBase::PXD) {
    PXDCluster* pxdCluster = spacePoint->getRelatedTo<PXDCluster>("ALL"); // COULDDO: search only certain StoreArrays
    if (pxdCluster == NULL) {
      B2ERROR("Found no relation to a PXDCluster for SpacePoint " << spacePoint->getArrayIndex() << " from StoreArray" << spacePoint->getArrayName() << ". This SpacePoint will be skipped and will not be contained in the genfit::TrackCand");
      throw (FoundNoCluster()); // Should never happen
    }
    return std::make_pair(pxdCluster->getU(), pxdCluster->getV());
  } else if (detType == VXD::SensorInfoBase::SVD) {
    RelationVector<SVDCluster> svdClusters = spacePoint->getRelationsTo<SVDCluster>("ALL"); // COULDDO: search only certain StoreArrays
    if (svdClusters.size() > 2) { throw SpacePoint::InvalidNumberOfClusters(); }
    else if (svdClusters.size() == 0) {
      B2ERROR("Found no relation to a SVDCluster for SpacePoint " << spacePoint->getArrayIndex() << " from StoreArray" << spacePoint->getArrayName() << ". This SpacePoint will be skipped and will not be contained in the genfit::TrackCand");
      throw (FoundNoCluster());
    }
    double uPos = 0.;
    double vPos = 0.;
    for (const SVDCluster & aCluster : svdClusters) {
      if (aCluster.isUCluster()) {
        uPos = aCluster.getPosition();
      } else {
        vPos = aCluster.getPosition();
      }
    }
    return std::make_pair(uPos, vPos);
  } else { // COULDDO: throw SpacePointTrackCand::UnsupportedDetType()
    return std::make_pair(0., 0.);
  }
}

// ====================================================== INITIALIZE COUNTERS =======================================================
void CurlingTrackCandSplitterModule::initializeCounters()
{
  m_createdTrackStubsCtr = 0;
  m_curlingTCCtr = 0;
  m_noDecisionPossibleCtr = 0;
  m_spacePointTCCtr = 0;
}