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

//framework-stuff
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>
#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <tracking/dataobjects/VXDTFInfoBoard.h>

#include <tracking/trackFindingVXD/displayInterfaceTF/AnalizerCollectorTFInfo.h>

//genfit-stuff
#include <genfit/TrackCand.h>

#include <tracking/trackFindingVXD/displayInterfaceTF/TrackCandidateTFInfo.h>


//stl-stuff
#include <string>
#include <algorithm>

//root-stuff
#include <TVector3.h>
#include <TTree.h>
#include <TFile.h>

//boost stuff
// // #include "boost/tuple/tuple.hpp" // a generalized version of pair



namespace Belle2 {
  namespace Tracking {


    typedef boost::tuple < std::vector<int>, /// indexNumbers of PXDClusters of TC
            std::vector<int>, /// ... of SVDClusters of TC
            std::vector<TVector3> , /// coordinates of hits of TC
            std::vector<const SVDTrueHit*> > /// pointer to SVDTrueHits of TC
            TcInfoTuple; /**< stores info to TC (more info in .h-file) */
    typedef boost::tuple < int, /// indexnumber of partner TC(for caTCs: mcTCs, for mcTCs: caTCs)
            int, /// number of identical hits between partners
            int,   /// number of hits only in this TC
            int, /// number of hits only in the partner TC
            double > /// qualityValue (calculated by other values)
            CompatibilityIndex; /**< stores info about compatipility between two TCs */

    /** The TFAnalizerModule
    *
    * this module analyzes and compares the output of the MCTrackFinder with the VXDTF.
    * It calculates the efficiency of the VXDTF based on the output of the MCTF.
    * run example file tracking/examples/VXDTFModuleDemo.py in debug mode (1 or more) for the TFAnalizer for more details
    *
    */
    class TFAnalizerModule : public Module {

    public:

      typedef std::pair<int, double> foundIDentry; /**< .first: id of mcTC assigned, .second: qi of tc, the higher, the better */

      // Member Variables for Collector/Display
      int m_display;  /**< Collector operating flag: 0 = no collector, 1 = collect for analysis, 2 = collect for display */
      AnalizerCollectorTFInfo ana_collector; /**< Object collectes/read all the Information needed for the collector*/


      std::string m_collectorFilePath; /**< File Path for the Collector */
      double m_collectorThreshold;  /**< Threshold for contaminated TC */


      /** internal datastore for root export */
      struct RootVariables  {
        std::vector<double>
        totalPXresiduals; /**< used to store all residuals (true - estimated) of the momentum in x-direction which were reconstructed by the VXDTF*/
        std::vector<double>
        totalPYresiduals; /**< used to store all residuals (true - estimated) of the momentum in y-direction which were reconstructed by the VXDTF */
        std::vector<double>
        totalPZresiduals; /**< used to store all residuals (true - estimated) of the momentum in z-direction which were reconstructed by the VXDTF */

        std::vector<double>
        cleanPXresiduals; /**< used to store all residuals (true - estimated) of the momentum in x-direction which were reconstructed by the VXDTF (clean) */
        std::vector<double>
        cleanPYresiduals;/**< used to store all residuals (true - estimated) of the momentum in y-direction which were reconstructed by the VXDTF (clean) */
        std::vector<double>
        cleanPZresiduals; /**< used to store all residuals (true - estimated) of the momentum in z-direction which were reconstructed by the VXDTF (clean) */

        std::vector<double>
        completePXresiduals; /**< used to store all residuals (true - estimated) of the momentum in x-direction which were reconstructed by the VXDTF  (clean and no hits missing) */
        std::vector<double>
        completePYresiduals; /**< used to store all residuals (true - estimated) of the momentum in y-direction which were reconstructed by the VXDTF (clean and no hits missing) */
        std::vector<double>
        completePZresiduals; /**< used to store all residuals (true - estimated) of the momentum in z-direction which were reconstructed by the VXDTF (clean and no hits missing) */

        std::vector<double> totalMCMomValues; /**< used to store all momentum values of tracks reconstructed by the MCTF */
        std::vector<double> totalCAMomValues; /**< used to store all momentum values of tracks reconstructed by the CATF */
        std::vector<double> cleanCAMomValues; /**< used to store all momentum values of clean tracks reconstructed by the CATF */
        std::vector<double> completeCAMomValues; /**< used to store all momentum values of full tracks reconstructed by the CATF */
        std::vector<double> totalMomValues; /**< used to store all momentum values of tracks existing no matter they produced hits or not */

