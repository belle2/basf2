/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDTFModule_H_
#define VXDTFModule_H_

//framework:
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/GearDir.h> // needed for reading xml-files
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>
#include <vxd/dataobjects/VxdID.h>

#include "tracking/dataobjects/VXDTFInfoBoard.h"

#include "tracking/vxdCaTracking/VXDTFHit.h"
#include "tracking/vxdCaTracking/VXDSegmentCell.h"
#include "tracking/vxdCaTracking/VXDSector.h"
#include "tracking/vxdCaTracking/VXDTFTrackCandidate.h"
#include "tracking/vxdCaTracking/SegFinderFilters.h"
#include "tracking/vxdCaTracking/NbFinderFilters.h"
#include "tracking/vxdCaTracking/TcFourHitFilters.h"
#include "tracking/vxdCaTracking/TrackletFilters.h"
#include "tracking/vxdCaTracking/ClusterInfo.h"
#include "tracking/vxdCaTracking/LittleHelper.h"
#include "tracking/vxdCaTracking/FullSecID.h"
#include "tracking/vxdCaTracking/SharedFunctions.h"


//C++ base / C++ stl:
#include <fstream>
#include <iostream>
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
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/casts.hpp>
#ifndef __CINT__
#include <boost/unordered_map.hpp>
#include <boost/chrono.hpp>
#endif

//genfit:
#include <GFTrackCand.h>


namespace Belle2 {
  namespace Tracking {
    /**
    *  \addtogroup modules
    *  @{
    *  \addtogroup tracking_modules
    *  \ingroup modules
    *  @{ VXDTFModule @} @}
    */

    /** The VXDTFModule is a low momentum Si-only trackfinder. It can use both VXD Detectors (SVD and PXD) to search for tracks.
    * It suports different momentum ranges which can be set via 'sectorSetup' Standard is a three pass setup covering the range of 50MeV/c < pT < 1GeV/c.
    * lower and higher momentum tracks can be found with reduced efficiency.
    * long term features like curling track support is currently not included
    * needed imput: PXD/SCDClusters
    * output: GFTrackCand
    */
    class VXDTFModule : public Module {

    public:

      struct CurrentPassData; /**< forward declaration  */

      //      boost::unordered_map

      //    typedef std::map<std::string, VXDSector*> MapOfSectors;
      //    typedef std::map<std::string, Cutoff*> CutoffMap;
      //     typedef std::map<std::string, CutoffMap*> MapOfCutoffTypes;
      typedef boost::unordered_map<unsigned int, VXDSector*> MapOfSectors; /**< stores whole sectorMap used for storing cutoffs */
      typedef std::pair<unsigned int, VXDSector* > secMapEntry; /**< represents an entry of the MapOfSectors */
      typedef boost::unordered_map<int, Cutoff*> CutoffMap; /**< Is a map storing cutoffs  */
      typedef boost::unordered_map<int, CutoffMap*> MapOfCutoffTypes; /**< represents an entry of the CutoffMap */
      typedef std::vector<VXDTFHit*> HitsOfEvent; /**< contains all hits of event */
      typedef std::list<VXDSegmentCell*> ActiveSegmentsOfEvent; /**< is list since random deleting processes are needed */
      typedef std::vector<VXDSegmentCell*> TotalSegmentsOfEvent; /**< is vector since no entries are deleted and random access is needed  */
      typedef std::pair<unsigned int, MapOfSectors::iterator> SectorNameAndPointerPair; /**< we are storing the name of the sector (which is encoded into an int) to be able to sort them! */
      typedef std::list<SectorNameAndPointerPair> OperationSequenceOfActivatedSectors; /**< contains all active sectors, can be sorted by name (first entry) */
      typedef std::vector<VXDTFTrackCandidate*> TCsOfEvent; /**< contains all track candidates of event */
      typedef std::vector<CurrentPassData*> PassSetupVector; /**< contains all passes used for track reconstruction */
      typedef std::pair<bool, double> Filter; /**< defines whether current filter is allowed (.first) and whether the cutoffvalues shall be tuned (.second).  */
      typedef boost::chrono::high_resolution_clock boostClock; /**< used for measuring time comsumption */ // high_resolution_clock, process_cpu_clock
      typedef boost::chrono::microseconds boostNsec; /**< defines time resolution (currently mictroseconds) */ // microseconds, milliseconds
      //    typedef boost::chrono::duration_cast durationCast;
      //    typedef std::chrono::high_resolution_clock boostClock;
      //    typedef std::chrono::nanoseconds boostNsec;



