/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/vxdHoughTracking/filters/pathFilters/TwoHitVirtualIPQIFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorMC.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/geometry/BFieldManager.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/BeamSpot.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace vxdHoughTracking;

void TwoHitVirtualIPQIFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
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

void TwoHitVirtualIPQIFilter::beginRun()
{
  const double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_estimator->setMagneticFieldStrength(bFieldZ);

  if (m_param_EstimationMethod == "mcInfo") {
    QualityEstimatorMC* MCestimator = static_cast<QualityEstimatorMC*>(m_estimator.get());
    MCestimator->forceUpdateClusterNames();
  }


  /// BeamSpot from DB
  DBObjPtr<BeamSpot> beamSpotDB;
  if (beamSpotDB.isValid()) {
    const B2Vector3D& BeamSpotPosition = (*beamSpotDB).getIPPosition();
    const TMatrixDSym posErr = (*beamSpotDB).getIPPositionCovMatrix();
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
    StoreArray<RecoTrack> mcRecoTracks;
    mcRecoTracks.isRequired(m_param_MCRecoTracksStoreArrayName);
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
  const std::vector<TrackFindingCDC::WithWeight<const VXDHoughState*>>& previousHits = pair.first;

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

  return estimatorResultVirtIP.qualityIndicator;
}
