/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "testbeam/vxd/modules/tracking/BaseLineTFModule.h"

// framework
#include <framework/gearbox/Const.h>
#include <framework/gearbox/GearDir.h> // needed for reading xml-files
#include <tracking/dataobjects/VXDTFSecMap.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <framework/datastore/StoreObjPtr.h>

#include "tracking/dataobjects/FilterID.h"
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingVXD/sectorMapTools/SectorTools.h>

//genfit

#include <genfit/Track.h>

//root packages:
#include <TMatrixT.h>
#include <TMatrixD.h>
#include <TGeoManager.h>
#include <TMath.h>

//STL or C/C++-packages;
#include <algorithm>
#include <functional>
#include <utility>
#include <math.h>
#include <time.h>
#include <fstream>
#include <iomanip>      // std::setprecision
#include <array>


//Boost-packages:
#include <boost/foreach.hpp>
#include <boost/bind/bind.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp> // needed for is_any_of
#include <boost/lexical_cast.hpp>
#include <boost/math/special_functions/sign.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/casts.hpp>


using namespace std;
using namespace Belle2;
using boost::format;
using boost::lexical_cast;
using boost::math::sign;
using boost::bind;
// using boost::chrono::high_resolution_clock;


// random generator function:
ptrdiff_t rngWrapper(ptrdiff_t i) { return gRandom->Integer(i); }
// pointer object to it:
ptrdiff_t (*p_rngWrapper)(ptrdiff_t) = rngWrapper;



//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BaseLineTF)


BaseLineTFModule::BaseLineTFModule() : Module()
{
  std::string sectorSetup = "std";


  bool activateTRUE = true;
  bool activateFALSE = false;
  bool activateZigZagXY = false;
  double tuneZERO = 0;
  double tuneCircleFit = 0.00000001; // 1st, 2nd, 3rd-pass chi2-threshold

  vector<string> rootFileNameVals = {"BLTFoutput", "UPDATE"};

  //Set module properties
  setDescription("Baseline TF for testbeams.");
  setPropertyFlags(c_ParallelProcessingCertified);

  ///Steering parameter import

  addParam("sectorSetup",
           m_PARAMsectorSetup,
           "lets you chose the sectorSetup (compatibility of sensors, individual cutoffs,...) accepts 'std', 'low', 'high' and 'personal', please note that the chosen setup has to exist as a xml-file in ../testbeam/vxd/data/XXX.xml. If you can not create your own xml files using e.g. the filterCalculatorModule, use params for  'tuneCutoffXXX' (for tuning single values) or 'tuneCutoffs' (for tuning all at once) instead.",
           sectorSetup);

  addParam("tuneCutoffs",
           m_PARAMtuneCutoffs,
           "for rapid changes of all cutoffs (no personal xml files needed), reduces/enlarges the range of the cutoffs in percent (lower and upper values are changed by this value). Only valid in range -99% < x < +1000%",
           double(0.22));
  addParam("GFTrackCandidatesColName",
           m_PARAMgfTrackCandsColName,
           "Name of collection holding the genfit::TrackCandidates (output)",
           string(""));
  addParam("InfoBoardName",
           m_PARAMinfoBoardName,
           "Name of container used for data transfer to TFAnalyzer, only used when TESTERexpandedTestingRoutines == true",
           string(""));
  addParam("pxdClustersName",
           m_PARAMpxdClustersName,
           "Name of storeArray containing pxd clusters (only valid when using secMap supporting pxd clusters)",
           string(""));
  addParam("svdClustersName",
           m_PARAMsvdClustersName,
           "Name of storeArray containing svd clusters (only valid when using secMap supporting svd clusters)",
           string(""));

  addParam("nameOfInstance",
           m_PARAMnameOfInstance,
           "Name of trackFinder, usefull, if there is more than one BLTF running at the same time. Note: please choose short names",
           string("BLTF"));

  addParam("activateZigZagXY",
           m_PARAMactivateZigZagXY,
           " set True/False for each setup individually",
           activateZigZagXY);
  addParam("activateZigZagXYWithSigma",
           m_PARAMactivateZigZagXYWithSigma,
           " set True/False for each setup individually",
           activateFALSE);
  addParam("activateDeltaPt",
           m_PARAMactivateDeltaPt,
           " set True/False for each setup individually",
           activateTRUE);
  addParam("activateCircleFit",
           m_PARAMactivateCircleFit,
           " set True/False for each setup individually",
           activateFALSE);

  addParam("tuneZigZagXY",
           m_PARAMtuneZigZagXY,
           " currently not in use, only here for symmetrical reasons",
           tuneZERO);
  addParam("tuneZigZagXYWithSigma",
           m_PARAMtuneZigZagXYWithSigma,
           " currently not in use, only here for symmetrical reasons",
           tuneZERO);
  addParam("tuneDeltaPt",
           m_PARAMtuneDeltaPt,
           " tune for each setup individually, in %",
           tuneZERO);
  addParam("tuneCircleFit",
           m_PARAMtuneCircleFit,
           " threshold for pValues calculated by the circleFiter for each tc. If pValue is lower than threshold, tc gets discarded",
           tuneCircleFit);

  addParam("standardPdgCode",
           m_PARAMpdGCode,
           "standard value is 211 (pi+), ATTENTION, instead of using inconsistent sign of PdGList, in this module positively charged particles are always positive and negatively charged ones are negative (relevant for leptons)",
           int(211));
  addParam("artificialMomentum",
           m_PARAMartificialMomentum,
           "standard value is 0. if StandardValue is changed to a nonZero value, the magnitude of the momentum seed is set artificially using this value, if value < 0, not only the magnitude is set using the norm of the value, but direction of momentum is reversed too, if you want to change charge, use parameter 'standardPdgCode'",
           double(0));

  addParam("qiSmear",
           m_PARAMqiSmear,
           " set True if you want to smear QI's of TCs (needed when no trackLength is chosen for QI calculation) ",
           bool(false));
  addParam("smearMean",
           m_PARAMsmearMean,
           " when qiSmear = True, bias of perturbation can be set here",
           double(0.0));
  addParam("smearSigma",
           m_PARAMsmearSigma,
           " when qiSmear = True, degree of perturbation can be set here",
           double(0.000001));

  addParam("calcQIType",
           m_PARAMcalcQIType,
           "allows you to chose the way, the QI's of the TC's shall be calculated. currently supported: 'kalman','trackLength', 'circleFit', 'straightLine'",
           string("circleFit"));
  addParam("calcSeedType",
           m_PARAMcalcSeedType,
           "allows you to chose the way, the seed-mometa of the TC's shall be calculated. currently supported: 'helixFit', 'straightLine'",
           string("straightLine"));

  addParam("TESTERexpandedTestingRoutines",
           m_TESTERexpandedTestingRoutines,
           "set true if you want to export expanded infos of TCs for further analysis (setting to false means that the DataObject called 'VXDTFInfoBoard' will not be stored)",
           bool(true));
  addParam("writeToRoot",
           m_PARAMwriteToRoot,
           "set true if you want to export the p-values of the fitters in a root file named by parameter 'rootFileName'",
           bool(false));
  addParam("rootFileName",
           m_PARAMrootFileName,
           "fileName used for p-value export. Will be ignored if parameter 'writeToRoot' is false (standard)",
           rootFileNameVals);

}


BaseLineTFModule::~BaseLineTFModule()
{

}

void BaseLineTFModule::initialize()
{
  B2DEBUG(1, "-----------------------------------------------\n       entering base-line track finder (" << m_PARAMnameOfInstance <<
          ") - initialize:");

  checkAndSetupModuleParameters();

  prepareExternalTools();

  B2DEBUG(1, "       leaving base-line track finder (" << m_PARAMnameOfInstance <<
          ") - initialize\n-----------------------------------------------");
}

/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** *******************************+  beginRun +******************************* **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/

void BaseLineTFModule::beginRun()
{
  B2INFO("-----------------------------------------------\n       entering VXD CA track finder (" << m_PARAMnameOfInstance <<
         ") - beginRun.\n       if you want to have some basic infos during begin- and endrun about it, set debug level 1 or 2. Debug level 3 or more gives you event wise output (the higher the level, the more verbose it gets, highest level: 175)");
  B2DEBUG(50,
          "##### be careful, current TF status does not support more than one run per initialization! #####"); /// WARNING TODO: check whether this is still valid

  setupBaseLineTF();

  resetCountersAtBeginRun();

  B2INFO(m_PARAMnameOfInstance <<
         "leaving VXD CA track finder (BaseLineTFModule) - beginRun...\n       -----------------------------------------------");
}

/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** *********************************+ event +********************************* **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/

void BaseLineTFModule::event()
{
  /** REDESIGNCOMMENT EVENT 1:
   ** short:
   * just some preparations to store Infos for debugging and other basic stuff
   *
   ** long (+personal comments):
   * EventInfoPackage is currently storing the info needed for the TFAnalyzerModule to determine the efficiency of the TF
   *
   ** dependency of module parameters (global):
   *
   ** dependency of global in-module variables:
   * m_eventCounter
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   ** in-module-function-calls:
   */

  EventInfoPackage thisInfoPackage;

  boostClock::time_point beginEvent = boostClock::now();

  m_eventCounter++;
  thisInfoPackage.evtNumber = m_eventCounter;
  B2DEBUG(3, "################## entering vxd CA track finder - event " << m_eventCounter << " ######################");
  /** cleaning will be done at the end of the event **/



  /** Section 3 - importing hits and find their papaSectors.**/
  /// Section 3a
  /** REDESIGNCOMMENT EVENT 2:
   * * short:
   * Section 3a - prepare/load in- and output StoreArray-stuff, create virtual VXDTFhits
   *
   ** long (+personal comments):
   * this section does have dependencies of the testbeam package
   *
   ** dependency of module parameters (global):
   * m_PARAMpxdClustersName, m_PARAMsvdClustersName,
   * m_PARAMgfTrackCandsColName, m_TESTERexpandedTestingRoutines,
   *
   ** dependency of global in-module variables:
   * m_usePXDHits, m_useSVDHits,
   * m_TESTERnoHitsAtEvent, m_TESTERtimeConsumption, m_TESTERlogEvents,
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   * m_TESTERtimeConsumption, m_TESTERexpandedTestingRoutines, m_TESTERnoHitsAtEvent,
   * m_TESTERlogEvents,
   *
   ** in-module-function-calls:
   */

  // importing hits

  StoreArray<PXDCluster> aPxdClusterArray(m_PARAMpxdClustersName);
  int nPxdClusters = 0;
  if (m_usePXDHits == true) { nPxdClusters = aPxdClusterArray.getEntries(); }

  StoreArray<SVDCluster> aSvdClusterArray(m_PARAMsvdClustersName);
  int nSvdClusters = 0;
  if (m_useSVDHits == true) { nSvdClusters = aSvdClusterArray.getEntries(); }

  thisInfoPackage.numPXDCluster = nPxdClusters;
  thisInfoPackage.numSVDCluster = nSvdClusters;
  int nTotalClusters = nPxdClusters + nSvdClusters; // counts number of clusters total

  B2DEBUG(3, "event: " << m_eventCounter << ": nPxdClusters: " << nPxdClusters << ", nSvdClusters: " << nSvdClusters);

  boostClock::time_point stopTimer = boostClock::now();
  if (nTotalClusters == 0) {
    B2DEBUG(3, "event: " << m_eventCounter << ": there are no hits, terminating event...");
    m_TESTERnoHitsAtEvent++;

    m_TESTERtimeConsumption.intermediateStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);
    thisInfoPackage.sectionConsumption.intermediateStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);
    thisInfoPackage.totalTime = boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);
    B2DEBUG(3, "event: " << m_eventCounter << ", duration : " << thisInfoPackage.totalTime.count() << "ns");
    m_TESTERlogEvents.push_back(thisInfoPackage);
    return;
  }

  // preparing storearray for trackCandidates and fitted tracks
  StoreArray<genfit::TrackCand> finalTrackCandidates(m_PARAMgfTrackCandsColName);

