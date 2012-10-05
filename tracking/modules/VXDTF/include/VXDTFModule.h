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
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>
#include <vxd/dataobjects/VxdID.h>
#include <tracking/dataobjects/VXDTFHit.h>
#include <tracking/dataobjects/VXDSegmentCell.h>
#include <tracking/dataobjects/VXDSector.h>
#include <tracking/dataobjects/VXDTFTrackCandidate.h>

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

//boost:
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/casts.hpp>

//genfit:
#include <GFTrackCand.h>


namespace Belle2 {

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

    typedef std::map<std::string, VXDSector*> MapOfSectors;
    typedef std::pair<std::string, VXDSector* > secMapEntry;
    typedef std::map<std::string, Cutoff*> CutoffMap;
    typedef std::map<std::string, CutoffMap*> MapOfCutoffTypes;
    typedef std::vector<VXDTFHit*> HitsOfEvent;
    typedef std::list<VXDSegmentCell*> ActiveSegmentsOfEvent; /**< is list since random deleting processes are needed */
    typedef std::vector<VXDSegmentCell*> TotalSegmentsOfEvent; /**< is vector since no entries are deleted and random access is needed  */
    typedef std::list<std::pair<std::string, MapOfSectors::iterator> > OperationSequenceOfActivatedSectors; /**< why string? we are saving the name of the sector to be able to sort them!  */
    typedef std::vector<VXDTFTrackCandidate*> TCsOfEvent;
    typedef std::vector<CurrentPassData*> PassSetupVector;
    typedef std::pair<bool, double> Filter; /**< defines whether current filter is allowed (.first) and whether the cutoffvalues shall be tuned (.second).  */


    /** structs for internal use **/
    struct SensorStruct { /**< needed for SVDCluster sorting  */
      std::vector<std::pair<int, Belle2::SVDCluster*> > uClusters; /**< .first is arrayIndex in StoreArray, .second is pointer to the Cluster itself */
      std::vector<std::pair<int, Belle2::SVDCluster*> > vClusters; /**< same as uClusters, but for vClusters  */
    };


    struct ClusterHit { /**< needed for SVDCluster sorting */
      SVDCluster* uCluster; /**< pointer to uCluster of current Hit */
      SVDCluster* vCluster; /**< pointer to vCluster of current Hit */
      int uClusterIndex; /**< index number of uCluster of current Hit */
      int vClusterIndex; /**< index number of uCluster of current Hit */
    };

    struct ClusterUsage { /**< stores information to each cluster which TC is using it */
      std::vector<TCsOfEvent > PXDClusters; /**< carries index numbers of all pxd clusters and stores pointers to all TCs using them */
      std::vector<TCsOfEvent> SVDClusters; /**< same as above but for svd clusters (information, whether its u or v is not important) */
    };


    struct CurrentPassData { /**< all the information of a pass is stored within a copy of that struct. This allows grouping of that information. */
      /** ** filled once ** **/
      MapOfSectors sectorMap; /**< carries the whole lookup-table including the information of which sectors can be combined */

      std::string sectorSetup; /**< name of setup, needed e.g. for XML-readout */
      std::string chosenDetectorType; /**< same as detectorType, but fully written ('SVD','PXD' or 'VXD'), needed during xml and for logging messages */
      int detectorType; /**< PXD = 0 , SVD = 1, VXD = -1 */
      double setupWeigh; /**< defines importance of current Pass. most important Passes stay at value 0.0, less important ones can be set between 0.0 and 100.0 (percent of QI-decrease). This affects the outcome of the neuronal network */
      int highestAllowedLayer; /**< needed for e.g. hitfinding. This value excludes Layernumbers higher than set value. (interesting for low momentum tracks)  */
      int numTotalLayers;  /**< needed e.g. for neuronal network. This value allows calculation of maximum track length (noncurling), carries implicit information about detectorType, while 'highestAllowedLayer' does not know whether its a SVD or VXD setup */
      int minLayer; /**< lowest layer considered for TCC */
      int minState; /**< lowest state considered for seeds during TCC */

