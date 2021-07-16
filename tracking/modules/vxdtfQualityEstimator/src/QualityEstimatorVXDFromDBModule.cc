/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/vxdtfQualityEstimator/QualityEstimatorVXDFromDBModule.h>

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorMC.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRandom.h>

#include <framework/geometry/BFieldManager.h>

using namespace Belle2;


REG_MODULE(QualityEstimatorVXDFromDB)

QualityEstimatorVXDFromDBModule::QualityEstimatorVXDFromDBModule() : Module()
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

  addParam("MaterialBudgetFactor", m_materialBudgetFactor,
           "MaterialBudgetFactor needed for the TripletFit.", float(1.45));

  addParam("MaxPt", m_maxPt,
           "Maximum pt needed for the TripletFit.", float(0.01));

  addParam("useDB", m_useDB, "If set, estimator parameter settings read from DB", true);
}

void QualityEstimatorVXDFromDBModule::initialize()
{
  m_spacePointTrackCands.isRequired(m_SpacePointTrackCandsStoreArrayName);

  if (m_useDB) {
    if (m_VXDQualityEstimatorParameters) {
      B2INFO("Loading VXDQuality estimator parameters from DB");
      m_EstimationMethod = m_VXDQualityEstimatorParameters->getEstimationMethod();
      m_materialBudgetFactor = m_VXDQualityEstimatorParameters->getMaterialBudgetFactor();
      m_maxPt = m_VXDQualityEstimatorParameters->getMaxPt();
    } else {
      B2FATAL("VXDQualityEstimatorParameters not found");
    }
  }

  // create pointer to chosen estimator
  if (m_EstimationMethod == "mcInfo") {
    m_estimator = std::make_unique<QualityEstimatorMC>(m_MCRecoTracksStoreArrayName, m_MCStrictQualityEstimator);
  } else if (m_EstimationMethod == "tripletFit") {
    m_estimator = std::make_unique<QualityEstimatorTripletFit>(m_materialBudgetFactor, m_maxPt);
  } else if (m_EstimationMethod == "circleFit") {
    m_estimator = std::make_unique<QualityEstimatorCircleFit>();
  } else if (m_EstimationMethod == "helixFit") {
    m_estimator = std::make_unique<QualityEstimatorRiemannHelixFit>();
  } else if (m_EstimationMethod == "random") {
    m_estimator = std::make_unique<QualityEstimatorRandom>();
  }
  B2ASSERT("QualityEstimator could not be initialized with method: " << m_EstimationMethod, m_estimator);
}

void QualityEstimatorVXDFromDBModule::beginRun()
{
  // BField is required by all QualityEstimators
  double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_estimator->setMagneticFieldStrength(bFieldZ);

  if (m_EstimationMethod == "mcInfo") {
    StoreArray<RecoTrack> mcRecoTracks;
    mcRecoTracks.isRequired(m_MCRecoTracksStoreArrayName);
    std::string svdClustersName = ""; std::string pxdClustersName = ""; std::string vtxClustersName = "";

    if (mcRecoTracks.getEntries() > 0) {
      svdClustersName = mcRecoTracks[0]->getStoreArrayNameOfSVDHits();
      pxdClustersName = mcRecoTracks[0]->getStoreArrayNameOfPXDHits();
      vtxClustersName = mcRecoTracks[0]->getStoreArrayNameOfVTXHits();
    } else {
      B2WARNING("No Entries in mcRecoTracksStoreArray: using empty cluster name for svd, pxd and vtx");
    }

    QualityEstimatorMC* MCestimator = static_cast<QualityEstimatorMC*>(m_estimator.get());
    MCestimator->setClustersNames(svdClustersName, pxdClustersName, vtxClustersName);
  }
}

void QualityEstimatorVXDFromDBModule::event()
{
  // assign a QI computed using the selected QualityEstimator for each given SpacePointTrackCand
  for (SpacePointTrackCand& aTC : m_spacePointTrackCands) {

    double qi = m_estimator->estimateQuality(aTC.getSortedHits());

    aTC.setQualityIndicator(qi);
  }
}
