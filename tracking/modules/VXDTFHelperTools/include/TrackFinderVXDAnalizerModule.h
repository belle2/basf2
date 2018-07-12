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



namespace Belle2 {
  namespace Tracking {



    /** The TrackFinderVXDAnalizerModule
      *
    * analyzes quality of SpacePointTrackCands delivered by a test-TF compared to a reference TF
      *
      */
    class TrackFinderVXDAnalizerModule : public Module {
    public:


      /** constructor */
      TrackFinderVXDAnalizerModule();


      /** destructor */
      ~TrackFinderVXDAnalizerModule() {}


      /** inizialize function */
      void initialize() override;


      /** beginRun function */
      void beginRun() override {}


      /** event function */
      void event() override;


      /** endRun function */
      void endRun() override;


      /** terminate function */
      void terminate() override;


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


      /** StoreArray for the reference TCs provided by a reference TF (like trackFinderMCTruth). */
      StoreArray<SpacePointTrackCand> m_referenceTCs;

      /** StoreArray for the TCs provided by a TF to be tested (like VXDTF). */
      StoreArray<SpacePointTrackCand> m_testTCs;

      /** StoreArray for accepted/successfully reconstructed track candidates. */
      StoreArray<SpacePointTrackCand> m_acceptedTCs;

      /** StoreArray for lost track candidates. */
      StoreArray<SpacePointTrackCand> m_lostTCs;

      /** takes care of collecting data and storing it to root branches */
      RootParameterTracker m_rootParameterTracker;


// parameters:

      /** depending on what value you set it, it will print data like momentum residuals or any other interesting info during endrun... (currently not in use). */
      std::string m_PARAMprintData;

      /** to narrow down the relevant mcTracks, this minFilter can be set to filter tracks having lower transverse momentum than this threshold. Relevant for checking efficiency of TFs with certain transverse momentum ranges */
      double m_PARAMminTMomentumFilter;

      /** to narrow down the relevant mcTracks, this maxFilter can be set to filter tracks having higher transverse momentum than this threshold. Relevant for checking efficiency of TFs with certain transverse momentum ranges */
      double m_PARAMmaxTMomentumFilter;

      /** set here all parameters to be tracked which use an algorithm storing one double per tc.
       *
       * Accepts a vector of vector of vector of strings of entries.
       * Sample usage in steering file:
       * param('trackedParametersDouble', [ [ ["Perfect"], ["AnalyzingAlgorithmValuePX", "AnalyzingAlgorithmResidualP"] ] ])
       * first innermost vector storest the TCType to be tracked, second the algorithms which shall be applied on that tcType */
      std::vector< std::vector<std::vector<std::string> > > m_PARAMtrackedParametersDouble;

      /** set here all parameters to be tracked which use an algorithm storing one int per tc.
       *
       * Accepts a vector of vector of vector of strings of entries.
       * Sample usage in steering file:
       * param('trackedParametersDouble', [ [ ["Contaminated"], ["AnalyzingAlgorithmLostUClusters", "AnalyzingAlgorithmLostVClusters"] ] ])
       * first innermost vector storest the TCType to be tracked, second the algorithms which shall be applied on that tcType */
      std::vector< std::vector<std::vector<std::string> > > m_PARAMtrackedParametersInt;

      /** set here all parameters to be tracked which use an algorithm storing one vector< double> per tc.
       *
       * Accepts a vector of vector of vector of strings of entries.
       * Sample usage in steering file:
       * param('trackedParametersDouble', [ [ ["Clean"], ["AnalyzingAlgorithmLostUEDep", "AnalyzingAlgorithmLostVEDep"] ] ])
       * first innermost vector storest the TCType to be tracked, second the algorithms which shall be applied on that tcType */
      std::vector< std::vector<std::vector<std::string> > > m_PARAMtrackedParametersVecDouble;

      /** if true, analysis data is stored to root file with file name chosen by 'rootFileName' */
      bool m_PARAMwriteToRoot;

      /** only two entries accepted, first one is the root filename, second one is 'RECREATE' or 'UPDATE' which is the write mode for the root file, parameter is used  */
      std::vector<std::string> m_PARAMrootFileName;

      /** only allowed size: 3. stores coordinates of the origin used */
      std::vector<double> m_PARAMorigin;

      /** if true, for testTC the values of attached refTC will be stored instead of own values. - why are there values of the refTC stored? we want to know the real data, not the guesses of the reconstructed data. Deviations of reference values to guesses of the reconstructed data will be stored in resiudals anyway. */
      bool m_PARAMuseMCDataForValues;

