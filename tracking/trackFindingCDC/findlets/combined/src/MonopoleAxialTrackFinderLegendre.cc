/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/MonopoleAxialTrackFinderLegendre.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

MonopoleAxialTrackFinderLegendre::MonopoleAxialTrackFinderLegendre()
{
  addProcessingSignalListener(&m_straightMonopoleAxialTrackCreatorHitLegendre);
}

std::string MonopoleAxialTrackFinderLegendre::getDescription()
{
  return "Performs the pattern recognition in the CDC with the legendre looking for straight tracks";
}

void MonopoleAxialTrackFinderLegendre::exposeParameters(ModuleParamList* moduleParamList,
                                                        const std::string& prefix)
{
  m_straightMonopoleAxialTrackCreatorHitLegendre.exposeParameters(moduleParamList, prefix);
}

void MonopoleAxialTrackFinderLegendre::apply(const std::vector<CDCWireHit>& wireHits,
                                             std::vector<CDCTrack>& tracks)
{
  B2DEBUG(100, "**********   CDCTrackingModule  ************");

  // Acquire the axial hits
  std::vector<const CDCWireHit*> axialWireHits;
  axialWireHits.reserve(wireHits.size());
  for (const CDCWireHit& wireHit : wireHits) {
    wireHit->unsetTemporaryFlags();
    wireHit->unsetMaskedFlag();
    if (not wireHit.isAxial()) continue;
    if (wireHit->hasBackgroundFlag()) continue;
    axialWireHits.emplace_back(&wireHit);
  }

  // Legendre pass
  m_straightMonopoleAxialTrackCreatorHitLegendre.apply(axialWireHits, tracks);

//   AxialTrackUtil::deleteShortTracks(tracks);
}
