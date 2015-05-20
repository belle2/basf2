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

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/analyzingTools/RootParameterTracker.h>

//stl-stuff
#include <string>
#include <algorithm>

//root-stuff
#include <TVector3.h>
#include <TTree.h>
#include <TFile.h>



namespace Belle2 {
  namespace Tracking {



    /** The TrackFinderVXDAnalizerModule
      *
    * analyzes quality of SpacePointTrackCands delivered by a test-TF compared to a reference TF
      *
      */
    class TrackFinderVXDAnalizerModule : public Module {
    public:






//    /** Base class (functor) for storing an algorithm determining the data one wants to have */
//    template <class DataType>
//    class AnalyzingAlgorithmBase {
//    protected:
//    /** carries unique ID */
//    const static std::string m_iD;
//    public:
//
//    /** operator for comparison. */
//    inline bool operator == (const AnalyzingAlgorithmBase& b) const { return m_iD == b.getID(); }
//
//    /** returns unique ID */
//    std::string getID() const { return m_iD; }
//
//    /** pure virtual class to calculate data. takes two TCInfos */
//    template <class TCInfoType, class VectorType>
//    virtual DataType calcData(const TCInfoType& refTC, const TCInfoType& testTC) = 0;
//    };
//
//    /** Root data collector */
//    template <class DataType>
//    class RootDataCollector {
//    protected:
//    std::vector<DataType> m_collectedData;
//    std::string m_name;
//    AnalyzingAlgorithmBase m_calcAlgorithm;
//
//    public:
//
      //    RootDataCollector(std::string name, AnalyzingAlgorithmBase<DataType> algorithm) : m_name(name), m_calcAlgorithm(algorithm) {}
//
//    std::string getName() { return m_name; }
//
//    std::vector<DataType>* linkToData() { return & m_collectedData; }
//
//    void reset4event() { m_collectedData.clear(); }
//
//    void addData(const AnalyzerTCInfo& refTC, const AnalyzerTCInfo& testTC) { m_collectedData.push_back(m_calcAlgorithm.calcData(refTC, testTC)); }
//    };


      /** internal datastore for root export */
      struct RootVariables  {
        std::vector<double>
        PXresiduals; /**< used to store all residuals (true - estimated) of the momentum in x-direction which were reconstructed by the VXDTF*/
        std::vector<double>
        PYresiduals; /**< used to store all residuals (true - estimated) of the momentum in y-direction which were reconstructed by the VXDTF */
        std::vector<double>
        PZresiduals; /**< used to store all residuals (true - estimated) of the momentum in z-direction which were reconstructed by the VXDTF */

        std::vector<double> MCMomValues; /**< used to store all momentum values of tracks reconstructed by the MCTF */
        std::vector<double> CAMomValues; /**< used to store all momentum values of tracks reconstructed by the CATF */

        std::vector<double>
        CAMomResiduals; /**< used to store all momentum residuals (true - estimated) of tracks reconstructed by the CATF */

        std::vector<double> MCpTValues; /**< used to store all pT values of tracks reconstructed by the MCTF */
        std::vector<double> CApTValues; /**< used to store all pT values of tracks reconstructed by the CATF */
        std::vector<double> totalpTValues; /**< used to store all pT values of tracks existing no matter they produced hits or not */

        std::vector<double>
        CApTResiduals; /**< used to store all pT residuals (true - estimated) of tracks reconstructed by the CATF */

        std::vector<double> MCThetaValues; /**< used to store all theta values of tracks reconstructed by the MCTF */
        std::vector<double> CAThetaValues; /**< used to store all theta values of tracks reconstructed by the CATF */
        std::vector<double> totalThetaValues; /**< used to store all theta values of tracks existing no matter they produced hits or not */

        std::vector<double>
        CAThetaResiduals; /**< used to store all theta residuals (true - estimated) of tracks reconstructed by the CATF */


        std::vector<double> MCPhiValues; /**< used to store all Phi values of tracks reconstructed by the MCTF */
        std::vector<double> CAPhiValues; /**< used to store all Phi values of tracks reconstructed by the CATF */
        std::vector<double> totalPhiValues; /**< used to store all Phi values of tracks existing no matter they produced hits or not */

        std::vector<double>
        CAPhiResiduals; /**< used to store all Phi residuals (true - estimated) of tracks reconstructed by the CATF */

