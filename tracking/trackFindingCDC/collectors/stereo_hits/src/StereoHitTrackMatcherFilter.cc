/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/collectors/stereo_hits/StereoHitTrackMatcherFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

void StereoHitTrackMatcherFilter::exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix)
{
  FilterBasedMatcher<StereoHitFilterFactory>::exposeParameters(moduleParameters, prefix);
  moduleParameters->addParameter(prefixed(prefix, "checkForB2BTracks"),
                                 m_param_checkForB2BTracks,
                                 "Set to false to skip the check for back-2-back tracks (good for cosmics)",
                                 m_param_checkForB2BTracks);

}

std::vector<WithWeight<const CDCRLWireHit*> >
StereoHitTrackMatcherFilter::match(const CDCTrack& track,
                                   const std::vector<CDCRLWireHit>& rlWireHits)
{
  if (m_filter.needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }

  // List of matches
  std::vector<WithWeight<const CDCRLWireHit*> > matches;

  // Reconstruct the hits to the track and check their filter result
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const double radius = trajectory2D.getGlobalCircle().absRadius();
  const bool isCurler = trajectory2D.isCurler();

  /*
   * Use the given trajectory to reconstruct the 2d hits in the vector in z direction
   * to match the trajectory perfectly. Then add the newly created reconstructed 3D hit to the matches, if the filter says "yes".
   */
  for (const CDCRLWireHit& rlWireHit : rlWireHits) {
    if (rlWireHit.getStereoKind() != EStereoKind::c_Axial and
        not rlWireHit.getWireHit().getAutomatonCell().hasTakenFlag()) {
      Vector3D recoPos3D = rlWireHit.reconstruct3D(trajectory2D);
      const CDCWire& wire = rlWireHit.getWire();
      // Skip hits out of CDC
      if (not wire.isInCellZBounds(recoPos3D)) {
        continue;
      }

      // If the track is a curler, shift all perpS values to positive ones. Else do not use this hit if m_param_checkForB2BTracks is enabled.
      double perpS = trajectory2D.calcArcLength2D(recoPos3D.xy());
      if (perpS < 0) {
        if (isCurler) {
          perpS += 2 * TMath::Pi() * radius;
        } else if (m_param_checkForB2BTracks) {
          continue;
        }
      }
      CDCRecoHit3D reconstructedHit(rlWireHit, recoPos3D, perpS);

      const Weight weight = m_filter({&reconstructedHit, &track});
      if (not std::isnan(weight)) {
        matches.emplace_back(&rlWireHit, weight);
      }
    }
  }

  return matches;
}
