/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <tracking/trackingUtilities/mva/MVAExpert.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>

#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/EventInfoExtractor.h>
#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/RecoTrackExtractor.h>
#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/SubRecoTrackExtractor.h>
#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/HitInfoExtractor.h>
#include <tracking/trackFindingVXD/variableExtractors/ClusterInfoExtractor.h>
#include <tracking/trackFindingVXD/variableExtractors/QEResultsExtractor.h>

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {

  /** Quality estimation module for SpacePointTrackCandidates using multivariate analysis (MVA).
  * This module calculates a QualityIndicator (QI) for each SpacePointTrackCandidate.
  * This module can support most estimation strategies that implement the interface QualityEstimatorBase
  * and use them together with cluster information to calculate a new QI using MVA.
  *  */
  class TrackQualityEstimatorMVAModule : public Module {

  public:

    /** Constructor of the module. */
    TrackQualityEstimatorMVAModule();

    /** Initializes the Module. */
    void initialize() override;

    /** Launches mvaExpert and sets the magnetic field strength */
    void beginRun() override;

    /** Applies the selected quality estimation method for a given set of TCs */
    void event() override;


  private:
    /// Name of the recoTrack StoreArray
    std::string m_recoTracksStoreArrayName = "RecoTracks";
    /// Name of the CDC StoreArray
    std::string m_cdcRecoTracksStoreArrayName = "CDCRecoTracks";
    /// Name of the SVD StoreArray
    std::string m_svdRecoTracksStoreArrayName = "SVDRecoTracks";
    /// Name of the PXD StoreArray
    std::string m_pxdRecoTracksStoreArrayName = "PXDRecoTracks";
    /** Name of the StoreArray with mdst Tracks from track fit */
    std::string m_tracksStoreArrayName = "Tracks";

    /** identifier of weightfile in Database or local root/xml file */
    std::string m_weightFileIdentifier = "trackfitting_MVATrackQualityIndicator";

    /// Parameter to enable event-wise features
    bool m_collectEventFeatures = false;

    /// Store Array of the recoTracks
    StoreArray<RecoTrack> m_recoTracks;

    /// pointer to the object to interact with the MVA package
    std::unique_ptr<TrackingUtilities::MVAExpert> m_mvaExpert;

    /// pointer to object that extracts info from the whole event
    std::unique_ptr<EventInfoExtractor> m_eventInfoExtractor;
    /// pointer to object that extracts info from the root RecoTrack
    std::unique_ptr<RecoTrackExtractor> m_recoTrackExtractor;
    /// pointer to object that extracts info from the related sub RecoTracks
    std::unique_ptr<SubRecoTrackExtractor> m_subRecoTrackExtractor;
    /// pointer to object that extracts info from the hits within the RecoTrack
    std::unique_ptr<HitInfoExtractor> m_hitInfoExtractor;

    /// set of named variables to be used in MVA
    std::vector<TrackingUtilities::Named<float*>>  m_variableSet;

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
  };
}
