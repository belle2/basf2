/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <tracking/trackFindingVXD/variableExtractors/SimpleVariableRecorder.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>

#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/EventInfoExtractor.h>
#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/RecoTrackExtractor.h>
#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/SubRecoTrackExtractor.h>
#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/HitInfoExtractor.h>
#include <tracking/trackFindingVXD/variableExtractors/ClusterInfoExtractor.h>
#include <tracking/trackFindingVXD/variableExtractors/QEResultsExtractor.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

#include <memory>


namespace Belle2 {

  /** Quality Estimator Data Collector Module to collect data for a MVA training using VXDQE_teacher.py.
   *  Runs in addition to VXDTF2 and mc_matcher, see VXDQE_TrainingDataCollector.py for example.
   * */
  class TrackQETrainingDataCollectorModule : public Module {

  public:
    /** Constructor of the module. */
    TrackQETrainingDataCollectorModule();

    /** Initializes the Module. */
    void initialize() override;

    /** sets magnetic field strength */
    void beginRun() override;

    /** applies the selected quality estimation method for a given set of TCs */
    void event() override;

    /** write out data from m_recorder */
    void terminate() override;


  protected:

    /// Name of the recoTrack StoreArray
    std::string m_recoTracksStoreArrayName = "RecoTracks";
    /// Name of the CDC StoreArray
    std::string m_cdcRecoTracksStoreArrayName = "CDCRecoTracks";
    /// Name of the SVD StoreArray
    std::string m_svdRecoTracksStoreArrayName = "SVDRecoTracks";
    /// Name of the PXD StoreArray
    std::string m_pxdRecoTracksStoreArrayName = "PXDRecoTracks";

    /** name of the output rootfile */
    std::string m_TrainingDataOutputName = "QETrainingOutput.root";

    /// Parameter to enable event-wise features
    bool m_collectEventFeatures = false;

    /// Store Array of the recoTracks
    StoreArray<RecoTrack> m_recoTracks;

    /// pointer to object that extracts info from the whole event
    std::unique_ptr<EventInfoExtractor> m_eventInfoExtractor;
    /// pointer to object that extracts info from the root RecoTrack
    std::unique_ptr<RecoTrackExtractor> m_recoTrackExtractor;
    /// pointer to object that extracts info from the related sub RecoTracks
    std::unique_ptr<SubRecoTrackExtractor> m_subRecoTrackExtractor;
    /// pointer to object that extracts info from the hits within the RecoTrack
    std::unique_ptr<HitInfoExtractor> m_hitInfoExtractor;

    /** pointer to the object that writes out the collected data into a root file */
    std::unique_ptr<SimpleVariableRecorder> m_recorder;

    /** set of named variables to be collected */
    std::vector<TrackingUtilities::Named<float*>> m_variableSet;

    /** pointer to object that extracts the results from the estimation method
    * (including QI, chi2, p_t and p_mag) */
    std::unique_ptr<QEResultsExtractor> m_qeResultsExtractor;

    /** pointer to object that extracts info from the clusters of a SPTC */
    std::unique_ptr<ClusterInfoExtractor> m_clusterInfoExtractor;

    /** pointer to object that extracts the results from the estimation method
    * (including QI, chi2, p_t and p_mag)
    * For the SVD track before CDC. */
    std::unique_ptr<QEResultsExtractor> m_qeResultsExtractorBefore;

    /** pointer to object that extracts info from the clusters of a SPTC
     * For the SVD track before CDC. */
    std::unique_ptr<ClusterInfoExtractor> m_clusterInfoExtractorBefore;

    /** pointer to the selected QualityEstimator */
    std::unique_ptr<QualityEstimatorBase> m_estimator;

    /** number of SpacePoints in SPTC as additional info for MVA,
     * type is float to be consistent with m_variableSet (and MVA implementation) */
    float m_nSpacePoints = NAN;
    /** number of SpacePoints in SPTC. For the SVD track before CDC. */
    float m_nSpacePointsBefore = NAN;

    /** Identifier which estimation method to use for SVD. Valid identifiers are:
     * tripletFit
     * circleFit
     * helixFit
     */
    std::string m_SVDEstimationMethod = "tripletFit";

    /** truth information collected with m_estimatorMC
     * type is float to be consistent with m_variableSet (and TTree + MVA implementation).
     * */
    float m_matched = NAN; ///< 1 if track is matched, 0 else
    float m_background = NAN; ///< 1 if track is background track, 0 else
    float m_ghost = NAN;///< 1 if track is ghost track, 0 else
    float m_fake = NAN;///< 1 if track is fake track, 0 else
    float m_clone = NAN;///< 1 if track is clone track, 0 else
  };
}
