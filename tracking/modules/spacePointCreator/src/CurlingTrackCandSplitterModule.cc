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
  setDescription("Module for checking SpacePointTrackCands for curling behaviour and (if wanted) splitting them into SpacePointTrackCands that no longer show curling behaviour. WARNING: MODULE IS DEPRECATED use SPTCReferee instead!");

  addParam("splitCurlers", m_PARAMsplitCurlers,
           "Split curling SpacePointTrackCands into non-curling SpacePointTrackCands and store them", true);
  addParam("nTrackStubs", m_PARAMnTrackStubs,
           "Maximum number of SpacePointTrackCand Stubs to be created from a curling SpacePointTrackCand. Set to 0 if you want all possible TrackCand Stubs",
           0);

  addParam("SpacePointTCName", m_PARAMsptcName, "Collection name of the SpacePointTrackCands to be analyzed for curling behaviour",
           std::string(""));
  addParam("curlingFirstOutName", m_PARAMcurlingOutFirstName,
           "Collection name under which the first outgoing part of a curling TrackCand will be stored in the StoreArray. The first part of a curling Track has its origin at the interaction point.",
           std::string(""));
  addParam("curlingAllInName", m_PARAMcurlingAllInName,
           "Collection name under which all ingoing parts of a curling TrackCand will be stored in the StoreArray", std::string(""));
  addParam("curlingRestOutName", m_PARAMcurlingOutRestName,
           "Collection name under which all but the first outgoing parts of a curling TrackCand will be stored in the StoreArray",
           std::string(""));
  addParam("completeCurlerName", m_PARAMcompleteCurlerName,
           "Collection name under which all parts of a curling TrackCand will be stored in the StoreArray together. NOTE: only if this parameter is set to a non-empty string a complete (but splitted) curling TrackCand will be stored!",
           std::string(""));

  // WARNING TODO: find out the units that are used internally!!!
  std::vector<double> defaultOrigin = { 0., 0., 0. };
  addParam("setOrigin", m_PARAMsetOrigin,
           "WARNING: still need to find out the units that are used internally! Reset origin to given point. Used for determining the direction of flight of a particle for a given hit. Needs to be reset for e.g. testbeam, where origin is not at (0,0,0)",
           defaultOrigin);

  addParam("positionAnalysis", m_PARAMpositionAnalysis,
           "Set to true to investigate the positions of SpacePoints and TrueHits and write them to a ROOT file", false);

  std::vector<std::string> defaultRootFName;
  defaultRootFName.push_back("PositionResiduals");
  defaultRootFName.push_back("RECREATE");

  addParam("rootFileName", m_PARAMrootFileName,
           "Filename and write-mode ('RECREATE' or 'UPDATE'). If given more than 2 strings this module will cause termination",
           defaultRootFName);


  addParam("useNonSingleTHinPA", m_PARAMuseNonSingleTHinPA,
           "Switch for using SpacePoints in position Analysis that are related to more than one TrueHit", false);

  initializeCounters(); // NOTE: they get initialized in initialize again!!

  // initialize other variables to some default values to avoid unintended behaviour
  m_saveCompleteCurler = false;
  m_treePtr = NULL;
  m_rootFilePtr = NULL;
}