//   generating virtual Hit at position (0, 0, 0) - needed for virtual segment.
  PositionInfo vertexInfo;
  unsigned int centerSector = FullSecID().getFullSecID(); // automatically produces secID of centerSector
  VxdID centerVxdID = VxdID(0, 0, 0); // dummy VxdID for virtual IP

  stopTimer = boostClock::now();
  m_TESTERtimeConsumption.intermediateStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);
  thisInfoPackage.sectionConsumption.intermediateStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);


  /// Section 3b
  /** REDESIGNCOMMENT EVENT 3:
   * * short:
   * Section 3b - store ClusterInfo for all Clusters (PXD & SVD)
   *
   ** long (+personal comments):
   *
   ** dependency of module parameters (global):
   *
   ** dependency of global in-module variables:
   * m_TESTERtimeConsumption,
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   * m_TESTERtimeConsumption,
   *
   ** in-module-function-calls:
   */

  boostClock::time_point timeStamp = boostClock::now();
  vector<ClusterInfo> clustersOfEvent(nPxdClusters + nSvdClusters); /// contains info which tc uses which clusters

  for (int i = 0; i < nPxdClusters; ++i) {
    ClusterInfo newCluster(i, i, true, false, false, aPxdClusterArray[i], NULL);
    B2DEBUG(50, "Pxd clusterInfo: realIndex " << newCluster.getRealIndex() << ", ownIndex " << newCluster.getOwnIndex());
    clustersOfEvent[i] = newCluster;
    B2DEBUG(50, " PXDcluster " << i << " in position " << i << " stores real Cluster " << clustersOfEvent.at(
              i).getRealIndex() << " at indexPosition of own list (clustersOfEvent): " << clustersOfEvent.at(
              i).getOwnIndex() << " withClustersOfeventSize: " << clustersOfEvent.size());

  }



  for (int i = 0; i < nSvdClusters; ++i) {
    ClusterInfo newCluster(i, i + nPxdClusters, false, true, false, NULL, aSvdClusterArray[i]);
    B2DEBUG(50, "Svd clusterInfo: realIndex " << newCluster.getRealIndex() << ", ownIndex " << newCluster.getOwnIndex());
    clustersOfEvent[i + nPxdClusters] = newCluster;
    B2DEBUG(50, " SVDcluster " << i << " in position " << i + nPxdClusters << " stores real Cluster " <<
            clustersOfEvent.at(i + nPxdClusters).getRealIndex() << " at indexPosition of own list (clustersOfEvent): " <<
            clustersOfEvent.at(i + nPxdClusters).getOwnIndex() << " withClustersOfeventSize: " << clustersOfEvent.size());

  } // the position in the vector is NOT the index it has stored (except if there are no PXDClusters)


  m_TESTERtimeConsumption.hitSorting += boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);
  thisInfoPackage.sectionConsumption.hitSorting += boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);


  /// Section 3c
  /** REDESIGNCOMMENT EVENT 4:
   * * short:
   * Section 3c - BaseLineTF
   *
   ** long (+personal comments):
   * That one should probably become its own module, has dependency of the testbeam package.
   * request: for the case of several TFs executed in the same event, there should be some small container lying in the storeArray which allows these TFs to communicate somehow. E.g. Testbeam: BaseLineTF has found something and is happy with the result, shall be able to inform the following real TFs that they do not have to execute that event any more.
   *
   ** dependency of module parameters (global):
   * m_TESTERexpandedTestingRoutines, m_PARAMinfoBoardName, m_PARAMwriteToRoot,
   *
   ** dependency of global in-module variables:
   * m_baselinePass, m_TESTERstartedBaselineTF, m_TESTERsucceededBaselineTF,
   * m_calcQiType, m_TESTERcountTotalUsedIndicesFinal,
   * m_TESTERcountTotalUsedHitsFinal, m_TESTERcountTotalTCsFinal,  m_TESTERtimeConsumption,
   * m_rootTimeConsumption, m_treeEventWisePtr, m_TESTERlogEvents,
   * m_eventCounter
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   * m_TESTERstartedBaselineTF, m_TESTERsucceededBaselineTF,
   * m_TESTERexpandedTestingRoutines, m_TESTERcountTotalUsedIndicesFinal,
   * m_TESTERcountTotalUsedHitsFinal, m_TESTERcountTotalTCsFinal, m_TESTERtimeConsumption,
   * m_rootTimeConsumption, m_treeEventWisePtr, m_TESTERlogEvents,
   * m_eventCounter
   *
   ** in-module-function-calls:
   * baselineTF(clustersOfEvent, &m_baselinePass)
   * calcInitialValues4TCs(&m_baselinePass)
   * generateGFTrackCand(aTC)
   * cleanEvent(&m_baselinePass)
   */

  timeStamp = boostClock::now();

  // count maximum number of allowed clusters to execute the baseline TF, is number of layers*2 (for svd it's number of Layers*4 because of 1D-Clusters):
  int nTotalClustersThreshold = 0;
  if (m_usePXDHits == true) { nTotalClustersThreshold += 4; }
  if (m_useSVDHits == true) { nTotalClustersThreshold += 16;}

  /// now follows a bypass for the simplistic case of one hit per layer (or <=2 hits per layer at the SVD ), this shall guarantee the reconstruction of single track events...
  /** the following bypass shall reconstruct very simple events like a cosmic event or a testbeam event with only
   * 1 track and no hits of secondary particles. the first check only tests for the most complicated case of a cosmic particle
   * passing each layer twice and hitting the overlapping regions (which results in 2 hits at the same layer in neighbouring ladders)
   * */
  bool useBaseLine = false;
  if ((nTotalClusters - 1 < nTotalClustersThreshold)) {
    useBaseLine = true;
    // in this case it's worth checking whether BaseLineTF is useFull or not
    for (ClusterInfo& aCluster : clustersOfEvent) {
      // collect number of Clusters per Sensor
      map<VxdID, uint> sensorIDs;
      VxdID currentID;
      if (aCluster.isSVD() == true) {
        currentID = aCluster.getSVDCluster()->getSensorID();
      } else if (aCluster.isPXD() == true) {
        currentID = aCluster.getPXDCluster()->getSensorID();
      }
      if (sensorIDs.find(currentID) == sensorIDs.end()) {
        sensorIDs.insert({currentID, 1});
      } else {
        sensorIDs.at(currentID) += 1;
      }
      for (auto& aSensorPack : sensorIDs) {
        if (aSensorPack.first.getLayerNumber() > 2 or aSensorPack.first.getLayerNumber() < 7) {
          if (aSensorPack.second > 2) { useBaseLine = false; }
        } else {
          if (aSensorPack.second > 1) { useBaseLine = false; }
        }
      }
    }
  }
  if (useBaseLine == true) {
    //   generating virtual Hit at position (0, 0, 0) - needed for virtual segment.
    m_baselinePass.centerSector = centerSector;
    vertexInfo.hitPosition = m_baselinePass.origin;
    if (vertexInfo.hitPosition.Mag() > 0.5) { // probably no Belle2-VXD-Setup -> Vertex not known
      vertexInfo.sigmaU = 1.5; //0.1;
      vertexInfo.sigmaV = 1.5; //0.1;
    } else {
      vertexInfo.sigmaU = 0.15;
      vertexInfo.sigmaV = 0.15;
    }
    VXDTFHit* baseLineVertexHit = new VXDTFHit(vertexInfo, 0, NULL, NULL, NULL, Const::IR, centerSector, centerVxdID, 0.0);
    MapOfSectors::iterator secIt = m_baselinePass.sectorMap.find(centerSector);
//     m_baselinePass.centerSector = centerSector;
    if (secIt != m_baselinePass.sectorMap.end()) {
      secIt->second->addHit(baseLineVertexHit);
    } else {
      B2FATAL("baseLinePass: could not add virtual center hit!");
    }
    m_baselinePass.hitVector.push_back(baseLineVertexHit); // used for event-cleaning

    B2DEBUG(3, "event " << m_eventCounter << ": requirements for baseline TF fullfilled, starting baselineTF...");
    m_TESTERstartedBaselineTF++;
    bool successfullyReconstructed = baselineTF(clustersOfEvent, &m_baselinePass);            /// baselineTF
    if (successfullyReconstructed == true) {
      m_TESTERsucceededBaselineTF++;
      if (m_calcQiType != 3) {
        /// determine initial values for all track Candidates:
        calcInitialValues4TCs(&m_baselinePass);                                                /// calcInitialValues4TCs
      } // QiType 3 = straightLine which calculates the seed by itself
      int nTotalIndices = 0, nTotalHitsInTCs = 0;

      for (VXDTFTrackCandidate* aTC : m_baselinePass.tcVector) {
        nTotalIndices += aTC->getPXDHitIndices().size() + aTC->getSVDHitIndices().size();
        nTotalHitsInTCs += aTC->size();
        genfit::TrackCand gfTC = generateGFTrackCand(aTC);                                          /// generateGFTrackCand

        // Collector replaces InfoBoard
        // Import TC and updates the Fit-Information to it (for baseLineVertexHit)


        if (m_TESTERexpandedTestingRoutines == true) {

          VXDTFInfoBoard newBoard;
          StoreArray<VXDTFInfoBoard> extraInfo4GFTCs(m_PARAMinfoBoardName); // needed since I use it only within if-parenthesis

          int indexNumber = finalTrackCandidates.getEntries(); // valid for both, GFTrackCand and InfoBoard
          gfTC.setMcTrackId(indexNumber); // so the GFTrackCand knows which index of infoBoard is assigned to it

          newBoard.assignGFTC(indexNumber); // same number aDEBUGs for the GFTrackCand
          newBoard.fitIsPossible(aTC->getFitSucceeded());
          newBoard.setProbValue(aTC->getTrackQuality());
          extraInfo4GFTCs.appendNew(newBoard);
        }
        finalTrackCandidates.appendNew(gfTC);
      }

      m_TESTERcountTotalUsedIndicesFinal += nTotalIndices;
      m_TESTERcountTotalUsedHitsFinal += nTotalHitsInTCs;
      m_TESTERcountTotalTCsFinal += finalTrackCandidates.getEntries();
      thisInfoPackage.numTCsfinal += finalTrackCandidates.getEntries();
    }

    stopTimer = boostClock::now();
    m_TESTERtimeConsumption.baselineTF += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
    thisInfoPackage.sectionConsumption.baselineTF += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);

    cleanEvent(&m_baselinePass);

    if (successfullyReconstructed == true) {
      if (m_PARAMwriteToRoot == true) {
        m_rootTimeConsumption = (stopTimer - beginEvent).count();
        m_treeEventWisePtr->Fill();
      }


      B2DEBUG(3, "event: " << m_eventCounter << ", baseline succeeded, duration : " << thisInfoPackage.totalTime.count() << "ns");

      m_TESTERlogEvents.push_back(thisInfoPackage);
//       cleanEvent(&m_baselinePass);
      return;
    } // else: classic CA shall do its job...
  }

  B2DEBUG(3, "event: " << m_eventCounter << ", baseLine-only - too many hits -> skipping event, duration : " <<
          thisInfoPackage.totalTime.count() << "ns");

  thisInfoPackage.totalTime = boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);
  m_TESTERlogEvents.push_back(thisInfoPackage);
  cleanEvent(&m_baselinePass);

}



/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** ********************************+  endRun +******************************** **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/


