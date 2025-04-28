/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <utility>
#include <array>

namespace Belle2 {
  // Struct containing the parameters for the clustering
  struct ClustererParameters  {
    // Cut on the total weight of all cells in the 3d volume
    unsigned short minTotalWeight = 100;
    // Cut on the peak cell weight
    unsigned short minPeakWeight = 32;
    // Number of iterations of the cluster searching for each Hough space quadrant
    unsigned short iterations = 1;
    // Number of deleted cells in omega in each direction of the peak
    unsigned short omegaTrim = 5;
    // Number of deleted cells in phi in each direction of the peak
    unsigned short phiTrim = 4;
    // The Hough space dimensions
    unsigned short nOmega = 40;
    unsigned short nPhi = 384;
    unsigned short nCot = 9;
  };

  // Type for found clusters
  class SimpleCluster {
  public:
    // Default constructor
    SimpleCluster() = default;

    // Add a track-space cell to the cluster
    void appendCell(const cell_index& newClusterCell) { m_clusterCells.push_back(newClusterCell); }
    // Relate a hit to the cluster
    void addHitToCluster(unsigned short hit) { m_clusterHits.push_back(hit); }

    // Set the peak index (found as the section peak) of the cluster
    void setPeakCell(const cell_index& peakCell) { m_clusterPeakCell = peakCell; }
    // Set the weight of the peak cluster cell
    void setPeakWeight(const unsigned int peakWeight) { m_clusterPeakWeight = peakWeight; }
    // Set the total weight of the cluster
    void setTotalWeight(const unsigned int totalWeight) { m_clusterTotalWeight = totalWeight; }

    // Get member cells in the cluster
    std::vector<cell_index> getCells() const { return m_clusterCells; }
    // Get the TS hits added to this cluster
    std::vector<unsigned short> getClusterHits() const { return m_clusterHits; }
    // Get the peak index (found as the section peak) from the cluster
    cell_index getPeakCell() const { return m_clusterPeakCell; }
    // Get the weight of the peak cluster cell
    unsigned int getPeakWeight() const { return m_clusterPeakWeight; }
    // Get the total weight of the cluster
    unsigned int getTotalWeight() const { return m_clusterTotalWeight; }

  private:
    // Cluster member cells
    std::vector<cell_index> m_clusterCells;
    // Cluster related hits ids
    std::vector<unsigned short> m_clusterHits;
    // Peak index (found as the section maximum)
    cell_index m_clusterPeakCell{{0, 0, 0}};
    // Weight of the peak index
    unsigned int m_clusterPeakWeight{0};
    // Total weight of the cluster
    unsigned int m_clusterTotalWeight{0};
  };

  // Clustering module
  class Clusterizend {
  public:
    // Struct containing the deletion bounds of a omega row
    struct DeletionBounds {
      c3index phiLowerBound;
      c3index phiUpperBound;
      c3index omega;
      c3index cot;
    };

    // Default constructor
    Clusterizend() = default;

    // To set custom clustering parameters
    explicit Clusterizend(const ClustererParameters& parameters): m_clustererParams(parameters) {}

    // Set a new Hough space for clustering and track finding
    void setNewPlane(c3array& houghSpace) { m_houghSpace = &houghSpace; }
    // Create all the clusters in the Hough space
    std::vector<SimpleCluster> makeClusters();

  private:
    // Get the phi bounds of one quadrant section
    std::array<c3index, 2> getSectionBounds(const unsigned short quadrant, const unsigned section);
    // Iterate m_clustererParams.iterations times over one section
    void iterateOverSection(const std::array<c3index, 2>& sectionBounds, std::vector<SimpleCluster>& candidateClusters);
    // Returns the global section peak index and weight
    std::pair<cell_index, unsigned int> getSectionPeak(const std::array<c3index, 2>& sectionBounds);
    // Creates the surrounding cluster (fixed shape) around the section peak index
    SimpleCluster createCluster(const cell_index& peakCell);
    // Adds the weight of all cluster cells together
    unsigned int calculateTotalWeight(const SimpleCluster& cluster);
    // Deletes the surroundings of such a cluster
    void deletePeakSurroundings(const cell_index& peakCell);
    // Method to delete a omega row for the cluster deletion in deletePeakSurroundings method
    void clearHoughSpaceRow(const DeletionBounds& bounds);
    // The struct holding the cluster parameters
    ClustererParameters m_clustererParams;
    // Pointer to the Hough space
    c3array* m_houghSpace{0};
  };
}
