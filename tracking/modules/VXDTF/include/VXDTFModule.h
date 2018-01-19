/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

//framework:
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>
#include <vxd/dataobjects/VxdID.h>

// tracking:
#include "tracking/vxdCaTracking/VXDTFHit.h"
#include "tracking/vxdCaTracking/VXDSegmentCell.h"
#include "tracking/vxdCaTracking/VXDSector.h"
#include "tracking/vxdCaTracking/ClusterInfo.h"
#include "tracking/vxdCaTracking/LittleHelper.h"
#include "tracking/dataobjects/FullSecID.h"
#include "tracking/vxdCaTracking/SharedFunctions.h"
#include "tracking/vxdCaTracking/PassData.h"
#include "tracking/dataobjects/VXDTFInfoBoard.h"

// Includes for the Collector/Display
#include <tracking/trackFindingVXD/displayInterfaceTF/CollectorTFInfo.h>
#include <tracking/trackFindingVXD/displayInterfaceTF/ClusterTFInfo.h>

//C++ base / C++ stl:
#include <fstream>
#include <iostream> // std::cout. std::fixed
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <map>
#include <utility>
#include <algorithm>

//root:
#include <TVector3.h>

#include <TRandom.h>
#include <TTree.h>
#include <TFile.h>

//boost:
#include "boost/tuple/tuple.hpp" // for tuple, tie
#ifndef __CINT__
#include <boost/chrono.hpp>
#endif

//genfit:
namespace genfit { class TrackCand; }
// #include <GFTrackCand.h>


namespace Belle2 {
  /** The VXDTFModule is a low momentum Si-only trackfinder.
   *
   * It can use both VXD Detectors (SVD and PXD) to search for tracks.
   * It suports different momentum ranges which can be set via 'sectorSetup' Standard is a three pass setup covering the whole transverse momentum range above 50MeV/c.
   * lower momentum tracks can be found with reduced efficiency.
   * long term features like curling track support is currently not included
   * needed imput: PXD/SCDClusters
   * output: genfit::TrackCand
   */
  class VXDTFModule : public Module {

  public:

    struct SensorStruct; /**< forward declaration  */


    typedef std::pair<unsigned int, VXDSector* > secMapEntry; /**< represents an entry of the MapOfSectors */
    typedef std::map<unsigned int, SensorStruct>
    ActiveSensorsOfEvent; /**< is map where adresses to each activated sensor (key->int = uniID/vxdID) are stored and all clusters which can be found on them */
    typedef std::list<unsigned int>
    BrokenSensorsOfEvent; /**< atm a list containing the keys to all sensors where number of u and v clusters don't fit */

    typedef std::vector<PassData*> PassSetupVector; /**< contains all passes used for track reconstruction */
    typedef boost::chrono::high_resolution_clock
    boostClock; /**< used for measuring time comsumption */ // high_resolution_clock, process_cpu_clock
    typedef boost::chrono::microseconds
    boostNsec; /**< defines time resolution (currently mictroseconds) */ // microseconds, milliseconds

    typedef boost::tuple<double, double, VXDTFHit*>
    HitExtraTuple; /**< get<0>: distance to origin, get<1>: distance to seedHit, get<2> pointer to hit. SeedHit is outermost hit of detector and will be used for cosmic search */

    /// will be moved to an extra file REDESIGN:
    /** SensorStruct needed for SVDCluster sorting, stores u and v clusters of Sensor  */
    struct SensorStruct {
      std::vector<ClusterInfo*> uClusters; /**< contains pointers to all uClusters(using ClusterInfo) on that sensor */
      std::vector<ClusterInfo*> vClusters; /**< contains pointers to all vClusters(using ClusterInfo) on that sensor */
      int layerID; /**< layer ID of the Cluster */
    };


    /** needed for SVDCluster sorting, represents a 2D-cluster (combining 2 1D-clusters) */
    struct ClusterHit {
      ClusterInfo* uCluster; /**< pointer to uCluster(using ClusterInfo) of current Hit */
      ClusterInfo* vCluster; /**< pointer to vCluster(using ClusterInfo) of current Hit */
    };


    /** for testing purposes, storing time consumption */
    struct TimeInfo {
      /** constructor */
      TimeInfo() {}
      boostNsec baselineTF; /**< time consumption of the baselineTF */
      boostNsec hitSorting; /**< time consumption of the hit sorting process */
      boostNsec segFinder; /**< time consumption of the segFinder */
      boostNsec nbFinder; /**< time consumption of the nbFinder */
      boostNsec cellularAutomaton; /**< time consumption of the CA */
      boostNsec tcc; /**< time consumption of the track candidate collector */
      boostNsec postCAFilter; /**< time consumption of the postCAFilter */
      boostNsec checkOverlap; /**< time consumption to find overlapping TCs */
      boostNsec intermediateStuff; /**< time consumption of intermediateStuff like importing data, generating GFTCs and cleaning event-based lists */
      boostNsec kalmanStuff; /**< time consumption of the stuff needed for the kalman filter */
      boostNsec cleanOverlap; /**< time consumption for cleaning overlapping TCs */
      boostNsec neuronalStuff; /**< time consumption of the neuronal network */
    };


    /** stores some information for testing purposes */
    class EventInfoPackage {
    public:
      bool operator<(const EventInfoPackage& b)  const { return totalTime < b.totalTime; } /**< overloaded '<'-operator for sorting algorithms */
      bool operator==(const EventInfoPackage& b) const { return totalTime == b.totalTime; } /**< overloaded '=='-operator for sorting algorithms */

      /** StandardConstructor for the EventInfoPackage - sets all values to zero */
      EventInfoPackage():
        evtNumber(0),
        numPXDCluster(0),
        numSVDCluster(0),
        numSVDHits(0),
        segFinderActivated(0),
        segFinderDiscarded(0),
        numHitCombisTotal(0),
        nbFinderActivated(0),
        nbFinderDiscarded(0),
        tccApprovedTCs(0),
        numTCsAfterTCC(0),
        numTCsAfterTCCfilter(0),
        numTCsKilledByCleanOverlap(0),
        numTCsfinal(0) {}

      /** printing collected data, returned string can be printed using B2INFO or similar */
      std::string Print();