void BaseLineTFModule::endRun()
{
  /** REDESIGNCOMMENT ENDRUN 1:
   * * short:
   * endRun
   *
   ** long (+personal comments):
   * debugging and DQM related stuff is happening here
   *
   ** dependency of module parameters (global):
   * m_PARAMnameOfInstance, m_PARAMcalcSeedType, m_PARAMtuneCutoffs,
   * m_PARAMcalcQIType, m_PARAMnameOfInstance,
   * m_PARAMactivateBaselineTF,
   *
   ** dependency of global in-module variables:
   * m_TESTERlogEvents, m_eventCounter, m_TESTERbadSectorRangeCounterForClusters,
   * m_TESTERclustersPersSectorNotMatching, m_TESTERovercrowdedStrangeSensors,
   * m_TESTERtimeConsumption, m_baselinePass,
   * m_TESTERsucceededBaselineTF, m_TESTERacceptedBrokenHitsTrack, m_TESTERrejectedBrokenHitsTrack,
   * m_TESTERnoHitsAtEvent, m_TESTERtriggeredZigZagXY,
   * m_TESTERtriggeredZigZagXYWithSigma, , m_TESTERtriggeredCircleFit,
   * m_TESTERtriggeredDpT, m_TESTERcountTotalTCsFinal,
   * m_TESTERcountTotalUsedIndicesFinal, m_TESTERcountTotalUsedHitsFinal
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   * m_PARAMnameOfInstance,
   * m_eventCounter, m_PARAMcalcSeedType,
   * m_PARAMtuneCutoffs, m_PARAMcalcQIType,
   * , ,
   * , m_TESTERbadSectorRangeCounterForClusters, m_TESTERclustersPersSectorNotMatching,
   * m_TESTERovercrowdedStrangeSensors,  m_TESTERtimeConsumption,
   * m_TESTERlogEvents, m_PARAMnameOfInstance, m_PARAMactivateBaselineTF,
   * m_baselinePass, m_TESTERsucceededBaselineTF, m_TESTERacceptedBrokenHitsTrack,
   * m_TESTERrejectedBrokenHitsTrack, m_TESTERnoHitsAtEvent,
   * m_TESTERtriggeredZigZagXY, m_TESTERtriggeredZigZagXYWithSigma, ,
   * m_TESTERtriggeredCircleFit, m_TESTERtriggeredDpT, m_TESTERcountTotalTCsFinal,
   * m_TESTERcountTotalUsedIndicesFinal, m_TESTERcountTotalUsedHitsFinal
   *
   ** in-module-function-calls:
   */


  vector<string>::iterator newEndOfVector;
  std::vector<EventInfoPackage>::iterator newEndOfAnothterVector;

  std::sort(m_TESTERlogEvents.begin(), m_TESTERlogEvents.end());
  newEndOfAnothterVector = std::unique(m_TESTERlogEvents.begin(), m_TESTERlogEvents.end());
  m_TESTERlogEvents.resize(std::distance(m_TESTERlogEvents.begin(), newEndOfAnothterVector));
  std::reverse(m_TESTERlogEvents.begin(), m_TESTERlogEvents.end());

  B2DEBUG(1, " ############### " << m_PARAMnameOfInstance << " endRun of " <<  m_eventCounter + 1 <<
          " events ###############\nfor explanations of the values, activate DEBUG-mode (level 2 or greater)!");

  if (m_eventCounter == 0) { m_eventCounter = 1; }
  float invNEvents = 1. / float(m_eventCounter);
  string lineHigh = "------------------------------------------------------------------------------------------";
  string lineApnd = "--------------------------";
  B2DEBUG(1, lineHigh << lineApnd << lineApnd);
  B2DEBUG(1, m_PARAMnameOfInstance << " settings: tuneCutoffs: " << m_PARAMtuneCutoffs << ", QIfilterMode: " << m_PARAMcalcQIType <<
          ", way to calc Seed: " << m_PARAMcalcSeedType << ", chosen settings: ");
  stringstream secInfo;


  B2DEBUG(1, "\nsecUConfig \t\t| secVConfig \t\t|\n" << secInfo.str());
  B2DEBUG(2,
          "Explanation: secConfigs are cuts for sector-calculation (cut*sensorWidth for U, cut*sensorLength for V), same value for each sensor");

  B2DEBUG(1, lineHigh  <<
          "\n m_TESTERbadSectorRangeCounterForClusters\t| m_TESTERclustersPersSectorNotMatching\t| m_TESTERovercrowdedStrangeSensors \t|\n "
          << m_TESTERbadSectorRangeCounterForClusters << "\t| " << m_TESTERclustersPersSectorNotMatching << "\t| " <<
          m_TESTERovercrowdedStrangeSensors << "\t|");

  B2DEBUG(2,
          "Explanation: pxdc: number of PXDClusters, 2D), svdc: SVDClusters(1D)\n svdH: # of SVDHits(guess of actual number of 2D-Track- and -BG-hits)\n catC: SVDClusterCombinations(combining u/v, including ghosthits)\n ghR: ghostHitRate(is only a guess, TODO: should be correctly determined!!)\n noSc: are hits discarded, where no fitting sector could be found\n noFd: hits having no hits in compatible sectors (friends), ooR: sensors which were not in sensitive plane of sensor, m_TESTERbadSectorRangeCounterForClusters: SVDclusters, were no 2nd cluster could be found\n m_TESTERclustersPersSectorNotMatching: times where a SVDsensor hat not the same number of u and v clusters, m_TESTERovercrowdedStrangeSensors: like m_TESTERclustersPersSectorNotMatching, but where a high occupancy case prevented reconstructing them");

  B2DEBUG(1, lineHigh << lineApnd << lineApnd <<
          "\n startedBL\t| succeededBL\t| acceptedTCsWith1DHits\t| rejectedTCsWith1DHits\t| noHitsAtEvent\t| zzXY\t| zXYS\t| cf\t| dpT\t| Tfin\t|\n "
          << m_TESTERstartedBaselineTF << "\t| " << m_TESTERsucceededBaselineTF << "\t| " << m_TESTERacceptedBrokenHitsTrack << "\t| " <<
          m_TESTERrejectedBrokenHitsTrack << "\t| " << m_TESTERnoHitsAtEvent << "\t| " << "\t|" << m_TESTERtriggeredZigZagXY << "\t| " <<
          m_TESTERtriggeredZigZagXYWithSigma << "\t| " << m_TESTERtriggeredCircleFit << "\t| " << m_TESTERtriggeredDpT << "\t| " <<
          m_TESTERcountTotalTCsFinal << "\t| ");

  B2DEBUG(2,
          "Explanation: startedBL: baseLineTF started, succeededBL baselineTF succeeded, acceptedTCsWith1DHits: TCs with 1D-svd-Hits were accepted, rejectedTCsWith1DHits: rejected, noHitsAtEvent: Events without hits\n zzXY: zigzagXY got triggered, cf: circleFit(tuneCircleFitValue), dpT: deltaPt, Tfin: total number of final TCs");

  B2DEBUG(1, lineHigh << lineApnd << lineApnd);

  B2DEBUG(1, std::fixed << std::setprecision(2) << " total time consumption in milliseconds: \n " <<
          "HSort\t|baseTF\t|sgFind\t|nbFind\t|CA \t|tcCol\t|tcFlt\t|kalmn\t|chkOvr\t|clnOvr\t|neuNet\t|others\t|\n" <<
          (m_TESTERtimeConsumption.hitSorting.count() * 0.001) << "\t|" << (m_TESTERtimeConsumption.baselineTF.count() * 0.001) << "\t|" <<
          (m_TESTERtimeConsumption.segFinder.count() * 0.001) << "\t|" << (m_TESTERtimeConsumption.nbFinder.count() * 0.001) << "\t|" <<
          (m_TESTERtimeConsumption.cellularAutomaton.count() * 0.001) << "\t|" << (m_TESTERtimeConsumption.tcc.count() * 0.001) << "\t|" <<
          (m_TESTERtimeConsumption.postCAFilter.count() * 0.001) << "\t|" << (m_TESTERtimeConsumption.kalmanStuff.count() * 0.001) << "\t|"
          << (m_TESTERtimeConsumption.checkOverlap.count() * 0.001) << "\t|" << (m_TESTERtimeConsumption.cleanOverlap.count() * 0.001) <<
          "\t|" << (m_TESTERtimeConsumption.neuronalStuff.count() * 0.001) << "\t|" << (m_TESTERtimeConsumption.intermediateStuff.count() *
              0.001) << "\t|");

//   m_eventCounter
  B2DEBUG(1, std::fixed << std::setprecision(2) << " mean time consumption in microseconds: \n " <<
          "HSort\t|baseTF\t|sgFind\t|nbFind\t|CA \t|tcCol\t|tcFlt\t|kalmn\t|chkOvr\t|clnOvr\t|neuNet\t|others\t|\n" <<
          (m_TESTERtimeConsumption.hitSorting.count() * invNEvents) << "\t|" << (m_TESTERtimeConsumption.baselineTF.count() * invNEvents) <<
          "\t|" << (m_TESTERtimeConsumption.segFinder.count() * invNEvents) << "\t|" << (m_TESTERtimeConsumption.nbFinder.count() *
              invNEvents) << "\t|" << (m_TESTERtimeConsumption.cellularAutomaton.count() * invNEvents) << "\t|" <<
          (m_TESTERtimeConsumption.tcc.count() * invNEvents) << "\t|" << (m_TESTERtimeConsumption.postCAFilter.count() * invNEvents) << "\t|"
          << (m_TESTERtimeConsumption.kalmanStuff.count() * invNEvents) << "\t|" << (m_TESTERtimeConsumption.checkOverlap.count() *
              invNEvents) << "\t|" << (m_TESTERtimeConsumption.cleanOverlap.count() * invNEvents) << "\t|" <<
          (m_TESTERtimeConsumption.neuronalStuff.count() * invNEvents) << "\t|" << (m_TESTERtimeConsumption.
              intermediateStuff.count() * invNEvents) << "\t|");

  B2DEBUG(2,
          "Explanation: HSort: hit sorting, baseTF: baseline TF, sgFind: segment finder , nbFind: neighbouring segments finder, CA: cellular automaton, tcCol: track candidate collector, tcFlt: track candidate filter (e.g. circleFit), kalmn: kalman filter, chkOvr: checking track candidates for overlapping clusters, clnOvr: cleaning track candidates for overlapping clusters, neuNet: neuronal network of Hopfield type, others: everything which was not listed above");

  B2DEBUG(1, lineHigh << lineApnd << lineApnd);

  int numLoggedEvents = m_TESTERlogEvents.size();
  int median = numLoggedEvents / 2;
  int q1 = numLoggedEvents / 100;
  int q25 = numLoggedEvents / 4;
  int q75 = median + q25;
  int q10 = numLoggedEvents / 10;
  int q90 = 9 * q10;
  int q99 = 99 * q1;
  B2DEBUG(1, m_PARAMnameOfInstance << " there were " << numLoggedEvents <<
          " events recorded by the eventLogger, listing slowest, fastest, median q0.1 and q0.9 event:" << endl);
  if (numLoggedEvents != 0) {
    int meanTimeConsumption = 0;
    for (EventInfoPackage& infoPackage : m_TESTERlogEvents) {
      meanTimeConsumption += infoPackage.totalTime.count();
    }
    B2DEBUG(2, "slowest event: " << m_TESTERlogEvents.at(0).Print());
    B2DEBUG(2, "q1 event: " << m_TESTERlogEvents.at(q1).Print());
    B2DEBUG(2, "q10 event: " << m_TESTERlogEvents.at(q10).Print());
    B2DEBUG(2, "q25 event: " << m_TESTERlogEvents.at(q25).Print());
    B2DEBUG(2, "median event: " << m_TESTERlogEvents.at(median).Print());
    B2DEBUG(2, "q75 event: " << m_TESTERlogEvents.at(q75).Print());
    B2DEBUG(2, "q90 event: " << m_TESTERlogEvents.at(q90).Print());
    B2DEBUG(2, "q99 event: " << m_TESTERlogEvents.at(q99).Print());
    B2DEBUG(2, "fastest event: " << m_TESTERlogEvents.at(numLoggedEvents - 1).Print());
    B2DEBUG(1, "manually calculated mean: " << meanTimeConsumption / numLoggedEvents << ", and median: " << m_TESTERlogEvents.at(
              median).totalTime.count() << " of time consumption per event");

    vector<EventInfoPackage> logEventsCopy =
      m_TESTERlogEvents; // copying original since we want to change the internal order now for several times and do not want to break the original
    vector<EventInfoPackage> infoQ(
      5); // for each value we want to find the key figures, we store one entry. first is min, third is median, last is max
    stringstream pxdClusterStream, svdClusterStream, svdHitStream, twoHitCombiStream, twoHitActivatedStream,
                 twoHitDiscardedStream;

    // sort by nPxdClusters:
    std::sort(
      logEventsCopy.begin(),
      logEventsCopy.end(),
      [](const EventInfoPackage & a, const EventInfoPackage & b) -> bool { return a.numPXDCluster < b.numPXDCluster; }
    );
    infoQ.at(0).numPXDCluster = logEventsCopy.at(0).numPXDCluster;
    infoQ.at(1).numPXDCluster = logEventsCopy.at(q25).numPXDCluster;
    infoQ.at(2).numPXDCluster = logEventsCopy.at(median).numPXDCluster;
    infoQ.at(3).numPXDCluster = logEventsCopy.at(q75).numPXDCluster;
    infoQ.at(4).numPXDCluster = logEventsCopy.at(numLoggedEvents - 1).numPXDCluster;
    pxdClusterStream << infoQ[0].numPXDCluster << " / " << infoQ[1].numPXDCluster << " / " << infoQ[2].numPXDCluster << " / " <<
                     infoQ[3].numPXDCluster << " / " << infoQ[4].numPXDCluster << "\n";

    // sort by nSVDClusters:
    std::sort(
      logEventsCopy.begin(),
      logEventsCopy.end(),
      [](const EventInfoPackage & a, const EventInfoPackage & b) -> bool { return a.numSVDCluster < b.numSVDCluster; }
    );
    infoQ.at(0).numSVDCluster = logEventsCopy.at(0).numSVDCluster;
    infoQ.at(1).numSVDCluster = logEventsCopy.at(q25).numSVDCluster;
    infoQ.at(2).numSVDCluster = logEventsCopy.at(median).numSVDCluster;
    infoQ.at(3).numSVDCluster = logEventsCopy.at(q75).numSVDCluster;
    infoQ.at(4).numSVDCluster = logEventsCopy.at(numLoggedEvents - 1).numSVDCluster;
    svdClusterStream << infoQ[0].numSVDCluster << " / " << infoQ[1].numSVDCluster << " / " << infoQ[2].numSVDCluster << " / " <<
                     infoQ[3].numSVDCluster << " / " << infoQ[4].numSVDCluster << "\n";

    // sort by nSVDClusterCombis:
    std::sort(
      logEventsCopy.begin(),
      logEventsCopy.end(),
      [](const EventInfoPackage & a, const EventInfoPackage & b) -> bool { return a.numSVDHits < b.numSVDHits; }
    );
    infoQ.at(0).numSVDHits = logEventsCopy.at(0).numSVDHits;
    infoQ.at(1).numSVDHits = logEventsCopy.at(q25).numSVDHits;
    infoQ.at(2).numSVDHits = logEventsCopy.at(median).numSVDHits;
    infoQ.at(3).numSVDHits = logEventsCopy.at(q75).numSVDHits;
    infoQ.at(4).numSVDHits = logEventsCopy.at(numLoggedEvents - 1).numSVDHits;
    svdHitStream << infoQ[0].numSVDHits << " / " << infoQ[1].numSVDHits << " / " << infoQ[2].numSVDHits << " / " << infoQ[3].numSVDHits
                 << " / " << infoQ[4].numSVDHits << "\n";

    // sort by 2HitCombis:
    std::sort(
      logEventsCopy.begin(),
      logEventsCopy.end(),
      [](const EventInfoPackage & a, const EventInfoPackage & b) -> bool { return a.numHitCombisTotal < b.numHitCombisTotal; }
    );
    infoQ.at(0).numHitCombisTotal = logEventsCopy.at(0).numHitCombisTotal;
    infoQ.at(1).numHitCombisTotal = logEventsCopy.at(q25).numHitCombisTotal;
    infoQ.at(2).numHitCombisTotal = logEventsCopy.at(median).numHitCombisTotal;
    infoQ.at(3).numHitCombisTotal = logEventsCopy.at(q75).numHitCombisTotal;
    infoQ.at(4).numHitCombisTotal = logEventsCopy.at(numLoggedEvents - 1).numHitCombisTotal;
    twoHitCombiStream << infoQ[0].numHitCombisTotal << " / " << infoQ[1].numHitCombisTotal << " / " << infoQ[2].numHitCombisTotal <<
                      " / " << infoQ[3].numHitCombisTotal << " / " << infoQ[4].numHitCombisTotal << "\n";

    // sort by 2HitCombisActivated:
    std::sort(
      logEventsCopy.begin(),
      logEventsCopy.end(),
      [](const EventInfoPackage & a, const EventInfoPackage & b) -> bool { return a.segFinderActivated < b.segFinderActivated; }
    );
    infoQ.at(0).segFinderActivated = logEventsCopy.at(0).segFinderActivated;
    infoQ.at(1).segFinderActivated = logEventsCopy.at(q25).segFinderActivated;
    infoQ.at(2).segFinderActivated = logEventsCopy.at(median).segFinderActivated;
    infoQ.at(3).segFinderActivated = logEventsCopy.at(q75).segFinderActivated;
    infoQ.at(4).segFinderActivated = logEventsCopy.at(numLoggedEvents - 1).segFinderActivated;
    twoHitActivatedStream << infoQ[0].segFinderActivated << " / " << infoQ[1].segFinderActivated << " / " << infoQ[2].segFinderActivated
                          << " / " << infoQ[3].segFinderActivated << " / " << infoQ[4].segFinderActivated << "\n";

    // sort by 2HitCombisDiscarded:
    std::sort(
      logEventsCopy.begin(),
      logEventsCopy.end(),
      [](const EventInfoPackage & a, const EventInfoPackage & b) -> bool { return a.segFinderDiscarded < b.segFinderDiscarded; }
    );
    infoQ.at(0).segFinderDiscarded = logEventsCopy.at(0).segFinderDiscarded;
    infoQ.at(1).segFinderDiscarded = logEventsCopy.at(q25).segFinderDiscarded;
    infoQ.at(2).segFinderDiscarded = logEventsCopy.at(median).segFinderDiscarded;
    infoQ.at(3).segFinderDiscarded = logEventsCopy.at(q75).segFinderDiscarded;
    infoQ.at(4).segFinderDiscarded = logEventsCopy.at(numLoggedEvents - 1).segFinderDiscarded;
    twoHitDiscardedStream << infoQ[0].segFinderDiscarded << " / " << infoQ[1].segFinderDiscarded << " / " << infoQ[2].segFinderDiscarded
                          << " / " << infoQ[3].segFinderDiscarded << " / " << infoQ[4].segFinderDiscarded << "\n";

    B2INFO(" BLTF - endRun: ###############\n" <<
           "within " << m_eventCounter << " events, there were a total number of " << m_TESTERcountTotalTCsFinal << " TCs " <<
           "and " << float(m_TESTERcountTotalTCsFinal) * invNEvents << " TCs per event.\n" <<
           "Mean track length (indices/hits): " << float(m_TESTERcountTotalUsedIndicesFinal) / float(m_TESTERcountTotalTCsFinal) << "/" <<
           float(m_TESTERcountTotalUsedHitsFinal) / float(m_TESTERcountTotalTCsFinal) << "\n\
	min / q0.25 / median / q0.75 / max\n" <<
           "nPxdClusters           " << pxdClusterStream.str() <<
           "nSVDClusters           " << svdClusterStream.str() <<
           "nSVDClusterCombis      " << svdHitStream.str() <<
           "2HitCombis             " << twoHitCombiStream.str() <<
           "2HitCombisActivated    " << twoHitActivatedStream.str() <<
           "2HitCombisDiscarded    " << twoHitDiscardedStream.str() <<
           "BLTF -endRun - end ###############");
  }


  // runWise cleanup:
  for (secMapEntry aSector : m_baselinePass.sectorMap) { // dealing with the baseline pass separately
    delete aSector.second;
  }
  m_baselinePass.sectorMap.clear();
}



