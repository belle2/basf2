/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sebastian Skambraks                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CLUSTERIZEND_H
#define CLUSTERIZEND_H

#include <cmath>


namespace Belle2 {
  struct clusterer_params  {
    unsigned short minweight      =    24; //27; //28;
    /** minimum number of neighbours for a cluster cell */
    unsigned short minpts         =    1; //2;
    bool diagonal                 =    true;
    std::vector<bool> var_cyclic = {false, true, false};
    std::vector<std::string> var_labels = {"omega", "phi", "theta"};
  };


  class SimpleCluster {
  public:
    SimpleCluster()
    {
      setParams(3);
      initClCellsNew();
    }
    SimpleCluster(cell_index entry)
    {
      setParams(3);
      initClCellsNew();
      append(entry);
    }
    virtual ~SimpleCluster() {}
    void initClCellsNew()
    {
      unsigned short init_ClSize = 0;
      unsigned short default_value = 0;
      cell_index cell(m_dim, default_value);
      std::vector<cell_index> C(init_ClSize, cell);
      m_C = C;
      vecOne oneAxis(init_ClSize, default_value);
    }
    void setParams(unsigned short dim)
    {
      m_dim = dim;
    }
    std::vector<cell_index> getEntries()
    {
      return m_C;
    }
    void append(cell_index next_entry)
    {
      m_C.push_back(next_entry);
    }
    void add_hit(unsigned short hit, unsigned short weight)
    {
      m_hits.push_back(hit);
      m_hitWeights.push_back(weight);
    }
    std::vector<unsigned short> get_hits()
    {
      return m_hits;
    }
    std::vector<unsigned short> get_weights()
    {
      return m_hitWeights;
    }
    /** SimpleCluster */
  private:
    std::vector<cell_index> m_C;
    unsigned short m_dim;
    std::vector<unsigned short> m_hits;
    std::vector<unsigned short> m_hitWeights;
  };


  class Clusterizend {
  public:
    Clusterizend()
    {
    }
    virtual ~Clusterizend() {}
    Clusterizend(clusterer_params params)
    {
      m_params = params;
    }

    clusterer_params getParams()
    {
      return m_params;
    }

    void setPlaneShape(std::vector<ushort> planeShape)
    {
      m_dimsize = planeShape.size();
      m_planeShape = planeShape;
      //m_planeShape.push_back(2); // visited?
      m_valmax = std::vector<ushort>(m_planeShape);
      for (ushort idim = 0; idim < m_dimsize; idim++) {
        m_valmax[idim] -= 1;
      }
      m_valmax.push_back(1);

    }
    void setNewPlane(c3array& houghmap_plain)
    {
      m_houghVals = &houghmap_plain;
      m_houghVisit = c3array(m_c3shape);
    }

    bool has_before(cell_index entry, ushort dim);

    cell_index before(cell_index entry, ushort dim);

    bool has_after(cell_index entry, ushort dim);

    cell_index after(cell_index entry, ushort dim);

    void blockcheck(std::vector<cell_index>* neighbors, cell_index elem, ushort dim);

    std::vector<cell_index>  regionQuery(cell_index entry);

    std::vector<SimpleCluster> dbscan();

    void expandCluster(std::vector<cell_index>& N, SimpleCluster& C);

    std::vector<cell_index> getCandidates();


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
    clusterer_params m_params;
    std::vector<ushort> m_planeShape;
    std::vector<ushort> m_valmax;
    boost::array<c4index, 4> m_c4shape =  {{ 40, 384, 9, 2 }};
    c4array m_houghmap = c4array(m_c4shape);
    ushort m_dimsize;
    boost::array<c3index, 3> m_c3shape =  {{ 40, 384, 9 }};
    c3array* m_houghVals;
    c3array m_houghVisit = c3array(m_c3shape);
  };
}

#endif
