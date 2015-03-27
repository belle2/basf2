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
  m_expert("data/tracking", "BackgroundHitFinder")
{
}

void TMVAClusterFilter::initialize()
{
  m_varset.initialize();

  // initialize the reader object
  m_expert.initializeReader([&](TMVA::Reader & reader) {
    std::vector<NamedFloatTuple*> allVariables = m_varset.getAllVariables();
    for (NamedFloatTuple* variables : allVariables) {
      size_t nVars = variables->size();
      for (size_t iVar = 0; iVar < nVars; ++iVar) {
        std::string name = variables->getNameWithPrefix(iVar);
        TString tName(name.c_str());
        Float_t& value = (*variables)[iVar];
        reader.AddVariable(tName, &value);
      }
    }
  });

  //   reader.AddVariable("avg_n_neignbors", &m_tmvaVariables.meanNNeighbors);
  //   reader.AddVariable("distance_to_superlayer_center", &m_tmvaVariables.distanceToSuperlayerCenter);
  //   reader.AddVariable("is_stereo", &m_tmvaVariables.isStereo);
  //   reader.AddVariable("mean_drift_length", &m_tmvaVariables.meanDriftLength);
  //   reader.AddVariable("mean_inner_distance", &m_tmvaVariables.meanInnerDistance);
  //   reader.AddVariable("size", &m_tmvaVariables.size);
  //   reader.AddVariable("superlayer_id", &m_tmvaVariables.superlayerID);
  //   reader.AddVariable("total_drift_length", &m_tmvaVariables.totalDriftLength);
  //   reader.AddVariable("total_inner_distance", &m_tmvaVariables.totalInnerDistance);
  //   reader.AddVariable("total_n_neighbors", &m_tmvaVariables.totalNNeighbors);
  //   reader.AddVariable("variance_drift_length", &m_tmvaVariables.varianceDriftLength);
  // });
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

CellWeight TMVAClusterFilter::isGoodCluster(const CDCWireHitCluster& cluster)
{
  bool extracted = m_varset.extract(&cluster);
  if (not extracted) return NOT_A_CELL;
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