        std::vector<double>
        MCVertex2IP3DValues; /**< used to store all Vertex2IP3D (distance of vertex to origin) values of tracks reconstructed by the MCTF */
        std::vector<double>
        CAVertex2IP3DValues; /**< used to store all Vertex2IP3D (distance of vertex to origin) values of tracks reconstructed by the CATF */
        std::vector<double>
        totalVertex2IP3DValues; /**< used to store all Vertex2IP3D (distance of vertex to origin) values of tracks existing no matter they produced hits or not */

        std::vector<double>
        MCVertex2IPXYValues; /**< used to store all Vertex2IPXY (distance of vertex to origin) values of tracks reconstructed by the MCTF */
        std::vector<double>
        CAVertex2IPXYValues; /**< used to store all Vertex2IPXY (distance of vertex to origin) values of tracks reconstructed by the CATF */
        std::vector<double>
        totalVertex2IPXYValues; /**< used to store all Vertex2IPXY (distance of vertex to origin) values of tracks existing no matter they produced hits or not */

        std::vector<double>
        MCVertex2IPZValues; /**< used to store all Vertex2IPZ (distance of vertex to origin) values of tracks reconstructed by the MCTF */
        std::vector<double>
        CAVertex2IPZValues; /**< used to store all Vertex2IPZ (distance of vertex to origin) values of tracks reconstructed by the CATF */
        std::vector<double>
        totalVertex2IPZValues; /**< used to store all Vertex2IPZ (distance of vertex to origin) values of tracks existing no matter they produced hits or not */

        std::vector<double>
        CAMomResidualAngles; /**< used to store all residuals (true - estimated) of the total momentum vectors difference in direction of full tracks reconstructed by the CATF */

        std::vector<double>
        CApTResidualAngles; /**< used to store all residuals (true - estimated) of the pT vectors difference in direction of full tracks reconstructed by the CATF */

        std::vector<double>
        CASeedPositionResiduals; /**< used to store all residuals (true - estimated) of position of the seed hit of tracks reconstructed by the CATF*/

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


      /** constructor */
      TrackFinderVXDAnalizerModule();

      /** destructor */
      virtual ~TrackFinderVXDAnalizerModule() {}

      /** inizialize function */
      virtual void initialize();

      /** beginRun function */
      virtual void beginRun() {}

      /** event function */
      virtual void event();

      /** endRun function */
      virtual void endRun();

      /** terminate function */
      virtual void terminate();


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
        m_countedContaminatedRecoveries = 0;
        m_countedTCsTooShort = 0;
        m_countedGhosts = 0;
        m_rootFilePtr = NULL;
        m_treePtr = NULL;
      }


      /** of a vector of given particleIDs with their purities it returns the one which had the highest purity */
      template<class PurityType>
      PurityType returnDominantParticleID(const std::vector<PurityType >& purities)
      {
        PurityType bestResult = PurityType();

        for (const PurityType& iD : purities) {
          if (iD > bestResult) { bestResult = iD; }
        }
        return bestResult;
      }


    protected:

      // TODO: to be sorted!

      StoreArray<SpacePointTrackCand> m_referenceTCs;
      StoreArray<SpacePointTrackCand> m_testTCs;
      StoreArray<SpacePointTrackCand> m_acceptedTCs;
      StoreArray<SpacePointTrackCand> m_lostTCs;

