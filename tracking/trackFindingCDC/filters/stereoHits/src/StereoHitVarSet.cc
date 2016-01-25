/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool StereoHitVarSet::extract(const std::pair<const CDCRecoHit3D*, const CDCTrack*>* testPair)
{
  const CDCRecoHit3D* recoHit = testPair->first;
  const CDCTrack* track = testPair->second;

  bool extracted = extractNested(testPair);
  if (not extracted or not testPair or not recoHit or not track) return false;


  const CDCTrajectory2D& trajectory2D = track->getStartTrajectory3D().getTrajectory2D();
  const double size = track->size();

  var<named("size")>() = size;
  setVariableIfNotNaN<named("pt")>(trajectory2D.getAbsMom2D());

  return true;
}