void BaseLineTFModule::terminate()
{
  /** REDESIGNCOMMENT TERMINATE 1:
   * * short:
   * root output for track- and event-wise info...
   *
   ** long (+personal comments):
   * only relevant for DQM-stuff
   *
   ** dependency of module parameters (global):
   *
   ** dependency of global in-module variables:
   * m_treeTrackWisePtr, m_treeEventWisePtr, m_rootFilePtr,
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   * m_treeTrackWisePtr, m_treeEventWisePtr, m_rootFilePtr,
   *
   ** in-module-function-calls:
   */
  if (m_treeTrackWisePtr != NULL and m_treeEventWisePtr != NULL) {
    m_rootFilePtr->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module
    m_treeTrackWisePtr->Write();
    m_treeEventWisePtr->Write();
    m_rootFilePtr->Close();
  }
  B2INFO("BLTF::terminate: random number check: " << gRandom->Uniform(1.0));
}



void BaseLineTFModule::calcInitialValues4TCs(PassData* currentPass)
{
  /** REDESIGNCOMMENT CALCINITIALVALUES4TCS 1:
   * * short:
   *
   ** long (+personal comments):
   *
   ** dependency of module parameters (global):
   * m_PARAMpdGCode, m_PARAMartificialMomentum,
   *
   ** dependency of global in-module variables:
   * m_calcSeedType, m_BackwardFilter, m_chargeSignFactor,
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   *
   ** in-module-function-calls:
   */
  int pdgCode = 0;
  pair<TVector3, int> seedValue; // first is momentum vector, second is signCurvature
  pair<double, TVector3> returnValues; // first is chi2, second is momentum vector

  for (VXDTFTrackCandidate* aTC : currentPass->tcVector) {

    if (aTC->getCondition() == false) { continue; }

    const vector<PositionInfo*>* currentHits = aTC->getPositionInfos();

    pdgCode = m_PARAMpdGCode; // general setting
    if (m_calcSeedType == 0) { // helixFit
      currentPass->trackletFilterBox.resetValues(currentHits);
      seedValue = currentPass->trackletFilterBox.calcMomentumSeed(m_BackwardFilter, m_PARAMartificialMomentum);
      pdgCode = seedValue.second * m_PARAMpdGCode * m_chargeSignFactor; // improved one for curved tracks
    } else if (m_calcSeedType == 1) {
      returnValues = currentPass->trackletFilterBox.simpleLineFit3D(currentHits, m_BackwardFilter, m_PARAMartificialMomentum);
      seedValue.first = returnValues.second; // storing the momentum vector at the right place
    } else {
      B2WARNING("calcInitialValues4TCs: unknown seedCalculating type set! Using helixFit instead...");
    }

    if (m_BackwardFilter == true) {
      aTC->setInitialValue((*currentHits)[0]->hitPosition, seedValue.first, pdgCode); // position, momentum, pdgCode
      B2DEBUG(5, " backward: TC has got seedRadius/momentum/pT of " << (*currentHits)[0]->hitPosition.Perp() << "/" <<
              seedValue.first.Mag() << "/" << seedValue.first.Perp() << "GeV and estimated pdgCode " << pdgCode);
    } else {
      aTC->setInitialValue((*currentHits).at(currentHits->size() - 1)->hitPosition, seedValue.first, pdgCode);
      B2DEBUG(5, "forward: TC has got seedRadius/momentum/pT of " << (*currentHits).at(currentHits->size() - 1)->hitPosition.Perp() << "/"
              << seedValue.first.Mag() << "/" << seedValue.first.Perp() << "GeV and estimated pdgCode " << pdgCode);
    }

  }
}


void BaseLineTFModule::calcQIbyStraightLine(TCsOfEvent& tcVector)
{
  /** REDESIGNCOMMENT CALCQIBYSTRAIGHTLINE 1:
   * * short:
   *
   ** long (+personal comments):
   *
   ** dependency of module parameters (global):
   * m_PARAMartificialMomentum, m_PARAMpdGCode,
   *
   ** dependency of global in-module variables:
   * m_BackwardFilter,
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   *
   ** in-module-function-calls:
   */
  TrackletFilters fitterBox;
  std::pair<double, TVector3> lineFitResult = { 0., TVector3()};

  for (VXDTFTrackCandidate* currentTC : tcVector) {
    if (currentTC->getCondition() == false) { continue; }

    const vector<PositionInfo*>* currentHits = currentTC->getPositionInfos();
    lineFitResult.first = 0;
    lineFitResult.second.SetXYZ(0, 0, 0);
    bool blaBackward = false;
    if (m_BackwardFilter == false) { blaBackward = true; }
    try {
      lineFitResult = fitterBox.simpleLineFit3D(currentHits, blaBackward, m_PARAMartificialMomentum);
    } catch (FilterExceptions::Straight_Up& anException) {
      B2ERROR("BaseLineTFModule::calcQIbyStraightLine:lineFit failed , reason: " << anException.what() << ", killing TC...");
      currentTC->setFitSucceeded(false);
      continue;
    }

    currentTC->setTrackQuality(TMath::Prob(lineFitResult.first,
                                           currentTC->size() * 2 - 4)); // 4 parameters are estimated, each hit is created by two measurements

    if (m_BackwardFilter == true) {
      currentTC->setInitialValue((*currentHits)[0]->hitPosition, lineFitResult.second, m_PARAMpdGCode); // position, momentum, pdgCode
      B2DEBUG(5, " backward: TC has got seedRadius/momentum/pT of " << (*currentHits)[0]->hitPosition.Perp() << "/" <<
              lineFitResult.second.Mag() << "/" << lineFitResult.second.Perp() << "GeV and estimated pdgCode " << m_PARAMpdGCode);
    } else {
      currentTC->setInitialValue((*currentHits).at(currentHits->size() - 1)->hitPosition, lineFitResult.second, m_PARAMpdGCode);
      B2DEBUG(5, "forward: TC has got seedRadius/momentum/pT of " << (*currentHits).at(currentHits->size() - 1)->hitPosition.Perp() << "/"
              << lineFitResult.second.Mag() << "/" << lineFitResult.second.Perp() << "GeV and estimated pdgCode " << m_PARAMpdGCode);
    }
    currentTC->setFitSucceeded(true);
  }
}



