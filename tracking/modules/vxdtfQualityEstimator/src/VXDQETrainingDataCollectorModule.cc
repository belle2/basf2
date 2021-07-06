/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/vxdtfQualityEstimator/VXDQETrainingDataCollectorModule.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorMC.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;


REG_MODULE(VXDQETrainingDataCollector)

VXDQETrainingDataCollectorModule::VXDQETrainingDataCollectorModule() : Module()
{
  //Set module properties
  setDescription("Module to collect training data for the VXDQualityEstimatorMVA and store it in a root file.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  addParam("EstimationMethod",
           m_EstimationMethod,
           "Identifier which estimation method to use. Valid identifiers are: [circleFit, "
           "tripletFit, helixFit]",
           m_EstimationMethod);

  addParam("MCInfo",
           m_MCInfo,
           "If true, MC information is used. Thus, to run over data, this needs to be set to false.",
           m_MCInfo);

  addParam("SpacePointTrackCandsStoreArrayName",
           m_SpacePointTrackCandsStoreArrayName,
           "Name of StoreArray containing the SpacePointTrackCandidates to be estimated.",
           m_SpacePointTrackCandsStoreArrayName);

  addParam("MCRecoTracksStoreArrayName",
           m_MCRecoTracksStoreArrayName,
           "Name of StoreArray containing MCRecoTracks. Only required for MCInfo method",
           m_MCRecoTracksStoreArrayName);

  addParam("MCStrictQualityEstimator",
           m_MCStrictQualityEstimator,
           "Only required for MCInfo method. If false combining several MCTracks is allowed.",
           m_MCStrictQualityEstimator);

  addParam("mva_target",
           m_mva_target,
           "Whether to write out MVA target which requires complete agreement between SVD CLusters "
           "of pattern "
           "recognition track and MC track to yield 1, else 0, and thus provides maximal hit "
           "purity and hit efficiency.",
           m_mva_target);

  addParam("TrainingDataOutputName",
           m_TrainingDataOutputName,
           "Name of the output rootfile.",
           m_TrainingDataOutputName);

  addParam("ClusterInformation",
           m_ClusterInformation,
           "How to compile information from clusters ['Average']",
           m_ClusterInformation);

  addParam("UseTimingInfo",
           m_UseTimingInfo,
           "Whether to collect timing information",
           m_UseTimingInfo);
}

void VXDQETrainingDataCollectorModule::initialize()
{
  m_spacePointTrackCands.isRequired(m_SpacePointTrackCandsStoreArrayName);

  m_qeResultsExtractor = std::make_unique<QEResultsExtractor>(m_EstimationMethod, m_variableSet);

  m_variableSet.emplace_back("NSpacePoints", &m_nSpacePoints);

  m_variableSet.emplace_back("truth", &m_truth);

  if (m_ClusterInformation == "Average") {
    m_clusterInfoExtractor = std::make_unique<ClusterInfoExtractor>(m_variableSet, m_UseTimingInfo);
  }

  m_recorder = std::make_unique<SimpleVariableRecorder>(m_variableSet, m_TrainingDataOutputName, "tree");

  // create pointer to chosen estimator
  if (m_EstimationMethod == "tripletFit") {
    m_estimator = std::make_unique<QualityEstimatorTripletFit>();
  } else if (m_EstimationMethod == "circleFit") {
    m_estimator = std::make_unique<QualityEstimatorCircleFit>();
  } else if (m_EstimationMethod == "helixFit") {
    m_estimator = std::make_unique<QualityEstimatorRiemannHelixFit>();
  }
  B2ASSERT("Not all QualityEstimators could be initialized!", m_estimator);

  if (m_MCInfo) {
    m_estimatorMC = std::make_unique<QualityEstimatorMC>(m_MCRecoTracksStoreArrayName, m_MCStrictQualityEstimator, m_mva_target);
    B2ASSERT("QualityEstimatorMC could be initialized!", m_estimatorMC);
  }
}

void VXDQETrainingDataCollectorModule::beginRun()
{
  // BField is required by all QualityEstimators
  const double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_estimator->setMagneticFieldStrength(bFieldZ);
  if (m_MCInfo) {
    m_estimatorMC->setMagneticFieldStrength(bFieldZ);
  }
}

void VXDQETrainingDataCollectorModule::event()
{
  for (SpacePointTrackCand& aTC : m_spacePointTrackCands) {

    if (not aTC.hasRefereeStatus(SpacePointTrackCand::c_isActive)) {
      continue;
    }

    std::vector<SpacePoint const*> const sortedHits = aTC.getSortedHits();
    if (m_ClusterInformation == "Average") {
      m_clusterInfoExtractor->extractVariables(sortedHits);
    }
    m_nSpacePoints = sortedHits.size();
    if (m_MCInfo) {
      const double mc_quality = m_estimatorMC->estimateQuality(sortedHits);
      m_truth = float(mc_quality > 0);
    }
    m_qeResultsExtractor->extractVariables(m_estimator->estimateQualityAndProperties(sortedHits));

    m_recorder->record();
  }
}

void VXDQETrainingDataCollectorModule::terminate()
{
  m_recorder->write();
  m_recorder.reset();
}