      /** structs for internal use **/
      /** SensorStruct needed for SVDCluster sorting, stores u and v clusters of Sensor  */
      struct SensorStruct {
        std::vector<std::pair<int, Belle2::SVDCluster*> > uClusters; /**< .first is arrayIndex in StoreArray, .second is pointer to the Cluster itself */
        std::vector<std::pair<int, Belle2::SVDCluster*> > vClusters; /**< same as uClusters, but for vClusters  */
        int layerID; /**< layer ID of the Cluster */
      };


      /** needed for SVDCluster sorting, represents a 2D-cluster (combining 2 1D-clusters) */
      struct ClusterHit {
        SVDCluster* uCluster; /**< pointer to uCluster of current Hit */
        SVDCluster* vCluster; /**< pointer to vCluster of current Hit */
        int uClusterIndex; /**< index number of uCluster of current Hit */
        int vClusterIndex; /**< index number of uCluster of current Hit */
      };

      /** stores information to each cluster which TC is using it */
      struct ClusterUsage {
        std::vector<TCsOfEvent > PXDClusters; /**< carries index numbers of all pxd clusters and stores pointers to all TCs using them */
        std::vector<TCsOfEvent> SVDClusters; /**< same as above but for svd clusters (information, whether its u or v is not important) */
      };


      /** for testing purposes, storing time consumption */
      struct TimeInfo {
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

        /** printing collected data, returned string can be printed using B2INFO or similar */
        std::string Print();

        /** clearing entries, nice after initialisation (TODO: convert into constructor for autoClear) */
        void clear() {
          numPXDCluster = 0;
          numSVDCluster = 0;
          numSVDHits = 0;
          segFinderActivated = 0;
          segFinderDiscarded = 0;
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
        //
        int numSVDHits; /**< number of possible svd-cluster-combinations. every combination of any pass will be counted  */
        int segFinderActivated; /**< number of segments which survived the segfinder. every segment of any pass will be counted  */
        int segFinderDiscarded; /**< number of segments which died in the segfinder. every segment of any pass will be counted  */
        int nbFinderActivated; /**< number of segments which survived the nbfinder. every segment of any pass will be counted  */
        int nbFinderDiscarded;  /**< number of segments which died in the nbfinder. every segment of any pass will be counted  */
        int tccApprovedTCs;  /**< number of tcs approved by the tcc. every tc of any pass will be counted  */
        int numTCsAfterTCC; /**< number of tcs alive after the tcc. every tc of any pass will be counted  */
        int numTCsAfterTCCfilter; /**< number of tcs alive after the tc-filter. every tc of any pass will be counted  */
        int numTCsKilledByCleanOverlap; /**< number of tcs killed by the cleanOverlappingSet-method */
        int numTCsfinal; /**< number of tcs alive in the end */

      };


      /** all the information of a pass is stored within a copy of that struct. This allows grouping of that information. */
      struct CurrentPassData {
        /** ** filled once ** **/
        MapOfSectors sectorMap; /**< carries the whole lookup-table including the information of which sectors can be combined */

        std::string sectorSetup; /**< name of setup, needed e.g. for XML-readout */
        std::string chosenDetectorType; /**< same as detectorType, but fully written ('SVD','PXD' or 'VXD'), needed during xml and for logging messages */
        int detectorType; /**< PXD = 0 , SVD = 1, VXD = -1 */
        double setupWeigh; /**< defines importance of current Pass. most important Passes stay at value 0.0, less important ones can be set between 0.0 and 100.0 (percent of QI-decrease). This affects the outcome of the neuronal network */
        short int highestAllowedLayer; /**< needed for e.g. hitfinding. This value excludes Layernumbers higher than set value. (interesting for low momentum tracks)  */
        int numTotalLayers;  /**< needed e.g. for neuronal network. This value allows calculation of maximum track length (noncurling), carries implicit information about detectorType, while 'highestAllowedLayer' does not know whether its a SVD or VXD setup */
        int minLayer; /**< lowest layer considered for TCC */
        int minState; /**< lowest state considered for seeds during TCC */

        /** soon to come (maybe even layer-specific): **/
        std::vector<double> secConfigU;  /**< defines subdivition of sensors U */
        std::vector<double> secConfigV; /**< defines subdivition of sensors V */