// ================================================= INITIALIZE =========================================================
void CurlingTrackCandSplitterModule::initialize()
{
  initializeCounters();
  B2INFO("CurlingTrackCandSplitter ----------------------------- initialize() -------------------------------------");
  B2WARNING("CurlingTrackCandSplitter is deprecated and will be removed from framework in the near future! use SPTCReferee instead!");
  // check if all necessary StoreArrays are present
  StoreArray<SpacePointTrackCand> spacePointTCs(m_PARAMsptcName);
  spacePointTCs.isRequired(m_PARAMsptcName);

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
  curlingFirstOuts.registerInDataStore(m_PARAMcurlingOutFirstName, DataStore::c_ErrorIfAlreadyRegistered);
  curlingFirstOuts.registerRelationTo(spacePointTCs);

  StoreArray<SpacePointTrackCand> curlingAllIns(m_PARAMcurlingAllInName);
  curlingAllIns.registerInDataStore(m_PARAMcurlingAllInName, DataStore::c_ErrorIfAlreadyRegistered);
  curlingAllIns.registerRelationTo(spacePointTCs);

  StoreArray<SpacePointTrackCand> curlingRestOuts(m_PARAMcurlingOutRestName);
  curlingRestOuts.registerInDataStore(m_PARAMcurlingOutRestName, DataStore::c_ErrorIfAlreadyRegistered);
  curlingRestOuts.registerRelationTo(spacePointTCs);

  // have to do this here, because in event() I do not want to check every time if this string is empty or not and act accordingly. If I register this with an empty string here, I can use it with an empty string in event() and only store stuff into it, when it is actually named with a non-empty string
  StoreArray<SpacePointTrackCand> curlingCompletes(m_PARAMcompleteCurlerName);
  curlingCompletes.registerInDataStore(m_PARAMcompleteCurlerName, DataStore::c_ErrorIfAlreadyRegistered);
  curlingCompletes.registerRelationTo(spacePointTCs);

  if (!m_PARAMcompleteCurlerName.empty()) {
    m_saveCompleteCurler = true;
    B2DEBUG(1, "You put in " << m_PARAMcompleteCurlerName <<
            " as collection name for complete curling TrackCands. Complete curling TrackCands will hence be stored.");
  } else {
    B2DEBUG(1,
            "You did not put in any under which complete curling TrackCands should be stored, hence curling TrackCands will only be stored in parts.");
    m_saveCompleteCurler = false;
  }

  // check value for nTrackStubs and reset if necessary
  if (m_PARAMnTrackStubs < 0) {
    B2WARNING("CurlingTrackCandSplitter::initialize> Value of nTrackStubs is below 0: nTrackStubs = " << m_PARAMnTrackStubs <<
              ". Resetting this value to 0 now! This means that all parts of curling TrackCands will be stored.");
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
      B2FATAL("CurlingTrackCandSplitter::initialize() : rootFileName is set wrong: entries are: " << output);
    }
    // create ROOT file
    m_PARAMrootFileName[0] += ".root";
    m_rootFilePtr = new TFile(m_PARAMrootFileName[0].c_str(), m_PARAMrootFileName[1].c_str());
    m_treePtr = new TTree("m_treePtr", "aTree");

    // link everything to the according variables
    for (int layer = 0; layer < c_nPlanes; ++layer) {
      string layerString = (boost::format("%1%") % (layer +
                                                    1)).str(); // layer numbering starts at 1 this way (plus cppcheck complains about division by zero otherwise)

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
      name = "TrueHitVLocal_" + layerString;
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

  B2DEBUG(15, "Found " << nTCs << " SpacePointTrackCands in StoreArray " << spacePointTCs.getName() << " for this event");

  RootVariables rootVariables;

  for (int iTC = 0; iTC < nTCs; ++iTC) {
    SpacePointTrackCand* spacePointTC = spacePointTCs[iTC];
    m_spacePointTCCtr++;

    B2DEBUG(15, "=========================== Processing SpacePointTrackCand " << iTC << " ===============================");
    try {
      const std::vector<int> splittingIndices = checkTrackCandForCurling(*spacePointTC, rootVariables);

      if (splittingIndices.empty()) {
        B2DEBUG(15, "This SpacePointTrackCand shows no curling behaviour and will be added to collection: " << m_PARAMcurlingOutFirstName);
        spacePointTC->setTrackStubIndex(0); // set TrackStubIndex to 0 (indicates, that this TrackCandidate shows no curling behaviour)
        // add this spacePoint to the StoreArray with the first outgoing parts since the whole TC is outgoing
        SpacePointTrackCand* newSPTC = outgoingFirstTCs.appendNew(*spacePointTC);
        newSPTC->addRelationTo(spacePointTC);
        m_NoCurlingTCsCtr++;
      } else {
        B2DEBUG(15, "This SpacePointTrackCand shows curling behaviour");
        if (!m_PARAMsplitCurlers) {
          B2DEBUG(15, "This SpacePointTrackCand could be split into " << splittingIndices.size() + 1 <<
                  " but will not, because splitCurlers is set to false");
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
      B2WARNING("Caught an exception during checking for curling behaviour: " << anE.what() <<
                " This TrackCandidate cannot be checked for curling behaviour");
      m_noDecisionPossibleCtr++;
    } catch (FoundNoCluster& anE) {
      B2WARNING("Caught an exception during checking for curling behaviour: " << anE.what() <<
                " This TrackCandidate cannot be checked for curling behaviour");
      m_noDecisionPossibleCtr++;
    } catch (TrueHitsNotMatching& anE) {
      B2WARNING("Caught an exception during checking for curling behaviour: " << anE.what() <<
                " This TrackCandidate cannot be checked for curling behaviour");
      m_noDecisionPossibleCtr++;
    } catch (SpacePointTrackCand::UnsupportedDetType& anE) {
      B2WARNING("Caught an exception during checking for curling behaviour: " << anE.what() <<
                " This TrackCandidate cannot be checked for curling behaviour");
      m_noDecisionPossibleCtr++;
    }
  }
  // only write to root once per event
  if (m_PARAMpositionAnalysis) { writeToRoot(rootVariables); }
}

// =================================================== TERMINATE ========================================================
void CurlingTrackCandSplitterModule::terminate()
{
  B2INFO("CurlingTrackCandSplitter::terminate(): checked " << m_spacePointTCCtr << " SpacePointTrackCands for curling behaviour. " <<
         m_curlingTCCtr << " of them were curling and " << m_createdTrackStubsCtr << " TrackStubs were created. " << m_NoCurlingTCsCtr <<
         " SPTCs were not curling and were merely copied into StoreArray " << m_PARAMcurlingOutFirstName << ". In " <<
         m_noDecisionPossibleCtr << " cases no decision could be made. There were " << m_NoSingleTrueHitCtr <<
         " SpacePoints that were related to more than one TrueHit");
  // do ROOT file stuff
  if (m_treePtr != NULL) {
    m_rootFilePtr->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module
    m_treePtr->Write();
    m_rootFilePtr->Close();
  }
}

// ============================================== CHECK FOR CURLING ======================================================
const std::vector<int> CurlingTrackCandSplitterModule::checkTrackCandForCurling(const Belle2::SpacePointTrackCand& SPTrackCand,
    RootVariables& rootVariables)
{
  const std::vector<const Belle2::SpacePoint*>& tcSpacePoints = SPTrackCand.getHits();
  unsigned int nHits = SPTrackCand.getNHits();

  B2DEBUG(70, "SpacePointTrackCand contains " << nHits << " SpacePoints");

  std::vector<int> returnVector; // fill this vector with indices, if no indices can be found, leave it empty

  std::pair<bool, bool>
  directions; // only store the last two directions to decide if it has changed or not. .first is always last hit, .second is present hit.
  directions.first =
    true; // assume that the track points outwards from the interaction point at first. NOTE: this assumption is not dangerous here (it is in other places because it does not have to be the case). The information on the direction of flight for the first hit is stored in the returnVector itself. If the first entry is 0 (this means that the trackCand first 'pointed' towards the interaction point)

  for (unsigned int iHit = 0; iHit < nHits; ++iHit) {
    const SpacePoint* spacePoint = tcSpacePoints[iHit];
    auto detType = spacePoint->getType();

    B2DEBUG(100, "Now checking SpacePoint " << iHit << " in SPTC. This SpacePoint has Index " << spacePoint->getArrayIndex() <<
            " in StoreArray " << spacePoint->getArrayName());

    // get global position and momentum for every spacePoint in the SpacePointTrackCand
    std::pair<B2Vector3<double>, B2Vector3<double> > hitGlobalPosMom;

    if (detType == VXD::SensorInfoBase::PXD) {
      // first get PXDCluster, from that get TrueHit
      PXDCluster* pxdCluster =
        spacePoint->getRelatedTo<PXDCluster>("ALL"); // COULDDO: search only certain Cluster Arrays -> get name somehow
      // CAUTION: only looking for one TrueHit here, but there could actually be more of them 'molded' into one Cluster
      PXDTrueHit* pxdTrueHit =
        pxdCluster->getRelatedTo<PXDTrueHit>("ALL"); // COULDDO: search only certain PXDTrueHit arrays -> new parameter for module

      if (pxdTrueHit == NULL) {
        B2DEBUG(1, "Found no PXDTrueHit for PXDCluster " << pxdCluster->getArrayIndex() << " from Array " << pxdCluster->getArrayName() <<
                ". This PXDCluster is related with SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName());
        throw FoundNoTrueHit();
      }

      B2DEBUG(100, "Found PXDCluster " << pxdCluster->getArrayIndex() << " and " << " PXDTrueHit " << pxdTrueHit->getArrayIndex() <<
              " from StoreArray " << pxdTrueHit->getArrayName() << " related to this SpacePoint");

      B2DEBUG(100, "Now getting global position and momentum for PXDCluster " << pxdCluster->getArrayIndex() << " from Array " <<
              pxdCluster->getArrayName());
      hitGlobalPosMom = getGlobalPositionAndMomentum(pxdTrueHit);

      // if position analysis is set to true, print to root file
      if (m_PARAMpositionAnalysis) { getValuesForRoot(spacePoint, pxdTrueHit, rootVariables); }

    } else if (detType == VXD::SensorInfoBase::SVD) {
      // get all related SVDClusters and do some sanity checks, before getting the SVDTrueHits and then using them to get global position and momentum
      RelationVector<SVDCluster> svdClusters =
        spacePoint->getRelationsTo<SVDCluster>("ALL"); // COULDDO: search only certain Cluster Arrays -> get name somehow (addidional parameter?)
      if (svdClusters.size() == 0) {
        B2WARNING("Found no related clusters for SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName()
                  << ". With no Cluster no information if a track is curling or not can be obtained");
        throw FoundNoCluster(); // this should also never happen, as the vice versa way is used to get to the SpacePoints in the first place (in the GFTC2SPTCConverterModule e.g.)
      } else {
        // collect the TrueHits, if there is more than one compare them, to see if both Clusters point to the same TrueHit
        // WARNING there can be more! more than one TrueHit can be 'hidden' in one Cluster!!!
        // TODO: look at this again, this seems not to work properly at the moment!!!
        std::vector<const SVDTrueHit*> svdTrueHits;
        for (const SVDCluster& aCluster : svdClusters) {
          // CAUTION: there can be more than one TrueHit for a given Cluster!!!
          RelationVector<SVDTrueHit> relTrueHits =
            aCluster.getRelationsTo<SVDTrueHit>("ALL"); // COULDDO: search only certain SVDTrueHit arrays -> new parameter for module
          if (relTrueHits.size() == 0) {
            B2DEBUG(1, "Found no SVDTrueHit for SVDCluster " << aCluster.getArrayIndex() << " from Array " << aCluster.getArrayName() <<
                    ". This SVDCluster is related with SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName());
            throw FoundNoTrueHit();
          }

          B2DEBUG(100, "Found " << relTrueHits.size() << " TrueHits for SVDCluster " << aCluster.getArrayIndex() << " from Array " <<
                  aCluster.getArrayName());
          for (unsigned int i = 0; i < relTrueHits.size(); ++i) { svdTrueHits.push_back(relTrueHits[i]); }
        }

        // if there is only one cluster related to the SpacePoint simply check if one (or more TrueHits are present). Additionally checking the size for svdTrueHits again is not necessary here, because if there was only one Cluster and no TrueHits were found this part is never reached!
        // WARNING: It is not guaranteed that this actually leads to a valid relation between SpacePoint and TrueHit!!
        // TODO: continuing with next SpacePoint skips check for curling behavior!!! FIX this!!!
//         if (svdClusters.size() == 1) {
//           stringstream inds;
//           for (const SVDTrueHit * trueHit : svdTrueHits) { inds << trueHit->getArrayIndex() << ", "; }
//           B2DEBUG(150, "Found only one Cluster related to SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << ". To this Cluster " << svdTrueHits.size() << " related TrueHits were found. Indices: " << inds.str());
//    m_NoSingleTrueHitCtr++;
//           continue; // start over with next SpacePoint
//         }

        // if there is at least one TrueHit in the vector check how many unique TrueHits there are
        if (svdTrueHits.size() >= 1) {
          B2DEBUG(150, "Found " << svdTrueHits.size() << " SVDTrueHits related to Clusters related to SpacePoint " <<
                  spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << ". Now checking if they are compatible");

          // sort & unique to find the unique entries of the relation vector
          std::sort(svdTrueHits.begin(), svdTrueHits.end());
          unsigned int oldSize = svdTrueHits.size();
          auto newEnd = std::unique(svdTrueHits.begin(), svdTrueHits.end());
          svdTrueHits.resize(std::distance(svdTrueHits.begin(), newEnd));

          // If there is no overlapping TrueHit get all indices of the TrueHits and print a warning. If position Analysis is enabled calculate the position differences of the TrueHits. In the end throw an Exception (only if the TrueHits belong to a non-single Cluster SVD SpacePoint)
          if (svdTrueHits.size() == oldSize) {
            stringstream trueHitInds;
            for (const SVDTrueHit* trueHit : svdTrueHits) { trueHitInds << trueHit->getArrayIndex() << ", "; }
            B2DEBUG(1, "There is no overlapping TrueHit for SpacePoint " << spacePoint->getArrayIndex() << " from Array " <<
                    spacePoint->getArrayName() << ". The Indices of the TrueHits are: " << trueHitInds.str());

            // Only do these calculations if output to root is is enabled
            if (m_PARAMpositionAnalysis) {
              std::vector<B2Vector3<double> > globalPositions;
              std::vector<B2Vector3<double> > globalMomenta;
              // collect all values
              for (unsigned int i = 0; i < svdTrueHits.size(); ++i) {
                auto posMom = getGlobalPositionAndMomentum(svdTrueHits[i]);
                globalPositions.push_back(posMom.first);
                globalMomenta.push_back(posMom.second);
              }
              // WARNING: getting only layer number of first TrueHit in vector here. Although this should not change, this is never actually checked!!
              int layer = svdTrueHits[0]->getSensorID().getLayerNumber() - 1; // layer numbering starts at 1, indexing of array at 0
              // do the calculations (starting from one because of comparison of two elements in each run through loop)
              for (unsigned int i = 1; i < globalPositions.size(); ++i) {
                rootVariables.MisMatchPosResiduals.at(layer).push_back((globalPositions[i] - globalPositions[i - 1]).Mag());

                rootVariables.MisMatchPosX.at(layer).push_back((globalPositions[i] - globalPositions[i - 1]).X());
                rootVariables.MisMatchPosY.at(layer).push_back((globalPositions[i] - globalPositions[i - 1]).Y());
                rootVariables.MisMatchPosZ.at(layer).push_back((globalPositions[i] - globalPositions[i - 1]).Z());

                rootVariables.MisMatchPosU.at(layer).push_back((svdTrueHits[i]->getU() - svdTrueHits[i - 1]->getU()));
                rootVariables.MisMatchPosV.at(layer).push_back((svdTrueHits[i]->getV() - svdTrueHits[i - 1]->getV()));

                B2Vector3<double> momDiff = globalMomenta[i] - globalMomenta[i - 1];
                rootVariables.MisMatchMomX.at(layer).push_back(momDiff.X());
                rootVariables.MisMatchMomY.at(layer).push_back(momDiff.Y());
                rootVariables.MisMatchMomZ.at(layer).push_back(momDiff.Z());
              }
            }
            // if the TrueHits are related from a singleCluster SVD SpacePoint (i.e. more than one TrueHits are molded into one Cluster) do not throw this exception but continue with the curling checking
            if (svdClusters.size() > 1) { TrueHitsNotMatching(); }
          }
        }

        // if there is more than one TrueHit remaining for one SpacePoint increase the counter
        if (svdTrueHits.size() > 1) {
          m_NoSingleTrueHitCtr++;
        }

        // WARNING if there are more than one matching TrueHits only the first TrueHit is used for comparison and for position analysis
        B2DEBUG(100, "Now getting global position and momentum for SVDCluster " << svdClusters[0]->getArrayIndex() << " from Array " <<
                svdClusters[0]->getArrayName() << " via SVDTrueHit " << svdTrueHits[0]->getArrayIndex() << " from StoreArray " <<
                svdTrueHits[0]->getArrayName());
        hitGlobalPosMom = getGlobalPositionAndMomentum(svdTrueHits[0]);

        // if position analysis is set to true, print to root file
        // only do so if there is only one TrueHit (this is only for the moment!!!) OR if the switch is set to do so even if there is more than one TrueHit
        // TODO: Decide how to handle such cases where more than one TrueHit is left and implement accordingly
        if (m_PARAMpositionAnalysis && (svdTrueHits.size() == 1 || m_PARAMuseNonSingleTHinPA)) { getValuesForRoot(spacePoint, svdTrueHits[0], rootVariables); }
      }
    } else { // this should never be reached, because it should be caught in the creation of the SpacePointTrackCand which is passed to this function!
      throw SpacePointTrackCand::UnsupportedDetType();
    }

    // get the direction of flight for the present SpacePoint
    directions.second = getDirectionOfFlight(hitGlobalPosMom, m_origin);

    // check if the directions have changed since the last hit, if so, add the number of the SpacePoint (inside the SpacePointTrackCand) to the returnVector
    if (directions.first != directions.second) {
      B2DEBUG(75, "The direction of flight has changed for SpacePoint " << iHit <<
              " in SpacePointTrackCand. The StoreArray index of this SpacePoint is " << spacePoint->getArrayIndex() << " in " <<
              spacePoint->getArrayName());
      returnVector.push_back(iHit);
    }
    // assign old value to .first, for next comparison
    directions.first = directions.second;
  }
  return returnVector;
}

// ======================================= GET GLOBAL POSITION AND MOMENTUM ============================================================
template<class TrueHit>
std::pair<const Belle2::B2Vector3<double>, const Belle2::B2Vector3<double> >
CurlingTrackCandSplitterModule::getGlobalPositionAndMomentum(TrueHit* aTrueHit)
{
  // get sensor stuff (needed for pointToGlobal)
  VxdID aVxdId = aTrueHit->getSensorID();

  B2DEBUG(100, "Getting global position and momentum vectors for TrueHit " << aTrueHit->getArrayIndex() << " from Array " <<
          aTrueHit->getArrayName() << ". This hit has VxdID " << aVxdId);

  const VXD::GeoCache& geometry = VXD::GeoCache::getInstance();
  const VXD::SensorInfoBase& sensorInfoBase = geometry.getSensorInfo(aVxdId);

  // get position
  B2Vector3<double> hitLocal = B2Vector3<double>(aTrueHit->getU(), aTrueHit->getV(), 0);
  B2Vector3<double> hitGlobal = sensorInfoBase.pointToGlobal(
                                  hitLocal, true); // should work like this, since local coordinates are only 2D
  B2DEBUG(100, "Local position of hit is (" << hitLocal.X() << "," << hitLocal.Y() << "," << hitLocal.Z() <<
          "), Global position of hit is (" << hitGlobal.X() << "," << hitGlobal.Y() << "," << hitGlobal.Z() << ")");

  // get momentum
  B2Vector3<double> pGlobal = sensorInfoBase.vectorToGlobal(aTrueHit->getMomentum(), true);
  B2DEBUG(100, "Global momentum of hit is (" << pGlobal.X() << "," << pGlobal.Y() << "," << pGlobal.Z() << ")");

  return std::make_pair(hitGlobal, pGlobal);
}

// ======================================= GET DIRECTION OF FLIGHT ======================================================================
bool CurlingTrackCandSplitterModule::getDirectionOfFlight(const
                                                          std::pair<const B2Vector3<double>, const B2Vector3<double>>& hitPosAndMom,
                                                          const B2Vector3<double> origin)
{
  B2Vector3<double> originToHit = hitPosAndMom.first - origin;
  B2Vector3<double> momentumAtHit = hitPosAndMom.second + originToHit;

  // additional debug output from developement to find a possible B2Vector3 bug
//   B2DEBUG(250, "hitPosMom.first (" << hitPosAndMom.first.X() << "," << hitPosAndMom.first.Y() << "," << hitPosAndMom.first.Z() << "). hitPosAndMom.second: (" << hitPosAndMom.second.X() << "," << hitPosAndMom.second.Y() << "," << hitPosAndMom.second.Z() << "). origin: (" << origin.X() << "," << origin.Y() << "," << origin.Z() << ")");

  B2DEBUG(100, "Position of hit relative to origin is (" << originToHit.X() << "," << originToHit.Y() << "," << originToHit.Z() <<
          "). Momentum relative to hit (relative to origin) (" << momentumAtHit.X() << "," << momentumAtHit.Y() << "," << momentumAtHit.Z() <<
          ")");

  // cylindrical coordinates (?) -> use B2Vector3.Perp() to get the radial component of a given vector
  // for spherical coordinates -> use B2Vector3.Mag() for the same purposes
  double hitRadComp = originToHit.Perp(); // radial component of hit coordinates
  double hitMomRadComp =
    momentumAtHit.Perp(); // radial component of the tip of the momentum vector, when its origin would be the hit position (as it is only the direction of the momentum that matters here, units are completely ignored -> COULDDO: use the unit() method from B2Vector3

  B2DEBUG(250, " radial component of hit coordinates: " << hitRadComp <<
          ", radial component of tip of momentum vector with its origin set to hit position: " << hitMomRadComp);

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
CurlingTrackCandSplitterModule::splitCurlingTrackCand(const Belle2::SpacePointTrackCand& SPTrackCand, int NTracklets,
                                                      const std::vector<int>& splitIndices)
{
  std::vector<SpacePointTrackCand> spacePointTCs;

  std::vector<std::pair<int, int> >
  rangeIndices; // store pairs of Indices indicating the first and the last index of a TrackStub inside a SpacePointTrackCand

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

    B2DEBUG(75, "Creating Track Stub " << iTr << " of " << splitIndices.size() <<
            " possible Track Stub for this SpacePointTrackCand. The indices for this Tracklet are (first,last): (" << firstInd << "," << lastInd
            << "). This SpacePointTrackCand contains " << SPTrackCand.getNHits() << " SpacePoints in total.");

    // encapsulate these functions into a try-clause since both throw
    try {
      const std::vector<const SpacePoint*> trackletSpacePoints = SPTrackCand.getHitsInRange(firstInd, lastInd);
      const std::vector<double> trackletSortingParams = SPTrackCand.getSortingParametersInRange(firstInd, lastInd);

      SpacePointTrackCand newSPTrackCand = SpacePointTrackCand(trackletSpacePoints, SPTrackCand.getPdgCode(), SPTrackCand.getChargeSeed(),
                                                               SPTrackCand.getMcTrackID());
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
    } catch (SpacePointTrackCand::SPTCIndexOutOfBounds& anE) {
      B2WARNING("Caught an exception while trying to split SpacePointTrackCands: " << anE.what() <<
                " This SPTC will be skipped from splitting!");
    }
  }

  return spacePointTCs;
}

// ======================================================= GET ROOT VALUES =========================================================
template <class TrueHit>
void CurlingTrackCandSplitterModule::getValuesForRoot(const Belle2::SpacePoint* spacePoint, const TrueHit* trueHit,
                                                      RootVariables& rootVariables)
{
  B2DEBUG(100, "Getting positions (for ROOT output) of SpacePoint " << spacePoint->getArrayIndex() << " from Array " <<
          spacePoint->getArrayName() << " and TrueHit " << trueHit->getArrayIndex() << " from Array " << trueHit->getArrayName());

  // get VxdIDs of spacePoint and trueHit (and their according layer numbers for storing the information in the appropriate arrays)
  VxdID spacePointVxdId = spacePoint->getVxdID();
  VxdID trueHitVxdId = trueHit->getSensorID();

  // get positions from SpacePoint
  const B2Vector3<double>& spacePointGlobal =
    spacePoint->getPosition(); // COULDDO: uneccesary, spacePoint->X(), etc. returns the same information!
//   std::pair<double, double> spacePointUV = getUV(spacePoint);
  TaggedUVPos spacePointUV = getUV(spacePoint);
  const B2Vector3<double> spacePointLocal = B2Vector3<double>(spacePointUV.m_U, spacePointUV.m_V, 0);

  // get local position from TrueHit
  const B2Vector3<double> trueHitLocal = B2Vector3<double>(trueHit->getU(), trueHit->getV(), 0);

  // get sensor Info for global position of TrueHit
  const VXD::GeoCache& geometry = VXD::GeoCache::getInstance();
  const VXD::SensorInfoBase& sensorInfoBase = geometry.getSensorInfo(trueHitVxdId);

  // get global position from TrueHit
  const B2Vector3<double> trueHitGlobal = sensorInfoBase.pointToGlobal(trueHitLocal, true);

  // Layer numbering starts at 1 not at 0 so deduce layer by one to access array
  int spLayer = spacePointVxdId.getLayerNumber() - 1;
  int thLayer = trueHitVxdId.getLayerNumber() - 1;

  if (spLayer != thLayer) {
    B2FATAL("Layer numbers of TrueHit and SpacePoint do not match!"); // this should never happen -> FATAL if it does, because something has gone amiss then
  }

  // all positions collected, but only write the values to ROOT that have been set appropriately!
  bool singleCluster = true; // for debug output
  if (spacePointUV.m_setU) {
    rootVariables.SpacePointULocal.at(spLayer).push_back(spacePointUV.m_U);
    rootVariables.TrueHitULocal.at(thLayer).push_back(trueHit->getU());
    rootVariables.PosResidueULocal.at(spLayer).push_back((spacePointUV.m_U - trueHit->getU()));
  }
  if (spacePointUV.m_setV) {
    rootVariables.SpacePointVLocal.at(spLayer).push_back(spacePointUV.m_V);
    rootVariables.TrueHitVLocal.at(thLayer).push_back(trueHit->getV());
    rootVariables.PosResidueVLocal.at(spLayer).push_back((spacePointUV.m_V - trueHit->getV()));
  }
  if (spacePointUV.m_setU && spacePointUV.m_setV) {
    rootVariables.SpacePointXGlobal.at(spLayer).push_back(spacePoint->X());
    rootVariables.SpacePointYGlobal.at(spLayer).push_back(spacePoint->Y());
    rootVariables.SpacePointZGlobal.at(spLayer).push_back(spacePoint->Z());

    rootVariables.TrueHitXGlobal.at(thLayer).push_back(trueHitGlobal.X());
    rootVariables.TrueHitYGlobal.at(thLayer).push_back(trueHitGlobal.Y());
    rootVariables.TrueHitZGlobal.at(thLayer).push_back(trueHitGlobal.Z());

    rootVariables.PosResidueXGlobal.at(spLayer).push_back((spacePointGlobal - trueHitGlobal).X());
    rootVariables.PosResidueYGlobal.at(spLayer).push_back((spacePointGlobal - trueHitGlobal).Y());
    rootVariables.PosResidueZGlobal.at(spLayer).push_back((spacePointGlobal - trueHitGlobal).Z());

    rootVariables.PosResiduesGlobal.at(spLayer).push_back((spacePointGlobal - trueHitGlobal).Mag());
    rootVariables.PosResiduesLocal.at(spLayer).push_back((spacePointLocal - trueHitLocal).Mag());

    singleCluster = false;
  }

  B2DEBUG(200, "Global (x,y,z)/Local (U,V) positions of SpacePoint: (" << spacePointGlobal.X() << "," << spacePointGlobal.Y() << ","
          << spacePointGlobal.Z() << ")/(" << spacePointLocal.X() << "," << spacePointLocal.Y() << "). This was a singleCluster SpacePoint: "
          << singleCluster);

  B2DEBUG(200, "Global (x,y,z)/Local (U,V) positions of TrueHit: (" << trueHitGlobal.X() << "," << trueHitGlobal.Y() << "," <<
          trueHitGlobal.Z() << ")/(" << trueHitLocal.X() << "," << trueHitLocal.Y() << ")");

  B2DEBUG(200, "This leads to position differences global/local: " << (spacePointGlobal - trueHitGlobal).Mag() << "/" <<
          (spacePointLocal - trueHitLocal).Mag());

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

CurlingTrackCandSplitterModule::TaggedUVPos CurlingTrackCandSplitterModule::getUV(const Belle2::SpacePoint* spacePoint)
{
  TaggedUVPos returnVals; // initialized to: both bools false, both doubles to 0.

  // get the normalized local coordinates from SpacePoint and convert them to local coordinates (have to do so because at the slanted parts the local U-position is dependant on the local V-position)
  double normU = spacePoint->getNormalizedLocalU();
  double normV = spacePoint->getNormalizedLocalV();

  // convert normalized coordinates to local coordinates (those are already 'unwedged')
  std::pair<double, double> localUV = SpacePoint::convertNormalizedToLocalCoordinates(std::make_pair(normU, normV),
                                      spacePoint->getVxdID());
  //double unwedgedU = SpacePoint::getUUnwedged(localUV, spacePoint->getVxdID());

  // set values for return: set both m_U and m_V regardless if they have actually been set in the SpacePoint and simply assign the m_setX tags, as they decide if a value is actually used
  std::pair<bool, bool> setCoords = spacePoint->getIfClustersAssigned();

  returnVals.m_setU = setCoords.first;
  returnVals.m_setV = setCoords.second;
  returnVals.m_U = localUV.first;
  returnVals.m_V = localUV.second;

  return returnVals;
}

// ====================================================== INITIALIZE COUNTERS =======================================================
void CurlingTrackCandSplitterModule::initializeCounters()
{
  m_createdTrackStubsCtr = 0;
  m_curlingTCCtr = 0;
  m_noDecisionPossibleCtr = 0;
  m_spacePointTCCtr = 0;
  m_NoSingleTrueHitCtr = 0;
  m_NoCurlingTCsCtr = 0;
}