      /** clearing entries, nice after initialisation (TODO: convert into constructor for autoClear) */
      void clear()
      {
        evtNumber = 0;
        numPXDCluster = 0;
        numSVDCluster = 0;
        numSVDHits = 0;
        segFinderActivated = 0;
        segFinderDiscarded = 0;
        numHitCombisTotal = 0;
        nbFinderActivated = 0;
        nbFinderDiscarded = 0;
        tccApprovedTCs = 0;
        numTCsAfterTCC = 0;
        numTCsAfterTCCfilter = 0;
        numTCsKilledByCleanOverlap = 0;
        numTCsfinal = 0;
      }

      boostNsec totalTime;  /**< time consumption of the whole event */
      TimeInfo sectionConsumption; /**< one-event-time-consumption */
      int evtNumber; /**< number of current event */
      int numPXDCluster; /**< number of pxdClusters (=number of pxd hits when tf in pxd is activated) */
      int numSVDCluster; /**< number of svdClusters */
      int numSVDHits; /**< number of possible svd-cluster-combinations. every combination of any pass will be counted  */
      int segFinderActivated; /**< number of segments which survived the segfinder. every segment of any pass will be counted  */
      int segFinderDiscarded; /**< number of segments which died in the segfinder. every segment of any pass will be counted  */
      int numHitCombisTotal; /**< total number of hit-combinations (segFinderActivated+segFinderDiscarded) */
      int nbFinderActivated; /**< number of segments which survived the nbfinder. every segment of any pass will be counted  */
      int nbFinderDiscarded;  /**< number of segments which died in the nbfinder. every segment of any pass will be counted  */
      int tccApprovedTCs;  /**< number of tcs approved by the tcc. every tc of any pass will be counted  */
      int numTCsAfterTCC; /**< number of tcs alive after the tcc. every tc of any pass will be counted  */
      int numTCsAfterTCCfilter; /**< number of tcs alive after the tc-filter. every tc of any pass will be counted  */
      int numTCsKilledByCleanOverlap; /**< number of tcs killed by the cleanOverlappingSet-method */
      int numTCsfinal; /**< number of tcs alive in the end */

    };



    //! Constructor
    VXDTFModule();


    //! Destructor
    ~VXDTFModule();


    void initialize() override;


    void beginRun() override;


    void the_real_event();


    void event() override;


    void endRun() override;


    void terminate() override;


    /** *************************************+************************************* **/
    /** ***********************************+ + +*********************************** **/
    /** *******************************+ functions +******************************* **/
    /** ***********************************+ + +*********************************** **/
    /** *************************************+************************************* **/


    /** works with VXDSegmentCells: checks state of inner neighbours and removes incompatible and virtual ones   */
    void delFalseFriends(PassData* currentPass,
                         TVector3 primaryVertex); // -> TODO: PassData


    /** Recursive CoreFunction of the Track Candidate Collector, stores every possible way starting at a Seed (VXDSegmentCell) */
    void findTCs(TCsOfEvent& tcList,
                 VXDTFTrackCandidate* currentTC,
                 short int maxLayer);


    /** Neuronal network filtering overlapping Track Candidates by searching best subset of TC's */
    void hopfield(TCsOfEvent& tcList,
                  double omega);
    void hopfieldVectorized(TCsOfEvent& tcVector, double omega); /**< copy of hopfield() but using eigen instead of root */

    /** search for nonOverlapping trackCandidates using Greedy algorithm (start with TC of highest QI, remove all TCs incompatible with current TC, if there are still TCs there, repeat step until no incompatible TCs are there any more) */
    void greedy(TCsOfEvent& tcVector);


    /** used by VXDTFModule::greedy, recursive function which takes tc with highest QI and kills all its rivals. After that, TC gets removed and process is repeated with shrinking list of TCs until no TCs alive has got rivals alive */
    void greedyRecursive(std::list< std::pair<double, VXDTFTrackCandidate*> >& overlappingTCs,
                         double& totalSurvivingQI,
                         int& countSurvivors,
                         int& countKills);


    /** for the easy situation of 2 overlapping TCs we dont need comlex algorithms for finding the best subset of clean TCs... */
    void tcDuel(TCsOfEvent& tcVector);


    /** searches for sectors fitting current hit coordinates, returns fullSecID with value uIntMax and pointer to end of SecMap if nothing could be found */
    Belle2::SectorNameAndPointerPair searchSector4Hit(VxdID aVxdID,
                                                      TVector3 localHit,
//                                                       TVector3 sensorSize,
                                                      Belle2::MapOfSectors& m_sectorMap,
                                                      std::vector<double>& uConfig,
                                                      std::vector<double>& vConfig); // -> TODO: generally usefull for VXD-related modules
    /// VXDTFModule::SectorNameAndPointerPair searchSector4Hit(VxdID aVxdID, FIXME alternative version needed?


    /** searches for segments in given pass and returns number of discarded segments */
    int segFinder(PassData* currentPass);


    /** checks segments in given pass with some extra tests for the segFinder (using nbFinder-Tests) for high occupancy cases, returns whether the segment passed the tests or not */
    bool SegFinderHighOccupancy(PassData* currentPass, NbFinderFilters& threeHitFilterBox);


    /** filters neighbouring segments in given pass and returns number of discarded segments */
    int neighbourFinder(PassData* currentPass);


    /** checks segments in given pass with some extra tests for the neighbourFinder (using tcc-filter-Tests) for high occupancy cases, returns whether the segment passed the tests or not */
    bool NbFinderHighOccupancy(PassData* currentPass, TcFourHitFilters& fourHitFilterBox);


    /** the cellular automaton used in the TF.
     * it evolves a combination of two hits called Cells depending of their neighbourhood (=inner Cells attached to current one). In the end, Cells with a useful chain of inner segments carry the highest states.
     * for more information about cellular automata, google "cellular automaton" or "game of life". if returned value is negative, CA was aborted because of infinity loop */
    int cellularAutomaton(PassData* currentPass);


    /** uses attribute "state" to find rows of compatible neighbours. State indicates length of row. This allows fast Track candidate collection */
    void tcCollector(PassData* currentPass); // -> TODO: generally usefull for VXD-related modules


    /** filters TCs via deltaPt, ZigZag and deltaDistance2IP */
    int tcFilter(PassData* currentPass,
                 int passNumber);



    /** name is program, needed for GFTrackCand export */
    void calcInitialValues4TCs(PassData* currentPass);


    /** simplest way to determine QI of track candidates, calculating them by track length */
    void calcQIbyLength(TCsOfEvent& tcVector,
                        PassSetupVector& passSetups);


