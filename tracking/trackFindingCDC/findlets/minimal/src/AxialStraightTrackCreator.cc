/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/findlets/minimal/AxialStraightTrackCreator.h>

#include <mdst/dataobjects/ECLCluster.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

AxialStraightTrackCreator::AxialStraightTrackCreator() = default;

std::string AxialStraightTrackCreator::getDescription()
{
  return "A findlet looking for straight tracks between IP and reconstructed ECL clusters.";
}

void AxialStraightTrackCreator::exposeParameters(ModuleParamList* moduleParamList,
                                                 const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "minEnergy"),
                                m_param_minEnergy,
                                "Parameter to define minimal threshold of ECL cluster energy.",
                                m_param_minEnergy);
  moduleParamList->addParameter(prefixed(prefix, "minNHits"),
                                m_param_minNHits,
                                "Parameter to define minimal threshold of track number of hits.",
                                m_param_minNHits);
}

void AxialStraightTrackCreator::initialize()
{
  Super::initialize();
}

void AxialStraightTrackCreator::apply(const std::vector<const ECLCluster*>& eclClusters,
                                      const std::vector<const CDCWireHit*>& axialWireHits,
                                      std::vector<CDCTrack>& tracks)
{
}

void AxialStraightTrackCreator::search(const std::vector<const CDCWireHit*>& axialWireHits, const Vector3D& endPosition)
{
}