        std::vector<double>
        totalCAMomResiduals; /**< used to store all momentum residuals (true - estimated) of tracks reconstructed by the CATF */
        std::vector<double>
        cleanCAMomResiduals; /**< used to store all momentum residuals (true - estimated) of clean tracks reconstructed by the CATF */
        std::vector<double>
        completeCAMomResiduals; /**< used to store all momentum residuals (true - estimated) of full tracks reconstructed by the CATF */

        std::vector<double> totalMCpTValues; /**< used to store all pT values of tracks reconstructed by the MCTF */
        std::vector<double> totalCApTValues; /**< used to store all pT values of tracks reconstructed by the CATF */
        std::vector<double> cleanCApTValues; /**< used to store all pT values of clean tracks reconstructed by the CATF */
        std::vector<double> completeCApTValues; /**< used to store all pT values of full tracks reconstructed by the CATF */
        std::vector<double> totalpTValues; /**< used to store all pT values of tracks existing no matter they produced hits or not */

        std::vector<double>
        totalCApTResiduals; /**< used to store all pT residuals (true - estimated) of tracks reconstructed by the CATF */
        std::vector<double>
        cleanCApTResiduals; /**< used to store all pT residuals (true - estimated) of clean tracks reconstructed by the CATF */
        std::vector<double>
        completeCApTResiduals; /**< used to store all pT residuals (true - estimated) of full tracks reconstructed by the CATF */

        std::vector<double> totalMCThetaValues; /**< used to store all theta values of tracks reconstructed by the MCTF */
        std::vector<double> totalCAThetaValues; /**< used to store all theta values of tracks reconstructed by the CATF */
        std::vector<double> cleanCAThetaValues; /**< used to store all theta values of clean tracks reconstructed by the CATF */
        std::vector<double> completeCAThetaValues; /**< used to store all theta values of full tracks reconstructed by the CATF */
        std::vector<double> totalThetaValues; /**< used to store all theta values of tracks existing no matter they produced hits or not */

        std::vector<double>
        totalCAThetaResiduals; /**< used to store all theta residuals (true - estimated) of tracks reconstructed by the CATF */
        std::vector<double>
        cleanCAThetaResiduals; /**< used to store all theta residuals (true - estimated) of clean tracks reconstructed by the CATF */
        std::vector<double>
        completeCAThetaResiduals; /**< used to store all theta residuals (true - estimated) of full tracks reconstructed by the CATF */

        std::vector<double> totalMCPhiValues; /**< used to store all Phi values of tracks reconstructed by the MCTF */
        std::vector<double> totalCAPhiValues; /**< used to store all Phi values of tracks reconstructed by the CATF */
        std::vector<double> cleanCAPhiValues; /**< used to store all Phi values of clean tracks reconstructed by the CATF */
        std::vector<double> completeCAPhiValues; /**< used to store all Phi values of full tracks reconstructed by the CATF */
        std::vector<double> totalPhiValues; /**< used to store all Phi values of tracks existing no matter they produced hits or not */

        std::vector<double>
        totalCAPhiResiduals; /**< used to store all Phi residuals (true - estimated) of tracks reconstructed by the CATF */
        std::vector<double>
        cleanCAPhiResiduals; /**< used to store all Phi residuals (true - estimated) of clean tracks reconstructed by the CATF */
        std::vector<double>
        completeCAPhiResiduals; /**< used to store all Phi residuals (true - estimated) of full tracks reconstructed by the CATF */

        std::vector<double>
        totalMCVertex2IP3DValues; /**< used to store all Vertex2IP3D (distance of vertex to origin) values of tracks reconstructed by the MCTF */
        std::vector<double>
        totalCAVertex2IP3DValues; /**< used to store all Vertex2IP3D (distance of vertex to origin) values of tracks reconstructed by the CATF */
        std::vector<double>
        cleanCAVertex2IP3DValues; /**< used to store all Vertex2IP3D (distance of vertex to origin) values of clean tracks reconstructed by the CATF */
        std::vector<double>
        completeCAVertex2IP3DValues; /**< used to store all Vertex2IP3D (distance of vertex to origin) values of full tracks reconstructed by the CATF */
        std::vector<double>
        totalVertex2IP3DValues; /**< used to store all Vertex2IP3D (distance of vertex to origin) values of tracks existing no matter they produced hits or not */