      /** soon to come (maybe even layer-specific): **/
      std::vector<float> secConfigU;  /**< defines subdivition of sensors U */
      std::vector<float> secConfigV; /**< defines subdivition of sensors V */

      /** for segFinder, compares 2 hits**/
      Filter distance3D; /**< carries information about the filter 'distance3D', type pair<bool isActivated, double tuningParameter> */
      Filter distanceXY; /**< carries information about the filter 'distanceXY', type pair<bool isActivated, double tuningParameter> */
      Filter distanceZ; /**< carries information about the filter 'distanceZ', type pair<bool isActivated, double tuningParameter> */
      Filter normedDistance3D; /**< carries information about the filter 'normedDistance3D', type pair<bool isActivated, double tuningParameter> */
      int activatedSegFinderTests; /**< counts number of tests activated for segFinder */

      /** for nbFinder, compares 3 hits **/
      Filter angles3D; /**< carries information about the filter 'angles3D', type pair<bool isActivated, double tuningParameter> */
      Filter anglesXY; /**< carries information about the filter 'anglesXY', type pair<bool isActivated, double tuningParameter> */
      Filter anglesRZ; /**< carries information about the filter 'anglesRZ', type pair<bool isActivated, double tuningParameter> */
      Filter distanceDeltaZ; /**< carries information about the filter 'distanceDeltaZ', type pair<bool isActivated, double tuningParameter> */
      Filter pT; /**< carries information about the filter 'pT', same type as others */
      Filter distance2IP; /**< carries information about the filter 'distance2IP', type pair<bool isActivated, double tuningParameter>  in XY-plane, determines center of projection circle of the track and measures the residual between distance IP <-> circleCenter minus circleRadius */
      int activatedNbFinderTests; /**< counts number of tests activated for nbFinder */

      /** for TCC filter, compares 4 hits **/
      Filter zigzag; /**< carries information about the filter 'zigZag', type pair<bool isActivated, double tuningParameter> , here the tuningparameter is currently ignored, possible future use is reserved for defining uncertainties because of measurement errors */
      Filter deltaPt; /**< carries information about the filter 'deltaPt', type pair<bool isActivated, double tuningParameter> */
      Filter deltaDistance2IP; /**< carries information about the filter 'deltaDistance2IP', type pair<bool isActivated, double tuningParameter> */
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
                         TVector3 primaryVertex);


    /** Recursive CoreFunction of the Track Candidate Collector, stores every possible way starting at a Seed (VXDSegmentCell) */
    void findTCs(TCsOfEvent& tcList,
                 VXDTFTrackCandidate* currentTC,
                 std::string maxLayer);


    /** Neuronal network filtering overlapping Track Candidates by searching best subset of TC's */
    void hopfield(TCsOfEvent& tcList,
                  double omega);


    /** calculates integer score for current filter (all filters combined deliver the QQQ (normed to 0-1)), works for filterTypes having both: min- and max-value */
    int calcQQQscore(std::vector<std::pair<std::string, double> > quantiles,
                     double currentValue,
                     int numOfQuantiles);


    /** calculates integer score for current filter (all filters combined deliver the QQQ (normed to 0-1)), works for filterTypes having only  max-value */
    int calcQQQscoreDeltas(std::vector<std::pair<std::string, double> > quantiles,
                           double currentValue,
                           int numOfQuantiles);


    /** searches for sectors fitting current hit coordinates, returns blank string if nothing could be found */
    std::pair<std::string, MapOfSectors::iterator> searchSector4Hit(VxdID aVxdID,
        TVector3 localHit,
        TVector3 sensorSize,
        VXDTFModule::MapOfSectors& m_sectorMap,
        std::vector<float>& uConfig,
        std::vector<float>& vConfig);


    /** needed for sorting sectorSequence and compares strings... */
    static bool compareSecSequence(std::pair<std::string, MapOfSectors::iterator>& lhs, std::pair<std::string, MapOfSectors::iterator>& rhs);


    /** searches for segments in given pass and returns number of discarded segments */
    int segFinder(CurrentPassData* currentPass);


