/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/AxialStraightTrackFinder.h>

#include <tracking/trackFindingCDC/processing/AxialTrackUtil.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <mdst/dataobjects/ECLCluster.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

using namespace Belle2;
using namespace TrackFindingCDC;

AxialStraightTrackFinder::AxialStraightTrackFinder()
{
  addProcessingSignalListener(&m_axialStraightTrackCreator);
}

void AxialStraightTrackFinder::initialize()
{
  m_storeArrayClusters.isRequired();

  Super::initialize();
}

std::string AxialStraightTrackFinder::getDescription()
{
  return "Performs a search for straight tracks coming from IP and ending in an ECL cluster";
}

void AxialStraightTrackFinder::exposeParameters(ModuleParamList* moduleParamList,
                                                const std::string& prefix)
{
  m_axialStraightTrackCreator.exposeParameters(moduleParamList, prefix);
}

void AxialStraightTrackFinder::apply(const std::vector<CDCWireHit>& wireHits,
                                     std::vector<CDCTrack>& tracks)
{
  B2DEBUG(100, "**********   CDCTrackingModule  ************");

// Acquire ecl clusters
  std::vector<const ECLCluster*> clusters;
  clusters.reserve(m_storeArrayClusters.getEntries());
  for (const ECLCluster& cluster : m_storeArrayClusters) {
    clusters.emplace_back(&cluster);
  }
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

  m_axialStraightTrackCreator.apply(clusters, axialWireHits, tracks);

//   AxialTrackUtil::deleteShortTracks(tracks);
}