genfit::TrackCand BaseLineTFModule::generateGFTrackCand(VXDTFTrackCandidate* currentTC)
{
  /** REDESIGNCOMMENT GENERATEGFTRACKCAND 1:
   * * short:
   *
   ** long (+personal comments):
   *
   ** dependency of module parameters (global):
   *
   ** dependency of global in-module variables:
   * m_eventCounter,
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   * m_eventCounter,
   *
   ** in-module-function-calls:
   */
  genfit::TrackCand newGFTrackCand;

  B2DEBUG(50, "BaseLineTFModule::generateGFTrackCand, after newGFTrackCand");

  TVector3 posIn = currentTC->getInitialCoordinates();
  TVector3 momIn = currentTC->getInitialMomentum();
  if (momIn.Mag2() == 0) { B2WARNING("event " << m_eventCounter << ", generateGFTrackCand: currentTC got momentum of 0! probably you forgot to activate magnetic field or you have set the magnetic field value to 0 (in your secMap) and you forgot to set 'artificialMomentum'");}
  TVectorD stateSeed(6); //(x,y,z,px,py,pz)
  TMatrixDSym covSeed(6);
  int pdgCode = currentTC->getPDGCode();
  const vector<int>& pxdHits = currentTC->getPXDHitIndices();
  const vector<int>& svdHits = currentTC->getSVDHitIndices();

  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 10, PACKAGENAME()) == true) {
    stringstream printIndices;
    printIndices << "PXD: ";
    for (int index : pxdHits) { printIndices << index << " "; }
    printIndices << ", SVD: ";
    for (int index : svdHits) { printIndices << index << " "; }
    B2DEBUG(10, "generated GFTC with seed.Mag/p/pT/PDGcode of " << posIn.Mag() << "/" << momIn.Mag() << "/" << momIn.Perp() << "/" <<
            pdgCode << " and the following hits: " << printIndices.str());
  }

  bool gotNan = false;

  auto lambdaCheckVector4NAN = [](TVector3 & aVector) -> bool { /// testing c++11 lambda functions...
    return std::isnan(aVector.Mag2()); // if one of them is 'nan', Mag2 will be 'nan' too
  }; // should be converted to normal function, since feature could be used much more often...
  if (lambdaCheckVector4NAN(posIn) == true) { B2ERROR("event " << m_eventCounter << ":helixParameterFit: posIn got 'nan'-entries x/y/z: " << posIn.X() << "/" << posIn.Y() << "/" << posIn.Z()); gotNan = true; }
  if (lambdaCheckVector4NAN(momIn) == true) { B2ERROR("event " << m_eventCounter << ":helixParameterFit: momIn got 'nan'-entries x/y/z: " << momIn.X() << "/" << momIn.Y() << "/" << momIn.Z()); gotNan = true; }

  stateSeed(0) = posIn[0]; stateSeed(1) = posIn[1]; stateSeed(2) = posIn[2];
  stateSeed(3) = momIn[0]; stateSeed(4) = momIn[1]; stateSeed(5) = momIn[2];
  covSeed(0, 0) = 0.01 ; covSeed(1, 1) = 0.01 ; covSeed(2, 2) = 0.04 ; // 0.01 = 0.1^2 = dx*dx =dy*dy. 0.04 = 0.2^2 = dz*dz
  covSeed(3, 3) = 0.01 ; covSeed(4, 4) = 0.01 ; covSeed(5, 5) = 0.04 ;
//   B2DEBUG(10, "generating GFTrackCandidate: posIn.Mag(): " << posIn.Mag() << ", momIn.Mag(): " << momIn.Mag() << ", pdgCode: " << pdgCode);

  //newGFTrackCand.set6DSeedAndPdgCode(stateSeed, pdgCode, covSeed);
  if (gotNan == true) {
    stringstream hitIndices;
    hitIndices << "PXD: ";
    for (int index : pxdHits) { hitIndices << index << " "; }
    hitIndices << ", SVD: ";
    for (int index : svdHits) { hitIndices << index << " "; }

    hitIndices << "\n Magnitudes: ";
    for (TVector3* hitPos : currentTC->getHitCoordinates()) {
      hitIndices << hitPos->Mag() << " ";
    }
    B2WARNING("pdgCode: " << pdgCode << ", stateSeed0-5: " << stateSeed(0) << "/" << stateSeed(1) << "/" << stateSeed(
                2) << "/" << stateSeed(3) << "/" << stateSeed(4) << "/" << stateSeed(5) << ", hitID/mag: " << hitIndices.str());
  }

  newGFTrackCand.set6DSeedAndPdgCode(stateSeed, pdgCode);

//   vector<int> hitIDs; // for checking hitIDs for double entries
  int hitIndex = -1; // temporary index for hit
  BOOST_REVERSE_FOREACH(auto * aHit,
                        currentTC->getHits()) { // order of hits within VXDTFTrackCandidate: outer->inner hits. GFTrackCand: inner->outer hits
    if (aHit->getDetectorType() == Const::SVD) {
      hitIndex = aHit->getClusterIndexU();
      if (hitIndex != -1) {
        newGFTrackCand.addHit(Const::SVD, hitIndex);
//        hitIDs.push_back(hitIndex);
      }

      hitIndex = aHit->getClusterIndexV();
      if (hitIndex != -1) {
        newGFTrackCand.addHit(Const::SVD, hitIndex);
//        hitIDs.push_back(hitIndex);
      }
    } else {
      hitIndex = aHit->getClusterIndexUV();
      if (hitIndex != -1) {
        newGFTrackCand.addHit(aHit->getDetectorType(), hitIndex);
//        hitIDs.push_back(hitIndex);
      }
    }
  }

  return newGFTrackCand;
}




string BaseLineTFModule::EventInfoPackage::Print()
{
  /** REDESIGNCOMMENT EVENTINFOPACKAGE::PRINT 1:
   * * short:
   *
   ** long (+personal comments):
   * only relevant for DQM/debugging/testing
   *
   ** dependency of module parameters (global):
   *
   ** dependency of global in-module variables:
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   *
   ** in-module-function-calls:
   */
  stringstream output;
  output << " timeConsumption of event " << evtNumber << " in microseconds: " << endl;

  output << "total: " << totalTime.count();
  output << ", baselineTF: " << sectionConsumption.baselineTF.count();
  output << ", hitsorting: " << sectionConsumption.hitSorting.count();
  output << ", sf: "  << sectionConsumption.segFinder.count();
  output << ", nf: " << sectionConsumption.nbFinder.count();
  output << ", ca: " << sectionConsumption.cellularAutomaton.count();
  output << ", tcc: " << sectionConsumption.tcc.count();
  output << ", tcfilter: " << sectionConsumption.postCAFilter.count() << endl;
  output << ", overlap: " << sectionConsumption.checkOverlap.count();
  output << ", kf: " << sectionConsumption.kalmanStuff.count();
  output << ", cleanOverlap: " << sectionConsumption.cleanOverlap.count();
  output << ", nn: " << sectionConsumption.neuronalStuff.count();
  output << ", other: " << sectionConsumption.intermediateStuff.count() << endl;

  output << " results: ";
  output << "nPXDCluster: " << numPXDCluster << ", nSVDCluster: " << numSVDCluster << ", nSVDHits(x-Passes): " << numSVDHits << endl;
  output << "sfActivated: " << segFinderActivated << ", discarded: " << segFinderDiscarded << ", nfActivated: " << nbFinderActivated
         << ", discarded: " << nbFinderDiscarded << endl;
  output << "tccApproved: " << tccApprovedTCs << ", nTCsAfterTCC: " << numTCsAfterTCC << ", nTCsPostTCfilter: " <<
         numTCsAfterTCCfilter << ", nTCsKilledOverlap: " << numTCsKilledByCleanOverlap << ", nTCsFinal: " << numTCsfinal << endl;

  return output.str();
}



