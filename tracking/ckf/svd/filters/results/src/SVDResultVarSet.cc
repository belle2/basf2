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
  const RecoTrack* seedTrack = result->getSeed();

  B2ASSERT("A result without a seed?", seedTrack);
  B2ASSERT("RecoTrack should be fitted at this stage!", seedTrack->wasFitSuccessful());

  const std::vector<const SpacePoint*> spacePoints = result->getHits();

  genfit::MeasuredStateOnPlane mSoP = result->getSeedMSoP();

  std::vector<unsigned int> layerUsed;
  layerUsed.resize(7, 0);

  for (const SpacePoint* spacePoint : spacePoints) {
    layerUsed[spacePoint->getVxdID().getLayerNumber()] += 1;
  }

  var<named("chi2")>() = result->getChi2();
  var<named("chi2_vxd_full")>() = result->getChi2();
  var<named("chi2_vxd_max")>() = result->getMaximalChi2();
  var<named("chi2_vxd_min")>() = result->getMinimalChi2();
  var<named("weight_sum")>() = result->getWeightSum();
  var<named("chi2_vxd_mean")>() = result->getChi2() / spacePoints.size();
  var<named("number_of_hits")>() = spacePoints.size();
  var<named("pt")>() = mSoP.getMom().Pt();
  var<named("chi2_cdc")>() = seedTrack->getTrackFitStatus()->getChi2();
  var<named("number_of_holes")>() = std::count(layerUsed.begin(), layerUsed.end(), 0);

  if (spacePoints.empty()) {
    var<named("last_hit_layer")>() = -1;
    var<named("first_hit_layer")>() = -1;
  } else {
    var<named("last_hit_layer")>() = spacePoints.back()->getVxdID().getLayerNumber();
    var<named("first_hit_layer")>() = spacePoints.front()->getVxdID().getLayerNumber();
  }

  var<named("has_missing_layer_1")>() = layerUsed[1] == 0;
  var<named("has_missing_layer_2")>() = layerUsed[2] == 0;
  var<named("has_missing_layer_3")>() = layerUsed[3] == 0;
  var<named("has_missing_layer_4")>() = layerUsed[4] == 0;
  var<named("has_missing_layer_5")>() = layerUsed[5] == 0;
  var<named("has_missing_layer_6")>() = layerUsed[6] == 0;

  var<named("number_of_overlap_hits")>() = std::count(layerUsed.begin(), layerUsed.end(), 2);

  var<named("theta")>() = mSoP.getMom().Theta();


  const genfit::MeasuredStateOnPlane& firstCDCHit = result->getSeedMSoP();
  m_advancer.extrapolateToPlane(mSoP, firstCDCHit.getPlane());

  const auto& distance = mSoP.getPos() - firstCDCHit.getPos();
  var<named("distance_to_cdc_track")>() = distance.Mag();
  var<named("distance_to_cdc_track_xy")>() = distance.Pt();



  const RecoTrack* relatedSVDRecoTrack = result->getRelatedSVDRecoTrack();
  if (relatedSVDRecoTrack) {
    var<named("number_of_hits_related_svd_track")>() = relatedSVDRecoTrack->getNumberOfSVDHits();
  } else {
    var<named("number_of_hits_related_svd_track")>() = -1;
  }

  return true;
}