        /** for segFinder, compares 2 hits**/
        Filter distance3D; /**< carries information about the filter 'distance3D', type pair<bool isActivated, double tuningParameter> */
        Filter distanceXY; /**< carries information about the filter 'distanceXY', type pair<bool isActivated, double tuningParameter> */
        Filter distanceZ; /**< carries information about the filter 'distanceZ', type pair<bool isActivated, double tuningParameter> */
        Filter slopeRZ; /**< carries information about the filter 'slopeRZ', type pair<bool isActivated, double tuningParameter> */
        Filter normedDistance3D; /**< carries information about the filter 'normedDistance3D', type pair<bool isActivated, double tuningParameter> */
        int activatedSegFinderTests; /**< counts number of tests activated for segFinder */
        /** for segFinder in high occupancy mode, compares 2+1 hits (using origin as third hit) */
        Filter anglesHighOccupancy3D; /**< exactly the same as 'angles3D' but for high occupancy cases */
        Filter anglesHighOccupancyXY; /**< exactly the same as 'anglesXY' but for high occupancy cases */
        Filter anglesHighOccupancyRZ; /**< exactly the same as 'anglesRZ' but for high occupancy cases */
        Filter deltaSlopeHighOccupancyRZ; /**< exactly the same as 'deltaSlopeRZ' but for high occupancy cases */
        Filter pTHighOccupancy; /**< exactly the same as 'pT' but for high occupancy cases */
        Filter distanceHighOccupancy2IP; /**< exactly the same as 'distance2IP' but for high occupancy cases */
        Filter helixHighOccupancyFit; /**< exactly the same as 'helixFit' but for high occupancy cases */
        int activatedHighOccupancySegFinderTests; /**< counts number of tests activated for segFinder in HighOccupancy mode */

        /** for nbFinder, compares 3 hits **/
        Filter angles3D; /**< carries information about the filter 'angles3D', type pair<bool isActivated, double tuningParameter> */
        Filter anglesXY; /**< carries information about the filter 'anglesXY', type pair<bool isActivated, double tuningParameter> */
        Filter anglesRZ; /**< carries information about the filter 'anglesRZ', type pair<bool isActivated, double tuningParameter> */
        Filter deltaSlopeRZ; /**< carries information about the filter 'deltaSlopeRZ', type pair<bool isActivated, double tuningParameter> */
        Filter pT; /**< carries information about the filter 'pT', same type as others */
        Filter distance2IP; /**< carries information about the filter 'distance2IP', type pair<bool isActivated, double tuningParameter>  in XY-plane, determines center of projection circle of the track and measures the residual between distance IP <-> circleCenter minus circleRadius */
        Filter helixFit; /**< carries information about the filter 'helixFit', same type as others. It uses the fact that a helix projection to the xy-plane forms a circle. Any angle of a segment of a circle divided by its deltaZ-value has to be the same value. Therefore any three hits lying on the same helix can be compared this way */
        int activatedNbFinderTests; /**< counts number of tests activated for nbFinder */
        /** for nbFinder in high occupancy mode, compares 3+1 hits (using origin as fourth hit) */
        Filter deltaPtHighOccupancy; /**< exactly the same as 'deltaPt' but for high occupancy cases */
        Filter deltaDistanceHighOccupancy2IP; /**< exactly the same as 'deltaDistance2IP' but for high occupancy cases */
        int activatedHighOccupancyNbFinderTests; /**< counts number of tests activated for nbFinder */

        /** for TCC filter, compares 4 hits **/
        Filter zigzagXY; /**< carries information about the filter 'zigZagXY', type pair<bool isActivated, double tuningParameter> , here the tuningparameter is currently ignored, possible future use is reserved for defining uncertainties because of measurement errors */
        Filter zigzagRZ; /**< carries information about the filter 'zigZagRZ', type pair<bool isActivated, double tuningParameter> , here the tuningparameter is currently ignored, possible future use is reserved for defining uncertainties because of measurement errors */
        Filter deltaPt; /**< carries information about the filter 'deltaPt', type pair<bool isActivated, double tuningParameter> */
        Filter deltaDistance2IP; /**< carries information about the filter 'deltaDistance2IP', type pair<bool isActivated, double tuningParameter> */
        Filter circleFit; /**< carries information about the filter 'circleFit', type pair<bool isActivated, doubletuningParameter> , here the tuningparameter is currently used to store the global chi2 threshold value. */
        int activatedTccFilterTests; /**< counts number of tests activated for tcc filter */


        /** filled and resetted each event **/
        OperationSequenceOfActivatedSectors sectorSequence; /**< carries pointers to sectors which are used in current event */
        ActiveSegmentsOfEvent activeCellList; /**< list of active cells, dead ones get kicked (therefore it's a list) */
        TotalSegmentsOfEvent totalCellVector;   /**< This vector contains throughout the whole event all segments found. It is needed since
        * VXDTFHits are not allowed to carry pointers to Segments any more. As a cheap replacement, indices of this vector are stored in
        * each hit. We still need this link since until the neighbourfinder did his work, no segmentCell does know its neighbour (the name
        * explains itself ;) ).  Therefore each hit carries the indices of its inner and outer neighbours so the neighbourfinder can easily
        * find neighbouring segments. */
        HitsOfEvent hitVector; /**< carries total number of hits of current event. atm only used for .size() and final delete of hits at end of event, can we remove it? */
        TCsOfEvent tcVector; /**< carries track candidates of current pass */
      };