bool BaseLineTFModule::baselineTF(vector<ClusterInfo>& clusters, PassData* passInfo)
{
  /** REDESIGNCOMMENT BASELINETF 1:
   * * short:
   *
   ** long (+personal comments):
   * should become its own module
   *
   ** dependency of module parameters (global):
   * m_PARAMnameOfInstance,
   *
   ** dependency of global in-module variables:
   * m_eventCounter, m_TESTERrejectedBrokenHitsTrack, m_TESTERtriggeredZigZagXY,
   * m_TESTERtriggeredCircleFit, m_calcQiType,
   * m_allTCsOfEvent, m_TESTERacceptedBrokenHitsTrack
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   * m_PARAMnameOfInstance, m_eventCounter, m_TESTERrejectedBrokenHitsTrack,
   * m_TESTERtriggeredZigZagXY, m_TESTERtriggeredCircleFit, m_TESTERacceptedBrokenHitsTrack
   *
   ** in-module-function-calls:
   * findSensor4Cluster(activatedSensors, aClusterInfo)
   * find2DSVDHits(activatedSensors, clusterHitList)
   * deliverVXDTFHitWrappedSVDHit(aClusterCombi.uCluster, aClusterCombi.vCluster)
   * dealWithStrangeSensors(activatedSensors, brokenSensors)
   * doTheCircleFit(passInfo, newTC, nHits, 0, 0)
   * calcQIbyStraightLine(singleTC)
   */

  /** overall principle:
   * generate hits (including 1D-hits)
   * sort them to be able to collect TCs (different sorting technique for different cases)
   * collect TCs
   * test them by some basic tests
   * store or break
   **/
  PositionInfo newPosition;
  TVector3 hitLocal;
  TVector3 sigmaVec; // stores globalized vector for sigma values
  VxdID aVxdID;
  int aLayerID, nHits;
  ActiveSensorsOfEvent activatedSensors;
  vector<ClusterHit> clusterHitList;
  vector<VXDTFHit> singleSidedHits;

  for (ClusterInfo& aClusterInfo : clusters) {
    bool isPXD = aClusterInfo.isPXD();

    if (isPXD == true) { // there are pxdHits, only if PXDHits were allowed. pxd-hits are easy, can be stored right away

      hitLocal.SetXYZ(aClusterInfo.getPXDCluster()->getU(), aClusterInfo.getPXDCluster()->getV(), 0);
      sigmaVec.SetXYZ(aClusterInfo.getPXDCluster()->getUSigma(), aClusterInfo.getPXDCluster()->getVSigma(), 9);

      aVxdID = aClusterInfo.getPXDCluster()->getSensorID();
      aLayerID = aVxdID.getLayerNumber();
      const VXD::SensorInfoBase& aSensorInfo = VXD::GeoCache::get(aVxdID);
      newPosition.hitPosition = aSensorInfo.pointToGlobal(hitLocal);
      newPosition.hitSigma = aSensorInfo.vectorToGlobal(sigmaVec);
      B2DEBUG(100, " baselineTF: pxdluster got global pos X/Y/Z: " << newPosition.hitPosition.X() << "/" << newPosition.hitPosition.Y() <<
              "/" << newPosition.hitPosition.Z() << ", global var X/Y/Z: " << newPosition.hitSigma.X() << "/" << newPosition.hitSigma.Y() << "/"
              << newPosition.hitSigma.Z());
      newPosition.sigmaU = aClusterInfo.getPXDCluster()->getUSigma();
      newPosition.sigmaV = aClusterInfo.getPXDCluster()->getVSigma();
      FullSecID aSecID = FullSecID(aVxdID, false, 0);
      VXDTFHit newHit = VXDTFHit(newPosition, 1, NULL, NULL, &aClusterInfo, Const::PXD, aSecID.getFullSecID(), aVxdID, 0);
      passInfo->fullHitsVector.push_back(newHit);

    } else if ((m_useSVDHits == true) && (isPXD == false)) { // svd-hits are tricky, therefore several steps needed
      findSensor4Cluster(activatedSensors, aClusterInfo);                 /// findSensor4Cluster
    }
  }

  // now we have to iterate through the sensors of SVD-cluster again to be able to define our Hits:
  BrokenSensorsOfEvent brokenSensors = find2DSVDHits(activatedSensors, clusterHitList);   /// findSVD2DHits
  int nBrokenSensors = brokenSensors.size();

  for (ClusterHit& aClusterCombi : clusterHitList) {
    VXDTFHit newHit = deliverVXDTFHitWrappedSVDHit(aClusterCombi.uCluster, aClusterCombi.vCluster);
    passInfo->fullHitsVector.push_back(newHit);
  }
  nHits = passInfo->fullHitsVector.size();

  if (nBrokenSensors != 0) {  /// in this case, we should build 1D-clusters

    if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 3, PACKAGENAME()) == true) {
      stringstream badSensors;
      for (int sensorID : brokenSensors) { badSensors << " " << FullSecID(sensorID).getFullSecString(); }
      B2DEBUG(3, m_PARAMnameOfInstance << " - event " << m_eventCounter << " baseline TF: there are " << nBrokenSensors <<
              " sensors with strange cluster-behavior (num of u and v clusters does not mach), sensors are: \n" << badSensors.str());
    }

    // now we check for each strange sensor whether it makes sense to generate 1D-VXDTFHits or not. we therefore filter by threshold to keep us from stumbling over messy sensors
    singleSidedHits = dealWithStrangeSensors(activatedSensors, brokenSensors);

    passInfo->fullHitsVector.insert(passInfo->fullHitsVector.end(), singleSidedHits.begin(), singleSidedHits.end());
    /// missing: could use the timeStamp to improve that situation -> if timestamp of both sides fit -> allowed to form a 2D-hit, else only 1D-option, for those who were allowed to form partners, they are also creating a 1D-hit (total number of possible combinations is reduced by forbidding to combine timing-incompatible hits ). This feature shall only be used, when there is a strange sensor (therefore can be combined with the find2DSVDHits-function as a possibility if strange sensor occurs) -> can not detect cases, where no u-cluster fits to a v-cluster (e.g. if they come from two track where each produced only one cluster)
  } else { B2DEBUG(3, m_PARAMnameOfInstance << " - event " << m_eventCounter << " baseline TF: there are no broken sensors with strange cluster-behavior"); }
  vector<int> hitsPerLayer(6, 0);
  aLayerID = 0;

  typedef pair< double, VXDTFHit*>
  HitExtra; // we use the first variable twice using different meanings (starts with: distance of hit to chosen origin, later: distance of hit to chosen seed). this is pretty messy, but I can not get the tuple-version to work...
  list<HitExtra> listOfHitExtras;

  int maxCounts = 0; // carries the highest number of hits per layer that occured
  for (VXDTFHit& hit : passInfo->fullHitsVector) {
    B2DEBUG(10, " VXDHit at sector " << hit.getSectorString() << " with radius " << hit.getHitCoordinates()->Perp() <<
            " stores real Cluster (u/v/uv)" << hit.getClusterIndexU() << "/" << hit.getClusterIndexV() << "/" << hit.getClusterIndexUV());
    aLayerID = hit.getVxdID().getLayerNumber();
    hitsPerLayer.at(aLayerID - 1) += 1;
    if (hitsPerLayer[aLayerID - 1] > maxCounts) { maxCounts = hitsPerLayer[aLayerID - 1]; }
    listOfHitExtras.push_back(make_pair((*hit.getHitCoordinates() - passInfo->origin).Mag(),
                                        &hit));    // first is distance of hit to chosen origin
  }

  if (int(listOfHitExtras.size()) == 0 or maxCounts < 1) {
    B2DEBUG(1, m_PARAMnameOfInstance << " - event " << m_eventCounter << " baseline TF: got maxCount of VXDHits per layer: " <<
            maxCounts << " while having " << clusters.size() << ", stopping baseline TF since no reconstruction possible");
    return false;
  }

  listOfHitExtras.sort(); // std-sorting starts with first entry, therefore it sorts by distance to origin
  listOfHitExtras.reverse(); // now hits with highest distance to origin are the outermost

  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 3, PACKAGENAME()) == true) {
    stringstream ssHitsPerLayer;
    for (int i = 0; i < 6; ++i) { ssHitsPerLayer << "at layer " << i + 1 << ": " << hitsPerLayer.at(i) << endl; }
    B2DEBUG(3, m_PARAMnameOfInstance << " - event " << m_eventCounter << " baseline TF: before creating TC: nHits (normal/strange) " <<
            nHits << "/" << singleSidedHits.size() << ", Hits per Layer:\n" << ssHitsPerLayer.str());
  }

  VXDTFTrackCandidate* newTC = new VXDTFTrackCandidate();
  B2DEBUG(5, "baselineTF, got maxCount of VXDHits per layer: " << maxCounts);

  // easiest case: no more than 1 Hit per Layer:
  if (maxCounts == 1) {
    for (HitExtra& bundle : listOfHitExtras) {  // collecting hits by distance to chosen origin
      newTC->addHits(bundle.second);
    }
  } else { // case of more than one hit per layer (e.g. overlapping hits, bhabha scattering, cosmic particle, background (last point not surpressed!))
    TVector3 seedPosition = *(*listOfHitExtras.begin()).second->getHitCoordinates();

    aVxdID = 0;
    for (HitExtra& bundle : listOfHitExtras) {  // prepare for sorting by distance to seed:
      bundle.first = (*bundle.second->getHitCoordinates() - seedPosition).Mag();
    }
    listOfHitExtras.sort(); // now first entry is seed, following entries are hits with growing distance

    for (HitExtra& bundle : listOfHitExtras) {  // collect hits for TC
      if (aVxdID == bundle.second->getVxdID()) { continue; }
      newTC->addHits(bundle.second);
      aVxdID = bundle.second->getVxdID();
    }
  }

  // now we have got exactly 1 TC, we do some filtering now:
  const vector<VXDTFHit*>& hitsOfTC = newTC->getHits();
  nHits = hitsOfTC.size();
  vector<PositionInfo*> currentHitPositions;
  currentHitPositions.reserve(nHits);
  for (VXDTFHit* aHit : hitsOfTC) { currentHitPositions.push_back(aHit->getPositionInfo()); }

  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 3, PACKAGENAME()) == true) {
    stringstream secNameOutput;
    secNameOutput << endl << " tc " << 0 << " got " << nHits << " hits and the following secIDs: ";
    for (VXDTFHit* aHit : hitsOfTC) {
      secNameOutput << aHit->getSectorString() << " ";
      B2DEBUG(10, " VXDHit at sector " << aHit->getSectorString() << " with radius " << aHit->getHitCoordinates()->Perp() <<
              " stores real Cluster (u/v/uv)" << aHit->getClusterIndexU() << "/" << aHit->getClusterIndexV() << "/" << aHit->getClusterIndexUV());
    } // used for output
    B2DEBUG(3, " " << secNameOutput.str() << " and " <<  nHits << " hits");
  }

  if (nHits < 3) {
    delete newTC;
    if (nBrokenSensors != 0) { m_TESTERrejectedBrokenHitsTrack++; }
    B2DEBUG(3, m_PARAMnameOfInstance << " - event " << m_eventCounter << " baseline TF: tc rejected, size too small: " << nHits);
    return false;
  }

  // checking for track-loops
  list<VxdID> collectedSensorIDs;
  for (VXDTFHit* aHit : hitsOfTC) {
    collectedSensorIDs.push_back(aHit->getVxdID());
  }
  collectedSensorIDs.sort();
  collectedSensorIDs.unique();
  int nTraversedSensors = collectedSensorIDs.size(); // counting sensors which where passed by the track

  if (nHits != nTraversedSensors) {
    delete newTC;
    if (nBrokenSensors != 0) { m_TESTERrejectedBrokenHitsTrack++; }
    B2DEBUG(3, m_PARAMnameOfInstance << " - event " << m_eventCounter << " baseline TF: tc rejected, got loop in hits: " << nHits);
    return false;
  }


  // feeding trackletFilterbox with hits:
  passInfo->trackletFilterBox.resetValues(&currentHitPositions);
  if (passInfo->zigzagXY.first == true) {
    bool isZiggZagging;

    isZiggZagging = passInfo->trackletFilterBox.ziggZaggXY();
    if (isZiggZagging == true) {
      B2DEBUG(3, m_PARAMnameOfInstance << " - event " << m_eventCounter << " baseline TF: tc rejected by ziggZaggXY! ");
      m_TESTERtriggeredZigZagXY++;
      newTC->setCondition(false);

      delete newTC;
      if (nBrokenSensors != 0) { m_TESTERrejectedBrokenHitsTrack++; }
      return false;
    }
    B2DEBUG(4, m_PARAMnameOfInstance << " - event " << m_eventCounter << " baseline TF: ziggZaggXY approved TC");
  } else { B2DEBUG(3, "baseline-TF: filtering zigzagXY is deactivated, passing test"); }



  if (passInfo->circleFit.first == true) {
    bool survivedCF = false;
    try {
      survivedCF = doTheCircleFit(passInfo, newTC, nHits, 0, 0);
    } catch (FilterExceptions::Calculating_Curvature_Failed& anException) {
      survivedCF = false;
      B2WARNING("baselineTF:doTheCircleFit failed, reason: " << anException.what() << ", trying lineFit instead...");
      pair<double, TVector3> lineFitResult;
      survivedCF = true;
      try {
        lineFitResult = passInfo->trackletFilterBox.simpleLineFit3D();
        newTC->setTrackQuality(TMath::Prob(lineFitResult.first, newTC->size() - 3));
        newTC->setFitSucceeded(true);
      } catch (FilterExceptions::Straight_Up& anException) {
        B2ERROR("baselineTF:loLineFit failed too , reason: " << anException.what() << ", killing TC...");
      }
    } catch (FilterExceptions::Center_Is_Origin& anException) {
      survivedCF = false;
      B2WARNING("baselineTF:doTheCircleFit failed, reason: " << anException.what());
    }  catch (FilterExceptions::Circle_too_small& anException) {
      survivedCF = false;
      B2WARNING("baselineTF:doTheCircleFit failed, reason: " << anException.what());
    }
    if (survivedCF == false) {

      delete newTC;
      m_TESTERtriggeredCircleFit++;
      if (nBrokenSensors != 0) { m_TESTERrejectedBrokenHitsTrack++; }
      B2DEBUG(3, m_PARAMnameOfInstance << " - event " << m_eventCounter << " baseline TF: rejected by circleFit");
      m_TESTERtriggeredCircleFit++;
      return false;
    }
    B2DEBUG(4, m_PARAMnameOfInstance << " - event " << m_eventCounter << " baseline TF: circleFit approved TC");
  } else { B2DEBUG(3, "baseline-TF: filtering circleFit is deactivated, passing test"); }

  if (m_calcQiType == 3) { // does fitting and calculates initial values for the TC
    TCsOfEvent singleTC = {newTC};
    calcQIbyStraightLine(singleTC);
  }


  if (passInfo->pT.first == true) {
    double pT = passInfo->trackletFilterBox.calcPt();
    if (pT < 0.01) {   // smaller than 10 MeV, WARNING: hardcoded!
      delete newTC;
      if (nBrokenSensors != 0) { m_TESTERrejectedBrokenHitsTrack++; }
      B2DEBUG(3, m_PARAMnameOfInstance << " - event " << m_eventCounter << " baseline TF: rejected by pT (too small: " << pT << "GeV/c)");
      //m_TESTERtriggeredpT++; TODO: implement!
      return false;
    }
    B2DEBUG(4, m_PARAMnameOfInstance << " - event " << m_eventCounter << " baseline TF: pT approved TC");
  } else { B2DEBUG(3, "baseline-TF: filtering pT is deactivated, passing test"); }

  passInfo->tcVector.push_back(newTC);
  m_allTCsOfEvent.push_back(newTC); // for garbage collection in the end

  if (nBrokenSensors != 0) { m_TESTERacceptedBrokenHitsTrack++; }

  B2DEBUG(3, m_PARAMnameOfInstance << " - event " << m_eventCounter << " baseline TF: tc approved!");
  return true; // is true if reconstruction was successfull
}




// store a cluster (as a clusterPtr) in a map(uniID, sensorStruct), where sensorStruct contains 2 vectors (uClusters and vClusters).
void BaseLineTFModule::findSensor4Cluster(ActiveSensorsOfEvent& activatedSensors, ClusterInfo& aClusterInfo)
{
  /** REDESIGNCOMMENT FINDSENSORS4CLUSTER 1:
   * * short:
   *
   ** long (+personal comments):
   *
   ** dependency of module parameters (global):
   *
   ** dependency of global in-module variables:
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   *
   ** in-module-function-calls:
   */
  ActiveSensorsOfEvent::iterator sensorIter;
  typedef pair<int, SensorStruct > mapEntry;

  VxdID aVxdID = aClusterInfo.getSVDCluster()->getSensorID();
  int aUniID = aVxdID.getID();
  sensorIter = activatedSensors.find(aUniID);
  if (sensorIter == activatedSensors.end()) {
    SensorStruct newSensor;
    newSensor.layerID = aVxdID.getLayerNumber();
    sensorIter = activatedSensors.insert(sensorIter, mapEntry(aUniID, newSensor)); //activatedSensors.find(aUniID);
  }
  if (aClusterInfo.getSVDCluster()->isUCluster() == true) {
    sensorIter->second.uClusters.push_back(&aClusterInfo);
  } else {
    sensorIter->second.vClusters.push_back(&aClusterInfo);
  } // else: it's no cluster
}



