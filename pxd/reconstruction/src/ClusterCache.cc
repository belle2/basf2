/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/reconstruction/ClusterCache.h>

namespace Belle2 {

  namespace PXD {

    ClusterCache::ClusterCache(unsigned int maxU): m_maxU(maxU + 2)
    {
      m_clsTop = new ClusterCandidate*[m_maxU];
      m_clsCur = new ClusterCandidate*[m_maxU];
      clear();
    }

    ClusterCache::~ClusterCache()
    {
      delete[] m_clsTop;
      delete[] m_clsCur;
    }

    /** clear the Cache */
    void ClusterCache::clear()
    {
      memset(m_clsTop, 0, m_maxU * sizeof(ClusterCandidate*));
      memset(m_clsCur, 0, m_maxU * sizeof(ClusterCandidate*));
      m_curV = 0;
      m_currCluster = m_clusters.begin();
    }

    /** Find a cluster adjacent to the given coordinates. If no cluster is found, 0 is returned */
    ClusterCandidate& ClusterCache::findCluster(unsigned int u, unsigned int v)
    {
      if (u >= m_maxU - 2) {
        throw std::out_of_range("u cell id is outside of valid range");
      }
      switchRow(v);
      const unsigned int u1 = u + 1;
      //Look for cluster top of current pixel (0,0 at top left corner, rows going
      //down, columns going left) The ClusterCache is two columns wider than the
      //actual pixel sensor to get rid of edge effects. Column 0 is at index 1 so
      //the three neighbors of column u have the indices (u,u+1,u+2)

      //So the left neighbor has index u in the current row
      ClusterCandidate* cls = m_clsCur[u];
      //And the topleft, top and topright clusters have u+i, i in 0..2 But if we
      //already have a left neighbor we do not need to check topleft and top as
      //those are guaranteed to be already merged with the left one
      if (!cls) {
        cls = mergeCluster(cls, m_clsTop[u]);
        cls = mergeCluster(cls, m_clsTop[u1]);
      }
      cls = mergeCluster(cls, m_clsTop[u + 2]);

      //If no cluster was found create a new one
      if (!cls) {
        if (m_currCluster == m_clusters.end()) {
          //We already use all ClusterCandidates, create a new one
          m_clusters.emplace_front();
          cls = &m_clusters.front();
        } else {
          //There are some Candidates left, use them
          cls = &(*m_currCluster++);
          cls->clear();
        }
      }
      //Save the cluster and the current position in the cache
      m_curV = v;
      m_clsCur[u1] = cls;
      //Return the cluster
      return *cls;
    }

    /** Merge two cluster and update the list of cached clusters */
    ClusterCandidate* ClusterCache::mergeCluster(ClusterCandidate* cls1, ClusterCandidate* cls2)
    {
      if (cls2) {
        if (!cls1 || cls1 == cls2) return cls2;
        return cls1->merge(*cls2);
      }
      return cls1;
    }

    /** Switch current and top row or clear cache */
    void ClusterCache::switchRow(unsigned int v)
    {
      if (v == m_curV) return;
      //Clear top row
      std::memset(m_clsTop, 0, m_maxU * sizeof(ClusterCandidate*));
      //We skipped a row, forget current row, no need to switch rows, both got emptied
      if (v > m_curV + 1) {
        std::memset(m_clsCur, 0, m_maxU * sizeof(ClusterCandidate*));
      } else {
        //Switch rows, Current row will be top and we reuse memory of last top
        //row as new current row
        std::swap(m_clsTop, m_clsCur);
      }
      //save current row coordinate
      m_curV = v;
    }
  }
} //Belle2 namespace