    /** easy way to determine QI of track candidates, currently only suitable for testbeam cases since the calculation is sensitive for high slopes */
    void calcQIbyStraightLine(TCsOfEvent& tcVector);


    /** produce GFTrackCand for current TC */
    genfit::TrackCand generateGFTrackCand(VXDTFTrackCandidate* currentTC);


    /** calculate real kalman-QI's for each currently living TC */
    void calcQIbyKalman(TCsOfEvent& tcVector);


    /** because of geometrical reasons and the multipass-support, it is a rather common situation that the same track will be recovered twice or more.
     * In such a case, the whole number or at least a subset of hits are shared by several tracks.
     * This leads to two different problems for the hopfield network.
     * First: it increases the number of overlapping TCs which drastically reduces the speed of the TF.
     * Second: in some cases, the only TCs which are overlapping are such a track which has been found multiple times.
     * In such a case, the hopfield network encounters the problem that all the TCs (it can see only the overlapping ones) are incompatible with each other.
     * This leads to a constant surpression of all overlapping TCs.
     * The result is that all the neuron values stay below the threshold until the neuronal network stops and all overlapping TCs get deactivated.
     * Since this is a problem the hopfield network was not developed for, we have to filter these cases. Another issue is the situation, where the majority of hits are shared, but there are some foreign hits included too.
     * They have to be treated differently and therefore can not be filtered the way currently implemented.
     * returns number of deleted TCs.
     */
    int cleanOverlappingSet(TCsOfEvent& tcVector); // -> TODO: generally usefull for VXD-related modules


    /** reset all reused containers and delete others which are existing only for one event. */
    void cleanEvent(PassData* currentPass)
    {
      /** REDESIGNCOMMENT FINDSENSORS4CLUSTER 1:
       * * short:
       *
       ** long (+personal comments):
       *
       ** dependency of module parameters (global):
       *
       ** dependency of global in-module variables:
       * m_allTCsOfEvent, m_tcVectorOverlapped, m_tcVector,
       *
       ** dependency of global stuff just because of B2XX-output or debugging only:
       *
       ** in-module-function-calls:
       */
      currentPass->cleanPass();

      for (VXDTFTrackCandidate* aTC : m_allTCsOfEvent) { delete  aTC; }
      m_allTCsOfEvent.clear();

      m_tcVectorOverlapped.clear();
      m_tcVector.clear();
    }


    /** general Function to write data into a root file*/
    void writeToRootFile(double pValue, double chi2, double circleRadius, int ndf)
    {
      /** REDESIGNCOMMENT FINDSENSORS4CLUSTER 1:
       * * short:
       *
       ** long (+personal comments):+
       * whole function is only for DQM/debugging/testing purposes...
       *
       ** dependency of module parameters (global):
       * m_PARAMwriteToRoot,
       *
       ** dependency of global in-module variables:
       * m_rootPvalues, m_rootChi2, m_rootCircleRadius,
       * m_rootNdf, m_treeTrackWisePtr
       *
       ** dependency of global stuff just because of B2XX-output or debugging only:
       * m_rootPvalues, m_rootChi2, m_rootCircleRadius,
       * m_rootNdf, m_treeTrackWisePtr, m_PARAMwriteToRoot
       *
       ** in-module-function-calls:
       */
      if (m_PARAMwriteToRoot == true) {
        m_rootPvalues = pValue;
        m_rootChi2 = chi2;
        m_rootCircleRadius = circleRadius;
        m_rootNdf = ndf;
        m_treeTrackWisePtr->Fill();
      }
    }


    /** fast bypass for very simple events having not more than 2 easily distinguishable tracks and cosmic events */
    bool baselineTF(std::vector<ClusterInfo>& clusters, PassData* passInfo);


    /** store each cluster (as a clusterPtr) in a map(uniID, sensorStruct), where sensorStruct contains 2 vectors (uClusters and vClusters). In the end a map containing illuminated sensors - and each cluster inhabiting them - exists */
    void findSensors4Clusters(ActiveSensorsOfEvent& activatedSensors, std::vector<ClusterInfo>& clusters);


    /** store a cluster (as a clusterPtr) in a map(uniID, sensorStruct), where sensorStruct contains 2 vectors (uClusters and vClusters), subroutine for findSensors4Clusters and simpleEventReco */
    void findSensor4Cluster(ActiveSensorsOfEvent& activatedSensors, ClusterInfo& aClusterInfo);


    /** iterate through map of activated sensors & combine each possible combination of clusters. Store them in a vector of structs, where each struct carries an u & a v cluster. return value is a vector of sensorIDs which contains strange sensors (sensors with missing clusters) */
    BrokenSensorsOfEvent find2DSVDHits(ActiveSensorsOfEvent& activatedSensors, std::vector<ClusterHit>& clusterHitList);


    /** checks for each strange sensor whether it makes sense to generate 1D-VXDTFHits or not. we therefore filter by threshold to keep us from stumbling over messy sensors. It returns a container full with 1D-Hits  */
    std::vector<VXDTFHit> dealWithStrangeSensors(ActiveSensorsOfEvent& activatedSensors, BrokenSensorsOfEvent& strangeSensors);


    /** produces VXDTFHits when getting at least one SVDCluster*, currently not compatible with normal svd-hit-cases (only for baseline-tf) */
    VXDTFHit deliverVXDTFHitWrappedSVDHit(ClusterInfo* uClusterInfo, ClusterInfo* vClusterInfo);


    /** executes the calculations needed for the circleFit */
    bool doTheCircleFit(PassData* thisPass, VXDTFTrackCandidate* aTc, int nHits, int tcCtr, int addDegreesOfFreedom = 1);


    /** reserves hits for the best TCs so far */
    void reserveHits(TCsOfEvent& tcVector, PassData* currentPass);