      //! Constructor
      VXDTFModule();

      //! Destructor
      virtual ~VXDTFModule();

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

      /** works with VXDSegmentCells: checks state of inner neighbours and removes incompatible and virtual ones   */
      void delFalseFriends(CurrentPassData* currentPass,
                           TVector3 primaryVertex); // -> TODO: dirty little helper


      /** Recursive CoreFunction of the Track Candidate Collector, stores every possible way starting at a Seed (VXDSegmentCell) */
      void findTCs(TCsOfEvent& tcList,
                   VXDTFTrackCandidate* currentTC,
                   short int maxLayer);


      /** Neuronal network filtering overlapping Track Candidates by searching best subset of TC's */
      void hopfield(TCsOfEvent& tcList,
                    double omega);


      /** search for nonOverlapping trackCandidates using Greedy algorithm (start with TC of highest QI, remove all TCs incompatible with current TC, if there are still TCs there, repeat step until no incompatible TCs are there any more) */
      void greedy(TCsOfEvent& tcVector);


      /** used by VXDTFModule::greedy, recursive function which takes tc with highest QI and kills all its rivals. After that, TC gets removed and process is repeated with shrinking list of TCs until no TCs alive has got rivals alive */
      void greedyRecursive(std::list< std::pair<double, VXDTFTrackCandidate*> >& overlappingTCs,
                           double& totalSurvivingQI,
                           int& countSurvivors,
                           int& countKills);


      /** for the easy situation of 2 overlapping TCs we dont need comlex algorithms for finding the best subset of clean TCs... */
      void tcDuel(TCsOfEvent& tcVector);


      /** calculates integer score for current filter (all filters combined deliver the QQQ (normed to 0-1)), works for filterTypes having both: min- and max-value */
      int calcQQQscore(std::vector<std::pair<std::string, double> > quantiles,
                       double currentValue,
                       int numOfQuantiles); // -> TODO: dirty little helper


      /** calculates integer score for current filter (all filters combined deliver the QQQ (normed to 0-1)), works for filterTypes having only  max-value */
      int calcQQQscoreDeltas(std::vector<std::pair<std::string, double> > quantiles,
                             double currentValue,
                             int numOfQuantiles); // -> TODO: dirty little helper


      /** searches for sectors fitting current hit coordinates, returns blank string if nothing could be found */
      VXDTFModule::SectorNameAndPointerPair searchSector4Hit(VxdID aVxdID,
                                                             TVector3 localHit,
                                                             TVector3 sensorSize,
                                                             VXDTFModule::MapOfSectors& m_sectorMap,
                                                             std::vector<double>& uConfig,
                                                             std::vector<double>& vConfig); // -> TODO: generally usefull for VXD-related modules


      /** needed for sorting sectorSequence and compares strings... */
      static bool compareSecSequence(SectorNameAndPointerPair& lhs, SectorNameAndPointerPair& rhs); // -> TODO: dirty little helper


      /** searches for segments in given pass and returns number of discarded segments */
      int segFinder(CurrentPassData* currentPass);


      /** checks segments in given pass with some extra tests for the segFinder (using nbFinder-Tests) for high occupancy cases, returns whether the segment passed the tests or not */
      bool SegFinderHighOccupancy(CurrentPassData* currentPass, NbFinderFilters& threeHitFilterBox);


      /** filters neighbouring segments in given pass and returns number of discarded segments */
      int neighbourFinder(CurrentPassData* currentPass);


      /** checks segments in given pass with some extra tests for the neighbourFinder (using tcc-filter-Tests) for high occupancy cases, returns whether the segment passed the tests or not */
      bool NbFinderHighOccupancy(CurrentPassData* currentPass, TcFourHitFilters& fourHitFilterBox);


      /** the cellular automaton used in the TF.
      * it evolves a combination of two hits called Cells depending of their neighbourhood (=inner Cells attached to current one). In the end, Cells with a useful chain of inner segments carry the highest states.
      * for more information about cellular automata, google "cellular automaton" or "game of life". if returned value is negative, CA was aborted because of infinity loop */
      int cellularAutomaton(CurrentPassData* currentPass);


