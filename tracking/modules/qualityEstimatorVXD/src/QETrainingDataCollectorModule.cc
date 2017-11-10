/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/qualityEstimatorVXD/QETrainingDataCollectorModule.h>

#include <framework/geometry/BFieldManager.h>

using namespace Belle2;


REG_MODULE(QETrainingDataCollector)

QETrainingDataCollectorModule::QETrainingDataCollectorModule() : QualityEstimatorBaseModule()
{
  //Set module properties
  setDescription("Module to collect training data for a specified qualityEstimator and store it in a root file.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  addParam("TrainingDataOutputName", m_TrainingDataOutputName, "Name of the output rootfile.", std::string("QETrainingOutput.root"));

  addParam("ClusterInformation", m_ClusterInformation, "Wether to use cluster infos or not", std::string(""));
}

void QETrainingDataCollectorModule::initialize()
{
  QualityEstimatorBaseModule::initialize();

  m_qeResultsExtractor = std::make_unique<QEResultsExtractor>(m_EstimationMethod, m_variableSet);

  m_variableSet.emplace_back("NSpacePoints", &m_nSpacePoints);

  m_variableSet.emplace_back("truth", &m_truth);

  if (m_ClusterInformation == "Average") {
    m_clusterInfoExtractor = std::make_unique<ClusterInfoExtractor>(m_variableSet);
  }

  m_recorder = std::make_unique<SimpleVariableRecorder>(m_variableSet, m_TrainingDataOutputName, "tree");
}

void QETrainingDataCollectorModule::beginRun()
{
  QualityEstimatorBaseModule::beginRun();
  double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_estimatorMC->setMagneticFieldStrength(bFieldZ);
}

void QETrainingDataCollectorModule::singleSPTCevent(SpacePointTrackCand& aTC)
{
  if (not aTC.hasRefereeStatus(SpacePointTrackCand::c_isActive)) {
    return;
  }

  std::vector<SpacePoint const*> const sortedHits = aTC.getSortedHits();

  if (m_ClusterInformation == "Average") {
    m_clusterInfoExtractor->extractVariables(sortedHits);
  }

  m_nSpacePoints = sortedHits.size();

  double tmp = m_estimatorMC->estimateQuality(sortedHits);
  m_truth = tmp > 0 ? 1 : 0;

  m_qeResultsExtractor->extractVariables(m_estimator->estimateQualityAndProperties(sortedHits));

  // record variables
  m_recorder->record();
}

void QETrainingDataCollectorModule::terminate()
{
  m_recorder->write();
  m_recorder.reset();
}
