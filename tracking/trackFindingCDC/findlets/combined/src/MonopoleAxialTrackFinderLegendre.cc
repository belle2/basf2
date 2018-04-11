/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth, Viktor Trusov,       *
 *               Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/MonopoleAxialTrackFinderLegendre.h>

#include <tracking/trackFindingCDC/processing/AxialTrackUtil.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

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
