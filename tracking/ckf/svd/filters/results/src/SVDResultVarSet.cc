/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/results/SVDResultVarSet.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/core/ModuleParamList.templateDetails.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

SVDResultVarSet::SVDResultVarSet() : Super()
{
  addProcessingSignalListener(&m_advancer);
}

void SVDResultVarSet::initialize()
{
  ModuleParamList moduleParamList;
  m_advancer.exposeParameters(&moduleParamList, "");
  moduleParamList.getParameter<std::string>("direction").setValue("both");

  Super::initialize();
}

bool SVDResultVarSet::extract(const CKFToSVDResult* result)
{
  const TVector3& resultMomentum = result->getMomentum();
  var<named("pt")>() = resultMomentum.Pt();
  var<named("theta")>() = resultMomentum.Theta();

  const std::vector<const SpacePoint*>& spacePoints = result->getHits();

  var<named("number_of_hits")>() = spacePoints.size();

  std::vector<bool> layerUsed;
  layerUsed.resize(7, false);

  for (const SpacePoint* spacePoint : spacePoints) {
    layerUsed[spacePoint->getVxdID().getLayerNumber()] = true;
  }
  var<named("number_of_holes")>() = std::count(layerUsed.begin(), layerUsed.end(), true);

  var<named("has_missing_layer_1")>() = layerUsed[1] == 0;
  var<named("has_missing_layer_2")>() = layerUsed[2] == 0;
  var<named("has_missing_layer_3")>() = layerUsed[3] == 0;
  var<named("has_missing_layer_4")>() = layerUsed[4] == 0;
  var<named("has_missing_layer_5")>() = layerUsed[5] == 0;
  var<named("has_missing_layer_6")>() = layerUsed[6] == 0;

  if (spacePoints.empty()) {
    var<named("last_hit_layer")>() = -1;
    var<named("first_hit_layer")>() = -1;
  } else {
    var<named("last_hit_layer")>() = spacePoints.back()->getVxdID().getLayerNumber();
    var<named("first_hit_layer")>() = spacePoints.front()->getVxdID().getLayerNumber();
  }

  const RecoTrack* cdcTrack = result->getSeed();
  const auto& cdcHits = cdcTrack->getCDCHitList();
  B2ASSERT("CDC hits must be present", not cdcHits.empty());

  const auto sortByCDCLayer = [](const CDCHit * lhs, const CDCHit * rhs) {
    return lhs->getICLayer() < rhs->getICLayer();
  };

  const auto& firstCDCHitIterator = std::min_element(cdcHits.begin(), cdcHits.end(), sortByCDCLayer);
  var<named("cdc_lowest_layer")>() = (*firstCDCHitIterator)->getICLayer();

  genfit::MeasuredStateOnPlane mSoP = result->getMSoP();
  const genfit::MeasuredStateOnPlane& firstCDCHit = result->getSeedMSoP();
  m_advancer.extrapolateToPlane(mSoP, firstCDCHit.getPlane());

  const auto& distance = mSoP.getPos() - firstCDCHit.getPos();
  var<named("distance_to_cdc_track")>() = distance.Mag();
  var<named("distance_to_cdc_track_xy")>() = distance.Pt();

  var<named("chi2")>() = result->getChi2();
  var<named("chi2_vxd_max")>() = result->getMaximalChi2();
  var<named("chi2_vxd_min")>() = result->getMinimalChi2();
  var<named("chi2_cdc")>() = cdcTrack->getTrackFitStatus()->getChi2();
  var<named("weight_sum")>() = result->getWeightSum();
  return true;
}