        std::vector<double>
        totalMCVertex2IPXYValues; /**< used to store all Vertex2IPXY (distance of vertex to origin) values of tracks reconstructed by the MCTF */
        std::vector<double>
        totalCAVertex2IPXYValues; /**< used to store all Vertex2IPXY (distance of vertex to origin) values of tracks reconstructed by the CATF */
        std::vector<double>
        cleanCAVertex2IPXYValues; /**< used to store all Vertex2IPXY (distance of vertex to origin) values of clean tracks reconstructed by the CATF */
        std::vector<double>
        completeCAVertex2IPXYValues; /**< used to store all Vertex2IPXY (distance of vertex to origin) values of full tracks reconstructed by the CATF */
        std::vector<double>
        totalVertex2IPXYValues; /**< used to store all Vertex2IPXY (distance of vertex to origin) values of tracks existing no matter they produced hits or not */

        std::vector<double>
        totalMCVertex2IPZValues; /**< used to store all Vertex2IPZ (distance of vertex to origin) values of tracks reconstructed by the MCTF */
        std::vector<double>
        totalCAVertex2IPZValues; /**< used to store all Vertex2IPZ (distance of vertex to origin) values of tracks reconstructed by the CATF */
        std::vector<double>
        cleanCAVertex2IPZValues; /**< used to store all Vertex2IPZ (distance of vertex to origin) values of clean tracks reconstructed by the CATF */
        std::vector<double>
        completeCAVertex2IPZValues; /**< used to store all Vertex2IPZ (distance of vertex to origin) values of full tracks reconstructed by the CATF */
        std::vector<double>
        totalVertex2IPZValues; /**< used to store all Vertex2IPZ (distance of vertex to origin) values of tracks existing no matter they produced hits or not */

        std::vector<double>
        totalCAMomResidualAngles; /**< used to store all residuals (true - estimated) of the total momentum vectors difference in direction of tracks reconstructed by the CATF */
        std::vector<double>
        cleanCAMomResidualAngles; /**< used to store all residuals (true - estimated) of the total momentum vectors difference in direction of clean tracks reconstructed by the CATF */
        std::vector<double>
        completeCAMomResidualAngles; /**< used to store all residuals (true - estimated) of the total momentum vectors difference in direction of full tracks reconstructed by the CATF */

        std::vector<double>
        totalCApTResidualAngles; /**< used to store all residuals (true - estimated) of the pT vectors difference in direction of tracks reconstructed by the CATF */
        std::vector<double>
        cleanCApTResidualAngles; /**< used to store all residuals (true - estimated) of the pT vectors difference in direction of clean tracks reconstructed by the CATF */
        std::vector<double>
        completeCApTResidualAngles; /**< used to store all residuals (true - estimated) of the pT vectors difference in direction of full tracks reconstructed by the CATF */

        std::vector<double>
        totalCASeedPositionResiduals; /**< used to store all residuals (true - estimated) of position of the seed hit of tracks reconstructed by the CATF*/
        std::vector<double>
        cleanCASeedPositionResiduals; /**< used to store all residuals (true - estimated) of position of the seed hit of clean tracks reconstructed by the CATF */
        std::vector<double>
        completeCASeedPositionResiduals; /**< used to store all residuals (true - estimated) of position of the seed hit of full tracks reconstructed by the CATF */

        std::vector<int> mCreconstructedTrackLength; /**< used to store all track length values of tracks reconstructed by the MCTF */
        std::vector<int> cAreconstructedTrackLength; /**< used to store all track length values of tracks reconstructed by the CATF */

        std::vector<int> lostUClusters; /**< used to store the number of uClusters of lost MCTCs not found by the TF */
        std::vector<int> lostVClusters; /**< used to store the number of vClusters of lost MCTCs not found by the TF */
        std::vector<int> totalMcUClusters; /**< used to store the number of uClusters of all MCTCs */
        std::vector<int> totalMcVClusters; /**< used to store the number of vClusters of all MCTCs */