      /** uses attribute "state" to find rows of compatible neighbours. State indicates length of row. This allows fast Track candidate collection */
      void tcCollector(CurrentPassData* currentPass); // -> TODO: generally usefull for VXD-related modules


      /** filters TCs via deltaPt, ZigZag and deltaDistance2IP */
      int tcFilter(CurrentPassData* currentPass,
                   int passNumber/*,
                   std::vector<ClusterInfo>& clustersOfEvent*/);


      /** represents a step between the fast but weak calcQIbyLength and the mighty but slow calcQIbyKalman. Only useful for Tracks having at least 4 hits (3-hit-tracks will be set with smearValue) */
      //     void calcQIbyCircleFit(TCsOfEvent& tcVector);


      /** name is program, needed for GFTrackCand export */
      void calcInitialValues4TCs(TCsOfEvent& tcVector); // -> TODO auslagern!


      /** simplest way to determine QI of track candidates, calculating them by track length */
      void calcQIbyLength(TCsOfEvent& tcVector,
                          PassSetupVector& passSetups); // -> auslagern!

      /** produce GFTrackCand for current TC */
      GFTrackCand generateGFTrackCand(VXDTFTrackCandidate* currentTC, std::vector<ClusterInfo>& clusters);

      /** calculate real kalman-QI's for each currently living TC */
      void calcQIbyKalman(TCsOfEvent& tcVector,
                          StoreArray<PXDCluster>& pxdClusters,
                          std::vector<ClusterInfo>& clusters); // ->auslagern!
      //    void calcQIbyKalman(TCsOfEvent& tcVector,
      //                         StoreArray<PXDCluster>& pxdClusters,
      //                         StoreArray<SVDCluster>& svdClusters,
      //                         std::vector<ClusterInfo>& clusters); // old version, backup 13-03-29


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
      void cleanEvent(CurrentPassData* currentPass, unsigned int centerSector);

      /** for streching the limits of the cutoff values for finetuning */
      double addExtraGain(double prefix, double cutOff, double generalTune, double specialTune); // -> TODO: dirty little helper

      /** safe way of importing cutoff values from the xml-file */
      double getXMLValue(GearDir& quantiles, std::string& valueType, std::string& filterType); // -> TODO: dirty little helper

      /** general Function to write data into a root file*/
      void writeToRootFile(double pValue, double chi2, int ndf);

      //    /** general Function to write data into a root file*/
      //    void VXDTFModule::writeToRootFile(const Tracking::T_type1& variable, const std::string& branchName, const std::string &treeName);
      /** random generator function */
      //    ptrdiff_t rngWrapper(ptrdiff_t i);
    protected:
      TCsOfEvent m_tcVector; /**< carries links to all track candidates found within event (during tcc filter, bad ones get kicked, lateron they simply get deactivated) */
      TCsOfEvent m_tcVectorOverlapped; /**< links only to track candidates which share at least one cluster with others*/

      /// module_parameters:
      bool m_PARAMDebugMode; /**< some code will only be executed if this mode is enabled */
      std::vector<std::string> m_PARAMsectorSetup; /**< lets you chose the sectorSetup (compatibility of sensors, individual cutoffs,...) accepts 'std', 'low', 'high' and 'personal', please note that the chosen setup has to exist as a xml-file in ../tracking/data/friendList_XXX.xml. If you can not create your own xml files using e.g. the filterCalculatorModule, use params for  'tuneCutoffXXX' or 'setupWeigh' instead. multipass supported by setting setups in a row */

