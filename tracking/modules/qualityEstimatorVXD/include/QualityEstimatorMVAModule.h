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

#include <tracking/modules/qualityEstimatorVXD/QualityEstimatorBaseModule.h>

#include <tracking/trackFindingVXD/mva/MVAExpert.h>
#include <tracking/trackFindingVXD/variableExtractors/ClusterInfoExtractor.h>
#include <tracking/trackFindingVXD/variableExtractors/QEResultsExtractor.h>

namespace Belle2 {

  /** Quality estimator module for SpacePointTrackCandidates */
  class QualityEstimatorMVAModule : public QualityEstimatorBaseModule {

  public:

    /** Constructor of the module. */
    QualityEstimatorMVAModule();

    /** Initializes the Module. */
    virtual void initialize() override;

    virtual void beginRun() override;

    /** Applies the selected quality estimation method to the SPTCs */
    virtual void singleSPTCevent(SpacePointTrackCand&) override;


  protected:

    // module parameters

    std::string m_WeightFileIdentifier;

    std::string m_ClusterInformation;

    // member variables

    std::unique_ptr<MVAExpert> m_mvaExpert;

    std::vector<Named<float*>>  m_variableSet;

    std::unique_ptr<QEResultsExtractor> m_qeResultsExtractor;

    float m_nSpacePoints = NAN;

    std::unique_ptr<ClusterInfoExtractor> m_clusterInfoExtractor;

  };
}
