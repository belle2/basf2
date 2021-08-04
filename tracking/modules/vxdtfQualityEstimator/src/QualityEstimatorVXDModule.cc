/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/vxdtfQualityEstimator/QualityEstimatorVXDModule.h>

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorMC.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRandom.h>

#include <framework/geometry/BFieldManager.h>

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
    // needs the mc Reco tracks so initialize them here
    StoreArray<RecoTrack> mcRecoTracks;
    mcRecoTracks.isRequired(m_MCRecoTracksStoreArrayName);
    m_estimator = std::make_unique<QualityEstimatorMC>(m_MCRecoTracksStoreArrayName, m_MCStrictQualityEstimator);
  } else if (m_EstimationMethod == "tripletFit") {
    m_estimator = std::make_unique<QualityEstimatorTripletFit>();
  } else if (m_EstimationMethod == "circleFit") {
    m_estimator = std::make_unique<QualityEstimatorCircleFit>();
  } else if (m_EstimationMethod == "helixFit") {
    m_estimator = std::make_unique<QualityEstimatorRiemannHelixFit>();
  } else if (m_EstimationMethod == "random") {
    m_estimator = std::make_unique<QualityEstimatorRandom>();
  }
  B2ASSERT("QualityEstimator could not be initialized with method: " << m_EstimationMethod, m_estimator);
}

void QualityEstimatorVXDModule::beginRun()
{
  // BField is required by all QualityEstimators
  double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_estimator->setMagneticFieldStrength(bFieldZ);

  if (m_EstimationMethod == "mcInfo") {
    QualityEstimatorMC* MCestimator = static_cast<QualityEstimatorMC*>(m_estimator.get());
    MCestimator->forceUpdateClusterNames();
  }
}

void QualityEstimatorVXDModule::event()
{

  // assign a QI computed using the selected QualityEstimator for each given SpacePointTrackCand
  for (SpacePointTrackCand& aTC : m_spacePointTrackCands) {

    double qi = m_estimator->estimateQuality(aTC.getSortedHits());

    aTC.setQualityIndicator(qi);
  }
}