        std::vector<double> lostUClusterEDep; /**< used to store the energy deposition of uClusters of lost MCTCs not found by the TF */
        std::vector<double> lostVClusterEDep; /**< used to store the energy deposition of vClusters of lost MCTCs not found by the TF */
        std::vector<double> totalMcUClusterEDep; /**< used to store the energy deposition of uClusters of all MCTCs */
        std::vector<double> totalMcVClusterEDep; /**< used to store the energy deposition of vClusters of all MCTCs */
      };

      /** internal datastore for TCs. Used by caTCs and mcTCs */
      struct VXDTrackCandidate {

        std::vector<int> pxdClusterIDs; /**< stores IDs of PXD-Clusters attached to current TC */
        std::vector<int> svdClusterIDs; /**< stores IDs of SVD-Clusters attached to current TC */
        std::vector<TVector3> coordinates; /**< TVector3-coordinates for each hit */
        std::vector<const PXDTrueHit*> pxdTrueHits; /**< pointer to PXD-trueHits which could be assigned to pxd-clusters */
        std::vector<const SVDTrueHit*> svdTrueHits; /**< pointer to SVD-trueHits which could be assigned to svd-clusters */
        std::vector<CompatibilityIndex> compatiblePartners; /**< knows each partner which shares hits with current TC */

        bool isMCtrackCandidate; /**< is it MC TF TC?  */
        int indexNumber; /**< index number in StoreArray  */
        int finalAssignedID; /**< for mcTCs, their own ID in the tcVector for mcTCs. for CA, the ID of the mcTC which got the highest number    of compatible hits  */
        double qualityIndex; /**< the higher, the better. numGoodHits / numTotalHitsOfmcTC  */
        int numOfCorrectlyAssignedHits; /**< numGoodHits  */
        int numOfBadAssignedHits; /**< Hits which are in the caTC but not in the assigned mcTC (contaminated)  */
        bool successfullyFound; /**< is true if reconstruction was successful */
        int pdgCode; /**<  for caTCs, guess of pdg-code, for mcTCs, actual pdg-code */
        TVector3 direction; /**< initial momentum vector (interesting for comparison)  */
        TVector3 seedHit; /**< position of innermost hit (interesting for comparison) */
        TVector3 vertex; /**< initial vertex position (only valid for mcTCs)  */
        bool survivedFit; /**< is true if GFTC survived fit within VXDTF */

        double probValue; /**< probability value of kalmanfit */
        double pTValue; /**< carries the real/guessed magnitude of the transverse part of the Momentum vector */
        double pValue; /**< magnitude of total momentum */

      };

      TFAnalizerModule();


      ~TFAnalizerModule();


      void initialize() override;


      void beginRun() override;


      void event() override;


      void endRun() override;


      void terminate() override;


      /** compares entries of a list which stores pair of values (which one is bigger?) */
      static bool isFirstValueBigger(foundIDentry& lhs, foundIDentry& rhs);


      /** compares entries of a list which stores pair of values (are both the same?)*/
      static bool isFirstValueTheSame(foundIDentry& lhs, foundIDentry& rhs);


      /** extracts hits from genfit::TrackCand. Working with both TC-types (mc (monte carlo) or ca (cellular automaton)) */
      void extractHits(genfit::TrackCand* aTC,
                       RelationIndex<PXDCluster, PXDTrueHit>& relationPXD,
                       RelationIndex<SVDCluster, SVDTrueHit>& relationSVD,
                       StoreArray<PXDCluster>& pxdClusters,
                       StoreArray<SVDCluster>& svdClusters,
                       StoreArray<VXDTFInfoBoard>& infoBoards,
                       StoreArray<TrackCandidateTFInfo>& infosTCs,
                       std::vector<VXDTrackCandidate>& tcVector,
                       bool isMCTC,
                       int index);


      /** checks to TCs (a caTC and a mcTC) whether they are compatible (which means that majority of hits of ca are part of mcTC too) */
      void checkCompatibility(VXDTrackCandidate& mcTC, VXDTrackCandidate& caTC);


      /** prints info to console of compatible mcTC and caTC */
      void printInfo(int recoveryState, VXDTrackCandidate& mcTC, VXDTrackCandidate& caTC,
                     RootVariables& rootVariables);  // used for exporting information