    void resetCountersAtBeginRun()
    {
      /** REDESIGNCOMMENT FINDSENSORS4CLUSTER 1:
       * * short:
       *
       ** long (+personal comments):
       * all the entries below are the dependencies...
       *
       ** dependency of module parameters (global):
       *
       ** dependency of global in-module variables:
       * m_eventCounter, m_badSectorRangeCounter, m_TESTERbadSectorRangeCounterForClusters,
       * m_TESTERclustersPersSectorNotMatching, m_badFriendCounter, m_totalPXDClusters,
       * m_totalTELClusters, m_totalSVDClusters, m_totalSVDClusterCombis,
       * m_TESTERhighOccupancyCtr, m_TESTERtriggeredZigZagXY, m_TESTERtriggeredZigZagXYWithSigma,
       * m_TESTERtriggeredZigZagRZ, m_TESTERtriggeredDpT, m_TESTERtriggeredCircleFit,
       * m_TESTERapprovedByTCC, m_TESTERcountTotalTCsAfterTCC, m_TESTERcountTotalTCsAfterTCCFilter,
       * m_TESTERcountTotalTCsFinal, m_TESTERcountTotalUsedIndicesFinal, m_TESTERcountTotalUsedHitsFinal,
       * m_TESTERbadHopfieldCtr, m_TESTERHopfieldLetsOverbookedTCsAliveCtr, m_TESTERfilteredOverlapsQI,
       * m_TESTERNotFilteredOverlapsQI, m_TESTERfilteredOverlapsQICtr, m_TESTERcleanOverlappingSetStartedCtr,
       * m_TESTERgoodFitsCtr, m_TESTERbadFitsCtr, m_TESTERbrokenEventsCtr,
       * m_TESTERfilteredBadSeedTCs, m_TESTERdistortedHitCtr, m_TESTERtotalsegmentsSFCtr,
       * m_TESTERtotalsegmentsNFCtr, m_TESTERdiscardedSegmentsSFCtr, m_TESTERdiscardedSegmentsNFCtr,
       * m_TESTERbrokenCaRound, m_TESTERkalmanSkipped, m_TESTERovercrowdedStrangeSensors,
       * m_TESTERstartedBaselineTF, m_TESTERsucceededBaselineTF, m_TESTERnoHitsAtEvent,
       * m_TESTERacceptedBrokenHitsTrack, m_TESTERrejectedBrokenHitsTrack, m_tcVectorOverlapped,
       * m_tcVector, m_allTCsOfEvent
       *
       ** dependency of global stuff just because of B2XX-output or debugging only:
       * m_eventCounter, m_badSectorRangeCounter, m_TESTERbadSectorRangeCounterForClusters,
       * m_TESTERclustersPersSectorNotMatching, m_badFriendCounter, m_totalPXDClusters,
       * m_totalTELClusters, m_totalSVDClusters, m_totalSVDClusterCombis,
       * m_TESTERhighOccupancyCtr, m_TESTERtriggeredZigZagXY, m_TESTERtriggeredZigZagXYWithSigma,
       * m_TESTERtriggeredZigZagRZ, m_TESTERtriggeredDpT, m_TESTERtriggeredCircleFit,
       * m_TESTERapprovedByTCC, m_TESTERcountTotalTCsAfterTCC, m_TESTERcountTotalTCsAfterTCCFilter,
       * m_TESTERcountTotalTCsFinal, m_TESTERcountTotalUsedIndicesFinal, m_TESTERcountTotalUsedHitsFinal,
       * m_TESTERbadHopfieldCtr, m_TESTERHopfieldLetsOverbookedTCsAliveCtr, m_TESTERfilteredOverlapsQI,
       * m_TESTERNotFilteredOverlapsQI, m_TESTERfilteredOverlapsQICtr, m_TESTERcleanOverlappingSetStartedCtr,
       * m_TESTERgoodFitsCtr, m_TESTERbadFitsCtr, m_TESTERbrokenEventsCtr,
       * m_TESTERfilteredBadSeedTCs, m_TESTERdistortedHitCtr, m_TESTERtotalsegmentsSFCtr,
       * m_TESTERtotalsegmentsNFCtr, m_TESTERdiscardedSegmentsSFCtr, m_TESTERdiscardedSegmentsNFCtr,
       * m_TESTERbrokenCaRound, m_TESTERkalmanSkipped, m_TESTERovercrowdedStrangeSensors,
       * m_TESTERstartedBaselineTF, m_TESTERsucceededBaselineTF, m_TESTERnoHitsAtEvent,
       * m_TESTERacceptedBrokenHitsTrack, m_TESTERrejectedBrokenHitsTrack
       *
       ** in-module-function-calls:
       */
      m_eventCounter = 0;
      m_badSectorRangeCounter = 0;
      m_TESTERbadSectorRangeCounterForClusters = 0;
      m_TESTERclustersPersSectorNotMatching = 0;
      m_badFriendCounter = 0;
      m_totalPXDClusters = 0;
      m_totalSVDClusters = 0;
      m_totalSVDClusterCombis = 0;
      m_TESTERhighOccupancyCtr = 0;
      m_TESTERtriggeredZigZagXY = 0;
      m_TESTERtriggeredZigZagXYWithSigma = 0;
      m_TESTERtriggeredZigZagRZ = 0;
      m_TESTERtriggeredDpT = 0;
      m_TESTERtriggeredDD2IP = 0;
      m_TESTERtriggeredCircleFit = 0;
      m_TESTERapprovedByTCC = 0;
      m_TESTERcountTotalTCsAfterTCC = 0;
      m_TESTERcountTotalTCsAfterTCCFilter = 0;
      m_TESTERcountTotalTCsFinal = 0;
      m_TESTERcountTotalUsedIndicesFinal = 0;
      m_TESTERcountTotalUsedHitsFinal = 0;
      m_TESTERbadHopfieldCtr = 0;
      m_TESTERHopfieldLetsOverbookedTCsAliveCtr = 0;
      m_TESTERfilteredOverlapsQI = 0;
      m_TESTERNotFilteredOverlapsQI = 0;
      m_TESTERfilteredOverlapsQICtr = 0;
      m_TESTERcleanOverlappingSetStartedCtr = 0;
      m_TESTERgoodFitsCtr = 0;
      m_TESTERbadFitsCtr = 0;
      m_TESTERbrokenEventsCtr = 0;
      m_TESTERfilteredBadSeedTCs = 0;
      m_TESTERdistortedHitCtr = 0;
      m_TESTERtotalsegmentsSFCtr = 0;
      m_TESTERtotalsegmentsNFCtr = 0;
      m_TESTERdiscardedSegmentsSFCtr = 0;
      m_TESTERdiscardedSegmentsNFCtr = 0;
      m_TESTERbrokenCaRound = 0;
      m_TESTERkalmanSkipped = 0;
      m_TESTERovercrowdedStrangeSensors = 0;
      m_TESTERstartedBaselineTF = 0;
      m_TESTERsucceededBaselineTF = 0;
      m_TESTERnoHitsAtEvent = 0;
      m_TESTERacceptedBrokenHitsTrack = 0;
      m_TESTERrejectedBrokenHitsTrack = 0;
      m_tcVectorOverlapped.clear();
      m_tcVector.clear();
      m_allTCsOfEvent.clear();
    }


