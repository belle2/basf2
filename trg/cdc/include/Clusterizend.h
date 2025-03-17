/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <cmath>
#include <Math/Vector3D.h>

namespace Belle2 {
  // Struct containing the parameters for the clustering
  struct clustererParams  {
    // Cut on the total weight of all cells in the 3d volume
    unsigned short minTotalWeight = 450;
    // Cut on the peak cell weight
    unsigned short minPeakWeight = 32;
    // Number of iterations of the cluster searching for each Hough space quadrant
    unsigned char iterations = 1;
    // Number of deleted cells in omega in each direction of the maximum
    unsigned char omegaTrim = 5;
    // Number of deleted cells in phi in each direction of the maximum
    unsigned char phiTrim = 4;
    // Number of deleted cells in theta in each direction of the maximum
    unsigned char thetaTrim = 4;
    // The Hough space dimensions (set in initBins() method of the NDFinder)
    unsigned short nOmega = 40;
    unsigned short nPhi = 384;
    unsigned short nTheta = 9;
  };

  // Struct containing the bounds of a 3x3 cluster
  struct ClusterBounds {
    c3index thetaLowerBound;
    c3index thetaUpperBound;
    c3index phiLowerBound;
    c3index phiUpperBound;
    c3index omega;
  };

  // Struct containing the deletion bounds of a omega row
  struct DeletionBounds {
    c3index phiLowerBound;
    c3index phiUpperBound;
    c3index omega;
    c3index theta;
  };

  // Type for found clusters
  class SimpleCluster {
  public:
    SimpleCluster() = default;
    explicit SimpleCluster(const cell_index& newClusterCell)
    {
      appendCell(newClusterCell);
    }
    // Get member cells in the cluster
    std::vector<cell_index> getCells() const { return m_clusterCells; }
    // Add a track-space cell to the cluster
    void appendCell(const cell_index& newClusterCell) { m_clusterCells.push_back(newClusterCell); }
    // Relate a hit to the cluster
    void addHitToCluster(unsigned short hit) { m_clusterHits.push_back(hit); }
    // Get the TS hits added to this cluster
    std::vector<unsigned short> getClusterHits() const { return m_clusterHits; }

  private:
    // Cluster member cells
    std::vector<cell_index> m_clusterCells;
    // Cluster related hits ids
    std::vector<unsigned short> m_clusterHits;
  };

  // Clustering module
  class Clusterizend {
  public:
    Clusterizend() = default;
    explicit Clusterizend(const clustererParams& params): m_params(params) {}

    // Set a new hough space for clustering and track finding
    void setNewPlane(c3array& houghSpace) { m_houghSpace = &houghSpace; }
    // Main function for the fixed clustering logic
    std::vector<SimpleCluster> makeClusters();

  private:
    // Returns the global maximum (of one quadrant)
    std::pair<cell_index, unsigned long> getGlobalMax(const unsigned char quadrant);
    // Creates the cluster around the global maximum
    std::pair<SimpleCluster, unsigned long> createCluster(const cell_index& maxIndex);
    // Deletes the surroundings of such a cluster
    void deleteGlobalMax(const cell_index& maxIndex);
    // Method to add a 3x3 to the cluster in createCluster
    void addClusterCells(const ClusterBounds& bounds, SimpleCluster& fixedCluster, unsigned long& totalClusterWeight);
    // Method to delete a omega row for the cluster deletion in deleteGlobalMax
    void clearHoughSpaceRow(const DeletionBounds& bounds);
    // The struct holding the cluster parameters
    clustererParams m_params;
    // Pointer to the Hough space
    c3array* m_houghSpace{0};
  };
}
