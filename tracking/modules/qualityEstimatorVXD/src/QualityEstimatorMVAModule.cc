/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/qualityEstimatorVXD/QualityEstimatorMVAModule.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRandom.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/utilities/UniquePointerHelper.h>
#include <framework/logging/Logger.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <numeric>


using namespace Belle2;


REG_MODULE(QualityEstimatorMVA)

QualityEstimatorMVAModule::QualityEstimatorMVAModule() : Module()
{
  //Set module properties
  setDescription("The quality estimator module for SpacePointTrackCandidates.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("EstimationMethod", m_EstimationMethod,
           "Identifier which estimation method to use. Valid identifiers are: [CircleFit, TripletFit, Random]", std::string(""));

  addParam("SpacePointTrackCandsStoreArrayName", m_SpacePointTrackCandsStoreArrayName,
           "Name of StoreArray containing the SpacePointTrackCandidates to be estimated.", std::string(""));

  addParam("WeightFileIdentifier", m_WeightFileIdentifier,
           "Identifier of weightfile in Database or local root/xml file.", std::string(""));
}

void QualityEstimatorMVAModule::initialize()
{
  m_spacePointTrackCands.isRequired(m_SpacePointTrackCandsStoreArrayName);

  // BField is required by all QualityEstimators
  double bFieldZ = BFieldMap::Instance().getBField(TVector3(0, 0, 0)).Z();

  // create pointer to chosen estimator
  if (m_EstimationMethod == "TripletFit") {
    m_estimator = make_unique<QualityEstimatorTripletFit>(bFieldZ);
  } else if (m_EstimationMethod == "CircleFit") {
    m_estimator = make_unique<QualityEstimatorCircleFit>(bFieldZ);
  } else if (m_EstimationMethod == "HelixFit") {
    m_estimator = make_unique<QualityEstimatorRiemannHelixFit>(bFieldZ);
  } else if (m_EstimationMethod == "Random") {
    m_estimator = make_unique<QualityEstimatorRandom>(bFieldZ);
  }
  B2ASSERT("QualityEstimator could not be initialized with method: " << m_EstimationMethod, m_estimator);

  // register variables
  m_variableSet.setVariable("NHits", -1);
  m_variableSet.setVariables(m_EstimationMethod, QualityEstimationResults());

  m_mvaExpert = make_unique<MVAExpert>(m_WeightFileIdentifier, m_variableSet);
  m_mvaExpert->initialize();
}

void QualityEstimatorMVAModule::beginRun()
{
  m_mvaExpert->beginRun();
}

void QualityEstimatorMVAModule::event()
{
  // assign a QI computed using the selected QualityEstimator for each given SpacePointTrackCand
  for (SpacePointTrackCand& aTC : m_spacePointTrackCands) {

    std::vector<SpacePoint const*> const sortedHits = aTC.getSortedHits();
    m_variableSet.setVariable("NHits", sortedHits.size());

    QualityEstimationResults results = m_estimator->estimateQualityAndProperties(sortedHits);
    m_variableSet.setVariables(m_EstimationMethod, results);

    float qi = m_mvaExpert->predict();
    aTC.setQualityIndex(qi);
  }
}
