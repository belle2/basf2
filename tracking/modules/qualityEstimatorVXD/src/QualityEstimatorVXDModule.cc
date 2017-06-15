/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/QualityEstimatorVXDModule.h"

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorMC.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRandom.h>
#include <tracking/trackFindingVXD/utilities/UniquePointerHelper.h>
#include <framework/logging/Logger.h>
#include <geometry/bfieldmap/BFieldMap.h>

using namespace Belle2;


REG_MODULE(QualityEstimatorVXD)

QualityEstimatorVXDModule::QualityEstimatorVXDModule() : Module()
{
  //Set module properties
  setDescription("The quality estimator module for SpacePointTrackCandidates.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("EstimationMethod", m_EstimationMethod,
           "Identifier which estimation method to use. Valid identifiers are: [mcInfo, circleFit, tripletFit, helixFit, random]",
           std::string("tripletFit"));

  addParam("SpacePointTrackCandsStoreArrayName", m_SpacePointTrackCandsStoreArrayName,
           "Name of StoreArray containing the SpacePointTrackCandidates to be estimated.", std::string(""));

  addParam("MCRecoTracksStoreArrayName", m_MCRecoTracksStoreArrayName,
           "Only required for MCInfo method. Name of StoreArray containing MCRecoTracks.", std::string("MCRecoTracks"));

  addParam("MCStrictQualityEstimator", m_MCStrictQualityEstimator,
           "Only required for MCInfo method. If false combining several MCTracks is allowed.", bool(true));
}

void QualityEstimatorVXDModule::initialize()
{
  m_spacePointTrackCands.isRequired(m_SpacePointTrackCandsStoreArrayName);

  // create pointer to chosen estimator
  if (m_EstimationMethod == "mcInfo") {
    m_estimator = make_unique<QualityEstimatorMC>(m_MCRecoTracksStoreArrayName, m_MCStrictQualityEstimator);
  } else if (m_EstimationMethod == "tripletFit") {
    m_estimator = make_unique<QualityEstimatorTripletFit>();
  } else if (m_EstimationMethod == "circleFit") {
    m_estimator = make_unique<QualityEstimatorCircleFit>();
  } else if (m_EstimationMethod == "helixFit") {
    m_estimator = make_unique<QualityEstimatorRiemannHelixFit>();
  } else if (m_EstimationMethod == "random") {
    m_estimator = make_unique<QualityEstimatorRandom>();
  }
  B2ASSERT("QualityEstimator could not be initialized with method: " << m_EstimationMethod, m_estimator);
}

void QualityEstimatorVXDModule::beginRun()
{
  // BField is required by all QualityEstimators
  double bFieldZ = BFieldMap::Instance().getBField(TVector3(0, 0, 0)).Z();
  m_estimator->setMagneticFieldStrength(bFieldZ);
}

void QualityEstimatorVXDModule::event()
{
  // assign a QI computed using the selected QualityEstimator for each given SpacePointTrackCand
  for (SpacePointTrackCand& aTC : m_spacePointTrackCands) {

    double qi = m_estimator->estimateQuality(aTC.getSortedHits());

    aTC.setQualityIndex(qi);
  }
}