    /** filters neighbouring segments in given pass and returns number of discarded segments */
    int neighbourFinder(CurrentPassData* currentPass);


    /** the cellular automaton used in the TF.
     * it evolves a combination of two hits called Cells depending of their neighbourhood (=inner Cells attached to current one). In the end, Cells with a useful chain of inner segments carry the highest states.
     * for more information about cellular automata, google "cellular automaton" or "game of life" */
    void cellularAutomaton(CurrentPassData* currentPass);


    /** uses attribute "state" to find rows of compatible neighbours. State indicates length of row. This allows fast Track candidate collection */
    void tcCollector(CurrentPassData* currentPass);


    /** filters TCs via deltaPt, ZigZag and deltaDistance2IP */
    void tcFilter(CurrentPassData* currentPass,
                  int passNumber);


    /** name is program, needed for GFTrackCand export */
    void calcInitialValues4TCs(TCsOfEvent& tcVector);


    /** simplest way to determine QI of track candidates, calculating them by track length */
    void calcQIbyLength(TCsOfEvent& tcVector,
                        PassSetupVector& passSetups);

    /** produce GFTrackCand for current TC */
    GFTrackCand generateGFTrackCand(VXDTFTrackCandidate* currentTC);

    /** calculate real kalman-QI's for each currently living TC */
    std::vector<GFTrackCand> calcQIbyKalman(TCsOfEvent& tcVector,
                                            StoreArray<PXDCluster>& pxdClusters,
                                            StoreArray<SVDCluster>& svdClusters);


    /** because of geometrical reasons and the multipass-support, it is a rather common situation that the same track will be recovered twice or more.
     * In such a case, the whole number or at least a subset of hits are shared by several tracks.
     * This leads to two different problems for the hopfield network.
     * First: it increases the number of overlapping TCs which drastically reduces the speed of the TF.
     * Second: in some cases, the only TCs which are overlapping are such a track which has been found multiple times.
     * In such a case, the hopfield network encounters the problem that all the TCs (it can see only the overlapping ones) are incompatible with each other.
     * This leads to a constant surpression of all overlapping TCs.
     * The result is that all the neuron values stay below the threshold until the neuronal network stops and all overlapping TCs get deactivated.
     * Since this is a problem the hopfield network was not developed for, we have to filter these cases. Another issue is the situation, where the majority of hits are shared, but there are some foreign hits included too.
     * They have to be treated differently and therefore can not be filtered the way currently implemented. */
    void cleanOverlappingSet(TCsOfEvent& tcVector);


  protected:
    TCsOfEvent m_tcVector; /**< carries links to all track candidates found within event (during tcc filter, bad ones get kicked, lateron they simply get deactivated) */
    TCsOfEvent m_tcVectorOverlapped; /**< links only to track candidates which share at least one cluster with others*/

    /// module_parameters:
    std::vector<std::string> m_PARAMsectorSetup; /**< lets you chose the sectorSetup (compatibility of sensors, individual cutoffs,...) accepts 'std', 'low', 'high' and 'personal', please note that the chosen setup has to exist as a xml-file in ../tracking/data/friendList_XXX.xml. If you can not create your own xml files using e.g. the filterCalculatorModule, use params for  'tuneCutoffXXX' or 'setupWeigh' instead. multipass supported by setting setups in a row */

    std::vector<int> m_PARAMdetectorType; /**< defines which detector type has to be exported. VXD: -1, PXD: 0, SVD: 1 */
    std::vector<int> m_PARAMhighestAllowedLayer; /**< set value below 6 if you want to exclude outer layers (standard is 6) */
    std::vector<int> m_PARAMminLayer; /**< determines lowest layer considered by track candidate collector */
    std::vector<int> m_PARAMminState; /**< determines lowest state of cells considered by track candidate collector */