      /** the name of the storeArray container provided by the reference TF (has to be set manually there and here) */
      std::string m_PARAMreferenceTCname;

      /** the name of the storeArray container provided by the TF to be evaluated (has to be set manually there and here) */
      std::string m_PARAMtestTCname;

      /** the name of the trackCandidateCollection of successfully reconstructed track candidates determined by the TFAnalizer */
      std::string m_PARAMacceptedTCname;

      /** the name of the trackCandidateCollection of lost track candidates determined by the TFAnalizer */
      std::string m_PARAMlostTCname;

      /** choose value to filter TCs found by VXDTF.
       *
       * TCs having purities lower than this value won't be marked as reconstructed (value 0-1).
       * e.g. having a TC with 4 hits, 1 foreign, 3 good ones. would mean 0.75,
       * a m_PARAMpurityThresholdThreshold with 0.7 would mark the track as 'reconstructed',
       * a threshold of 0.8 would neglect this TC */
      double m_PARAMpurityThreshold;

      /** defines how many measurements (numbers of degrees of freedom) the TC must have to be accepted as reconstructed.
       *
       * Standard is 5, values lower than 5 wouldn't make sense because of minimal info needed for track parameters */
      unsigned int m_PARAMminNDFThreshold;

      /** if true, test-tc whose activation-state is set to false are skipped for analysis. */
      bool m_PARAMignoreDeadTCs;

      /** if true, for each event a summary will be given (WARNING produces a lot of output! */
      bool m_PARAMdoEventSummary;


// counters:

      /** counts number of reconstructed TCs. */
      unsigned int m_countReconstructedTCs = 0;

      /** counts number of accepted TCs which are stored in separate container for external tests (e.g. trackFitChecker) in storaArray with name m_PARAMacceptedTCname. */
      unsigned int m_countAcceptedGFTCs = 0;

      /** counts number of TCs found by MCTF but lost by VXDTF, they are stored for external tests in storaArray with name m_PARAMlostTCname. */
      unsigned int m_lostGFTCs = 0;

      /** knows current event number. */
      unsigned int m_eventCounter = 0;

      /** counts number of tracks reconstructed by the mcTrackFinder. */
      unsigned int m_mcTrackCounter = 0;

      /** total number of hits (clusters/2) attached to mcTCs (therefore total number of real hits). */
      unsigned int m_totalRealHits = 0;

      /** counts number of tracks reconstructed by the CATF. */
      unsigned int m_caTrackCounter = 0;

      /** counts number of tracks, where no foreign hits were attached ('clean') AND all hits of the mcTC were reconstructed. */
      unsigned int m_countedPerfectRecoveries = 0;

      /** counts number of tracks, where no foreign hits were attached ('clean'), does NOT mean that all reconstructable hits had been found by CATF! */
      unsigned int m_countedCleanRecoveries = 0;

      /** counts number of tracks, where foreign hits werew attached but its purity was above the threshold. */
      unsigned int m_countedContaminatedRecoveries = 0;

      /** counts number of tracks, which did not have enough hits at all. */
      unsigned int m_countedTCsTooShort = 0;

      /** counts number of tracks, where a dominating TC was found, but the purity did not reach the threshold. */
      unsigned int m_countedGhosts = 0;

      /** if a TC was found more than once with good (contaminated or clean ones) caTCs, it will be counted to find out how many of the ghost tcs are in fact good tcs but not combined to one tc. */
      unsigned int m_countedDoubleEntries = 0;

      /** counts number of Lost test tcs. */
      unsigned int m_countedLostTest = 0;

      /** counts number of Lost reference tcs. */
      unsigned int m_countedLostRef = 0;

      /** counts number reference TCs which were marked as clones. */
      unsigned int m_countedReferenceClones = 0;

      /** counts number of times, where assigned caTC guessed wrong sign of charge. */
      unsigned int m_wrongChargeSignCounter = 0;

      /** another counter of mcTCs, consideres size of datastores containing mcTCs. */
      unsigned int m_mcTrackVectorCounter = 0;

      /** counts total number of pxdHits added by mcTF. */
      unsigned int m_nMcPXDHits = 0;

      /** counts total number of svdHits added by mcTF. */
      unsigned int m_nMcSVDHits = 0;

      /** counts total number of pxdHits added by caTF. */
      unsigned int m_nCaPXDHits = 0;

      /** counts total number of svdHits added by caTF. */
      unsigned int m_nCaSVDHits = 0;



    private:
    };
  } // Tracking namespace
}