      /** used for exporting info to console about mcTC (should not be used for caTCs, use printCA instead). This function is somewhat redundant to void PrintInfo, but allows better structure of output  */
      void printMC(bool info, VXDTrackCandidate& mcTC, RootVariables& rootVariables);


      /** used for exporting info to console about caTC (should not be used for mcTCs, use printMC instead). This function is somewhat redundant to void PrintInfo, but allows better structure of output  */
      void printCA(bool type, VXDTrackCandidate& caTC);


      /** initialize variables to avoid nondeterministic behavior */
      void InitializeVariables()
      {
        m_countReconstructedTCs = 0;
        m_countAcceptedGFTCs = 0;
        m_lostGFTCs = 0;
        m_mcTrackCounter = 0;
        m_eventCounter = 0;
        m_totalRealHits = 0;
        m_caTrackCounter = 0;
        m_countedPerfectRecoveries = 0;
        m_countedCleanRecoveries = 0;
        m_wrongChargeSignCounter = 0;
        m_mcTrackVectorCounter = 0;
        m_nMcPXDHits = 0;
        m_nMcSVDHits = 0;
        m_nCaPXDHits = 0;
        m_nCaSVDHits = 0;
        m_countedDoubleEntries = 0;
        m_rootFilePtr = NULL;
        m_treePtr = NULL;
      }


    protected:
      bool m_PARAMFileExportMcTracks; /**< possibly needed later (currently not in use), exports McTracks to File */
      bool m_PARAMFileExportTfTracks; /**< possibly needed later (currently not in use), exports TfTracks to File */
      bool m_PARAMprintExtentialAnalysisData; /**< set true, if you want to cout special Info to the shell (where it can be stored into a file and grep-ed to find specific info) */
      std::string
      m_PARAMmcTCname; /**< the name of the mctrackCandidateCollection provided by the mcTrackFinder (has to be set manually there and here) */
      std::string
      m_PARAMcaTCname; /**< the name of the catrackCandidateCollection provided by the caTrackFinder (has to be set manually there and here) */
      std::string
      m_PARAMacceptedTCname; /**< the name of the trackCandidateCollection of successfully reconstructed track candidates determined by the TFAnalizer */
      std::string
      m_PARAMlostTCname; /**< the name of the trackCandidateCollection of lost track candidates determined by the TFAnalizer */
      std::string m_PARAMinfoBoardName; /**< InfoContainer collection name */
      double m_PARAMqiThreshold; /**<  chose value to filter TCs found by VXDTF. TCs having QIs lower than this value won't be marked as reconstructed (value 0-1). e.g. having a TC with 4 hits, 1 foreign, 3 good ones. would meand 0.75, a qiThreshold with 0.7 would mark the track as 'reconstructed', a threshold of 0.8 would neglect the TC */
      int m_PARAMminNumOfHitsThreshold; /**< defines how many hits of current TC has to be found again to be accepted as recovered, values lower than 3 wouldn't make sense because of minimal info needed for track parameters */
      int m_countReconstructedTCs; /**< counts number of reconstructed TCs */
      int m_countAcceptedGFTCs; /**< counts number of accepted TCs which are stored in separate container for external tests (e.g. trackFitChecker) in storaArray with name m_PARAMacceptedTCname */
      int m_lostGFTCs; /**< counts number of TCs found by MCTF but lost by VXDTF, they are stored for external tests in storaArray with name m_PARAMlostTCname */
      int m_eventCounter; /**< knows current event number */
      int m_mcTrackCounter; /**< counts number of tracks reconstructed by the mcTrackFinder */
      int m_totalRealHits; /**< total number of hits (clusters/2) attached to mcTCs (therefore total number of real hits) */
      int m_caTrackCounter; /**< counts number of tracks reconstructed by the CATF */
      int m_countedPerfectRecoveries; /**< counts number of tracks, where no foreign hits were attached ('clean') AND all hits of the mcTC were reconstructed */
      int m_countedCleanRecoveries; /**< counts number of tracks, where no foreign hits were attached ('clean'), does NOT mean that all reconstructable hits had been found by CATF! */
      int m_countedDoubleEntries; /**< if a TC was found more than once with good (contaminated or clean ones) caTCs, it will be counted to find out how many of the ghost tcs are in fact good tcs but not combined to one tc */
      int m_wrongChargeSignCounter; /**< counts number of times, where assigned caTC guessed wrong sign of charge */
      std::string
      m_PARAMprintData; /**< depending on what value you set it, it will print data like momentum residuals or any other interesting info during endrun... (currently not in use)*/
      int m_mcTrackVectorCounter; /**< another counter of mcTCs, consideres size of datastores containing mcTCs */
      int m_nMcPXDHits; /**< counts total number of pxdHits added by mcTF */
      int m_nMcSVDHits; /**< counts total number of svdHits added by mcTF */
      int m_nCaPXDHits; /**< counts total number of pxdHits added by caTF */
      int m_nCaSVDHits; /**< counts total number of svdHits added by caTF */
      double m_PARAMminTMomentumFilter; /**< to narrow down the relevant mcTracks, this minFilter can be set to filter tracks having lower transverse momentum than this threshold. Relevant for checking efficiency of TFs with certain transverse momentum ranges */
      double m_PARAMmaxTMomentumFilter; /**< to narrow down the relevant mcTracks, this maxFilter can be set to filter tracks having higher transverse momentum than this threshold. Relevant for checking efficiency of TFs with certain transverse momentum ranges */

