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
#include <testbeam/vxd/dataobjects/TelCluster.h>
#include <vxd/dataobjects/VxdID.h>

// tracking:
#include "tracking/vxdCaTracking/VXDTFHit.h"
#include "tracking/vxdCaTracking/VXDSegmentCell.h"
#include "tracking/vxdCaTracking/VXDSector.h"
#include "tracking/vxdCaTracking/ClusterInfo.h"
#include "tracking/vxdCaTracking/LittleHelper.h"
#include "tracking/dataobjects/FullSecID.h"
#include "tracking/vxdCaTracking/PassData.h"
#include "tracking/dataobjects/VXDTFInfoBoard.h"

//C++ base / C++ stl:
#include <fstream>
#include <iostream> // std::cout. std::fixed
#include <string>
#include <vector>
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
#ifndef __CINT__
#include <boost/chrono.hpp>
#endif

//genfit:
namespace genfit { class TrackCand; }
// #include <GFTrackCand.h>


namespace Belle2 {
  /** The BaseLineTFModule is a simplistic track finder for testbeams.
   *
   * It can use both VXD Detectors (SVD and PXD) to search for tracks and test structures like telescope sensors as well.
   * needed imput: PXD/SVDClusters and/or TelClusters
   * output: genfit::TrackCand
   */
  class BaseLineTFModule : public Module {

  public:

    struct SensorStruct; /**< forward declaration  */


    typedef std::pair<unsigned int, VXDSector* > secMapEntry; /**< represents an entry of the MapOfSectors */
    typedef std::map<unsigned int, SensorStruct> ActiveSensorsOfEvent; /**< is map where adresses to each activated sensor (key->int = uniID/vxdID) are stored and all clusters which can be found on them */
    typedef std::list<unsigned int> BrokenSensorsOfEvent; /**< atm a list containing the keys to all sensors where number of u and v clusters don't fit */

    typedef std::vector<PassData*> PassSetupVector; /**< contains all passes used for track reconstruction */
    typedef boost::chrono::high_resolution_clock boostClock; /**< used for measuring time comsumption */ // high_resolution_clock, process_cpu_clock
    typedef boost::chrono::microseconds boostNsec; /**< defines time resolution (currently mictroseconds) */ // microseconds, milliseconds