      std::vector<std::string> m_PARAMdetectorType; /**< defines which detector type has to be exported. VXD, PXD, SVD */
      std::vector<double> m_PARAMsetOrigin; /**< allows to reset orign (e.g. usefull for special cases like testbeams), only valid if 3 entries are found */
      double m_PARAMmagneticFieldStrength; /**< strength of magnetic field in Tesla, standard is 1.5T */
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
      std::vector<bool> m_PARAMactivateHelixFit; /**< activates/deactivates current filter HelixFit for each pass individually */
      std::vector<bool> m_PARAMactivateZigZagXY; /**< activates/deactivates current filter zzXY for each pass individually */
      std::vector<bool> m_PARAMactivateZigZagRZ; /**< activates/deactivates current filter zzRZ for each pass individually */
      std::vector<bool> m_PARAMactivateDeltaPt; /**< activates/deactivates current filter dPt for each pass individually */
      std::vector<bool> m_PARAMactivateDeltaDistance2IP; /**< activates/deactivates current filter dd2IP for each pass individually*/
      std::vector<bool> m_PARAMactivateCircleFit; /**< activates/deactivates current filter CircleFit for each pass individually */
      std::vector<bool> m_PARAMactivateAngles3DHioC; /**< activates/deactivates current filter a3D (high occupancy mode) for each pass individually */
      std::vector<bool> m_PARAMactivateAnglesXYHioC; /**< activates/deactivates current filter aXY (high occupancy mode) for each pass individually */
      std::vector<bool> m_PARAMactivateAnglesRZHioC; /**< activates/deactivates current filter aRZ (high occupancy mode) for each pass individually */
      std::vector<bool> m_PARAMactivateDeltaSlopeRZHioC; /**< activates/deactivates current filter dslopeRZ (high occupancy mode) for each pass individually */
      std::vector<bool> m_PARAMactivateDistance2IPHioC; /**< activates/deactivates current filter d2IP (high occupancy mode) for each pass individually */
      std::vector<bool> m_PARAMactivatePTHioC; /**< activates/deactivates current filter PT (high occupancy mode) for each pass individually */
      std::vector<bool> m_PARAMactivateHelixFitHioC; /**< activates/deactivates current filter HelixFit (high occupancy mode) for each pass individually */
      std::vector<bool> m_PARAMactivateDeltaPtHioC; /**< activates/deactivates current filter dPt (high occupancy mode) for each pass individually */
      std::vector<bool> m_PARAMactivateDeltaDistance2IPHioC; /**< activates/deactivates current filter dd2IP (high occupancy mode) for each pass individually*/

      std::vector<double> m_PARAMtuneDistance3D; /**< tunes current filter d3D for each pass individually */
      std::vector<double> m_PARAMtuneDistanceXY; /**< tunes current filter dXY for each pass individually */
      std::vector<double> m_PARAMtuneDistanceZ; /**< tunes current filter dZ for each pass individually */
      std::vector<double> m_PARAMtuneNormedDistance3D; /**< tunes current nd3D filter for each pass individually */
      std::vector<double> m_PARAMtuneSlopeRZ; /**< tunes current filter slopeRZ for each pass individually */
      std::vector<double> m_PARAMtuneAngles3D; /**< tunes current filter a3D for each pass individually */
      std::vector<double> m_PARAMtuneAnglesXY; /**< tunes current filter aXY for each pass individually */
      std::vector<double> m_PARAMtuneAnglesRZ; /**< tunes current filter aRZ for each pass individually */
      std::vector<double> m_PARAMtuneDeltaSlopeRZ; /**< tunes current filter dslopeRZ for each pass individually */
      std::vector<double> m_PARAMtuneDistance2IP; /**< tunes current filter d2IP for each pass individually */
      std::vector<double> m_PARAMtunePT; /**< tunes current filter PT for each pass individually */
      std::vector<double> m_PARAMtuneHelixFit; /**< tunes current filter HelixFit for each pass individually */
      std::vector<double> m_PARAMtuneZigZagXY; /**< tunes current filter zzXY for each pass individually */
      std::vector<double> m_PARAMtuneZigZagRZ; /**< tunes current filter zzRZ for each pass individually */
      std::vector<double> m_PARAMtuneDeltaPt; /**< tunes current filter dPt for each pass individually */
      std::vector<double> m_PARAMtuneDeltaDistance2IP; /**< tunes current filter dd2IP for each pass individually */
      std::vector<double> m_PARAMtuneCircleFit; /**< tunes current filter CircleFit for each pass individually, here it is the global threshold value for the chi2-values produced by the fit which is stored */
      std::vector<double> m_PARAMtuneAngles3DHioC; /**< tunes current filter a3D (high occupancy mode) for each pass individually */
      std::vector<double> m_PARAMtuneAnglesXYHioC; /**< tunes current filter aXY (high occupancy mode) for each pass individually */
      std::vector<double> m_PARAMtuneAnglesRZHioC; /**< tunes current filter aRZ (high occupancy mode) for each pass individually */
      std::vector<double> m_PARAMtuneDeltaSlopeRZHioC; /**< tunes current filter dslopeRZ (high occupancy mode) for each pass individually */
      std::vector<double> m_PARAMtuneDistanceDeltaZHioC; /**< tunes current filter ddZ (high occupancy mode) for each pass individually */
      std::vector<double> m_PARAMtuneDistance2IPHioC; /**< tunes current filter d2IP (high occupancy mode) for each pass individually */
      std::vector<double> m_PARAMtunePTHioC; /**< tunes current filter PT (high occupancy mode) for each pass individually */
      std::vector<double> m_PARAMtuneHelixFitHioC; /**< tunes current filter HelixFit (high occupancy mode) for each pass individually */
      std::vector<double> m_PARAMtuneDeltaPtHioC; /**< tunes current filter dPt (high occupancy mode) for each pass individually */
      std::vector<double> m_PARAMtuneDeltaDistance2IPHioC; /**< tunes current filter dd2IP (high occupancy mode) for each pass individually */


