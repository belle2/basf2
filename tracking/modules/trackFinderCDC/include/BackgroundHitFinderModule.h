/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils BRaun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/modules/trackFinderCDC/TrackFinderCDCBaseModule.h>
#include <tracking/trackFindingCDC/algorithms/WeightedNeighborhood.h>
#include <tracking/trackFindingCDC/algorithms/Clusterizer.h>
#include <tracking/trackFindingCDC/tmva/Expert.h>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCWireHitCluster;
    class CDCWireHit;
    class Expert;
    class CDCWireTopology;
  }

  class BackgroundHitFinderModule: public Belle2::TrackFinderCDCBaseModule {

    struct TMVAVariables {
      float superlayerID;
      float size;
      float totalNNeighbors;
      float meanNNeighbors;

      float totalDriftLength;
      float meanDriftLength;
      float varianceDriftLength;

      float totalInnerDistance;
      float meanInnerDistance;
      float isStereo;
      float distanceToSuperlayerCenter;
    } tmvaVariables;

  public:
    /// Constructor of the module. Setting up parameters and description.
    BackgroundHitFinderModule();

    /// Initialize the Module before event processing
    void initialize() override;

    /// Event method executed for each event.
    void event() override;

  private:
    /// Use the technique of the automaton to create clusters of hits
    void generateClustersWithAutomaton();

    /// Use the TMVA methods to remove background segments from the m_clusters
    void deleteBadSegmentsUsingTMVAMethod();

    /// Before using the TMVA we should set the variables calculated from the cluster
    void setVariables(struct TMVAVariables& tmvaVariables, const TrackFindingCDC::CDCWireHitCluster& cluster);

    // Prepare the super layer center array with information coming from the CDCWireTopology object.
    void prepareSuperLayerCenterArray(const TrackFindingCDC::CDCWireTopology& wireTopology);

    // parameters
    std::string m_param_clustersStoreObjName;   /**< Name of the output StoreObjPtr of the clusters generated within this module. */
    std::string m_param_goodCDCHitsStoreObjName;  /**< Name of the output CDCHits assumed to by good by this module. */
    double m_param_tmvaCut;             /**< The cut on the TMVA output. */

    // object pools
    /// Memory for the hit clusters in the current superlayer
    std::vector<TrackFindingCDC::CDCWireHitCluster> m_clustersInSuperLayer;
    /// Memory for the wire hit neighborhood.
    TrackFindingCDC::WeightedNeighborhood<const TrackFindingCDC::CDCWireHit> m_wirehitNeighborhood;
    /// Instance of the hit cluster generator
    TrackFindingCDC::Clusterizer<TrackFindingCDC::CDCWireHit, TrackFindingCDC::CDCWireHitCluster> m_wirehitClusterizer;
    /// Vector with the found clusters
    std::vector<TrackFindingCDC::CDCWireHitCluster> m_clusters;

    /// TMVA Expert to decide if a cluster is background or not
    TrackFindingCDC::Expert m_expert;

    /// the polar R of the super layer centers
    std::vector<double> m_superLayerCenters;
  };
}
