/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdReconstruction/ClusterCache.h>

using namespace std;

namespace Belle2 {

  namespace PXD {

    /** clear the Cache */
    void ClusterCache::clear()
    {
      memset(m_clsTop, 0, 2*MAX_PIXELS_U*sizeof(ClusterCandidate*));
      m_clsLeft = 0;
      m_curU = 0;
      m_curV = 0;
      m_idxCurrent = 0;
      m_idxNext = 1;
    }

    /** update the cluster for a given coordinate and remember the last updated position */
    void ClusterCache::setLast(unsigned int u, unsigned int v, ClusterCandidate* cls)
    {
      m_curV = v;
      m_curU = u;
      m_clsTop[m_idxNext][u] = cls;
      m_clsLeft = cls;
    }

    /** Find a cluster adjacent to the given coordinates. If no cluster is found, 0 is returned */
    ClusterCandidate* ClusterCache::findCluster(unsigned int u, unsigned int v)
    {
      switchRow(v);

      ClusterCandidate *cls = m_clsTop[m_idxCurrent][u];
      if (u == m_curU + 1) cls = mergeCluster(u, cls, m_clsLeft);
      ClusterCandidate *clsTopLeft = (u > 0) ? m_clsTop[m_idxCurrent][u-1] : 0;
      cls = mergeCluster(u - 1, cls, clsTopLeft);
      ClusterCandidate *clsTopRight = (u < MAX_PIXELS_U - 1) ? m_clsTop[m_idxCurrent][u+1] : 0;
      cls = mergeCluster(u + 1, cls, clsTopRight);
      return cls;
    }


    /** Merge two cluster and update the list of cached clusters */
    ClusterCandidate* ClusterCache::mergeCluster(unsigned int u, ClusterCandidate* cls1, ClusterCandidate* cls2)
    {
      if (cls2) {
        if (!cls1 || cls1 == cls2) return cls2;
        cls1->merge(*cls2);
        for (int i = u - 1; i >= 0; --i) {
          if (m_clsTop[m_idxNext][i] == cls2)
            m_clsTop[m_idxNext][i] = cls1;
        }
        for (int i = u; i < MAX_PIXELS_U; i++) {
          if (m_clsTop[m_idxCurrent][i] == cls2)
            m_clsTop[m_idxCurrent][i] = cls1;
        }
        if (m_clsLeft == cls2) m_clsLeft = cls1;
      }
      return cls1;
    }

    void ClusterCache::switchRow(unsigned int v)
    {
      if (v == m_curV) return;
      //if we skip a row, then forget the clusters of the last row
      if (v > m_curV + 1) memset(m_clsTop[m_idxNext], 0, MAX_PIXELS_U*sizeof(ClusterCandidate*));
      //clean current row since we will use it now as cache for next row
      memset(m_clsTop[m_idxCurrent], 0, MAX_PIXELS_U*sizeof(ClusterCandidate*));

      //reset variables
      m_curV = v;
      m_curU = -1;
      m_clsLeft = 0;
      m_idxCurrent  = !m_idxCurrent;
      m_idxNext = !m_idxNext;
    }
  }
} //Belle2 namespace
