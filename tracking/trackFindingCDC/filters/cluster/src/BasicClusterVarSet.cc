/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/cluster/BasicClusterVarSet.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <cdc/dataobjects/CDCHit.h>

#include <cassert>

using namespace Belle2;
using namespace TrackFindingCDC;

bool BasicClusterVarSet::extract(const CDCWireHitCluster* ptrCluster)
{
  if (not ptrCluster) return false;
  const CDCWireHitCluster& cluster = *ptrCluster;

  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
  ISuperLayer iSuperLayer = ISuperLayerUtil::getFrom(cluster.front());
  if (not ISuperLayerUtil::isInCDC(iSuperLayer)) {
    return false;
  }
  const CDCWireSuperLayer& superLayer = wireTopology.getWireSuperLayer(iSuperLayer);
  double superLayerCenter = superLayer.getMiddleCylindricalR();
  unsigned int size =  cluster.size();

  int totalNNeighbors = 0;
  double totalInnerDistance = 0;
  double totalDriftLength = 0;
  double totalDriftLengthSquared = 0;
  double driftVariance = 0;
  double totalADCCount = 0;
  double totalADCCountSquared = 0;
  double adcCountVariance;

  for (const CDCWireHit* wireHit : cluster) {
    assert(wireHit);
    // Clusterizer writes the number of neighbors into the cell weight
    int nNeighbors = wireHit->getAutomatonCell().getCellWeight();
    totalNNeighbors += nNeighbors;

    // hit position information
    totalInnerDistance += wireHit->getRefPos2D().norm();

    // Drift circle information
    double driftLength = wireHit->getRefDriftLength();
    totalDriftLength += driftLength;
    totalDriftLengthSquared += driftLength * driftLength;

    // ADC information
    double adc = static_cast<double>(wireHit->getHit()->getADCCount());
    totalADCCount += adc;
    totalADCCountSquared += adc * adc;
  }
  if (size > 1) {
    double driftLengthVarianceSquared =
      (totalDriftLengthSquared - totalDriftLength * totalDriftLength / size) / (size - 1.0);
    double adcVarianceSquared =
      (totalADCCountSquared - totalADCCount * totalADCCount / size) / (size - 1.0);

    if (driftLengthVarianceSquared > 0) {
      driftVariance = std::sqrt(driftLengthVarianceSquared);
    } else {
      driftVariance = 0;
    }

    if (adcVarianceSquared > 0) {
      adcCountVariance = std::sqrt(adcVarianceSquared);
    } else {
      adcCountVariance = 0;
    }

  } else {
    driftVariance = -1;
    adcCountVariance = -1;
  }

  var<named("is_stereo")>() = not ISuperLayerUtil::isAxial(iSuperLayer);
  var<named("size")>() = size;

  var<named("total_number_of_neighbors")>() = totalNNeighbors;
  var<named("total_drift_length")>() = totalDriftLength;
  var<named("total_adc_count")>() = totalADCCount;
  var<named("total_inner_distance")>() = totalInnerDistance;

  var<named("variance_drift_length")>() = driftVariance;
  var<named("variance_adc_count")>() = adcCountVariance;

  var<named("distance_to_superlayer_center")>() = superLayerCenter - totalInnerDistance / size;
  var<named("superlayer_id")>() = iSuperLayer;

  var<named("mean_drift_length")>() = totalDriftLength / size;
  var<named("mean_adc_count")>() = totalADCCount / size;
  var<named("mean_inner_distance")>() = totalInnerDistance / size;
  var<named("mean_number_of_neighbors")>() = 1.0 * totalNNeighbors / size;
  return true;
}
