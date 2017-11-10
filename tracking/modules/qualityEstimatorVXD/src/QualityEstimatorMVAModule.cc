/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner, Sebastian Racs                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/qualityEstimatorVXD/QualityEstimatorMVAModule.h>

using namespace Belle2;


REG_MODULE(QualityEstimatorMVA)

QualityEstimatorMVAModule::QualityEstimatorMVAModule() : QualityEstimatorBaseModule()
{
  //Set module properties
  setDescription("The quality estimator module for SpacePointTrackCandidates using MVA.");

  addParam("WeightFileIdentifier", m_WeightFileIdentifier,
           "Identifier of weightfile in Database or local root/xml file.", std::string(""));

  addParam("ClusterInformation", m_ClusterInformation, "Whether to use cluster infos or not", std::string(""));
}

void QualityEstimatorMVAModule::initialize()
{
  QualityEstimatorBaseModule::initialize();

  m_qeResultsExtractor = std::make_unique<QEResultsExtractor>(m_EstimationMethod, m_variableSet);

  m_variableSet.emplace_back("NSpacePoints", &m_nSpacePoints);

  if (m_ClusterInformation == "Average") {
    m_clusterInfoExtractor = std::make_unique<ClusterInfoExtractor>(m_variableSet);
  }

  m_mvaExpert = std::make_unique<MVAExpert>(m_WeightFileIdentifier, m_variableSet);
  m_mvaExpert->initialize();

}

void QualityEstimatorMVAModule::beginRun()
{
  m_mvaExpert->beginRun();
  QualityEstimatorBaseModule::beginRun();
}

// assign a QI computed using the selected QualityEstimator for each given SpacePointTrackCand
void QualityEstimatorMVAModule::singleSPTCevent(SpacePointTrackCand& aTC)
{
  if (not aTC.hasRefereeStatus(SpacePointTrackCand::c_isActive)) {
    return;
  }

  std::vector<SpacePoint const*> const sortedHits = aTC.getSortedHits();

  if (m_ClusterInformation == "Average") {
    m_clusterInfoExtractor->extractVariables(sortedHits);
  }

  m_nSpacePoints = sortedHits.size();

  m_qeResultsExtractor->extractVariables(m_estimator->estimateQualityAndProperties(sortedHits));

  float qi = m_mvaExpert->predict();
  aTC.setQualityIndex(qi);
}
