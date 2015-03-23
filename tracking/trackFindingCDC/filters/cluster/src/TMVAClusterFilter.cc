/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/TMVAClusterFilter.h"

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

TMVAClusterFilter::TMVAClusterFilter() :
  BaseClusterFilter(),
  m_param_cut(NAN),
  m_expert("data/tracking", "BackgroundHitFinder"),
  m_tmvaVariables{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  m_superLayerCenters()
{
}

void TMVAClusterFilter::initialize()
{
  // initialize the reader object
  m_expert.initializeReader([&](TMVA::Reader & reader) {
    reader.AddVariable("avg_n_neignbors", &m_tmvaVariables.meanNNeighbors);
    reader.AddVariable("distance_to_superlayer_center", &m_tmvaVariables.distanceToSuperlayerCenter);
    reader.AddVariable("is_stereo", &m_tmvaVariables.isStereo);
    reader.AddVariable("mean_drift_length", &m_tmvaVariables.meanDriftLength);
    reader.AddVariable("mean_inner_distance", &m_tmvaVariables.meanInnerDistance);
    reader.AddVariable("size", &m_tmvaVariables.size);
    reader.AddVariable("superlayer_id", &m_tmvaVariables.superlayerID);
    reader.AddVariable("total_drift_length", &m_tmvaVariables.totalDriftLength);
    reader.AddVariable("total_inner_distance", &m_tmvaVariables.totalInnerDistance);
    reader.AddVariable("total_n_neighbors", &m_tmvaVariables.totalNNeighbors);
    reader.AddVariable("variance_drift_length", &m_tmvaVariables.varianceDriftLength);
  });

  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
  prepareSuperLayerCenterArray(wireTopology);
}

void TMVAClusterFilter::terminate()
{
}

void TMVAClusterFilter::setParameters(const std::map<string, string>& parameterMap)
{
  std::string key;
  std::string value;

  for (auto& keyValue : parameterMap) {
    tie(key, value) = keyValue;
    if (key == "cut") {
      m_param_cut = stod(value);
      B2INFO("ClusterFilter received parameter 'cut' = " << m_param_cut);
    } else {
      B2WARNING("ClusterFilter received unknown parameter '" << key << "' = " << value);
    }
  }
}

void TMVAClusterFilter::prepareSuperLayerCenterArray(const CDCWireTopology& wireTopology)
{
  m_superLayerCenters.clear();
  m_superLayerCenters.reserve(wireTopology.getNSuperLayers());

  for (const CDCWireSuperLayer& superLayer : wireTopology.getWireSuperLayers()) {
    m_superLayerCenters.push_back(0.5 * (superLayer.getInnerPolarR() + superLayer.getOuterPolarR()));
  }
}

CellWeight TMVAClusterFilter::isGoodCluster(const CDCWireHitCluster& cluster)
{
  setVariables(cluster, m_tmvaVariables);
  double tmvaOutput = m_expert.useWeight();
  B2INFO("TMVAOutput " << tmvaOutput);
  if (not(tmvaOutput < m_param_cut)) {
    // Using the negation here to not cut if only the default non value is provided.
    B2INFO("Accepted");
    return tmvaOutput;
  } else {
    B2INFO("Rejected");
    return NOT_A_CELL;
  }
}

void TMVAClusterFilter::setVariables(const CDCWireHitCluster& cluster,
                                     struct TMVAVariables& tmvaVariables)
{
  unsigned int superlayerID = cluster.getISuperLayer();
  tmvaVariables.isStereo = cluster.getStereoType() != AXIAL;
  tmvaVariables.size = cluster.size();

  tmvaVariables.totalNNeighbors = 0;

  tmvaVariables.totalDriftLength = 0;
  float total_drift_length_squared = 0;

  tmvaVariables.totalInnerDistance = 0;

  for (const CDCWireHit* wireHit : cluster) {
    assert(wireHit);
    // Clusterizer writes the number of neighbors into the cell weight
    int n_neighbors = wireHit->getAutomatonCell().getCellWeight();
    tmvaVariables.totalNNeighbors += n_neighbors;

    // hit position information
    tmvaVariables.totalInnerDistance += wireHit->getRefPos2D().norm();

    // Drift circle information
    tmvaVariables.totalDriftLength += wireHit->getRefDriftLength();
    total_drift_length_squared += wireHit->getRefDriftLength() * wireHit->getRefDriftLength();
  }

  if (tmvaVariables.size > 1) {
    float varianceSquared = 1.0 / (tmvaVariables.size - 1) * (total_drift_length_squared - 1.0 * tmvaVariables.totalDriftLength *
                                                              tmvaVariables.totalDriftLength / tmvaVariables.size);
    if (varianceSquared > 0) {
      tmvaVariables.varianceDriftLength = std::sqrt(varianceSquared);
    } else {
      tmvaVariables.varianceDriftLength = 0;
    }
  } else {
    tmvaVariables.varianceDriftLength = -1;
  }

  tmvaVariables.distanceToSuperlayerCenter =  m_superLayerCenters[superlayerID] - 1.0 * tmvaVariables.totalInnerDistance /
                                              tmvaVariables.size;
  tmvaVariables.superlayerID = superlayerID;
  tmvaVariables.meanDriftLength = tmvaVariables.totalDriftLength / tmvaVariables.size;
  tmvaVariables.meanInnerDistance = tmvaVariables.totalInnerDistance / tmvaVariables.size;
  tmvaVariables.meanNNeighbors = tmvaVariables.totalNNeighbors / tmvaVariables.size;
}


