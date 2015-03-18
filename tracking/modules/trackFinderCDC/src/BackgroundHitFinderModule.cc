/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/BackgroundHitFinderModule.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/filters/wirehit_wirehit/WireHitNeighborChooser.h>

#include <tracking/trackFindingCDC/tmva/Expert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(BackgroundHitFinder);

BackgroundHitFinderModule::BackgroundHitFinderModule() : TrackFinderCDCBaseModule(), m_expert("data/tracking",
      "BackgroundHitFinder")
{
  setDescription("");
  addParam("ClustersStoreObjName",
           m_param_clustersStoreObjName,
           "Name of the output StoreObjPtr of the clusters generated within this module.",
           std::string("CDCWireHitClusterVector"));

  addParam("GoodCDCHitsStoreObjName",
           m_param_goodCDCHitsStoreObjName,
           "Name of the output CDCHits assumed to by good by this module.",
           std::string("GoodCDCHits"));

  addParam("TMVACut",
           m_param_tmvaCut,
           "The cut on the TMVA output.");
}

void BackgroundHitFinderModule::initialize()
{
  // Register the store arrays
  StoreWrappedObjPtr<std::vector<CDCWireHitCluster>>::registerTransient(m_param_clustersStoreObjName);

  StoreArray<CDCHit> goodCDCHits(m_param_goodCDCHitsStoreObjName);
  goodCDCHits.registerInDataStore();

  StoreArray<CDCHit> oldCDCHits("CDCHits");
  goodCDCHits.registerRelationTo(oldCDCHits);

  // initialize the reader object
  m_expert.initializeReader([&](TMVA::Reader & reader) {
    reader.AddVariable("avg_n_neignbors", &tmvaVariables.meanNNeighbors);
    reader.AddVariable("mean_drift_length", &tmvaVariables.meanDriftLength);
    reader.AddVariable("mean_inner_distance", &tmvaVariables.meanInnerDistance);
    reader.AddVariable("size", &tmvaVariables.size);
    reader.AddVariable("superlayer_id", &tmvaVariables.superlayerID);
    reader.AddVariable("total_drift_length", &tmvaVariables.totalDriftLength);
    reader.AddVariable("total_inner_distance", &tmvaVariables.totalInnerDistance);
    reader.AddVariable("total_n_neighbors", &tmvaVariables.totalNNeighbors);
    reader.AddVariable("variance_drift_length", &tmvaVariables.varianceDriftLength);
  });
}

void BackgroundHitFinderModule::event()
{
  size_t nHits = prepareHits();
  if (nHits == 0) return;

  // Generate the clusters
  generateClustersWithAutomaton();
  StoreWrappedObjPtr< std::vector<CDCWireHitCluster> > storedClusters(m_param_clustersStoreObjName);
  storedClusters.create();
  *storedClusters = m_clusters;

  // Use TMVA to differ between good and bad hit clusters and
  // save the good hit clusters in the store array
  deleteBadSegmentsUsingTMVAMethod();
}

void BackgroundHitFinderModule::deleteBadSegmentsUsingTMVAMethod()
{
  StoreArray<CDCHit> goodCDCHits(m_param_goodCDCHitsStoreObjName);
  goodCDCHits.create();

  unsigned int clusterNumber = 0;
  for (CDCWireHitCluster& cluster : m_clusters) {
    setVariables(tmvaVariables, cluster);
    double tmvaOutput = m_expert.useWeight();
    if (tmvaOutput > m_param_tmvaCut) {
      for (const CDCWireHit*  wireHit : cluster) {
        const CDCHit* cdcHit = wireHit->getHit();
        CDCHit* newAddedCDCHit = goodCDCHits.appendNew(*cdcHit);
        newAddedCDCHit->addRelationTo(cdcHit);
      }
    }
    clusterNumber++;
  }
}

void BackgroundHitFinderModule::setVariables(struct TMVAVariables& tmvaVariables, const CDCWireHitCluster& cluster)
{
  tmvaVariables.superlayerID = cluster.getISuperLayer();
  tmvaVariables.size = cluster.size();

  tmvaVariables.totalNNeighbors = 0;

  tmvaVariables.totalDriftLength = 0;
  float total_drift_length_squared = 0;

  tmvaVariables.totalInnerDistance = 0;

  for (const CDCWireHit* wireHit : cluster) {
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
    if (varianceSquared > 0)
      tmvaVariables.varianceDriftLength = std::sqrt(varianceSquared);
    else
      tmvaVariables.varianceDriftLength = 0;
  } else {
    tmvaVariables.varianceDriftLength = -1;
  }

  tmvaVariables.meanDriftLength = tmvaVariables.totalDriftLength / tmvaVariables.size;
  tmvaVariables.meanInnerDistance = tmvaVariables.totalInnerDistance / tmvaVariables.size;
  tmvaVariables.meanNNeighbors = tmvaVariables.totalNNeighbors / tmvaVariables.size;
}

void BackgroundHitFinderModule::generateClustersWithAutomaton()
{
  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  m_clusters.clear();
  m_clusters.reserve(100);

  for (const CDCWireSuperLayer& wireSuperLayer : wireTopology.getWireSuperLayers()) {
    const CDCWireHitTopology::CDCWireHitRange wireHitsInSuperlayer = wireHitTopology.getWireHits(wireSuperLayer);
    //create the neighborhood
    B2DEBUG(100, "Creating the CDCWireHit neighborhood");
    m_wirehitNeighborhood.clear();

    B2DEBUG(100, "  Append clockwise neighborhood");
    m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CW_NEIGHBOR>>(wireHitsInSuperlayer);

    B2DEBUG(100, "  Append clockwise out neighborhood");
    m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CW_OUT_NEIGHBOR>>(wireHitsInSuperlayer);

    B2DEBUG(100, "  Append clockwise in neighborhood");
    m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CW_IN_NEIGHBOR>>(wireHitsInSuperlayer);

    B2DEBUG(100, "  Append counter clockwise neighborhood");
    m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CCW_NEIGHBOR>>(wireHitsInSuperlayer);

    B2DEBUG(100, "  Append counter clockwise out neighborhood");
    m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CCW_OUT_NEIGHBOR>>(wireHitsInSuperlayer);

    B2DEBUG(100, "  Append counter clockwise in neighborhood");
    m_wirehitNeighborhood.appendUsing<WireHitNeighborChooser<CCW_IN_NEIGHBOR>>(wireHitsInSuperlayer);

    assert(m_wirehitNeighborhood.isSymmetric());

    B2DEBUG(100, "  allWirehitNeighbors.size() = " << m_wirehitNeighborhood.size());

    //create the clusters
    B2DEBUG(100, "Creating the CDCWireHit clusters");
    m_clustersInSuperLayer.clear();
    m_wirehitClusterizer.create(wireHitsInSuperlayer, m_wirehitNeighborhood, m_clustersInSuperLayer);
    B2DEBUG(100, "Created " << m_clustersInSuperLayer.size() << " CDCWireHit clusters in superlayer");

    // Move clusters to the DataStore
    m_clusters.insert(m_clusters.end(),
                      std::make_move_iterator(m_clustersInSuperLayer.begin()),
                      std::make_move_iterator(m_clustersInSuperLayer.end()));

    m_clustersInSuperLayer.clear();
    m_wirehitNeighborhood.clear();
  }
}