    std::vector<bool> m_PARAMactivateDistance3D; /**< activates/deactivates current filter d3D for each pass individually */
    std::vector<bool> m_PARAMactivateDistanceXY; /**< activates/deactivates current filter dXY for each pass individually */
    std::vector<bool> m_PARAMactivateDistanceZ; /**< activates/deactivates current filter dZ for each pass individually */
    std::vector<bool> m_PARAMactivateNormedDistance3D; /**< activates/deactivates current nd3D filter for each pass individually */
    std::vector<bool> m_PARAMactivateAngles3D; /**< activates/deactivates current filter a3D for each pass individually */
    std::vector<bool> m_PARAMactivateAnglesXY; /**< activates/deactivates current filter aXY for each pass individually */
    std::vector<bool> m_PARAMactivateAnglesRZ; /**< activates/deactivates current filter aRZ for each pass individually */
    std::vector<bool> m_PARAMactivateDistanceDeltaZ; /**< activates/deactivates current filter ddZ for each pass individually */
    std::vector<bool> m_PARAMactivateDistance2IP; /**< activates/deactivates current filter d2IP for each pass individually */
    std::vector<bool> m_PARAMactivatePT; /**< activates/deactivates current filter PT for each pass individually */
    std::vector<bool> m_PARAMactivateZigZag; /**< activates/deactivates current filter zz for each pass individually */
    std::vector<bool> m_PARAMactivateDeltaPt; /**< activates/deactivates current filter dPt for each pass individually */
    std::vector<bool> m_PARAMactivateDeltaDistance2IP; /**< activates/deactivates current filter dd2IP for each pass individually*/

    std::vector<double> m_PARAMtuneDistance3D; /**< tunes current filter d3D for each pass individually */
    std::vector<double> m_PARAMtuneDistanceXY; /**< tunes current filter dXY for each pass individually */
    std::vector<double> m_PARAMtuneDistanceZ; /**< tunes current filter dZ for each pass individually */
    std::vector<double> m_PARAMtuneNormedDistance3D; /**< tunes current nd3D filter for each pass individually */
    std::vector<double> m_PARAMtuneAngles3D; /**< tunes current filter a3D for each pass individually */
    std::vector<double> m_PARAMtuneAnglesXY; /**< tunes current filter aXY for each pass individually */
    std::vector<double> m_PARAMtuneAnglesRZ; /**< tunes current filter aRZ for each pass individually */
    std::vector<double> m_PARAMtuneDistanceDeltaZ; /**< tunes current filter ddZ for each pass individually */
    std::vector<double> m_PARAMtuneDistance2IP; /**< tunes current filter d2IP for each pass individually */
    std::vector<double> m_PARAMtunePT; /**< tunes current filter PT for each pass individually */
    std::vector<double> m_PARAMtuneZigZag; /**< tunes current filter zz for each pass individually */
    std::vector<double> m_PARAMtuneDeltaPt; /**< tunes current filter dPt for each pass individually */
    std::vector<double> m_PARAMtuneDeltaDistance2IP; /**< tunes current filter dd2IP for each pass individually */

    std::vector<double> m_PARAMsetupWeigh; /**< allows to individually weigh (level of surpression) each setup/pass. Please choose between 0 and -100 (%), higher momentum setups are more reliable, therefore should be weighed best */

    /// needed for pass handling:
    PassSetupVector m_passSetupVector; /**< contains information for each pass */

    std::vector<float> m_PARAMsectorConfigU; /**< allows defining the the config of the sectors in U direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0 */
    std::vector<float> m_PARAMsectorConfigV; /**< allows defining the the config of the sectors in V direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0 */
    int m_PARAMpdGCode; /**< tandard value is 211 (pi+), ATTENTION, instead of using inconsistent sign of PdGList, in this module positively charged particles are always positive and negatively charged ones are negative (relevant for leptons) */

    int m_chargeSignFactor; /**< particle dependent. for leptons it is 1, for other particles it's -1... */

    int m_usePXDorSVDorVXDhits; /**< when having more than one pass per event, sector maps using PXD, SVD or VXD can be set independently. To produce TFHits only when needed, this value is set to -1,0 or 1 */