      RootParameterTracker m_rootParameterTracker; /**< takes care of collecting data and storing it to root branches */
      std::vector<double> m_PARAMorigin; /**< only allowed size: 3. stores coordinates of the origin used */
      bool m_PARAMFileExportMcTracks; /**< possibly needed later (currently not in use), exports McTracks to File */
      bool m_PARAMFileExportTfTracks; /**< possibly needed later (currently not in use), exports TfTracks to File */
      bool m_PARAMprintExtentialAnalysisData; /**< set true, if you want to cout special Info to the shell (where it can be stored into a file and grep-ed to find specific info) */
      std::string
      m_PARAMreferenceTCname; /**< the name of the storeArray container provided by the reference TF (has to be set manually there and here) */
      std::string
      m_PARAMtestTCname; /**< the name of the storeArray container provided by the TF to be evaluated (has to be set manually there and here) */
      std::string
      m_PARAMacceptedTCname; /**< the name of the trackCandidateCollection of successfully reconstructed track candidates determined by the TFAnalizer */
      std::string
      m_PARAMlostTCname; /**< the name of the trackCandidateCollection of lost track candidates determined by the TFAnalizer */
      std::string m_PARAMinfoBoardName; /**< InfoContainer collection name */
      double m_PARAMpurityThreshold; /**<  chose value to filter TCs found by VXDTF. TCs having purities lower than this value won't be marked as reconstructed (value 0-1). e.g. having a TC with 4 hits, 1 foreign, 3 good ones. would mean 0.75, a m_PARAMpurityThresholdThreshold with 0.7 would mark the track as 'reconstructed', a threshold of 0.8 would neglect this TC */
      unsigned int
      m_PARAMminNDFThreshold; /**< defines how many measurements (numbers of degrees of freedom) the TC must have to be accepted as reconstructed, standard is 5, values lower than 5 wouldn't make sense because of minimal info needed for track parameters */
      unsigned int m_countReconstructedTCs; /**< counts number of reconstructed TCs */
      unsigned int
      m_countAcceptedGFTCs; /**< counts number of accepted TCs which are stored in separate container for external tests (e.g. trackFitChecker) in storaArray with name m_PARAMacceptedTCname */
      unsigned int
      m_lostGFTCs; /**< counts number of TCs found by MCTF but lost by VXDTF, they are stored for external tests in storaArray with name m_PARAMlostTCname */
      unsigned int m_eventCounter; /**< knows current event number */
      unsigned int m_mcTrackCounter; /**< counts number of tracks reconstructed by the mcTrackFinder */
      unsigned int m_totalRealHits; /**< total number of hits (clusters/2) attached to mcTCs (therefore total number of real hits) */
      unsigned int m_caTrackCounter; /**< counts number of tracks reconstructed by the CATF */
      unsigned int
      m_countedPerfectRecoveries; /**< counts number of tracks, where no foreign hits were attached ('clean') AND all hits of the mcTC were reconstructed */
      unsigned int
      m_countedCleanRecoveries; /**< counts number of tracks, where no foreign hits were attached ('clean'), does NOT mean that all reconstructable hits had been found by CATF! */
      unsigned int
      m_countedContaminatedRecoveries; /**< counts number of tracks, where foreign hits werew attached but its purity was above the threshold */
      unsigned int m_countedTCsTooShort; /**< counts number of tracks, which did not have enough hits at all */
      unsigned int
      m_countedGhosts; /**< counts number of tracks, where a dominating TC was found, but the purity did not reach the threshold */
      unsigned int
      m_countedDoubleEntries; /**< if a TC was found more than once with good (contaminated or clean ones) caTCs, it will be counted to find out how many of the ghost tcs are in fact good tcs but not combined to one tc */
      unsigned int m_wrongChargeSignCounter; /**< counts number of times, where assigned caTC guessed wrong sign of charge */
      std::string
      m_PARAMprintData; /**< depending on what value you set it, it will print data like momentum residuals or any other interesting info during endrun... (currently not in use)*/
      unsigned int m_mcTrackVectorCounter; /**< another counter of mcTCs, consideres size of datastores containing mcTCs */
      unsigned int m_nMcPXDHits; /**< counts total number of pxdHits added by mcTF */
      unsigned int m_nMcSVDHits; /**< counts total number of svdHits added by mcTF */
      unsigned int m_nCaPXDHits; /**< counts total number of pxdHits added by caTF */
      unsigned int m_nCaSVDHits; /**< counts total number of svdHits added by caTF */
      double m_PARAMminTMomentumFilter; /**< to narrow down the relevant mcTracks, this minFilter can be set to filter tracks having lower transverse momentum than this threshold. Relevant for checking efficiency of TFs with certain transverse momentum ranges */
      double m_PARAMmaxTMomentumFilter; /**< to narrow down the relevant mcTracks, this maxFilter can be set to filter tracks having higher transverse momentum than this threshold. Relevant for checking efficiency of TFs with certain transverse momentum ranges */

      std::vector< std::vector < std::string > >
      m_PARAMparametersToBeTracked; /**< accepts a vector of pairs of entries, each pair determines a TC type (first entry) and the algorithm to be used (second entry */
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
