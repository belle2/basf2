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
    /** minimum weight for a cluster cell */
    unsigned short minWeight      =    24;
    /** minimum number of neighbours for a cluster core cell */
    unsigned char minPts         =    1;
    /** Consider diagonal adjacent cells as neighbors */
    bool diagonal                 =    true;
    /** Cut on the total weight of all cells in the 3d volume */
    unsigned short minTotalWeight  =    450;
    /** Cut on the peak cell weight */
    unsigned short minPeakWeight   =    32;
    /** Number of iterations of the cluster searching for each Hough space */
    unsigned char iterations      =    5;
    /** Number of deleted cells in omega in each direction of the maximum */
    unsigned char omegaTrim                =    5;
    /** Number of deleted cells in phi in each direction of the maximum */
    unsigned char phiTrim                  =    4;
    /** Number of deleted cells in theta in each direction of the maximum */
    unsigned char thetaTrim                =    4;
    /** Ordering of track parameters and position of cyclic variable (phi) */
    std::vector<bool> varCyclic = {false, true, false};
    std::vector<std::string> varLabels = {"omega", "phi", "theta"};
  };


  /** Type for found clusters */
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
    /** Get member cells in the cluster */
    std::vector<cell_index> getEntries()
    {
      return m_C;
    }
    /** Add a track-space cell to the cluster */
    void append(cell_index nextEntry)
    {
      m_C.push_back(nextEntry);
    }
    /** Relate a hit to the cluster */
    void addHit(unsigned short hit, unsigned short weight, unsigned short orient)
    {
      m_hits.push_back(hit);
      m_hitWeights.push_back(weight);
      m_orientSum += orient; /** orient == 1: axial, orient == 0: stereo */
    }
    /** Get number related axial hits */
    unsigned long getNAxial()
    {
      return m_orientSum;
    }
    /** Get number related stereo hits */
    unsigned long getNStereo()
    {
      return m_hits.size() - m_orientSum;
    }
    /** Get ids of related hits (indices of the TS StoreArray) */
    std::vector<unsigned short> getHits()
    {
      return m_hits;
    }
    /** Get weight contribution of each related hit to the cluster */
    std::vector<unsigned short> getWeights()
    {
      return m_hitWeights;
    }
    /** SimpleCluster */
  private:
    /** Cluster member cells */
    std::vector<cell_index> m_C;
    /** Dimension of the track space (3 for omega, phi, theta) */
    unsigned short m_dim;
    /** Cluster related hits ids */
    std::vector<unsigned short> m_hits;
    /** Cluster related hits weights */
    std::vector<unsigned short> m_hitWeights;
    /** Sum of related hit orientations (== number of related axials) */
    unsigned short m_orientSum;
  };


  /** Clustering module */
  class Clusterizend {
  public:
    Clusterizend()
    {
    }
    virtual ~Clusterizend() {}
    explicit Clusterizend(const clustererParams& params): m_params(params)
    {
    }

    clustererParams getParams()
    {
      return m_params;
    }

    void setPlaneShape(std::vector<ushort> planeShape)
    {
      m_dimSize = planeShape.size();
      m_planeShape = planeShape;
      m_valMax = std::vector<ushort>(m_planeShape);
      for (ushort idim = 0; idim < m_dimSize; idim++) {
        m_valMax[idim] -= 1;
      }
      m_valMax.push_back(1);

    }
    /** Next event initialization:
     * set a new hough space for clustering and track finding */
    void setNewPlane(c3array& houghmapPlain)
    {
      m_houghVals = &houghmapPlain;
      m_houghVisit = c3array(m_c3shape);
    }

    /** Clustering logic */

    /** Get neighboring cells before and after a cell in track space
     * before and after is defined along the track parameter axes given by dim. */
    bool hasBefore(cell_index entry, ushort dim);

    cell_index before(cell_index entry, ushort dim);

    bool hasAfter(cell_index entry, ushort dim);

    cell_index after(cell_index entry, ushort dim);

    void blockcheck(std::vector<cell_index>* neighbors, cell_index elem, ushort dim);

    std::vector<cell_index>  regionQuery(cell_index entry);

    std::vector<SimpleCluster> dbscan();

    void expandCluster(std::vector<cell_index>& N, SimpleCluster& C);

    std::vector<cell_index> getCandidates();

    std::pair<cell_index, unsigned long> getGlobalMax();

    void deleteMax(cell_index maxIndex);

    std::vector<SimpleCluster> makeClusters();

    std::pair<SimpleCluster, unsigned long> createCluster(cell_index maxIndex);

    unsigned long checkSurroundings(cell_index maxIndex);

    template<class T>
    std::string printVector(std::vector<T> vecX)
    {
      std::stringstream result;
      result << " ";
      for (T& elem : vecX) { result << elem << " ";}
      std::string rest;
      result >> rest;
      result >> rest;
      return result.str();
    }

    template<class T>
    std::string printCells(std::vector<T> vecX)
    {
      std::stringstream result;
      for (T& elem : vecX) { result << " {" << printVector(elem) << "}";}
      return result.str();
    }
    /** Clusterizend */
  private:
    clustererParams m_params;
    std::vector<ushort> m_planeShape;
    std::vector<ushort> m_valMax;
    ushort m_dimSize;
    boost::array<c3index, 3> m_c3shape =  {{ 40, 384, 9 }};
    c3array* m_houghVals{0};
    c3array m_houghVisit = c3array(m_c3shape);
  };
}