      // rootStuff:
      bool m_PARAMwriteToRoot; /**< if true, analysis data is stored to root file with file name chosen by 'rootFileName' */
      std::vector<std::string>
      m_PARAMrootFileName; /**< only two entries accepted, first one is the root filename, second one is 'RECREATE' or 'UPDATE' which is the write mode for the root file, parameter is used only if 'writeToRoot' = true */
      TFile* m_rootFilePtr; /**< pointer at root file used for p-value-output */
      TTree* m_treePtr; /**< pointer at root tree used for p-value-output */

      std::vector<double>
      m_rootTotalPXresiduals; /**< used to store all residuals (true - estimated) of the momentum in x-direction which were reconstructed by the VXDTF*/
      std::vector<double>
      m_rootTotalPYresiduals; /**< used to store all residuals (true - estimated) of the momentum in y-direction which were reconstructed by the VXDTF */
      std::vector<double>
      m_rootTotalPZresiduals; /**< used to store all residuals (true - estimated) of the momentum in z-direction which were reconstructed by the VXDTF */

      std::vector<double>
      m_rootCleanPXresiduals; /**< used to store all residuals (true - estimated) of the momentum in x-direction which were reconstructed by the VXDTF (clean) */
      std::vector<double>
      m_rootCleanPYresiduals;/**< used to store all residuals (true - estimated) of the momentum in y-direction which were reconstructed by the VXDTF (clean) */
      std::vector<double>
      m_rootCleanPZresiduals; /**< used to store all residuals (true - estimated) of the momentum in z-direction which were reconstructed by the VXDTF (clean) */

      std::vector<double>
      m_rootCompletePXresiduals; /**< used to store all residuals (true - estimated) of the momentum in x-direction which were reconstructed by the VXDTF  (clean and no hits missing) */
      std::vector<double>
      m_rootCompletePYresiduals; /**< used to store all residuals (true - estimated) of the momentum in y-direction which were reconstructed by the VXDTF (clean and no hits missing) */
      std::vector<double>
      m_rootCompletePZresiduals; /**< used to store all residuals (true - estimated) of the momentum in z-direction which were reconstructed by the VXDTF (clean and no hits missing) */

      std::vector<double> m_rootTotalMCMomValues; /**< used to store all momentum values of tracks reconstructed by the MCTF */
      std::vector<double> m_rootTotalCAMomValues; /**< used to store all momentum values of tracks reconstructed by the CATF */
      std::vector<double> m_rootCleanCAMomValues; /**< used to store all momentum values of clean tracks reconstructed by the CATF */
      std::vector<double> m_rootCompleteCAMomValues; /**< used to store all momentum values of full tracks reconstructed by the CATF */
      std::vector<double>
      m_rootTotalMomValues; /**< used to store all momentum values of tracks existing no matter they produced hits or not */