    /** initialize variables in constructor to avoid nondeterministic behavior */
    void InitializeInConstructor();


    // Methods for Collector
    int importCollectorCell(int pass_index, std::string died_at, int died_id, std::vector<std::pair<int, bool>> acceptedRejectedFilters,
                            int hit1, int hit2);  /**< generates Information and imports a Cell for the Collector */




    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// REDESIGN - new functions encapsulating smaller tasks
    /////////////////////////////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////
    /// REDESIGN - Functions for initialize:
    ////////////////////////////////////////

    /** check given parameters by the user for validity and prepare some initial settings depending on the settings */
    void checkAndSetupModuleParameters();

    /** register storeArrays, set up geometryManager, activate genfit-tools used */
    void prepareExternalTools();


    //////////////////////////////////////
    /// REDESIGN - Functions for beginRun:
    //////////////////////////////////////

    /** imports sectorMaps chosen from the storage and sets up all passes for that run */
    void setupPasses();

    /** sets up all the relevant settings for the baseLineTF.
     *
     * The BaseLineTF is a very simplistic pseudo-TF only relevant for testbeams
     * copying first pass for the BaselineTF (just to be sure that they can be used independently from each other)
     * */
    void setupBaseLineTF();

    /** Import all Sectors for all events (Collector).
     *
     * They will be copied to the displayCollector to have a 1:1 metaInfo for each sector of each pass */
    void importSectorMapsToDisplayCollector();



  protected:
    TCsOfEvent
    m_tcVector; /**< carries links to all track candidates found within event (during tcc filter, bad ones get kicked, lateron they simply get deactivated) */
    TCsOfEvent
    m_allTCsOfEvent; /**< carries links to really all track candidates found within event (used for deleting TrackCandidates at end of event) TODO: check whether use of m_tcVector can not be merged this one. Seems like redundant steps*/
    TCsOfEvent m_tcVectorOverlapped; /**< links only to track candidates which share at least one cluster with others*/

    /// module_parameters:
    bool m_PARAMDebugMode; /**< some code will only be executed if this mode is enabled */
    std::vector<std::string>
    m_PARAMsectorSetup; /**< lets you chose the sectorSetup (compatibility of sensors, individual cutoffs,...) accepts 'std', 'low', 'high' and 'personal', please note that the chosen setup has to exist as a xml-file in ../tracking/data/friendList_XXX.xml. If you can not create your own xml files using e.g. the filterCalculatorModule, use params for  'tuneCutoffXXX' or 'setupWeigh' instead. multipass supported by setting setups in a row */

    std::vector<int> m_PARAMhighestAllowedLayer; /**< set value below 6 if you want to exclude outer layers (standard is 6) */
    std::vector<int> m_PARAMminLayer; /**< determines lowest layer considered by track candidate collector */
    std::vector<int> m_PARAMminState; /**< determines lowest state of cells considered by track candidate collector */

    std::vector<bool> m_PARAMactivateDistance3D; /**< activates/deactivates current filter d3D for each pass individually */
    std::vector<bool> m_PARAMactivateDistanceXY; /**< activates/deactivates current filter dXY for each pass individually */
    std::vector<bool> m_PARAMactivateDistanceZ; /**< activates/deactivates current filter dZ for each pass individually */
    std::vector<bool> m_PARAMactivateSlopeRZ; /**< activates/deactivates current filter slopeRZ for each pass individually */
    std::vector<bool> m_PARAMactivateNormedDistance3D; /**< activates/deactivates current nd3D filter for each pass individually */
    std::vector<bool> m_PARAMactivateAngles3D; /**< activates/deactivates current filter a3D for each pass individually */
    std::vector<bool> m_PARAMactivateAnglesXY; /**< activates/deactivates current filter aXY for each pass individually */
    std::vector<bool> m_PARAMactivateAnglesRZ; /**< activates/deactivates current filter aRZ for each pass individually */
    std::vector<bool> m_PARAMactivateDeltaSlopeRZ; /**< activates/deactivates current filter dslopeRZ for each pass individually */
    std::vector<bool> m_PARAMactivateDistance2IP; /**< activates/deactivates current filter d2IP for each pass individually */
    std::vector<bool> m_PARAMactivatePT; /**< activates/deactivates current filter PT for each pass individually */
    std::vector<bool>
    m_PARAMactivateHelixParameterFit; /**< activates/deactivates current filter HelixParameterFit for each pass individually */
    std::vector<bool> m_PARAMactivateDeltaSOverZ; /**< activates/deactivates current filter DeltaSOverZ for each pass individually */
    std::vector<bool>
    m_PARAMactivateDeltaSlopeZOverS; /**< activates/deactivates current filter DeltaSlopeZOverS for each pass individually */
    std::vector<bool> m_PARAMactivateZigZagXY; /**< activates/deactivates current filter zzXY for each pass individually */
    std::vector<bool>
    m_PARAMactivateZigZagXYWithSigma; /**< activates/deactivates current filter zzXY with sigmas for each pass individually */
    std::vector<bool> m_PARAMactivateZigZagRZ; /**< activates/deactivates current filter zzRZ for each pass individually */
    std::vector<bool> m_PARAMactivateDeltaPt; /**< activates/deactivates current filter dPt for each pass individually */
    std::vector<bool> m_PARAMactivateDeltaDistance2IP; /**< activates/deactivates current filter dd2IP for each pass individually*/
    std::vector<bool> m_PARAMactivateCircleFit; /**< activates/deactivates current filter CircleFit for each pass individually */
    std::vector<bool>
    m_PARAMactivateAngles3DHioC; /**< activates/deactivates current filter a3D (high occupancy mode) for each pass individually */
    std::vector<bool>
    m_PARAMactivateAnglesXYHioC; /**< activates/deactivates current filter aXY (high occupancy mode) for each pass individually */
    std::vector<bool>
    m_PARAMactivateAnglesRZHioC; /**< activates/deactivates current filter aRZ (high occupancy mode) for each pass individually */
    std::vector<bool>
    m_PARAMactivateDeltaSlopeRZHioC; /**< activates/deactivates current filter dslopeRZ (high occupancy mode) for each pass individually */
    std::vector<bool>
    m_PARAMactivateDistance2IPHioC; /**< activates/deactivates current filter d2IP (high occupancy mode) for each pass individually */
    std::vector<bool>
    m_PARAMactivatePTHioC; /**< activates/deactivates current filter PT (high occupancy mode) for each pass individually */
    std::vector<bool>
    m_PARAMactivateHelixParameterFitHioC; /**< activates/deactivates current filter HelixParameterFit (high occupancy mode) for each pass individually */
    std::vector<bool>
    m_PARAMactivateDeltaPtHioC; /**< activates/deactivates current filter dPt (high occupancy mode) for each pass individually */
    std::vector<bool>
    m_PARAMactivateDeltaDistance2IPHioC; /**< activates/deactivates current filter dd2IP (high occupancy mode) for each pass individually*/