    typedef boost::tuple<double, double, VXDTFHit*> HitExtraTuple; /**< get<0>: distance to origin, get<1>: distance to seedHit, get<2> pointer to hit. SeedHit is outermost hit of detector and will be used for cosmic search */

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
        numPXDCluster(0),
        numTELCluster(0),
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
      void clear() {
        numPXDCluster = 0;
        numTELCluster = 0;
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
      int numTELCluster; /**< number of TELClusters (=number of TEL hits when tf in TEL is activated) */
      int numSVDCluster; /**< number of svdClusters */
      //
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
    BaseLineTFModule();


    //! Destructor
    virtual ~BaseLineTFModule();


    virtual void initialize();


    virtual void beginRun();


    virtual void event();


    virtual void endRun();


    virtual void terminate();


    /** *************************************+************************************* **/
    /** ***********************************+ + +*********************************** **/
    /** *******************************+ functions +******************************* **/
    /** ***********************************+ + +*********************************** **/
    /** *************************************+************************************* **/


    /** name is program, needed for GFTrackCand export */
    void calcInitialValues4TCs(PassData* currentPass);

    /** easy way to determine QI of track candidates, currently only suitable for testbeam cases since the calculation is sensitive for high slopes */
    void calcQIbyStraightLine(TCsOfEvent& tcVector);

    /** produce GFTrackCand for current TC */
    genfit::TrackCand generateGFTrackCand(VXDTFTrackCandidate* currentTC);

    /** reset all reused containers and delete others which are existing only for one event. */
    void cleanEvent(PassData* currentPass) {
      /** REDESIGNCOMMENT FINDSENSORS4CLUSTER 1:
       * * short:
       *
       ** long (+personal comments):
       *
       ** dependency of module parameters (global):
       *
       ** dependency of global in-module variables:
       * m_allTCsOfEvent,
       *
       ** dependency of global stuff just because of B2XX-output or debugging only:
       *
       ** in-module-function-calls:
       */
      currentPass->cleanPass();

      for (VXDTFTrackCandidate * aTC : m_allTCsOfEvent) { delete  aTC; }
      m_allTCsOfEvent.clear();
    }

    /** general Function to write data into a root file*/
    void writeToRootFile(double pValue, double chi2, double circleRadius, int ndf) {
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

    void resetCountersAtBeginRun() {
      /** REDESIGNCOMMENT FINDSENSORS4CLUSTER 1:
       * * short:
       *
       ** long (+personal comments):
       * all the entries below are the dependencies...
       *
       ** dependency of module parameters (global):
       *
       ** dependency of global in-module variables:
       * m_eventCounter,, m_TESTERbadSectorRangeCounterForClusters,
      * m_TESTERtriggeredZigZagXY, m_TESTERtriggeredZigZagXYWithSigma, m_TESTERclustersPersSectorNotMatching,
      * m_TESTERtriggeredDpT, m_TESTERtriggeredCircleFit, m_TESTERovercrowdedStrangeSensors,
       * m_TESTERcountTotalTCsFinal, m_TESTERcountTotalUsedIndicesFinal, m_TESTERcountTotalUsedHitsFinal,
       * m_TESTERstartedBaselineTF, m_TESTERsucceededBaselineTF, m_TESTERnoHitsAtEvent,
       * m_TESTERacceptedBrokenHitsTrack, m_TESTERrejectedBrokenHitsTrack, m_allTCsOfEvent
       *
       ** dependency of global stuff just because of B2XX-output or debugging only:
       * m_eventCounter, m_TESTERbadSectorRangeCounterForClusters, m_TESTERclustersPersSectorNotMatching,
       * m_TESTERtriggeredZigZagXY, m_TESTERtriggeredZigZagXYWithSigma, m_TESTERtriggeredDpT,
      * m_TESTERtriggeredCircleFit, m_TESTERcountTotalTCsFinal, m_TESTERcountTotalUsedIndicesFinal,
      * m_TESTERcountTotalUsedHitsFinal, m_TESTERovercrowdedStrangeSensors,
       * m_TESTERstartedBaselineTF, m_TESTERsucceededBaselineTF, m_TESTERnoHitsAtEvent,
       * m_TESTERacceptedBrokenHitsTrack, m_TESTERrejectedBrokenHitsTrack
       *
       ** in-module-function-calls:
       */
      m_eventCounter = 0;
      m_TESTERbadSectorRangeCounterForClusters = 0;
      m_TESTERclustersPersSectorNotMatching = 0;
      m_TESTERtriggeredZigZagXY = 0;
      m_TESTERtriggeredZigZagXYWithSigma = 0;
      m_TESTERtriggeredDpT = 0;
      m_TESTERtriggeredCircleFit = 0;
      m_TESTERcountTotalTCsFinal = 0;
      m_TESTERcountTotalUsedIndicesFinal = 0;
      m_TESTERcountTotalUsedHitsFinal = 0;
      m_TESTERovercrowdedStrangeSensors = 0;
      m_TESTERstartedBaselineTF = 0;
      m_TESTERsucceededBaselineTF = 0;
      m_TESTERnoHitsAtEvent = 0;
      m_TESTERacceptedBrokenHitsTrack = 0;
      m_TESTERrejectedBrokenHitsTrack = 0;
      m_allTCsOfEvent.clear();
    }

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

    /** sets up all the relevant settings for the baseLineTF.
     *
     * The BaseLineTF is a very simplistic pseudo-TF only relevant for testbeams
     * copying first pass for the BaselineTF (just to be sure that they can be used independently from each other)
     * */
    void setupBaseLineTF();


  protected:
    TCsOfEvent m_allTCsOfEvent; /**< carries links to really all track candidates found within event (used for deleting TrackCandidates at end of event) TODO: check whether use of m_tcVector can not be merged this one. Seems like redundant steps*/

    /// module_parameters:
    std::string m_PARAMsectorSetup; /**< lets you chose the sectorSetup (compatibility of sensors, individual cutoffs,...) accepts 'std', 'low', 'high' and 'personal', please note that the chosen setup has to exist as a xml-file in ../testbeam/vxd/data/XXX.xml. If you can not create your own xml files using e.g. the filterCalculatorModule, use params for  'tuneCutoffXXX' or 'setupWeigh' instead.*/

    bool m_PARAMactivateZigZagXY; /**< activates/deactivates current filter zzXY for each pass individually */
    bool m_PARAMactivateZigZagXYWithSigma; /**< activates/deactivates current filter zzXY with sigmas for each pass individually */
    bool m_PARAMactivateDeltaPt; /**< activates/deactivates current filter dPt for each pass individually */
    bool m_PARAMactivateCircleFit; /**< activates/deactivates current filter CircleFit for each pass individually */

    // tuning parameters:
    double m_PARAMtuneZigZagXY; /**< tunes current filter zzXY for each pass individually, same formula as in tuneDistance3D */
    double m_PARAMtuneZigZagXYWithSigma; /**< tunes current filter zzXY with sigmas for each pass individually */
    double m_PARAMtuneDeltaPt; /**< tunes current filter dPt for each pass individually, same formula as in tuneDistance3D */
    double m_PARAMtuneCircleFit; /**< tunes current filter CircleFit for each pass individually, here it is the global threshold value for the chi2-values produced by the fit which is stored */

    // needed for pass handling:
    PassData m_baselinePass; /**< baselineTF gets his own pass, gets some settings from the first pass of the PassSetupVector */


    int m_PARAMpdGCode; /**< tandard value is 211 (pi+), ATTENTION, instead of using inconsistent sign of PdGList, in this module positively charged particles are always positive and negatively charged ones are negative (relevant for leptons) */
    double m_PARAMartificialMomentum; /**< "standard value is 0. if StandardValue is changed to a nonZero value, the magnitude of the momentum seed is set artificially using this value, if value < 0, not only the magnitude is set using the norm of the value, but direction of momentum is reversed too, if you want to change charge, use parameter 'standardPdgCode'"*/

    LittleHelper m_littleHelperBox; /**< bundles small but often used functions for smearing and others.  */

    int m_chargeSignFactor; /**< particle dependent. for leptons it is 1, for other particles it's -1... */

    bool m_usePXDHits; /**< when having more than one pass per event, sector maps using PXD, SVD or TEL can be set independently. To produce TFHits for PXD, this value is set to true */
    bool m_useSVDHits; /**< when having more than one pass per event, sector maps using PXD, SVD or TEL can be set independently. To produce TFHits for SVD, this value is set to true */
    bool m_useTELHits; /**< when having more than one pass per event, sector maps using PXD, SVD or TEL can be set independently. To produce TFHits for TEL, this value is set to true */
    bool m_BackwardFilter; /**< determines whether the kalman filter moves inwards or backwards, bool means inwards */

    double m_PARAMtuneCutoffs; /**< for rapid changes of cutoffs (no personal xml files needed), reduces/enlarges the range of the cutoffs in percent (lower and upper values are changed by this value). Only valid in range -50% < x < +1000% */

    int m_eventCounter; /**< knows current event number */
    bool m_PARAMqiSmear; /**<  allows to smear QIs via qqq-Interface, needed when having more than one TC with the same QI */

    double m_PARAMsmearMean; /**< allows to introduce a bias for QI (e.g. surpressing all values, ...)*/
    double m_PARAMsmearSigma; /**< bigger values deliver broader distribution*/
    bool m_TESTERexpandedTestingRoutines; /**< set true if you want to export expanded infos of TCs for further analysis */
    bool m_PARAMwriteToRoot; /**< if true, a rootFile named by m_PARAMrootFileName will be filled with info */
    std::vector<std::string> m_PARAMrootFileName; /**< only two entries accepted, first one is the root filename, second one is 'RECREATE' or 'UPDATE' which is the write mode for the root file, parameter is used only if 'writeToRoot' = true */

    TFile* m_rootFilePtr; /**< pointer at root file used for p-value-output */
    TTree* m_treeTrackWisePtr; /**< pointer at root tree used for information stored once per track (e.g. p-value-output) */
    TTree* m_treeEventWisePtr; /**< pointer at root tree used for information stored once per event */
    long long int m_rootTimeConsumption; /**< used for storing duration of event in a root file */
    double m_rootPvalues; /**< used for storing pValues in a root file */
    double m_rootChi2; /**< used for storing chi2values in a root file */
    double m_rootCircleRadius; /**< used for storing the circle radii calculated by the circle fitter in a root file */
    int m_rootNdf; /**< used for storing numbers of degrees of freedom in a root file */

    std::string m_PARAMcalcQIType; /**< allows you to chose the way, the QI's of the TC's shall be calculated. currently supported: 'kalman','trackLength', 'circleFit' */
    int m_calcQiType; /**< is set by m_PARAMcalcQIType and defines which qi type shall be calculated */
    std::string m_PARAMcalcSeedType; /**< allows you to chose the way, the seed-mometa of the TC's shall be calculated. currently supported: 'helixFit', 'straightLine' */
    int m_calcSeedType; /**< is set by m_PARAMcalcSeedType and defines which seed type shall be calculated */
    std::string m_PARAMgfTrackCandsColName;       /**< TrackCandidates collection name */
    std::string m_PARAMinfoBoardName;             /**< InfoContainer collection name */
    std::string m_PARAMpxdClustersName;         /** name of storeArray containing pxd clusters */
    std::string m_PARAMtelClustersName;         /** name of storeArray containing tel clusters */
    std::string m_PARAMsvdClustersName;         /** name of storeArray containing svd clusters */
    std::string m_PARAMnameOfInstance;           /**< Name of trackFinder, usefull, if there is more than one VXDTF running at the same time. Note: please choose short names */

    /// the following variables are nimimal testing routines within the TF
    int m_TESTERnoHitsAtEvent;  /**< counts number of times, where there were no hits at the event */
    int m_TESTERstartedBaselineTF; /**< counts number of times, the baselineTF was started */
    int m_TESTERacceptedBrokenHitsTrack; /**< counts number of times, where a tc having at least 1 1D-SVD hit was accepted */
    int m_TESTERrejectedBrokenHitsTrack; /**< counts number of times, where a tc having at least 1 1D-SVD hit was rejected */
    int m_TESTERsucceededBaselineTF; /**< counts number of times, the baselineTF found a track */
    int m_TESTERtriggeredZigZagXY;/**< counts how many times zigZagXY filter found bad TCs */
    int m_TESTERtriggeredZigZagXYWithSigma;/**< counts how many times zigZagXYWithSigma filter found bad TCs */
    int m_TESTERtriggeredDpT; /**< counts how many times deltaPt filter found bad TCs  */
    int m_TESTERtriggeredCircleFit; /**< counts how many times circleFit filter found bad TCs  */
    int m_TESTERcountTotalTCsFinal; /**< counts number of TCs which survived until the end of event */
    int m_TESTERcountTotalUsedIndicesFinal; /**< counts number of indices used by TCs which survived until the end of event */
    int m_TESTERcountTotalUsedHitsFinal; /**< counts number of indices used by TCs which survived until the end of event */
    TimeInfo m_TESTERtimeConsumption; /**< a struct to store amount of time needed for special blocks of the program */
    std::vector<EventInfoPackage> m_TESTERlogEvents; /**< a list containing some information about each event (time consumption and quality) */
    int m_TESTERbadSectorRangeCounterForClusters; /**< counts number of times when only 1 cluster has been found on a complete sensor (therefore no 2D hits possible) */
    int m_TESTERclustersPersSectorNotMatching; /**< counts number of times when numofUclusters and nVclusters per sensor do not match */
    int m_TESTERovercrowdedStrangeSensors; /**< counts number of times when there was a strange sensor (svd-only: mismatching number of u/v clusters) but too many hits on it to be able to try to rescue Clusters by forming 1D-VXDTFHits */

  private:

  };
} // Belle2 namespace
