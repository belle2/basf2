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
  /* Struct containing the parameters for the clustering */
  struct clustererParams  {
    /* Cut on the total weight of all cells in the 3d volume */
    unsigned short minTotalWeight = 450;
    /* Cut on the peak cell weight */
    unsigned short minPeakWeight = 32;
    /* Number of iterations of the cluster searching for each Hough space quadrant */
    unsigned char iterations = 2;
    /* Number of deleted cells in omega in each direction of the maximum */
    unsigned char omegaTrim = 5;
    /* Number of deleted cells in phi in each direction of the maximum */
    unsigned char phiTrim = 4;
    /* Number of deleted cells in theta in each direction of the maximum */
    unsigned char thetaTrim = 4;
    /* The Hough space dimensions (set in initBins() method of the NDFinder) */
    unsigned short nOmega = 40;
    unsigned short nPhi = 384;
    unsigned short nTheta = 9;
  };

  /* Struct containing the bounds of a 3x3 cluster */
  struct ClusterBounds {
    c3index thetaLowerBound;
    c3index thetaUpperBound;
    c3index phiLowerBound;
    c3index phiUpperBound;
    c3index omega;
  };

  /* Struct containing the deletion bounds of a omega row */
  struct DeletionBounds {
    c3index phiLowerBound;
    c3index phiUpperBound;
    c3index omega;
    c3index theta;
  };

  /* Type for found clusters */
  class SimpleCluster {
  public:
    SimpleCluster()
    {
      setParams(3);
      initClCellsNew();
    }
    explicit SimpleCluster(cell_index entry)
    {
      setParams(3);
      initClCellsNew();
      append(entry);
    }
    virtual ~SimpleCluster() {}
    void initClCellsNew()
    {
      unsigned short init_ClSize = 0;
      unsigned short defaultValue = 0;
      cell_index cell(m_dim, defaultValue);
      std::vector<cell_index> C(init_ClSize, cell);
      m_C = C;
    }
    void setParams(unsigned short dim)
    {
      m_dim = dim;
      m_orientSum = 0;
    }
    /* Get member cells in the cluster */
    std::vector<cell_index> getEntries() const
    {
      return m_C;
    }
    /* Add a track-space cell to the cluster */
    void append(cell_index nextEntry)
    {
      m_C.push_back(nextEntry);
    }
    /* Relate a hit to the cluster */
    void addHit(unsigned short hit, unsigned short weight, unsigned short orient)
    {
      m_hits.push_back(hit);
      m_hitWeights.push_back(weight);
      m_orientSum += orient; /* orient == 1: axial, orient == 0: stereo */
    }
    /* Get number related axial hits */
    unsigned long getNAxial() const
    {
      return m_orientSum;
    }
    /* Get number related stereo hits */
    unsigned long getNStereo() const
    {
      return m_hits.size() - m_orientSum;
    }
    /* Get ids of related hits (indices of the TS StoreArray) */
    std::vector<unsigned short> getHits() const
    {
      return m_hits;
    }
    /* Get weight contribution of each related hit to the cluster */
    std::vector<unsigned short> getWeights() const
    {
      return m_hitWeights;
    }
    /* SimpleCluster */
  private:
    /* Cluster member cells */
    std::vector<cell_index> m_C;
    /* Dimension of the track space (3 for omega, phi, theta) */
    unsigned short m_dim;
    /* Cluster related hits ids */
    std::vector<unsigned short> m_hits;
    /* Cluster related hits weights */
    std::vector<unsigned short> m_hitWeights;
    /* Sum of related hit orientations (== number of related axials) */
    unsigned short m_orientSum;
  };


  /* Clustering module */
  class Clusterizend {
  public:
    Clusterizend() {}
    virtual ~Clusterizend() {}
    explicit Clusterizend(const clustererParams& params): m_params(params) {}

    /* Set a new hough space for clustering and track finding */
    void setNewPlane(c3array& houghSpace)
    {
      m_houghSpace = &houghSpace;
    }

    /* Clustering logic */
    /* Main function for the fixed clustering logic */
    std::vector<SimpleCluster> makeClusters();
    /* Returns the global maximum (of one quadrant) */
    std::pair<cell_index, unsigned long> getGlobalMax(const unsigned char quadrant);
    /* Creates the cluster around the global maximum */
    std::pair<SimpleCluster, unsigned long> createCluster(const cell_index& maxIndex);
    /* Deletes the surroundings of such a cluster */
    void deleteMax(const cell_index& maxIndex);

    /* Clusterizend */
  private:
    /* Method to add a 3x3 to the cluster in createCluster */
    void addClusterCells(const ClusterBounds& bounds, SimpleCluster& fixedCluster, unsigned long& totalClusterWeight);
    /* Method to delete a omega row for the cluster deletion in deleteMax */
    void clearHoughSpaceRow(const DeletionBounds& bounds);
    /* The struct holding the cluster parameters */
    clustererParams m_params;
    /* Pointer to the Hough space */
    c3array* m_houghSpace{0};
  };
}