VXDTFHit BaseLineTFModule::deliverVXDTFHitWrappedSVDHit(ClusterInfo* uClusterInfo, ClusterInfo* vClusterInfo)
{
  /** REDESIGNCOMMENT DELIVERVXDTFHITWRAPPEDSVDHIT 1:
   * * short:
   *
   ** long (+personal comments):
   * here are some hardcoded values. a general way to store such stuff is extremely apprechiated. Ideas?
   * That function is only needed by the baseLineTF
   *
   ** dependency of module parameters (global):
   *
   ** dependency of global in-module variables:
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   *
   ** in-module-function-calls:
   */
  float timeStampU = 0, timeStampV = 0;
  TVector3 hitLocal;
  TVector3 sigmaVec; // stores globalized vector for sigma values
  PositionInfo newPosition;
  VxdID aVxdID;

  if (uClusterInfo != NULL) {
    timeStampU = uClusterInfo->getSVDCluster()->getClsTime();
    aVxdID = uClusterInfo->getSVDCluster()->getSensorID();
  }
  if (vClusterInfo != NULL) {
    timeStampV = vClusterInfo->getSVDCluster()->getClsTime();
    aVxdID = vClusterInfo->getSVDCluster()->getSensorID(); // could overwrite the value assigned by uCluster, but it's not important, since they are on the same sensor
  }

//   int aLayerID = aVxdID.getLayerNumber();

  const VXD::SensorInfoBase& aSensorInfo = dynamic_cast<const VXD::SensorInfoBase&>(VXD::GeoCache::get(aVxdID));

  /// WARNING we are ignoring the case of a missing cluster at a wedge sensor (at least for the calculation of the error. For the Kalman filter, this should be unimportant since it is working with local coordinates and an axis transformation (where the problem of the dependency of clusters at both side does not occur), this will be a problem for the circleFitter, which is working with global coordinates, where the dependeny is still there!)
  if (vClusterInfo != NULL) {
    hitLocal.SetY(vClusterInfo->getSVDCluster()->getPosition()); // always correct
    newPosition.sigmaV = vClusterInfo->getSVDCluster()->getPositionSigma();
    sigmaVec.SetY(vClusterInfo->getSVDCluster()->getPositionSigma());
  } else {
    hitLocal.SetY(0.); // is center of the plane
    newPosition.sigmaV = aSensorInfo.getBackwardWidth() * 0.288675135; // std deviation of uniformly distributed value (b-a)/sqrt(12)
    sigmaVec.SetY(aSensorInfo.getBackwardWidth() * 0.288675135);
  }

  if (uClusterInfo != NULL) {
    if ((aSensorInfo.getBackwardWidth() > aSensorInfo.getForwardWidth()) == true
        && vClusterInfo != NULL) {   // isWedgeSensor and 2D-Info
      hitLocal.SetX((hitLocal.Y() / aSensorInfo.getWidth(0)) * uClusterInfo->getSVDCluster()->getPosition()); // hitLocal.Y is already set
    } else { // rectangular Sensor and/or no 2D-info (in this case the X-value of the center of the sensor is taken)
      hitLocal.SetX(uClusterInfo->getSVDCluster()->getPosition());
    }
    newPosition.sigmaU = uClusterInfo->getSVDCluster()->getPositionSigma();
    sigmaVec.SetX(uClusterInfo->getSVDCluster()->getPositionSigma());
  } else {
    hitLocal.SetX(0.); // is center of the plane
    newPosition.sigmaU = aSensorInfo.getLength() * 0.288675135; // std deviation of uniformly distributed value (b-a)/sqrt(12)
    sigmaVec.SetX(aSensorInfo.getLength() * 0.288675135);
  }

  newPosition.hitPosition = aSensorInfo.pointToGlobal(hitLocal);
  newPosition.hitSigma = aSensorInfo.vectorToGlobal(sigmaVec);
  B2DEBUG(10, "deliverVXDTFHitWrappedSVDHit: got global pos X/Y/Z: " << newPosition.hitPosition.X() << "/" <<
          newPosition.hitPosition.Y() << "/" << newPosition.hitPosition.Z() << ", global var X/Y/Z: " << newPosition.hitSigma.X() << "/" <<
          newPosition.hitSigma.Y() << "/" << newPosition.hitSigma.Z()); /// WARNING TODO: set to debug level 100

  FullSecID aSecID = FullSecID(aVxdID, false, 0);
  return VXDTFHit(newPosition, 1, uClusterInfo, vClusterInfo, NULL, Const::SVD, aSecID.getFullSecID(), aVxdID,
                  0.5 * (timeStampU + timeStampV));
}



// now we check for each strange sensor whether it makes sense to generate 1D-VXDTFHits or not. we therefore filter by threshold to keep us from stumbling over messy sensors
std::vector<VXDTFHit> BaseLineTFModule::dealWithStrangeSensors(ActiveSensorsOfEvent& activatedSensors,
    BrokenSensorsOfEvent& strangeSensors)
{
  /** REDESIGNCOMMENT DEALWITHSTRANGESENSORS 1:
   * * short:
   *
   ** long (+personal comments):
   * That function is only needed by the baseLineTF
   *
   ** dependency of module parameters (global):
   *
   ** dependency of global in-module variables:
   * m_TESTERovercrowdedStrangeSensors,
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   * m_TESTERovercrowdedStrangeSensors,
   *
   ** in-module-function-calls:
   * deliverVXDTFHitWrappedSVDHit(aClusterInfo, NULL)
   * deliverVXDTFHitWrappedSVDHit(NULL, aClusterInfo)
   */
  ActiveSensorsOfEvent::iterator itCurrentSensor;
  vector<VXDTFHit> singleSidedHits;
  int nClusters = 0,
      threshold =
        2; // if there are 1 or 0 clusters at the sensor (the latter should not occur at all), there can not be formed any u + v - combination of clusters
  singleSidedHits.reserve(strangeSensors.size()*threshold);
  bool takeUside = false;

  // check which type (u/v) got more clusters (most crowded side) -> nClusters
  // if nClusters < threshold
  // -> generate foreach cluster of most crowded side a 1D-VXDTF-Hit, take the center of the sensorplane for the missing side and huge error for it (needed for kalman/circleFit)
  for (int sensorID : strangeSensors) {
    itCurrentSensor = activatedSensors.find(sensorID);
    if (itCurrentSensor == activatedSensors.end()) { continue; } // well, should not occur, but safety first ;)

    int numUclusters = itCurrentSensor->second.uClusters.size();
    int numVclusters = itCurrentSensor->second.vClusters.size();
    if (numUclusters > numVclusters) { nClusters = numUclusters; takeUside = true; } else { nClusters = numVclusters; takeUside = false; }
    if (nClusters > threshold) { m_TESTERovercrowdedStrangeSensors++; continue; }

    if (takeUside == true) {
      for (ClusterInfo* aClusterInfo : itCurrentSensor->second.uClusters) {
        if (aClusterInfo != NULL) { singleSidedHits.push_back(deliverVXDTFHitWrappedSVDHit(aClusterInfo, NULL)); }
      }
    } else {
      for (ClusterInfo* aClusterInfo : itCurrentSensor->second.vClusters) {
        if (aClusterInfo != NULL) { singleSidedHits.push_back(deliverVXDTFHitWrappedSVDHit(NULL, aClusterInfo)); }
      }
    }

  }
  return singleSidedHits;
}



// iterate through map of activated sensors & combine each possible combination of clusters. Store them in a vector of structs, where each struct carries an u & a v cluster
BaseLineTFModule::BrokenSensorsOfEvent BaseLineTFModule::find2DSVDHits(ActiveSensorsOfEvent& activatedSensors,
    std::vector<ClusterHit>& clusterHitList)
{
  /** REDESIGNCOMMENT FIND2DSVDHITS 1:
   * * short:
   *
   ** long (+personal comments):
   * is used by the baseLineTF
   *
   ** dependency of module parameters (global):
   * m_PARAMnameOfInstance,
   *
   ** dependency of global in-module variables:
   * m_TESTERbadSectorRangeCounterForClusters, m_eventCounter, m_TESTERclustersPersSectorNotMatching,
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   * m_TESTERbadSectorRangeCounterForClusters, m_eventCounter, m_TESTERclustersPersSectorNotMatching,
   * m_PARAMnameOfInstance,
   *
   ** in-module-function-calls:
   */
  typedef pair<unsigned int, SensorStruct > mapEntry;
  BrokenSensorsOfEvent strangeSensors;
  int numHits = 0;
  bool isStrange = false;
  for (const mapEntry& aSensor : activatedSensors) {
    int numUclusters = aSensor.second.uClusters.size();
    int numVclusters = aSensor.second.vClusters.size();
    B2DEBUG(100, " sensor " << FullSecID(VxdID(aSensor.first), false,
                                         0) << " has got " << numUclusters << " uClusters and " << numVclusters << " vClusters");
    if (numUclusters == 0 || numVclusters == 0) {
      m_TESTERbadSectorRangeCounterForClusters++;
      B2DEBUG(3, "at event: " << m_eventCounter << " sensor " << FullSecID(VxdID(aSensor.first), false,
              0) << " at layer " << aSensor.second.layerID << " has got " << numUclusters << "/" << numVclusters << " u/vclusters!");
      isStrange = true;
    }
    if (numUclusters != numVclusters) {
      m_TESTERclustersPersSectorNotMatching++;
      B2DEBUG(4, "at event: " << m_eventCounter << " at sensor " << FullSecID(VxdID(aSensor.first), false,
              0) << " at layer " << aSensor.second.layerID << " number of clusters do not match: Has got " << numUclusters << "/" << numVclusters
              << " u/vclusters!");
      isStrange = true;
    }
    if (isStrange == true) { strangeSensors.push_back(aSensor.first); isStrange = false; }
    /// possible extension: adding here the case for strange sensors (instead of returning them), code down below would be used only for normal ones, see

    for (int uClNum = 0; uClNum < numUclusters; ++uClNum) {
      for (int vClNum = 0; vClNum < numVclusters; ++vClNum) {
        ClusterHit aClusterCombi;
        aClusterCombi.uCluster = aSensor.second.uClusters[uClNum];
        aClusterCombi.vCluster = aSensor.second.vClusters[vClNum];
        clusterHitList.push_back(aClusterCombi);
        ++numHits;
      }
    }

    numHits = 0;
  }

  if (strangeSensors.size() != 0 and LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 4, PACKAGENAME()) == true) {
    stringstream output;
    output << m_PARAMnameOfInstance << " - event: " << m_eventCounter <<
           ": there were strange sensors (having missing clusters) during this event, activated Sensors (ATTENTION: these are sensors, not sectors, therefore no sublayer and sector-info) were:\n";
    for (const mapEntry& aSensor : activatedSensors) { output << " " << FullSecID(VxdID(aSensor.first), false, 0); }
    output << "\n strange sensors were:\n";
    for (unsigned int sensorID : strangeSensors) { output << " " << FullSecID(VxdID(sensorID), false, 0); }
    B2DEBUG(4, "\n\n" << output.str() << "\n\n");
  }

  return strangeSensors;
}



