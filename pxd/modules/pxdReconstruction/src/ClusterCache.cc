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
#include <iostream>

using namespace std;

namespace Belle2 {

  namespace PXD {

    /** clear the Cache */
    void ClusterCache::clear()
    {
      memset(m_clsTop, 0, MAX_PIXELS_U * sizeof(ClusterCandidate*));
      memset(m_clsCur, 0, MAX_PIXELS_U * sizeof(ClusterCandidate*));
      m_curU = 0;
      m_curV = 0;
    }

    /** update the cluster for a given coordinate and remember the last updated position */
    void ClusterCache::setLast(unsigned int u, unsigned int v, ClusterCandidate* cls)
    {
      m_curV = v;
      m_curU = u;
      m_clsCur[u] = cls;
    }

    /** Find a cluster adjacent to the given coordinates. If no cluster is found, 0 is returned */
    ClusterCandidate* ClusterCache::findCluster(unsigned int u, unsigned int v)
    {
      switchRow(v);

      //Look for cluster top of current pixel (0,0 at top left corner, rows going down, columns going left)
      ClusterCandidate* cls = m_clsTop[u];
      //Look for left cluster
      ClusterCandidate* clsLeft = (u > 0) ? m_clsCur[u - 1] : 0;
      cls = mergeCluster(u, cls, clsLeft);
      //Look for top left cluster
      ClusterCandidate* clsTopLeft = (u > 0) ? m_clsTop[u - 1] : 0;
      cls = mergeCluster(u - 1, cls, clsTopLeft);
      //Look for top right cluster
      ClusterCandidate* clsTopRight = (u < MAX_PIXELS_U - 1) ? m_clsTop[u + 1] : 0;
      cls = mergeCluster(u + 1, cls, clsTopRight);
      //Return found cluster, 0 if none was found
      return cls;
    }


    /** Merge two cluster and update the list of cached clusters */
    ClusterCandidate* ClusterCache::mergeCluster(unsigned int u, ClusterCandidate* cls1, ClusterCandidate* cls2)
    {
      if (cls2) {
        if (!cls1 || cls1 == cls2) return cls2;
        cls1->merge(*cls2);
        //Change the pointers of all pixels which could have changed
        for (int i = u - 1; i >= 0; --i) {
          if (m_clsCur[i] == cls2) m_clsCur[i] = cls1;
        }
        for (int i = u; i < MAX_PIXELS_U; i++) {
          if (m_clsTop[i] == cls2) m_clsTop[i] = cls1;
        }
      }
      return cls1;
    }

    void ClusterCache::switchRow(unsigned int v)
    {
      if (v == m_curV) return;
      //We skipped a row, forget current row
      if (v > m_curV + 1) memset(m_clsCur, 0, MAX_PIXELS_U * sizeof(ClusterCandidate*));
      //Clear top row
      memset(m_clsTop, 0, MAX_PIXELS_U * sizeof(ClusterCandidate*));

      //reset variables
      m_curV = v;
      m_curU = MAX_PIXELS_U + 1;
      //Switch rows, Current row will be top and we reuse memory of last top
      //row as new current row
      //FIXME: swap(m_clsTop, m_clsCur);
    }
  }
} //Belle2 namespace
