/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/TwoHitVirtualIPQIFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorMC.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void TwoHitVirtualIPQIFilter::beginRun()
{
  const double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_estimator->setMagneticFieldStrength(bFieldZ);

  if (m_param_EstimationMethod == "mcInfo") {
    StoreArray<RecoTrack> mcRecoTracks;
    mcRecoTracks.isRequired(m_param_MCRecoTracksStoreArrayName);
    std::string svdClustersName = ""; std::string pxdClustersName = "";

    if (mcRecoTracks.getEntries() > 0) {
      svdClustersName = mcRecoTracks[0]->getStoreArrayNameOfSVDHits();
      pxdClustersName = mcRecoTracks[0]->getStoreArrayNameOfPXDHits();
    } else {
      B2WARNING("No Entries in mcRecoTracksStoreArray: using empty cluster name for svd and pxd");
    }

    QualityEstimatorMC* MCestimator = static_cast<QualityEstimatorMC*>(m_estimator.get());
    MCestimator->setClustersNames(svdClustersName, pxdClustersName);
  }


  if (m_BeamSpotDB.isValid()) {
    m_BeamSpot = *m_BeamSpotDB;
    const B2Vector3D& BeamSpotPosition = m_BeamSpot.getIPPosition();
    const TMatrixDSym posErr = m_BeamSpot.getIPPositionCovMatrix();
    const B2Vector3D BeamSpotPositionError(sqrt(posErr[0][0]), sqrt(posErr[1][1]), sqrt(posErr[2][2]));
    m_virtualIPSpacePoint = SpacePoint(BeamSpotPosition, BeamSpotPositionError, {0.5, 0.5}, {false, false}, VxdID(0),
                                       Belle2::VXD::SensorInfoBase::VXD);
  } else {
    m_virtualIPSpacePoint = SpacePoint(B2Vector3D(0., 0., 0.), B2Vector3D(0.1, 0.1, 0.5), {0.5, 0.5}, {false, false}, VxdID(0),
                                       Belle2::VXD::SensorInfoBase::VXD);
  }

}

void TwoHitVirtualIPQIFilter::initialize()
{
  // create pointer to chosen estimator
  if (m_param_EstimationMethod == "mcInfo") {
    m_estimator = std::make_unique<QualityEstimatorMC>(m_param_MCRecoTracksStoreArrayName, m_param_MCStrictQualityEstimator);
  } else if (m_param_EstimationMethod == "tripletFit") {
    m_estimator = std::make_unique<QualityEstimatorTripletFit>();
  }  else if (m_param_EstimationMethod == "helixFit") {
    m_estimator = std::make_unique<QualityEstimatorRiemannHelixFit>();
  }
  B2ASSERT("QualityEstimator could not be initialized with method: " << m_param_EstimationMethod, m_estimator);
}

TrackFindingCDC::Weight
TwoHitVirtualIPQIFilter::operator()(const BasePathFilter::Object& pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const HitData*>>& previousHits = pair.first;

  // Do nothing if path is too short or too long
  if (previousHits.size() != 1) {
    return NAN;
  }

  std::vector<const SpacePoint*> spacePointsVirtIP;
  spacePointsVirtIP.reserve(previousHits.size() + 2);
  // Note that the path is always created outside-in.
  // The tripletFit only works with hits being ordered inside-out, so add the hits in that direction.
  // First the virtual IP, then the hit that is currently checked, and last the single hit in the path.
  spacePointsVirtIP.emplace_back(&m_virtualIPSpacePoint);
  spacePointsVirtIP.emplace_back(pair.second->getHit());
  spacePointsVirtIP.emplace_back(previousHits.at(0)->getHit());
  const auto& estimatorResultVirtIP = m_estimator->estimateQualityAndProperties(spacePointsVirtIP);

//   const double absHelixPocaDVirtIP = (estimatorResultVirtIP.pocaD) ? fabs(*estimatorResultVirtIP.pocaD) : 1e-6;
//   const double chi2VirtIP = (estimatorResultVirtIP.chiSquared) ? *estimatorResultVirtIP.chiSquared : 1e6;
//
//   if (absHelixPocaDVirtIP > m_helixFitPocaVirtIPDCut) {
//     return NAN;
//   }

  return estimatorResultVirtIP.qualityIndicator;
}

void TwoHitVirtualIPQIFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "helixFitPocaVirtIPDCut"), m_param_helixFitPocaVirtIPDCut,
                                "Cut on the POCA difference in xy with the POCA obtained from a helix fit, adding a virtual IP at the origin "
                                "(tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit).",
                                m_param_helixFitPocaVirtIPDCut);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "trackQualityEstimationMethod"), m_param_EstimationMethod,
                                "Identifier which estimation method to use. Valid identifiers are: [mcInfo, tripletFit, helixFit]",
                                m_param_EstimationMethod);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "MCRecoTracksStoreArrayName"), m_param_MCRecoTracksStoreArrayName,
                                "Only required for MCInfo method. Name of StoreArray containing MCRecoTracks.",
                                m_param_MCRecoTracksStoreArrayName);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "MCStrictQualityEstimator"), m_param_MCStrictQualityEstimator,
                                "Only required for MCInfo method. If false combining several MCTracks is allowed.",
                                m_param_MCStrictQualityEstimator);
}