    // debugging tests:
    std::vector<bool>
    m_PARAMactivateAlwaysTrue2Hit; /**< activates/deactivates current filter AlwaysTrue2Hit for each pass individually*/
    std::vector<bool>
    m_PARAMactivateAlwaysFalse2Hit; /**< activates/deactivates current filter AlwaysFalse2Hit for each pass individually*/
    std::vector<bool> m_PARAMactivateRandom2Hit; /**< activates/deactivates current filter Random2Hit for each pass individually*/
    std::vector<bool>
    m_PARAMactivateAlwaysTrue3Hit; /**< activates/deactivates current filter AlwaysTrue3Hit for each pass individually*/
    std::vector<bool>
    m_PARAMactivateAlwaysFalse3Hit; /**< activates/deactivates current filter AlwaysFalse3Hit for each pass individually*/
    std::vector<bool> m_PARAMactivateRandom3Hit; /**< activates/deactivates current filter Random3Hit for each pass individually*/
    std::vector<bool>
    m_PARAMactivateAlwaysTrue4Hit; /**< activates/deactivates current filter AlwaysTrue4Hit for each pass individually*/
    std::vector<bool>
    m_PARAMactivateAlwaysFalse4Hit; /**< activates/deactivates current filter AlwaysFalse4Hit for each pass individually*/
    std::vector<bool> m_PARAMactivateRandom4Hit; /**< activates/deactivates current filter Random4Hit for each pass individually*/

    // tuning parameters:
    std::vector<double>
    m_PARAMtuneDistance3D; /**< tunes current filter d3D for each pass individually, formula: min: cut = cut-tuneValue*cut, max: cut = cut + tuneValue*cut -> tuneValue of 0, no changes, 1, stretch by 100% of own size... */
    std::vector<double>
    m_PARAMtuneDistanceXY; /**< tunes current filter dXY for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneDistanceZ; /**< tunes current filter dZ for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneNormedDistance3D; /**< tunes current nd3D filter for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneSlopeRZ; /**< tunes current filter slopeRZ for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneAngles3D; /**< tunes current filter a3D for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneAnglesXY; /**< tunes current filter aXY for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneAnglesRZ; /**< tunes current filter aRZ for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneDeltaSlopeRZ; /**< tunes current filter dslopeRZ for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneDistance2IP; /**< tunes current filter d2IP for each pass individually, same formula as in tuneDistance3D */
    std::vector<double> m_PARAMtunePT; /**< tunes current filter PT for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneHelixParameterFit; /**< tunes current filter HelixParameterFit for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneDeltaSOverZ; /**< tunes current filter DeltaSOverZ for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneDeltaSlopeZOverS; /**< tunes current filter DeltaSlopeZOverS for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneZigZagXY; /**< tunes current filter zzXY for each pass individually, same formula as in tuneDistance3D */
    std::vector<double> m_PARAMtuneZigZagXYWithSigma; /**< tunes current filter zzXY with sigmas for each pass individually */
    std::vector<double>
    m_PARAMtuneZigZagRZ; /**< tunes current filter zzRZ for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneDeltaPt; /**< tunes current filter dPt for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneDeltaDistance2IP; /**< tunes current filter dd2IP for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneCircleFit; /**< tunes current filter CircleFit for each pass individually, here it is the global threshold value for the chi2-values produced by the fit which is stored */
    std::vector<double>
    m_PARAMtuneAngles3DHioC; /**< tunes current filter a3D (high occupancy mode) for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneAnglesXYHioC; /**< tunes current filter aXY (high occupancy mode) for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneAnglesRZHioC; /**< tunes current filter aRZ (high occupancy mode) for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneDeltaSlopeRZHioC; /**< tunes current filter dslopeRZ (high occupancy mode) for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneDistanceDeltaZHioC; /**< tunes current filter ddZ (high occupancy mode) for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneDistance2IPHioC; /**< tunes current filter d2IP (high occupancy mode) for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtunePTHioC; /**< tunes current filter PT (high occupancy mode) for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneHelixParameterFitHioC; /**< tunes current filter HelixParameterFit (high occupancy mode) for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneDeltaPtHioC; /**< tunes current filter dPt (high occupancy mode) for each pass individually, same formula as in tuneDistance3D */
    std::vector<double>
    m_PARAMtuneDeltaDistance2IPHioC; /**< tunes current filter dd2IP (high occupancy mode) for each pass individually, same formula as in tuneDistance3D */

    std::vector<double> m_PARAMtuneAlwaysTrue2Hit; /**< tunes current filter AlwaysTrue2Hit for each pass individually*/
    std::vector<double> m_PARAMtuneAlwaysFalse2Hit; /**< tunes current filter AlwaysFalse2Hit for each pass individually*/
    std::vector<double> m_PARAMtuneRandom2Hit; /**< tunes current filter Random2Hit for each pass individually*/
    std::vector<double> m_PARAMtuneAlwaysTrue3Hit; /**< tunes current filter AlwaysTrue3Hit for each pass individually*/
    std::vector<double> m_PARAMtuneAlwaysFalse3Hit; /**< tunes current filter AlwaysFalse3Hit for each pass individually*/
    std::vector<double> m_PARAMtuneRandom3Hit; /**< tunes current filter Random3Hit for each pass individually*/
    std::vector<double> m_PARAMtuneAlwaysTrue4Hit; /**< tunes current filter AlwaysTrue4Hit for each pass individually*/
    std::vector<double> m_PARAMtuneAlwaysFalse4Hit; /**< tunes current filter AlwaysFalse4Hit for each pass individually*/
    std::vector<double> m_PARAMtuneRandom4Hit; /**< tunes current filter Random4Hit for each pass individually*/


