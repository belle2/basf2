/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/qualityEstimatorVXD/QualityEstimatorModule.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorMC.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRandom.h>
#include <framework/logging/Logger.h>
#include <geometry/bfieldmap/BFieldMap.h>

using namespace Belle2;


REG_MODULE(QualityEstimator)

QualityEstimatorModule::QualityEstimatorModule() : Module()
{
  //Set module properties
  setDescription("The quality estimator module for SpacePointTrackCandidates.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("EstimationMethod", m_EstimationMethod,
           "Identifier which estimation method to use. Valid identifiers are: [MCInfo, CircleFit, TripletFit, Random]", std::string(""));

  addParam("SpacePointTrackCandsStoreArrayName", m_SpacePointTrackCandsStoreArrayName,
           "Name of StoreArray containing the SpacePointTrackCandidates to be estimated.", std::string(""));

  addParam("MCRecoTracksStoreArrayName", m_MCRecoTracksStoreArrayName,
           "Name of StoreArray containing MCRecoTracks. Only required for MCInfo method", std::string("MCRecoTracks"));

  addParam("MCStrictQualityEstimator", m_MCStrictQualityEstimator,
           "Only required for MCInfo method. If false combining several MCTracks is allowed.", bool(true));
}

void QualityEstimatorModule::initialize()
{
  m_spacePointTrackCands.isRequired(m_SpacePointTrackCandsStoreArrayName);

  // BField is required by all QualityEstimators
  double bFieldZ = BFieldMap::Instance().getBField(TVector3(0, 0, 0)).Z();

  // create pointer to chosen estimator
  if (m_EstimationMethod == "MCInfo") {
    m_estimator = std::unique_ptr<QualityEstimatorBase>(
                    new QualityEstimatorMC(bFieldZ, m_MCRecoTracksStoreArrayName, m_MCStrictQualityEstimator));
  } else if (m_EstimationMethod == "TripletFit") {
    m_estimator = std::unique_ptr<QualityEstimatorBase>(new QualityEstimatorTripletFit(bFieldZ));
  } else if (m_EstimationMethod == "CircleFit") {
    m_estimator = std::unique_ptr<QualityEstimatorBase>(new QualityEstimatorCircleFit(bFieldZ));
  } else if (m_EstimationMethod == "Random") {
    m_estimator = std::unique_ptr<QualityEstimatorBase>(new QualityEstimatorRandom(bFieldZ));
  }
  B2ASSERT("QualityEstimator could not be initialized with method: " << m_EstimationMethod, m_estimator);
}

void QualityEstimatorModule::event()
{
  // assign a QI computed using the selected QualityEstimator for each given SpacePointTrackCand
  for (SpacePointTrackCand& aTC : m_spacePointTrackCands) {

    double qi = m_estimator->estimateQuality(aTC.getSortedHits());

    aTC.setQualityIndex(qi);
  }
}
