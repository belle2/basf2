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

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>
#include <tracking/trackFindingVXD/variableExtractors/SimpleVariableRecorder.h>
#include <tracking/trackFindingVXD/variableExtractors/ClusterInfoExtractor.h>
#include <tracking/trackFindingVXD/variableExtractors/QEResultsExtractor.h>


#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

#include <memory>


namespace Belle2 {

  /** Quality Estimator Data Collector Module to collect data for a MVA training using VXDQE_teacher.py.
   *  Runs in addition to VXDTF2 and mc_matcher, see VXDQE_TrainingDataCollector.py for example.
   * */
  class QETrainingDataCollectorModule : public Module {

  public:
    /** Constructor of the module. */
    QETrainingDataCollectorModule();

    /** Initializes the Module. */
    void initialize() override;

    /** sets magnetic field strength */
    void beginRun() override;

    /** applies the selected quality estimation method for a given set of TCs */
    void event() override;

    /** write out data from m_recorder */
    void terminate() override;


  protected:

    // -----module parameters---

    /** Identifier which estimation method to use. Valid identifiers are:
     * tripletFit
     * circleFit
     * helixFit
     */
    std::string m_EstimationMethod;

    /** sets the name of the expected StoreArray containing SpacePointTrackCands */
    std::string m_SpacePointTrackCandsStoreArrayName;

    /** sets the name of the expected StoreArray containing MCRecoTracks. Only required for MCInfo method */
    std::string m_MCRecoTracksStoreArrayName;

    /** Required for MCInfo method, activates its strict version */
    bool m_MCStrictQualityEstimator;

    /** name of the output rootfile */
    std::string m_TrainingDataOutputName;

    /** how to compile information from clusters ['Average'] **/
    std::string m_ClusterInformation;

    /** whether to collect timing information */
    bool m_UseTimingInfo;

    // -------------------------

    /** the storeArray for SpacePointTrackCands as member, is faster than recreating link for each event */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

    /** pointer to the selected QualityEstimator */
    std::unique_ptr<QualityEstimatorBase> m_estimator;

    /** QualityEstimatorMC as target for training */
    std::unique_ptr<QualityEstimatorBase> m_estimatorMC;

    /** pointer to object that extracts the results from the estimation mehtod
    * (including QI, chi2, p_t and p_mag) */
    std::unique_ptr<QEResultsExtractor> m_qeResultsExtractor;

    /** pointer to object that extracts info from the clusters of a SPTC */
    std::unique_ptr<ClusterInfoExtractor> m_clusterInfoExtractor;

    /** pointer to the object that writes out the collected data into a root file */
    std::unique_ptr<SimpleVariableRecorder> m_recorder;

    /** set of named variables to be collected */
    std::vector<Named<float*>> m_variableSet;

    /** number of SpacePoints in SPTC as additional info to be collected,
     * type is float to be consistend with m_variableSet (and TTree + MVA implementation) */
    float m_nSpacePoints = NAN;

    /** truth information collected with m_estimatorMC
     * type is float to be consistend with m_variableSet (and TTree + MVA implementation) */
    float m_truth = NAN;

  };
}
