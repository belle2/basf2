/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/vtx/filters/results/VTXResultVarSet.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

VTXResultVarSet::VTXResultVarSet() : Super()
{
  addProcessingSignalListener(&m_advancer);
}

void VTXResultVarSet::initialize()
{
  ModuleParamList moduleParamList;
  m_advancer.exposeParameters(&moduleParamList, "");
  moduleParamList.getParameter<std::string>("direction").setValue("both");

  Super::initialize();
}

bool VTXResultVarSet::extract(const CKFToVTXResult* result)
{
  const TVector3& resultMomentum = result->getMomentum();
  var<named("pt")>() = resultMomentum.Pt();
  var<named("theta")>() = resultMomentum.Theta();

  const std::vector<const SpacePoint*>& spacePoints = result->getHits();

  var<named("number_of_hits")>() = spacePoints.size();

  std::vector<bool> layerUsed;
  layerUsed.resize(8, false);

  for (const SpacePoint* spacePoint : spacePoints) {
    layerUsed[spacePoint->getVxdID().getLayerNumber()] = true;
  }
  var<named("number_of_used_layers")>() = std::count(layerUsed.begin(), layerUsed.end(), true);

  // ATTENTION: Here we have a hardcoded number of layers. Change if there is a different number of layers in the VTX
  var<named("has_missing_layer_1")>() = layerUsed[1] == 0;
  var<named("has_missing_layer_2")>() = layerUsed[2] == 0;
  var<named("has_missing_layer_3")>() = layerUsed[3] == 0;
  var<named("has_missing_layer_4")>() = layerUsed[4] == 0;
  var<named("has_missing_layer_5")>() = layerUsed[5] == 0;
  var<named("has_missing_layer_6")>() = layerUsed[6] == 0;
  var<named("has_missing_layer_7")>() = layerUsed[7] == 0;

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

  const RecoTrack* seedTrack = result->getSeed();

  var<named("chi2")>() = result->getChi2();
  var<named("chi2_vtx_max")>() = result->getMaximalChi2();
  var<named("chi2_vtx_min")>() = result->getMinimalChi2();
  var<named("chi2_cdc")>() = seedTrack->getTrackFitStatus()->getChi2();
  var<named("weight_sum")>() = result->getWeightSum();
  return true;
}
