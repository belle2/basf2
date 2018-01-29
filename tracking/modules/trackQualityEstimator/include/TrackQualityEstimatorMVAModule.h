/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sebastian Racs                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <tracking/trackFindingVXD/mva/MVAExpert.h>

#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/EventInfoExtractor.h>
#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/RecoTrackExtractor.h>
#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/SubRecoTrackExtractor.h>
#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/HitInfoExtractor.h>

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {

  /** Quality estimation module for SpacePointTrackCandidates using multivariate analysis (MVA).
  * This module calculates a QualityIndex (QI) for each SpacePointTrackCandidate.
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
    std::string m_recoTracksStoreArrayName;
    /// Name of the SVD-CDC StoreArray
    std::string m_svdcdcRecoTracksStoreArrayName;
    /// Name of the CDC StoreArray
    std::string m_cdcRecoTracksStoreArrayName;
    /// Name of the SVD StoreArray
    std::string m_svdRecoTracksStoreArrayName;
    /// Name of the PXD StoreArray
    std::string m_pxdRecoTracksStoreArrayName;

    /** identifier of weightfile in Database or local root/xml file */
    std::string m_WeightFileIdentifier;

    /// Store Array of the recoTracks
    StoreArray<RecoTrack> m_recoTracks;

    /// pointer to the object to interact with the MVA package
    std::unique_ptr<MVAExpert> m_mvaExpert;

    /// pointer to object that extracts info from the whole event
    std::unique_ptr<EventInfoExtractor> m_eventInfoExtractor;
    /// pointer to object that extracts info from the root RecoTrack
    std::unique_ptr<RecoTrackExtractor> m_recoTrackExtractor;
    /// pointer to object that extracts info from the related sub RecoTracks
    std::unique_ptr<SubRecoTrackExtractor> m_subRecoTrackExtractor;
    /// pointer to object that extracts info from the hits within the RecoTrack
    std::unique_ptr<HitInfoExtractor> m_hitInfoExtractor;

    /// set of named variables to be used in MVA
    std::vector<Named<float*>>  m_variableSet;
  };
}