      std::vector<double>
      m_rootTotalCAMomResiduals; /**< used to store all momentum residuals (true - estimated) of tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCleanCAMomResiduals; /**< used to store all momentum residuals (true - estimated) of clean tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCompleteCAMomResiduals; /**< used to store all momentum residuals (true - estimated) of full tracks reconstructed by the CATF */

      std::vector<double> m_rootTotalMCpTValues; /**< used to store all pT values of tracks reconstructed by the MCTF */
      std::vector<double> m_rootTotalCApTValues; /**< used to store all pT values of tracks reconstructed by the CATF */
      std::vector<double> m_rootCleanCApTValues; /**< used to store all pT values of clean tracks reconstructed by the CATF */
      std::vector<double> m_rootCompleteCApTValues; /**< used to store all pT values of full tracks reconstructed by the CATF */
      std::vector<double> m_rootTotalpTValues; /**< used to store all pT values of tracks existing no matter they produced hits or not */

      std::vector<double>
      m_rootTotalCApTResiduals; /**< used to store all pT residuals (true - estimated) of tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCleanCApTResiduals; /**< used to store all pT residuals (true - estimated) of clean tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCompleteCApTResiduals; /**< used to store all pT residuals (true - estimated) of full tracks reconstructed by the CATF */

      std::vector<double> m_rootTotalMCThetaValues; /**< used to store all theta values of tracks reconstructed by the MCTF */
      std::vector<double> m_rootTotalCAThetaValues; /**< used to store all theta values of tracks reconstructed by the CATF */
      std::vector<double> m_rootCleanCAThetaValues; /**< used to store all theta values of clean tracks reconstructed by the CATF */
      std::vector<double> m_rootCompleteCAThetaValues; /**< used to store all theta values of full tracks reconstructed by the CATF */
      std::vector<double>
      m_rootTotalThetaValues; /**< used to store all theta values of tracks existing no matter they produced hits or not */

      std::vector<double>
      m_rootTotalCAThetaResiduals; /**< used to store all theta residuals (true - estimated) of tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCleanCAThetaResiduals; /**< used to store all theta residuals (true - estimated) of clean tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCompleteCAThetaResiduals; /**< used to store all theta residuals (true - estimated) of full tracks reconstructed by the CATF */

      std::vector<double> m_rootTotalMCPhiValues; /**< used to store all Phi values of tracks reconstructed by the MCTF */
      std::vector<double> m_rootTotalCAPhiValues; /**< used to store all Phi values of tracks reconstructed by the CATF */
      std::vector<double> m_rootCleanCAPhiValues; /**< used to store all Phi values of clean tracks reconstructed by the CATF */
      std::vector<double> m_rootCompleteCAPhiValues; /**< used to store all Phi values of full tracks reconstructed by the CATF */
      std::vector<double>
      m_rootTotalPhiValues; /**< used to store all Phi values of tracks existing no matter they produced hits or not */

      std::vector<double>
      m_rootTotalCAPhiResiduals; /**< used to store all Phi residuals (true - estimated) of tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCleanCAPhiResiduals; /**< used to store all Phi residuals (true - estimated) of clean tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCompleteCAPhiResiduals; /**< used to store all Phi residuals (true - estimated) of full tracks reconstructed by the CATF */


      std::vector<double>
      m_rootTotalMCVertex2IP3DValues; /**< used to store all Vertex2IP3D (distance of vertex to origin) values of tracks reconstructed by the MCTF */
      std::vector<double>
      m_rootTotalCAVertex2IP3DValues; /**< used to store all Vertex2IP3D (distance of vertex to origin) values of tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCleanCAVertex2IP3DValues; /**< used to store all Vertex2IP3D (distance of vertex to origin) values of clean tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCompleteCAVertex2IP3DValues; /**< used to store all Vertex2IP3D (distance of vertex to origin) values of full tracks reconstructed by the CATF */
      std::vector<double>
      m_rootTotalVertex2IP3DValues; /**< used to store all Vertex2IP3D (distance of vertex to origin) values of tracks existing no matter they produced hits or not */

