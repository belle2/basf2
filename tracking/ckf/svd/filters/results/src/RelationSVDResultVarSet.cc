/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/results/RelationSVDResultVarSet.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/core/ModuleParamList.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

RelationSVDResultVarSet::RelationSVDResultVarSet() : Super()
{
  addProcessingSignalListener(&m_advancer);
}

void RelationSVDResultVarSet::initialize()
{
  ModuleParamList moduleParamList;
  m_advancer.exposeParameters(&moduleParamList, "");
  moduleParamList.getParameter<std::string>("direction").setValue("both");

  Super::initialize();
}

bool RelationSVDResultVarSet::extract(const CKFToSVDResult* result)
{
  const std::vector<const SpacePoint*>& spacePoints = result->getHits();

  std::vector<bool> layerUsed;
  layerUsed.resize(7, false);

  for (const SpacePoint* spacePoint : spacePoints) {
    layerUsed[spacePoint->getVxdID().getLayerNumber()] = true;
  }

  const TVector3& resultMomentum = result->getMomentum();
  var<named("pt")>() = resultMomentum.Pt();
  var<named("theta")>() = resultMomentum.Theta();

  const RecoTrack* cdcTrack = result->getSeed();
  const RecoTrack* svdTrack = result->getRelatedSVDRecoTrack();
  B2ASSERT("Should have a related SVD track at this stage;", svdTrack);

  const auto& cdcHits = cdcTrack->getCDCHitList();
  const auto& svdHits = svdTrack->getSVDHitList();

  B2ASSERT("CDC hits must be present", not cdcHits.empty());
  B2ASSERT("SVD hits must be present", not svdHits.empty());

  const auto sortByCDCLayer = [](const CDCHit * lhs, const CDCHit * rhs) {
    return lhs->getICLayer() < rhs->getICLayer();
  };
  const auto sortBySVDLayer = [](const SVDCluster * lhs, const SVDCluster * rhs) {
    return lhs->getSensorID().getLayerNumber() < rhs->getSensorID().getLayerNumber();
  };

  const auto& firstCDCHitIterator = std::min_element(cdcHits.begin(), cdcHits.end(), sortByCDCLayer);
  const auto& firstCVDHitIterator = std::max_element(svdHits.begin(), svdHits.end(), sortBySVDLayer);

  var<named("cdc_lowest_layer")>() = (*firstCDCHitIterator)->getICLayer();
  var<named("svd_highest_layer")>() = (*firstCVDHitIterator)->getSensorID().getLayerNumber();

  var<named("number_of_hits")>() = spacePoints.size();
  var<named("number_of_holes")>() = std::count(layerUsed.begin(), layerUsed.end(), true);

  if (spacePoints.empty()) {
    var<named("last_hit_layer")>() = -1;
    var<named("first_hit_layer")>() = -1;
  } else {
    var<named("last_hit_layer")>() = spacePoints.back()->getVxdID().getLayerNumber();
    var<named("first_hit_layer")>() = spacePoints.front()->getVxdID().getLayerNumber();
  }

  genfit::MeasuredStateOnPlane mSoP = result->getMSoP();
  const genfit::MeasuredStateOnPlane& firstCDCHit = result->getSeedMSoP();
  m_advancer.extrapolateToPlane(mSoP, firstCDCHit.getPlane());

  const TVector3& distance = mSoP.getPos() - firstCDCHit.getPos();
  var<named("distance_to_cdc_track")>() = distance.Mag();
  var<named("distance_to_cdc_track_xy")>() = distance.Pt();

  const RecoTrack* relatedSVDRecoTrack = result->getRelatedSVDRecoTrack();
  if (relatedSVDRecoTrack) {
    var<named("number_of_hits_related_svd_track")>() = relatedSVDRecoTrack->getNumberOfSVDHits();
  } else {
    var<named("number_of_hits_related_svd_track")>() = -1;
  }

  var<named("chi2")>() = result->getChi2();
  var<named("chi2_vxd_max")>() = result->getMaximalChi2();
  var<named("chi2_vxd_min")>() = result->getMinimalChi2();
  var<named("chi2_cdc")>() = cdcTrack->getTrackFitStatus()->getChi2();
  return true;
}
