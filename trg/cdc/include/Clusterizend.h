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
    std::vector<cell_index> getEntries()
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
    unsigned long getNAxial()
    {
      return m_orientSum;
    }
    /* Get number related stereo hits */
    unsigned long getNStereo()
    {
      return m_hits.size() - m_orientSum;
    }
    /* Get ids of related hits (indices of the TS StoreArray) */
    std::vector<unsigned short> getHits()
    {
      return m_hits;
    }
    /* Get weight contribution of each related hit to the cluster */
    std::vector<unsigned short> getWeights()
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

    /* Returns the struct containing the clustering parameters */
    clustererParams getParams()
    {
      return m_params;
    }

    /* Event initialization: */
    /* Set a new hough space for clustering and track finding */
    void setNewPlane(c3array& houghmapPlain)
    {
      m_houghVals = &houghmapPlain;
    }

    /* Clustering logic */
    /* Main function for the fixed clustering logic */
    std::vector<SimpleCluster> makeClusters();
    /* Returns the global maximum (of one quadrant) */
    std::pair<cell_index, unsigned long> getGlobalMax(unsigned char quadrant);
    /* Creates the cluster around the global maximum */
    std::pair<SimpleCluster, unsigned long> createCluster(cell_index maxIndex);
    /* Deletes the surroundings of such a cluster */
    void deleteMax(cell_index maxIndex);

    /* Clusterizend */
  private:
    clustererParams m_params;
    /* Pointer to the Hough space */
    c3array* m_houghVals{0};
  };
}
