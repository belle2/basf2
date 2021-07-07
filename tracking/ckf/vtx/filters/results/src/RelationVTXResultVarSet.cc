/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/vtx/filters/results/RelationVTXResultVarSet.h>

#include <tracking/dataobjects/RecoTrack.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool RelationVTXResultVarSet::extract(const CKFToVTXResult* result)
{
  const RecoTrack* vtxTrack = result->getRelatedVTXRecoTrack();
  B2ASSERT("Should have a related VTX track at this stage;", vtxTrack);

  const auto& vtxHits = vtxTrack->getVTXHitList();
  B2ASSERT("VTX hits must be present", not vtxHits.empty());

  const auto sortByVTXLayer = [](const VTXCluster * lhs, const VTXCluster * rhs) {
    return lhs->getSensorID().getLayerNumber() < rhs->getSensorID().getLayerNumber();
  };

  const auto& firstVTXHitIterator = std::max_element(vtxHits.begin(), vtxHits.end(), sortByVTXLayer);
  var<named("vtx_highest_layer")>() = (*firstVTXHitIterator)->getSensorID().getLayerNumber();

  const RecoTrack* relatedVTXRecoTrack = result->getRelatedVTXRecoTrack();
  if (relatedVTXRecoTrack) {
    var<named("number_of_hits_related_vtx_track")>() = relatedVTXRecoTrack->getNumberOfVTXHits();
  } else {
    var<named("number_of_hits_related_vtx_track")>() = -1;
  }
  return true;
}