bool BaseLineTFModule::doTheCircleFit(PassData* thisPass, VXDTFTrackCandidate* aTc, int nHits, int tcCtr, int addDegreesOfFreedom)
{
  /** REDESIGNCOMMENT TERMINATE 1:
   * * short:
   *
   ** long (+personal comments):
   *
   ** dependency of module parameters (global):
   *  m_PARAMqiSmear
   *
   ** dependency of global in-module variables:
   * m_TESTERtriggeredCircleFit, m_calcQiType, m_littleHelperBox,
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   * m_TESTERtriggeredCircleFit,
   *
   ** in-module-function-calls:
   *  writeToRootFile(probability, chi2, estimatedRadius, nHits - 3 + addDegreesOfFreedom)
   */
  boostNsec duration;
  boostClock::time_point timer = boostClock::now();
  double closestApproachPhi, closestApproachR, estimatedRadius, estimatedCurvature;
  double chi2 = thisPass->trackletFilterBox.circleFit(closestApproachPhi, closestApproachR, estimatedCurvature);
  estimatedRadius = 1. / estimatedCurvature;
  if (estimatedRadius < 0) { estimatedRadius = -estimatedRadius; }
  aTc->setEstRadius(estimatedRadius);
  if (chi2 < 0) { chi2 = 0; }
  double probability = TMath::Prob(chi2, nHits - 3 + addDegreesOfFreedom);
  // why is there nHits - 3 + addDegreesOfFreedom? Answer: each hit is one additional degree of freedom (since only the measurement of the u-coordinate of the sensors can be used) but 3 parameters are measured, therefore 3 has to be substracted from the number of hits to get the ndf. The additional degree of freedom (+1) is there, since the origin is used as another hit for the circlefitter but has two degrees of freedom instead of one for the normal hits. therefore +1 has to be added again.
  B2DEBUG(10, "TCC Filter at tc " << tcCtr << ": estimated closestApproachPhi, closestApproachR, estimatedRadius: " <<
          closestApproachPhi << ", " << closestApproachR << ", " << estimatedRadius << " got fitted with chi2 of " << chi2 <<
          " and probability of " << probability << " with ndf: " << nHits - 3 + addDegreesOfFreedom);
  if (thisPass->circleFit.first == true and probability < thisPass->circleFit.second) {  // means tc is bad
    B2DEBUG(20, "TCC filter: tc " << tcCtr << " rejected by circleFit! ");
    m_TESTERtriggeredCircleFit++; tcCtr++;
    aTc->setCondition(false);


    writeToRootFile(probability, chi2, estimatedRadius, nHits - 3 + addDegreesOfFreedom);
    return false;
  }
  if (m_calcQiType == 2) {
    writeToRootFile(probability, chi2, estimatedRadius, nHits - 3 + addDegreesOfFreedom);
    if (m_PARAMqiSmear == true) { probability = m_littleHelperBox.smearNormalizedGauss(probability); }
    aTc->setTrackQuality(probability);
    aTc->setFitSucceeded(true);
  }
  boostClock::time_point timer2 = boostClock::now();
  duration = boost::chrono::duration_cast<boostNsec>(timer2 - timer);

  B2DEBUG(20, " TCC filter circleFit approved TC " << tcCtr << " with nHits: " <<  nHits << ", time consumption: " << duration.count()
          << " ns");
  return true;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// REDESIGN - new functions encapsulating smaller tasks
/////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////
/// REDESIGN - Functions for initialize:
////////////////////////////////////////

void BaseLineTFModule::checkAndSetupModuleParameters()
{
  /** REDESIGNCOMMENT INITIALIZE 1:
   * * short:
   * safety checks for many parameters
   *
   ** long (+personal comments):
   * safety checks are not done for each parameter, but for those which are often set wrong (by me or others).
   * one can notice that some parameters appear as variables too.
   * Reason for this is to make the parameters human readable and work internally with faster data types
   *
   ** dependency of module parameters (global):
   * , m_PARAMpdGCode, m_PARAMtuneCutoffs, m_PARAMnameOfInstance,
   * m_PARAMcalcQIType, m_PARAMcalcSeedType, m_PARAMsmearSigma,
   * m_PARAMwriteToRoot, m_PARAMrootFileName, m_PARAMsmearMean,
   *
   ** dependency of global in-module variables:
   * m_chargeSignFactor, m_calcQiType, m_calcSeedType,
   * m_littleHelperBox, m_rootFilePtr, m_treeEventWisePtr,
   * m_treeTrackWisePtr, m_rootTimeConsumption, m_rootPvalues,
   * m_rootChi2, m_rootCircleRadius, m_rootNdf,
   *
   ** dependency of global stuff just because of B2XX-output:
   * m_PARAMnameOfInstance,
   *
   ** in-module-function-calls:
   */

  m_littleHelperBox.resetValues(m_PARAMsmearMean, m_PARAMsmearSigma);

  if (m_PARAMpdGCode > 10
      and m_PARAMpdGCode <
      18) { // in this case, its a lepton. since leptons with positive sign have got negative codes, this must be taken into account
    m_chargeSignFactor = 1;
  } else { m_chargeSignFactor = -1; }

  // check misusage of parameters:
  if (m_PARAMtuneCutoffs <= -99.0 or m_PARAMtuneCutoffs > 1000.0) {
    B2WARNING(m_PARAMnameOfInstance << ": chosen value for parameter 'tuneCutoffs' is invalid, reseting value to standard (=0.0)...");
    m_PARAMtuneCutoffs = 0.;
  } else {
    m_PARAMtuneCutoffs = m_PARAMtuneCutoffs * 0.01; // reformatting to faster calculation
  }


  B2DEBUG(1, m_PARAMnameOfInstance << "::initialize: chosen calcQIType is '" << m_PARAMcalcQIType << "'");
  if (m_PARAMcalcQIType == "circleFit") {
    m_calcQiType = 2;
  } else if (m_PARAMcalcQIType == "straightLine") {
    m_calcQiType = 3;
  } else {
    B2WARNING(m_PARAMnameOfInstance << "::initialize: chosen qiType '" << m_PARAMcalcQIType <<
              "' is unknown, setting standard to circleFit...");
    m_calcQiType = 2;
  }

  B2DEBUG(1, m_PARAMnameOfInstance << "::initialize: chosen calcSeedType is '" << m_PARAMcalcSeedType << "'");
  if (m_PARAMcalcSeedType == "helixFit") {
    m_calcSeedType = 0;
  } else if (m_PARAMcalcSeedType == "straightLine") {
    m_calcSeedType = 1;
  } else {
    B2WARNING(m_PARAMnameOfInstance << "::initialize: chosen seedType '" << m_PARAMcalcSeedType <<
              "' is unknown, setting standard to helixFit...");
    m_calcSeedType = 0;
  }


  if (m_PARAMwriteToRoot == true) {
    m_PARAMrootFileName.at(0) += ".root";
    m_rootFilePtr = new TFile(m_PARAMrootFileName.at(0).c_str(), m_PARAMrootFileName.at(1).c_str()); // alternative: UPDATE

    m_treeEventWisePtr = new TTree("m_treeEventWisePtr", "anEventWiseTree");
    m_treeEventWisePtr->Branch("duration", &m_rootTimeConsumption);

    m_treeTrackWisePtr = new TTree("m_treeTrackWisePtr", "aTrackWiseTree");
    m_treeTrackWisePtr->Branch("pValues", &m_rootPvalues);
    m_treeTrackWisePtr->Branch("chi2Values", &m_rootChi2);
    m_treeTrackWisePtr->Branch("circleRadii", &m_rootCircleRadius);
    m_treeTrackWisePtr->Branch("ndfValues", &m_rootNdf);
  } else {
    m_rootFilePtr = NULL;
    m_treeTrackWisePtr = NULL;
  }
}



void BaseLineTFModule::prepareExternalTools()
{
  /** REDESIGNCOMMENT INITIALIZE 2:
   * * short:
   * register storearray-related stuff
   *
   ** long (+personal comments):
   * some are only created if their related parameters are set to the values needed.
   * additionally, one module parameter is not flagged with m_PARAM... but with m_TESTER... (which are parameters related only to testing purposes),
   * such an inconsistency shall be surpressed for the redesign.
   * DisplayCollector-related stuff is needed for display and for debugging, therefore always listed for debug-stuff too.
   *
   ** dependency of module parameters (global):
   * m_PARAMgfTrackCandsColName, m_TESTERexpandedTestingRoutines, m_PARAMinfoBoardName,
   * m_PARAMpxdClustersName, m_PARAMsvdClustersName
   *
   ** dependency of global in-module variables:
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   * m_PARAMinfoBoardName,
   *
   ** in-module-function-calls:
   */


  /// genfit::TrackCandidate
  StoreArray<genfit::TrackCand>::registerPersistent(m_PARAMgfTrackCandsColName);


  /// temporary members for testing purposes (minimal testing routines)
  if (m_TESTERexpandedTestingRoutines == true) {
    StoreArray<VXDTFInfoBoard>::registerPersistent(m_PARAMinfoBoardName);
  }


  StoreArray<PXDCluster>::optional(m_PARAMpxdClustersName);
  StoreArray<SVDCluster>::optional(m_PARAMsvdClustersName);
}



//////////////////////////////////////
/// REDESIGN - Functions for beginRun:
//////////////////////////////////////

void BaseLineTFModule::setupBaseLineTF()
{
  /** REDESIGNCOMMENT BEGINRUN 1:
   * * short:
   * setup for sectorMap used for the baseline tf
   *
   ** long (+personal comments):
   *
   ** dependency of module parameters (global):
   * m_PARAMsectorSetup,
   * m_PARAMactivateZigZagXY, m_PARAMactivateZigZagXYWithSigma,
   * m_PARAMactivateDeltaPt, m_PARAMactivateCircleFit,
   * m_PARAMtuneZigZagXY, m_PARAMtuneZigZagXYWithSigma,
   * m_PARAMtuneDeltaPt, m_PARAMtuneCircleFit,
   *
   ** dependency of global in-module variables:
   * m_usePXDHits, m_useSVDHits
   *
   *
   ** dependency of global stuff just because of B2XX-output or debugging only:
   * m_PARAMnameOfInstance,
   *
   ** in-module-function-calls:
   */

  // here some variables copied from the passes (will be rewritten for each pass, therefore only the settings of the last pass will survive)
  TVector3 origin;
  string detectorType;
  /// for each setup, fill parameters, calc numTotalLayers... TODO: failsafe implementation (currently no protection against bad user imput) lacks of style, longterm goal, export that procedure into a function

  m_usePXDHits = false, m_useSVDHits = false;

  m_baselinePass.sectorSetup = m_PARAMsectorSetup;

  VXDTFSecMap::Class(); // essential, needed for root, waiting for root 6 to be removed (hopefully)
  string chosenSetup = m_baselinePass.sectorSetup;
  string directory = "/Detector/Tracking/CATFParameters/" + chosenSetup;
  const VXDTFSecMap* newMap = NULL;
  try {
    newMap = dynamic_cast<const VXDTFSecMap*>(Gearbox::getInstance().getTObject(directory.c_str()));
  } catch (exception& e) {
    B2FATAL("BaseLineTFModule::initialize: could not load sectorMap. Reason: exception thrown: " << e.what() <<
            ", this means you have to check whether the sectorMaps stored in ../tracking/data/VXDTFindex.xml and/or ../testbeam/vxd/data/VXDTFindexTF.xml are uncommented and locally unpacked and available!");
  }

  const double magneticField = newMap->getMagneticFieldStrength();
  m_baselinePass.magneticFieldStrength = magneticField;
  m_baselinePass.threeHitFilterBox.resetMagneticField(magneticField);
  m_baselinePass.fourHitFilterBox.resetMagneticField(magneticField);
  m_baselinePass.trackletFilterBox.resetMagneticField(magneticField);

  m_baselinePass.additionalInfo = newMap->getAdditionalInfo();

  m_baselinePass.secConfigU = newMap->getSectorConfigU();
  m_baselinePass.secConfigV = newMap->getSectorConfigV();
  origin = newMap->getOrigin();
  m_baselinePass.origin = origin;
  stringstream secConU, secConV;
  for (double entry : m_baselinePass.secConfigU) { secConU << " " << entry; }
  for (double entry : m_baselinePass.secConfigV) { secConV << " " << entry; }
  B2INFO(m_baselinePass.sectorSetup << "-setting: got magneticFieldStrength: " << magneticField << ", origin at: (" << origin[0] <<
         "," << origin[1] << "," << origin[2] << ") and sectorConfig \n U: " << secConU.str() << endl << " V: " << secConV.str() << endl <<
         " and additional Info: " << m_baselinePass.additionalInfo);

  detectorType = newMap->getDetectorType();
  m_baselinePass.chosenDetectorType = detectorType;
  m_baselinePass.numTotalLayers = 0;

  if (detectorType.find("SVD") != std::string::npos) {
    m_useSVDHits = true;
    m_baselinePass.useSVDHits = true;
    m_baselinePass.numTotalLayers +=
      4; // WARNING hardcoded! can we get this info from the system itself? WARNING find where this is still used and find out its purpose (dangerous when some layers are missing?)

  }
  if (detectorType.find("PXD") != std::string::npos) {
    m_usePXDHits = true;
    m_baselinePass.usePXDHits = true;
    m_baselinePass.numTotalLayers +=
      2; // WARNING hardcoded! can we get this info from the system itself? WARNING find where this is still used and find out its purpose (dangerous when some layers are missing?)

  }
  if (m_usePXDHits == false and m_useSVDHits == false) {
    B2ERROR(m_PARAMnameOfInstance << " with setting '" << chosenSetup << "': chosen detectorType via param 'detectorType' (" <<
            detectorType << ") is invalid, resetting value to standard (=VXD)");
    m_useSVDHits = true;
    m_usePXDHits = true;
    m_baselinePass.chosenDetectorType = "VXD";
    m_baselinePass.usePXDHits = true;
    m_baselinePass.useSVDHits = true;
  }


  m_baselinePass.numTotalLayers = m_baselinePass.numTotalLayers - (6 - m_baselinePass.highestAllowedLayer);
  if (m_baselinePass.numTotalLayers < 2) { m_baselinePass.numTotalLayers = 2; }   // to avoid division by zero in some cases
  B2DEBUG(1, m_PARAMnameOfInstance << "chosen detectorType: " << m_baselinePass.chosenDetectorType << " and chosen sectorSetup: " <<
          m_baselinePass.sectorSetup);

  m_baselinePass.zigzagXY.first = m_PARAMactivateZigZagXY;
  m_baselinePass.zigzagXYWithSigma.first = m_PARAMactivateZigZagXYWithSigma;
  m_baselinePass.deltaPt.first =  m_PARAMactivateDeltaPt;
  m_baselinePass.circleFit.first = m_PARAMactivateCircleFit;


  m_baselinePass.zigzagXY.second = m_PARAMtuneZigZagXY;
  m_baselinePass.zigzagXYWithSigma.second = m_PARAMtuneZigZagXYWithSigma;
  m_baselinePass.deltaPt.second = m_PARAMtuneDeltaPt;
  m_baselinePass.circleFit.second = m_PARAMtuneCircleFit;



  /// importing sectorMap including friend Information and friend specific cutoffs
  std::pair<int, int> countedFriendsAndCutoffs = m_baselinePass.importSectorMap(newMap->getSectorMap(), newMap->getDistances(),
                                                 newMap->isFilterByDistance2OriginActivated());

  B2DEBUG(1, m_PARAMnameOfInstance << " setup " << chosenSetup << ": importing secMap with " << newMap->getSectorMap().size() <<
          " sectors -> imported: " << m_baselinePass.sectorMap.size() << "/" << countedFriendsAndCutoffs.first << "/" <<
          countedFriendsAndCutoffs.second << " sectors/friends/(friends w/o existing filters)");


  unsigned int centerSecID = FullSecID().getFullSecID(); // automatically produces secID of centerSector
  VXDSector* pCenterSector = new VXDSector(centerSecID);
  m_baselinePass.sectorMap.insert({centerSecID, pCenterSector});
  B2DEBUG(100, "Baseline-Pass: adding virtual centerSector with " << m_baselinePass.sectorMap.find(
            centerSecID)->second->getFriends().size() << " friends.");

  int countActivatedTCTests = 0;
  if (m_baselinePass.zigzagXY.first == true) { countActivatedTCTests++; }
  if (m_baselinePass.zigzagXYWithSigma.first == true) { countActivatedTCTests++; }
  if (m_baselinePass.deltaPt.first == true) { countActivatedTCTests++; }
  if (m_baselinePass.circleFit.first == true) { countActivatedTCTests++; }
  m_baselinePass.activatedTccFilterTests = countActivatedTCTests; // pT, zzXY, circleFit
}
