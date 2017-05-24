/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcTrackSpacePointCombination/CDCVXDTrackCombinationVarSet.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool CDCVXDTrackCombinationVarSet::extract(const BaseCDCVXDTrackCombinationFilter::Object* result)
{
  RecoTrack* cdcTrack = result->first;
  if (not cdcTrack) return false;

  B2ASSERT("RecoTrack should be fitted at this stage!", cdcTrack->wasFitSuccessful());

  const std::vector<const SpacePoint*> spacePoints = result->second;

  genfit::MeasuredStateOnPlane mSoP = cdcTrack->getMeasuredStateOnPlaneFromFirstHit();

  double chi2_vxd_full = 0;
  double chi2_vxd_max = std::nan("");
  double chi2_vxd_min = std::nan("");


  std::vector<bool> layerUsed;
  layerUsed.resize(7, false);

  for (const SpacePoint* spacePoint : spacePoints) {
    layerUsed[spacePoint->getVxdID().getLayerNumber()] = true;

    if (not m_advanceAlgorithm.extrapolate(mSoP, spacePoint)) {
      return std::nan("");
    }
    const double chi2 = m_kalmanAlgorithm.kalmanStep(mSoP, spacePoint);

    chi2_vxd_full += chi2;

    if (chi2 > chi2_vxd_max or std::isnan(chi2_vxd_max)) {
      chi2_vxd_max = chi2;
    }

    if (chi2 < chi2_vxd_min or std::isnan(chi2_vxd_min)) {
      chi2_vxd_min = chi2;
    }
  }

  var<named("weight")>() = result->getWeight();
  var<named("chi2_vxd_full")>() = chi2_vxd_full;
  var<named("chi2_vxd_max")>() = chi2_vxd_max;
  var<named("chi2_vxd_min")>() = chi2_vxd_min;
  var<named("chi2_vxd_mean")>() = chi2_vxd_full / spacePoints.size();
  var<named("number_of_hits")>() = spacePoints.size();
  var<named("prob")>() = 0; // TODO
  var<named("pt")>() = mSoP.getMom().Pt();
  var<named("chi2_cdc")>() = cdcTrack->getTrackFitStatus()->getChi2();
  var<named("number_of_holes")>() = std::count(layerUsed.begin(), layerUsed.end(), false) - 3;

  if (spacePoints.empty()) {
    var<named("last_hit_layer")>() = -1;
    var<named("first_hit_layer")>() = -1;
  } else {
    var<named("last_hit_layer")>() = spacePoints.front()->getVxdID().getLayerNumber();
    var<named("first_hit_layer")>() = spacePoints.back()->getVxdID().getLayerNumber();
  }

  return true;
}
