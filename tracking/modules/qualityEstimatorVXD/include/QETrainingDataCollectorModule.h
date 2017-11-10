/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/modules/qualityEstimatorVXD/QualityEstimatorBaseModule.h>

// TODO: This should probably not be in the mva folder?
#include <tracking/trackFindingVXD/mva/SimpleVariableRecorder.h>
#include <tracking/trackFindingVXD/variableExtractors/ClusterInfoExtractor.h>
#include <tracking/trackFindingVXD/variableExtractors/QEResultsExtractor.h>

namespace Belle2 {
  class QETrainingDataCollectorModule : public QualityEstimatorBaseModule {

  public:
    /** Constructor of the module. */
    QETrainingDataCollectorModule();

    /** Initializes the Module. */
    virtual void initialize() override;

    virtual void beginRun() override;

    /** TODO: docu */
    virtual void singleSPTCevent(SpacePointTrackCand&) override;

    virtual void terminate() override;


  protected:

    // module parameters

    std::string m_ClusterInformation;

    // member variables

    /** QualityEstimatorMC as target for training */
    std::unique_ptr<QualityEstimatorBase> m_estimatorMC;

    std::string m_TrainingDataOutputName;

    std::unique_ptr<SimpleVariableRecorder> m_recorder;

    std::vector<Named<float*>> m_variableSet;

    std::unique_ptr<QEResultsExtractor> m_qeResultsExtractor;

    float m_nSpacePoints = NAN;

    float m_truth = NAN;

    std::unique_ptr<ClusterInfoExtractor> m_clusterInfoExtractor;
  };
}