    std::vector<double>
    m_PARAMsetupWeigh; /**< allows to individually weigh (level of surpression) each setup/pass. Please choose between 0 and -100 (%), higher momentum setups are more reliable, therefore should be weighed best */

    /// needed for pass handling:
    PassSetupVector m_passSetupVector; /**< contains information for each pass */
    PassData m_baselinePass; /**< baselineTF gets his own pass, gets some settings from the first pass of the PassSetupVector */


    int m_PARAMpdGCode; /**< tandard value is 211 (pi+), ATTENTION, instead of using inconsistent sign of PdGList, in this module positively charged particles are always positive and negatively charged ones are negative (relevant for leptons) */
    double m_PARAMartificialMomentum; /**< "standard value is 0. if StandardValue is changed to a nonZero value, the magnitude of the momentum seed is set artificially using this value, if value < 0, not only the magnitude is set using the norm of the value, but direction of momentum is reversed too, if you want to change charge, use parameter 'standardPdgCode'"*/

    LittleHelper m_littleHelperBox; /**< bundles small but often used functions for smearing and others.  */

    int m_chargeSignFactor; /**< particle dependent. for leptons it is 1, for other particles it's -1... */

    bool m_usePXDHits; /**< when having more than one pass per event, sector maps using PXD, SVD or TEL can be set independently. To produce TFHits for PXD, this value is set to true */
    bool m_useSVDHits; /**< when having more than one pass per event, sector maps using PXD, SVD or TEL can be set independently. To produce TFHits for SVD, this value is set to true */

    double m_PARAMtuneCutoffs; /**< for rapid changes of cutoffs (no personal xml files needed), reduces/enlarges the range of the cutoffs in percent (lower and upper values are changed by this value). Only valid in range -50% < x < +1000% */

    int m_eventCounter; /**< knows current event number */
    int m_badSectorRangeCounter; /**< counts number of hits which couldn't be attached to an existing sector of a pass */
    int m_badFriendCounter; /**< counts number of hits having no neighbour hits in friend sectors of current sector */
    int m_totalPXDClusters; /**< counts total number of PXDClusters during run */
    int m_totalSVDClusters; /**< counts total number of SVdClusters during run */
    int m_totalSVDClusterCombis; /**< counts total number of possible combinations of SVDClusters during run */
    int m_nSectorSetups; /**< stores info about number of sector setups loaded into the track finder */
    bool m_KFBackwardFilter; /**< determines whether the kalman filter moves inwards or backwards, bool means inwards */
    bool m_highOccupancyCase; /**< is determined by a userdefined threshold. If there are more hits in the event than threshold value, high occupancy filters are activated (segFinder and nbFinder only) */
    int m_PARAMhighOccupancyThreshold; /**< If there are more hits in a sensor than threshold value, high occupancy filters are activated (segFinder and nbFinder only) */
    int m_PARAMkillBecauseOfOverlappsThreshold; /**< if there are more TCs overlapping than threshold value, event kalman gets replaced by circleFit. If there are 10 times more than threshold value of TCs, the complete event gets aborted */
    int m_PARAMkillEventForHighOccupancyThreshold; /**< if there are more segments than threshold value, the complete event gets aborted */

    double m_PARAMomega; /**< tuning parameter for hopfield network */
    double m_tcThreshold;   /**< defines threshold for hopfield network. neurons having values below threshold are discarded */
    std::vector<double>
    m_PARAMreserveHitsThreshold; /**< tuning parameter for passes, valid values 0-1 ( = 0-100%). It defines how many percent of the TCs (sorted by QI) are allowed to reserve their hits (which disallows further passes to use these hits). This does not mean that TCs which were not allowed to reserve their hits will be deleted, this only means that they have to compete with TCs of other passes for their hits again. Setting the values to 100% = 1 means, no hits used by tcs surviving that pass are reused, 0% = 0 means every tc has to compete with all tcs of other passes (quite similar to former behavior) */

    bool m_PARAMqiSmear; /**<  allows to smear QIs via qqq-Interface, needed when having more than one TC with the same QI */
    bool m_PARAMcleanOverlappingSet; /**< when true, TCs which are found more than once (possible because of multipass) will get filtered */

    std::string
    m_PARAMfilterOverlappingTCs; /**< defines which technique shall be used for filtering overlapping TCs, currently supported: 'hopfield', 'greedy', 'none' */
    int m_filterOverlappingTCs; /**< is set by m_PARAMfilterOverlappingTCs and defines which technique shall be used for filtering overlapping TCs */
    double m_PARAMsmearMean; /**< allows to introduce a bias for QI (e.g. surpressing all values, ...)*/
    double m_PARAMsmearSigma; /**< bigger values deliver broader distribution*/
    bool m_PARAMstoreBrokenQI;/**< if true, TC survives QI-calculation-process even if fit was not possible */
    bool m_TESTERexpandedTestingRoutines; /**< set true if you want to export expanded infos of TCs for further analysis */
    bool m_PARAMwriteToRoot; /**< if true, a rootFile named by m_PARAMrootFileName will be filled with info */
    std::vector<std::string>
    m_PARAMrootFileName; /**< only two entries accepted, first one is the root filename, second one is 'RECREATE' or 'UPDATE' which is the write mode for the root file, parameter is used only if 'writeToRoot' = true */

    TFile* m_rootFilePtr; /**< pointer at root file used for p-value-output */
    TTree* m_treeTrackWisePtr; /**< pointer at root tree used for information stored once per track (e.g. p-value-output) */
    TTree* m_treeEventWisePtr; /**< pointer at root tree used for information stored once per event */
    long long int m_rootTimeConsumption; /**< used for storing duration of event in a root file */
    double m_rootPvalues; /**< used for storing pValues in a root file */
    double m_rootChi2; /**< used for storing chi2values in a root file */
    double m_rootCircleRadius; /**< used for storing the circle radii calculated by the circle fitter in a root file */
    int m_rootNdf; /**< used for storing numbers of degrees of freedom in a root file */