      std::vector<double> m_PARAMsetupWeigh; /**< allows to individually weigh (level of surpression) each setup/pass. Please choose between 0 and -100 (%), higher momentum setups are more reliable, therefore should be weighed best */

      /// needed for pass handling:
      PassSetupVector m_passSetupVector; /**< contains information for each pass */

      std::vector<double> m_PARAMsectorConfigU; /**< allows defining the the config of the sectors in U direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0 */
      std::vector<double> m_PARAMsectorConfigV; /**< allows defining the the config of the sectors in V direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0 */
      int m_PARAMpdGCode; /**< tandard value is 211 (pi+), ATTENTION, instead of using inconsistent sign of PdGList, in this module positively charged particles are always positive and negatively charged ones are negative (relevant for leptons) */

      SegFinderFilters m_twoHitFilterBox; /**< contains all the two hit filters needed by the segFinder */
      NbFinderFilters m_threeHitFilterBox; /**< contains all the three hit filters needed by the nbFinder */
      TcFourHitFilters m_fourHitFilterBox; /**< contains all the four hit filters needed by the post-ca-Filter */
      TrackletFilters m_trackletFilterBox; /**< contains all the four-or-more hit filters needed by the post-ca-Filter */
      LittleHelper m_littleHelperBox; /**< bundles small but often used functions for smearing and others.  */

      int m_chargeSignFactor; /**< particle dependent. for leptons it is 1, for other particles it's -1... */

      std::vector< std::vector< std::pair<double, double> > > m_globalizedErrorContainer; /**< stores error of u coordinates of each ladder in the vxd converted to x and y global coordinates. These values are needed by the circleFitter. How to access: container[layerID][ladderID].first = sigmaX, container[layerID][ladderID].second = sigmaY */
      std::vector< std::pair<double, double> > m_errorContainer; /**< stores error of u and v coordinates of each layer in the vxd. These values are needed by the circleFitter. How to access: container[layerID].first = sigmaU, container[layerID].second = sigmaV */
      int m_usePXDorSVDorVXDhits; /**< when having more than one pass per event, sector maps using PXD, SVD or VXD can be set independently. To produce TFHits only when needed, this value is set to -1,0 or 1 */

      //     std::vector<std::string> m_PARAMmultiPassSectorSetup; /**< controls usage of one or several passes for TF per event (allows differnt pT's or curling track support) */
      double m_PARAMtuneCutoffs; /**< for rapid changes of cutoffs (no personal xml files needed), reduces/enlarges the range of the cutoffs in percent (lower and upper values are changed by this value). Only valid in range -50% < x < +1000% */

      int m_eventCounter; /**< knows current event number */
      int m_badSectorRangeCounter; /**< counts number of hits which couldn't be attached to an existing sector of a pass */
      int m_badFriendCounter; /**< counts number of hits having no neighbour hits in friend sectors of current sector */
      int m_totalPXDClusters; /**< counts total number of PXDClusters during run */
      int m_totalSVDClusters; /**< counts total number of SVdClusters during run */
      int m_totalSVDClusterCombis; /**< counts total number of possible combinations of SVDClusters during run */
      int m_numOfSectorSetups; /**< stores info about number of sector setups loaded into the track finder */
      bool m_KFBackwardFilter; /**< determines whether the kalman filter moves inwards or backwards, bool means inwards */
      bool m_highOccupancyCase; /**< is determined by a userdefined threshold. If there are more hits in the event than threshold value, high occupancy filters are activated (segFinder and nbFinder only) */
      int m_PARAMhighOccupancyThreshold; /**< If there are more hits in a sensor than threshold value, high occupancy filters are activated (segFinder and nbFinder only) */
      int m_PARAMkillBecauseOfOverlappsThreshold; /**< if there are more TCs overlapping than threshold value, event kalman gets replaced by circleFit. If there are 10 times more than threshold value of TCs, the complete event gets aborted */
      int m_PARAMkillEventForHighOccupancyThreshold; /**< if there are more segments than threshold value, the complete event gets aborted */

      double m_PARAMomega; /**< tuning parameter for hopfield network */
      double m_tcThreshold;   /**< defines threshold for hopfield network. neurons having values below threshold are discarded */