    std::vector<std::string> m_PARAMmultiPassSectorSetup; /**< */// controls usage of one or several passes for TF per event (allows differnt pT's or curling track support)
    double m_PARAMtuneCutoffs; /**< for rapid changes of cutoffs (no personal xml files needed), reduces/enlarges the range of the cutoffs in percent (lower and upper values are changed by this value). Only valid in range -50% < x < +1000% */

    int m_eventCounter; /**< knows current event number */
    int m_badSectorRangeCounter; /**< counts number of hits which couldn't be attached to an existing sector of a pass */
    int m_badFriendCounter; /**< counts number of hits having no neighbour hits in friend sectors of current sector */

    double m_PARAMomega; /**< tuning parameter for hopfield network */
    double m_tcThreshold;   /**< defines threshold for hopfield network. neurons having values below threshold are discarded */

    bool m_PARAMqiSmear; /**<  allows to smear QIs via qqq-Interface, needed when having more than one TC with the same QI */
    bool m_PARAMcleanOverlappingSet; /**< when true, TCs which are found more than once (possible because of multipass) will get filtered */
    bool m_PARAMuseHopfield; /**< allows to deactivate hopfield, so overlapping TCs are exported */
    double m_PARAMsmearMean; /**< allows to introduce a bias for QI (e.g. surpressing all values, ...)*/
    double m_PARAMsmearSigma; /**< bigger values deliver broader distribution*/
    bool m_PARAMstoreBrokenQI;/**< if true, TC survives QI-calculation-process even if fit was not possible */

    std::string m_PARAMcalcQIType; /**< allows you to chose the way, the QI's of the TC's shall be calculated. currently supported: 'kalman','trackLength' */

    std::string m_nameAngles3D; /**< string name of filter a3D */
    std::string m_nameAnglesRZ; /**< string name of filter aRZ */
    std::string m_nameAnglesXY; /**< string name of filter aXY */
    std::string m_nameDistance3D; /**< string name of filter d3D */
    std::string m_nameDistanceXY; /**< string name of filter dXY */
    std::string m_nameDistanceZ; /**< string name of filter dZ */
    std::string m_nameDistanceDeltaZ; /**< string name of filter ddZ */
    std::string m_namePT; /**< string name of filter pT */
    std::string m_nameDeltapT; /**< string name of filter dPt */
    std::string m_nameNormedDistance3D; /**< string name of filter nd3D */
    std::string m_nameTRadius2IPDistance; /**< string name of filter d2IP */
    std::string m_nameDeltaTRadius2IPDistance; /**< string name of filter dd2IP */


    /// the following variables are nimimal testing routines within the TF
    int m_TESTERtriggeredZigZag;/**< counts how many times zigZag filter found bad TCs */
    int m_TESTERtriggeredDpT; /**< counts how many times deltaPt filter found bad TCs  */
    int m_TESTERapprovedByTCC; /**< counts how many TCs got approved by TCC */
    int m_TESTERcountTotalTCsAfterTCC; /**< counts number of TCs found by TCC */
    int m_TESTERcountTotalTCsAfterTCCFilter; /**< counts number of TCs which survived the tcc filter */
    int m_TESTERcountTotalTCsFinal; /**< counts number of TCs which survived until the end of event */
    int m_TESTERbadHopfieldCtr; /**< counts number of events, where no TC survived the Hopfield network */
    int m_TESTERfilteredOverlapsQI; /**< counts number of TCs filtered by total overlap rule (cleanOverlappingSet) */
    int m_TESTERNotFilteredOverlapsQI; /**< counts number of events, when cleanOverlappingSet was started but didn't filter TCs */
    int m_TESTERfilteredOverlapsQICtr; /**< counts number of times, when TCs get filtered by cleanOverlappingSet */
    int m_TESTERcleanOverlappingSetStartedCtr; /**< counts number of times, when cleanOverlappingSet get started */
    int m_TESTERgoodFitsCtr; /**< counts number of times, the kalman fit worked well */
    int m_TESTERbadFitsCtr; /**< counts number of times, the kalman fit didn't work */

  private:

  };
} // Belle2 namespace

#endif /* VXDTFModule_H_ */