    std::string
    m_PARAMcalcQIType; /**< allows you to chose the way, the QI's of the TC's shall be calculated. currently supported: 'kalman','trackLength', 'circleFit' */
    int m_calcQiType; /**< is set by m_PARAMcalcQIType and defines which qi type shall be calculated */
    std::string
    m_PARAMcalcSeedType; /**< allows you to chose the way, the seed-mometa of the TC's shall be calculated. currently supported: 'helixFit', 'straightLine' */
    int m_calcSeedType; /**< is set by m_PARAMcalcSeedType and defines which seed type shall be calculated */
    std::string m_PARAMgfTrackCandsColName;       /**< TrackCandidates collection name */
    std::string m_PARAMinfoBoardName;             /**< InfoContainer collection name */
    std::string m_PARAMpxdClustersName;         /**< name of storeArray containing pxd clusters */
    std::string m_PARAMsvdClustersName;         /**< name of storeArray containing svd clusters */
    std::string
    m_PARAMnameOfInstance;           /**< Name of trackFinder, usefull, if there is more than one VXDTF running at the same time. Note: please choose short names */
    int m_PARAMactivateBaselineTF; /**< there is a baseline trackfinder which catches events with a very small number of hits, e.g. bhabha, cosmic and single-track-events. Settings: 0 = deactivate baseLineTF, 1=activate it and use normal TF as fallback, 2= baseline-TF-only */

    /// the following variables are nimimal testing routines within the TF
    int m_TESTERnoHitsAtEvent;  /**< counts number of times, where there were no hits at the event */
    int m_TESTERstartedBaselineTF; /**< counts number of times, the baselineTF was started */
    int m_TESTERacceptedBrokenHitsTrack; /**< counts number of times, where a tc having at least 1 1D-SVD hit was accepted */
    int m_TESTERrejectedBrokenHitsTrack; /**< counts number of times, where a tc having at least 1 1D-SVD hit was rejected */
    int m_TESTERsucceededBaselineTF; /**< counts number of times, the baselineTF found a track */
    int m_TESTERtriggeredZigZagXY;/**< counts how many times zigZagXY filter found bad TCs */
    int m_TESTERtriggeredZigZagXYWithSigma;/**< counts how many times zigZagXYWithSigma filter found bad TCs */
    int m_TESTERtriggeredZigZagRZ;/**< counts how many times zigZagRZ filter found bad TCs */
    int m_TESTERtriggeredDpT; /**< counts how many times deltaPt filter found bad TCs  */
    int m_TESTERtriggeredCircleFit; /**< counts how many times circleFit filter found bad TCs  */
    int m_TESTERtriggeredDD2IP;  /**< counts how many times deltaDistance2IP filter found bad TCs  */
    int m_TESTERapprovedByTCC; /**< counts how many TCs got approved by TCC */
    int m_TESTERcountTotalTCsAfterTCC; /**< counts number of TCs found by TCC */
    int m_TESTERcountTotalTCsAfterTCCFilter; /**< counts number of TCs which survived the tcc filter */
    int m_TESTERcountTotalTCsFinal; /**< counts number of TCs which survived until the end of event */
    int m_TESTERcountTotalUsedIndicesFinal; /**< counts number of indices used by TCs which survived until the end of event */
    int m_TESTERcountTotalUsedHitsFinal; /**< counts number of indices used by TCs which survived until the end of event */
    int m_TESTERbadHopfieldCtr; /**< counts number of events, where no TC survived the Hopfield network */
    int m_TESTERHopfieldLetsOverbookedTCsAliveCtr;/**< counts number of times, where TC survived the Hopfield network although being still overbooked */
    int m_TESTERfilteredBadSeedTCs; /**< counts number of TCs which were filtered by calcInitialValues4TCs because of bad seed properties */
    int m_TESTERfilteredOverlapsQI; /**< counts number of TCs filtered by total overlap rule (cleanOverlappingSet) */
    int m_TESTERNotFilteredOverlapsQI; /**< counts number of events, when cleanOverlappingSet was started but didn't filter TCs */
    int m_TESTERfilteredOverlapsQICtr; /**< counts number of times, when TCs get filtered by cleanOverlappingSet */
    int m_TESTERcleanOverlappingSetStartedCtr; /**< counts number of times, when cleanOverlappingSet get started */
    int m_TESTERgoodFitsCtr; /**< counts number of times, the kalman fit worked well */
    int m_TESTERbadFitsCtr; /**< counts number of times, the kalman fit didn't work */
    int m_TESTERbrokenEventsCtr; /**< counts number of events, where the TF had to be stopped */
    int m_TESTERdistortedHitCtr; /**< counts number of times, where hit was not situated on the sensor plane */
    std::vector<std::string> m_TESTERbadSensors; /**< collects SensorIDs of outofrange-hits coding: -layer_ladder_sensor */
    int m_TESTERtotalsegmentsSFCtr; /**< counts total number of segments produced by the SF */
    int m_TESTERtotalsegmentsNFCtr; /**< counts total number of segments surviving the NF */
    int m_TESTERdiscardedSegmentsSFCtr; /**< counts total number of segments discarded by the SF */
    int m_TESTERdiscardedSegmentsNFCtr; /**< counts total number of segments discarded by the SF */
    int m_TESTERbrokenCaRound; /**< counts number of times where CA produced an infinity loop */
    std::vector<int> m_TESTERSVDOccupancy; /**< counts number of hits per sensor (inclusive ghost hits) */
    int m_TESTERkalmanSkipped; /**< counts number of times when there were too many overlapped TCs for kalman filtering */
    TimeInfo m_TESTERtimeConsumption; /**< a struct to store amount of time needed for special blocks of the program */
    std::vector<EventInfoPackage>
    m_TESTERlogEvents; /**< a list containing some information about each event (time consumption and quality) */
    int m_TESTERbadSectorRangeCounterForClusters; /**< counts number of times when only 1 cluster has been found on a complete sensor (therefore no 2D hits possible) */
    int m_TESTERclustersPersSectorNotMatching; /**< counts number of times when numofUclusters and nVclusters per sensor do not match */
    int m_TESTERhighOccupancyCtr; /**< counts number of times when high occupancy mode was activated */
    int m_TESTERovercrowdedStrangeSensors; /**< counts number of times when there was a strange sensor (svd-only: mismatching number of u/v clusters) but too many hits on it to be able to try to rescue Clusters by forming 1D-VXDTFHits */


    // Member Variables for Collector/Display
    CollectorTFInfo m_collector; /**< Object that collectes all the Information needed for the collector*/
    int m_PARAMdisplayCollector;  /**< Collector operating flag: 0 = no collector, 1 = collect for analysis, 2 = collect for display */
    int m_aktpassNumber;  /**< Pass Number Information used for the collector */


  private:

  };
} // Belle2 namespace
