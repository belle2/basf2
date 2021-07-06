/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/AxialStraightTrackFinder.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <mdst/dataobjects/ECLCluster.h>

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
    if (cluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) { //FIXME reasonable to pick other hypothesis?
      clusters.emplace_back(&cluster);
    }
  }
  // Acquire the axial hits
  std::vector<const CDCWireHit*> axialWireHits;
  axialWireHits.reserve(wireHits.size());
  for (const CDCWireHit& wireHit : wireHits) {
    if (not wireHit.isAxial()) continue;
    axialWireHits.emplace_back(&wireHit);
  }

  m_axialStraightTrackCreator.apply(clusters, axialWireHits, tracks);

//   AxialTrackUtil::deleteShortTracks(tracks);
}

