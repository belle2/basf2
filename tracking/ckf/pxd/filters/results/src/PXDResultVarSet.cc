/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/results/PXDResultVarSet.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

PXDResultVarSet::PXDResultVarSet() : Super()
{
  addProcessingSignalListener(&m_advancer);
}

void PXDResultVarSet::initialize()
{
  ModuleParamList moduleParamList;
  m_advancer.exposeParameters(&moduleParamList, "");
  moduleParamList.getParameter<std::string>("direction").setValue("both");

  Super::initialize();
}

bool PXDResultVarSet::extract(const CKFToPXDResult* result)
{
  const ROOT::Math::XYZVector& resultMomentum = result->getMomentum();
  var<named("pt")>() = resultMomentum.Rho();
  var<named("theta")>() = resultMomentum.Theta();

  const std::vector<const SpacePoint*>& spacePoints = result->getHits();

  var<named("number_of_hits")>() = spacePoints.size();

  std::vector<bool> layerUsed;
  layerUsed.resize(7, false);

  for (const SpacePoint* spacePoint : spacePoints) {
    layerUsed[spacePoint->getVxdID().getLayerNumber()] = true;
  }
  // Counting the occurences of 'true' rather counts the number of layers used,
  // not the number of holes. But renaming this variable would break the MVA-based result filter.
  // Could be renamed if the weight file for the MVA result filter was retrained afterwards and
  // the new weight file was then uploaded to the DB and would replace the current one.
  var<named("number_of_holes")>() = std::count(layerUsed.begin(), layerUsed.end(), true);

  var<named("has_missing_layer_1")>() = layerUsed[1] == 0;
  var<named("has_missing_layer_2")>() = layerUsed[2] == 0;
  var<named("has_missing_layer_3")>() = layerUsed[3] == 0;
  var<named("has_missing_layer_4")>() = layerUsed[4] == 0;
  var<named("has_missing_layer_5")>() = layerUsed[5] == 0;
  var<named("has_missing_layer_6")>() = layerUsed[6] == 0;

  // Since the vector layerUsed only contains bool values, there can be no number 2, so this
  // variable is basically useless. But also here, just removing the variable would likely break
  // the MVA-based result filter, so this could only be removed in case the MVA is retrained and
  // the new weight file was uploaded to the DB.
  var<named("number_of_overlap_hits")>() = std::count(layerUsed.begin(), layerUsed.end(), 2);

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

  const auto& distance = mSoP.getPos() - firstCDCHit.getPos();
  var<named("distance_to_seed_track")>() = distance.Mag();
  var<named("distance_to_seed_track_xy")>() = distance.Pt();

  const RecoTrack* seedTrack = result->getSeed();

  var<named("chi2")>() = result->getChi2();
  var<named("chi2_vxd_max")>() = result->getMaximalChi2();
  var<named("chi2_vxd_min")>() = result->getMinimalChi2();
  var<named("chi2_seed")>() = seedTrack->getTrackFitStatus()->getChi2();
  var<named("weight_sum")>() = result->getWeightSum();
  return true;
}