      bool m_PARAMqiSmear; /**<  allows to smear QIs via qqq-Interface, needed when having more than one TC with the same QI */
      bool m_PARAMcleanOverlappingSet; /**< when true, TCs which are found more than once (possible because of multipass) will get filtered */
      //     bool m_PARAMuseHopfield;
      std::string m_PARAMfilterOverlappingTCs; /**< defines which technique shall be used for filtering overlapping TCs, currently supported: 'hopfield', 'greedy', 'none' */
      int m_filterOverlappingTCs; /**< is set by m_PARAMfilterOverlappingTCs and defines which technique shall be used for filtering overlapping TCs */
      double m_PARAMsmearMean; /**< allows to introduce a bias for QI (e.g. surpressing all values, ...)*/
      double m_PARAMsmearSigma; /**< bigger values deliver broader distribution*/
      bool m_PARAMstoreBrokenQI;/**< if true, TC survives QI-calculation-process even if fit was not possible */
      bool m_TESTERexpandedTestingRoutines; /**< set true if you want to export expanded infos of TCs for further analysis */
      bool m_PARAMwriteToRoot; /**< if true, a rootFile named by m_PARAMrootFileName will be filled with info */
      std::vector<std::string> m_PARAMrootFileName; /**< only two entries accepted, first one is the root filename, second one is 'RECREATE' or 'UPDATE' which is the write mode for the root file, parameter is used only if 'writeToRoot' = true */

      TFile* m_rootFilePtr; /**< pointer at root file used for p-value-output */
      TTree* m_treeTrackWisePtr; /**< pointer at root tree used for information stored once per track (e.g. p-value-output) */
      TTree* m_treeEventWisePtr; /**< pointer at root tree used for information stored once per event */
      long long int m_rootTimeConsumption; /**< used for storing duration of event in a root file */
      double m_rootPvalues; /**< used for storing pValues in a root file */
      double m_rootChi2; /**< used for storing chi2values in a root file */
      int m_rootNdf; /**< used for storing numbers of degrees of freedom in a root file */
      std::vector<double>  m_rootVecPvalues; /**< used for storing grouped pValues in a root file */
      std::vector<double> m_rootVecChi2; /**< used for storing grouped chi2values in a root file */
      std::vector<int> m_rootVecNdf; /**< used for storing grouped numbers of degrees of freedom in a root file */

      std::string m_PARAMcalcQIType; /**< allows you to chose the way, the QI's of the TC's shall be calculated. currently supported: 'kalman','trackLength', 'circleFit' */
      int m_calcQiType; /**< is set by m_PARAMcalcQIType and defines which qi type shall be calculated */
      std::string m_PARAMgfTrackCandsColName;     /**< TrackCandidates collection name */
      std::string m_PARAMinfoBoardName;           /**< InfoContainer collection name */
      std::string m_PARAMnameOfInstance;           /**< Name of trackFinder, usefull, if there is more than one VXDTF running at the same time. Note: please choose short names */

      /// the following variables are nimimal testing routines within the TF
      int m_TESTERtriggeredZigZagXY;/**< counts how many times zigZagXY filter found bad TCs */
      int m_TESTERtriggeredZigZagRZ;/**< counts how many times zigZagRZ filter found bad TCs */
      int m_TESTERtriggeredDpT; /**< counts how many times deltaPt filter found bad TCs  */
      int m_TESTERtriggeredCircleFit; /**< counts how many times circleFit filter found bad TCs  */
      int m_TESTERtriggeredDD2IP;  /**< counts how many times deltaDistance2IP filter found bad TCs  */
      int m_TESTERapprovedByTCC; /**< counts how many TCs got approved by TCC */
      int m_TESTERcountTotalTCsAfterTCC; /**< counts number of TCs found by TCC */
      int m_TESTERcountTotalTCsAfterTCCFilter; /**< counts number of TCs which survived the tcc filter */
      int m_TESTERcountTotalTCsFinal; /**< counts number of TCs which survived until the end of event */
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
      std::vector<EventInfoPackage> m_TESTERlogEvents; /**< a list containing some information about each event (time consumption and quality) */
      int m_TESTERbadSectorRangeCounterForClusters; /**< counts number of times when only 1 cluster has been found on a complete sensor (therefore no 2D hits possible) */
      int m_TESTERclustersPersSectorNotMatching; /**< counts number of times when numofUclusters and numOfVclusters per sensor do not match */
      int m_TESTERhighOccupancyCtr; /**< counts number of times when high occupancy mode was activated */


      /// frequently used constants:
      double m_CONSTaThird; /**< simply 1/3, will be calculated once since it's needed very often */

    private:

    };
  } // Tracking namespace
} // Belle2 namespace

#endif /* VXDTFModule_H_ */
