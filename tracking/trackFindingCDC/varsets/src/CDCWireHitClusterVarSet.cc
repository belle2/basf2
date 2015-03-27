/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/CDCWireHitClusterVarSet.h"

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

/// Important to define the constexpr here such that it gains external linkage.
constexpr const char* const CDCWireHitClusterVarNames::names[nNames];

CDCWireHitClusterVarSet::CDCWireHitClusterVarSet(const std::string& prefix) :
  VarSet<CDCWireHitClusterVarNames>(prefix),
  m_superLayerCenters()
{
}

void CDCWireHitClusterVarSet::initialize()
{
  prepareSuperLayerCenterArray();
}

bool CDCWireHitClusterVarSet::extract(const CDCWireHitCluster* ptrCluster)
{
  if (not ptrCluster) return false;
  const CDCWireHitCluster& cluster = *ptrCluster;

  unsigned int superlayerID = cluster.getISuperLayer();
  unsigned int size =  cluster.size();

  int totalNNeighbors = 0;
  double totalInnerDistance = 0;
  double totalDriftLength = 0;
  double totalDriftLengthSquared = 0;
  double driftVariance = 0;
  for (const CDCWireHit* wireHit : cluster) {
    assert(wireHit);
    // Clusterizer writes the number of neighbors into the cell weight
    int nNeighbors = wireHit->getAutomatonCell().getCellWeight();
    totalNNeighbors += nNeighbors;

    // hit position informnamedion
    totalInnerDistance += wireHit->getRefPos2D().norm();

    // Drift circle informnamedion
    double driftLength = wireHit->getRefDriftLength();
    totalDriftLength += driftLength;
    totalDriftLengthSquared += driftLength * driftLength;
  }

  if (size > 1) {
    double varianceSquared = (totalDriftLengthSquared - totalDriftLength * totalDriftLength / size)  / (size - 1.0) ;

    if (varianceSquared > 0) {
      driftVariance = std::sqrt(varianceSquared);

    } else {
      driftVariance = 0;

    }

  } else {
    driftVariance = -1;
  }

  var<named("is_stereo")>() = cluster.getStereoType() != AXIAL;
  var<named("size")>() = size;

  var<named("total_n_neighbors")>() = totalNNeighbors;
  var<named("total_drift_length")>() = totalDriftLength;
  var<named("total_inner_distance")>() = totalInnerDistance;
  var<named("variance_drift_length")>() = driftVariance;

  var<named("distance_to_superlayer_center")>() = m_superLayerCenters[superlayerID] - totalInnerDistance / size;
  var<named("superlayer_id")>() = superlayerID;
  var<named("mean_drift_length")>() = totalDriftLength / size;
  var<named("mean_inner_distance")>() = totalInnerDistance / size;
  var<named("avg_n_neignbors")>() = 1.0 * totalNNeighbors / size;
  return true;
}

void CDCWireHitClusterVarSet::prepareSuperLayerCenterArray()
{
  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  m_superLayerCenters.clear();
  m_superLayerCenters.reserve(wireTopology.getNSuperLayers());

  for (const CDCWireSuperLayer& superLayer : wireTopology.getWireSuperLayers()) {
    Float_t superLayerCenter = superLayer.getMiddleCylindricalR();
    m_superLayerCenters.push_back(superLayerCenter);
  }
}