      std::vector<double>
      m_rootTotalMCVertex2IPXYValues; /**< used to store all Vertex2IPXY (distance of vertex to origin) values of tracks reconstructed by the MCTF */
      std::vector<double>
      m_rootTotalCAVertex2IPXYValues; /**< used to store all Vertex2IPXY (distance of vertex to origin) values of tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCleanCAVertex2IPXYValues; /**< used to store all Vertex2IPXY (distance of vertex to origin) values of clean tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCompleteCAVertex2IPXYValues; /**< used to store all Vertex2IPXY (distance of vertex to origin) values of full tracks reconstructed by the CATF */
      std::vector<double>
      m_rootTotalVertex2IPXYValues; /**< used to store all Vertex2IPXY (distance of vertex to origin) values of tracks existing no matter they produced hits or not */

      std::vector<double>
      m_rootTotalMCVertex2IPZValues; /**< used to store all Vertex2IPZ (distance of vertex to origin) values of tracks reconstructed by the MCTF */
      std::vector<double>
      m_rootTotalCAVertex2IPZValues; /**< used to store all Vertex2IPZ (distance of vertex to origin) values of tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCleanCAVertex2IPZValues; /**< used to store all Vertex2IPZ (distance of vertex to origin) values of clean tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCompleteCAVertex2IPZValues; /**< used to store all Vertex2IPZ (distance of vertex to origin) values of full tracks reconstructed by the CATF */
      std::vector<double>
      m_rootTotalVertex2IPZValues; /**< used to store all Vertex2IPZ (distance of vertex to origin) values of tracks existing no matter they produced hits or not */


      std::vector<double>
      m_rootTotalCAMomResidualsAngles; /**< used to store all residuals (true - estimated) of the total momentum vectors difference in direction of tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCleanCAMomResidualsAngles; /**< used to store all residuals (true - estimated) of the total momentum vectors difference in direction of clean tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCompleteCAMomResidualsAngles; /**< used to store all residuals (true - estimated) of the total momentum vectors difference in direction of full tracks reconstructed by the CATF */

      std::vector<double>
      m_rootTotalCApTResidualsAngles; /**< used to store all residuals (true - estimated) of the pT vectors difference in direction of tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCleanCApTResidualsAngles; /**< used to store all residuals (true - estimated) of the pT vectors difference in direction of clean tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCompleteCApTResidualsAngles; /**< used to store all residuals (true - estimated) of the pT vectors difference in direction of full tracks reconstructed by the CATF */

      std::vector<double>
      m_rootTotalCASeedPositionResiduals; /**< used to store all residuals (true - estimated) of position of the seed hit of tracks reconstructed by the CATF*/
      std::vector<double>
      m_rootCleanCASeedPositionResiduals; /**< used to store all residuals (true - estimated) of position of the seed hit of clean tracks reconstructed by the CATF */
      std::vector<double>
      m_rootCompleteCASeedPositionResiduals; /**< used to store all residuals (true - estimated) of position of the seed hit of full tracks reconstructed by the CATF */

      std::vector<int> m_rootMCreconstructedTrackLength; /**< used to store all track length values of tracks reconstructed by the MCTF */
      std::vector<int> m_rootCAreconstructedTrackLength; /**< used to store all track length values of tracks reconstructed by the CATF */


      std::vector<int> m_rootLostUClusters; /**< used to store the number of uClusters of lost MCTCs not found by the TF */
      std::vector<int> m_rootLostVClusters; /**< used to store the number of vClusters of lost MCTCs not found by the TF */
      std::vector<int> m_rootTotalMcUClusters; /**< used to store the number of uClusters of all MCTCs */
      std::vector<int> m_rootTotalMcVClusters; /**< used to store the number of vClusters of all MCTCs */

      std::vector<double>
      m_rootLostUClusterEDep; /**< used to store the energy deposition of uClusters of lost MCTCs not found by the TF */
      std::vector<double>
      m_rootLostVClusterEDep; /**< used to store the energy deposition of vClusters of lost MCTCs not found by the TF */
      std::vector<double> m_rootTotalMcUClusterEDep; /**< used to store the energy deposition of uClusters of all MCTCs */
      std::vector<double> m_rootTotalMcVClusterEDep; /**< used to store the energy deposition of vClusters of all MCTCs */

      std::vector<int>
      m_forRootCountFoundIDs; /**< dirty hack to guarantee that no double found TCs are counted for the efficiency, gets deleted at the beginning of each event and stores each found ID of the event once */
    private:

    };
  } // Tracking namespace
}
