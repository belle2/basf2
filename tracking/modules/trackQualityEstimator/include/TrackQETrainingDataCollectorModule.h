/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner, Sebastian Racs                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <tracking/trackFindingVXD/variableExtractors/SimpleVariableRecorder.h>

#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/EventInfoExtractor.h>
#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/RecoTrackExtractor.h>
#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/SubRecoTrackExtractor.h>
#include <tracking/trackFitting/trackQualityEstimator/variableExtractors/HitInfoExtractor.h>

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
    std::string m_recoTracksStoreArrayName;
    /// Name of the SVD-CDC StoreArray
    std::string m_svdcdcRecoTracksStoreArrayName;
    /// Name of the CDC StoreArray
    std::string m_cdcRecoTracksStoreArrayName;
    /// Name of the SVD StoreArray
    std::string m_svdRecoTracksStoreArrayName;
    /// Name of the PXD StoreArray
    std::string m_pxdRecoTracksStoreArrayName;

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
    std::vector<Named<float*>> m_variableSet;

    /** name of the output rootfile */
    std::string m_TrainingDataOutputName;

    /** truth information collected with m_estimatorMC
     * type is float to be consistend with m_variableSet (and TTree + MVA implementation) */
    float m_truth = NAN;

//    /** number of RecoTracks in an event*/
//    float m_nRecoTracks = NAN;

  };
}
