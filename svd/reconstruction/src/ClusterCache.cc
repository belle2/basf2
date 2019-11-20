/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/reconstruction/ClusterCache.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  namespace SVD {

    ClusterCache::ClusterCache(unsigned int maxSamples): m_maxSamples(maxSamples)
    {
      m_clsTop = new ClusterCandidate*[m_maxSamples];
      m_clsCur = new ClusterCandidate*[m_maxSamples];
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
      memset(m_clsTop, 0, m_maxSamples * sizeof(ClusterCandidate*));
      memset(m_clsCur, 0, m_maxSamples * sizeof(ClusterCandidate*));
      m_curTime = 0;
      m_curStrip = 0;
    }

    /** update the cluster for a given coordinate and remember the last updated position */
    void ClusterCache::setLast(unsigned int iTime, unsigned int iStrip, ClusterCandidate* cls)
    {
      switchRow(iStrip);
      m_curStrip = iStrip;
      m_curTime = iTime;
      m_clsCur[iTime] = cls;
    }

    /** Find a cluster adjacent to the given coordinates. If no cluster is found, 0 is returned */
    ClusterCandidate* ClusterCache::findCluster(unsigned int iTime, unsigned int iStrip)
    {
      switchRow(iStrip);

      //Look for cluster above the current position (0,0 at top left corner,
      // rows going down, columns going left)
      ClusterCandidate* cls = m_clsTop[iTime];
      //Look for left cluster
      ClusterCandidate* clsLeft = (iTime > 0) ? m_clsCur[iTime - 1] : 0;
      cls = mergeCluster(iTime, cls, clsLeft);
      //Look for top left cluster
      ClusterCandidate* clsTopLeft = (iTime > 0) ? m_clsTop[iTime - 1] : 0;
      cls = mergeCluster(iTime - 1, cls, clsTopLeft);
      //Look for top right cluster
      ClusterCandidate* clsTopRight = (iTime < m_maxSamples - 1) ? m_clsTop[iTime + 1] : 0;
      cls = mergeCluster(iTime + 1, cls, clsTopRight);
      //Return found cluster, 0 if none was found
      return cls;
    }


    /** Merge two clusters and update the list of cached clusters */
    ClusterCandidate* ClusterCache::mergeCluster(unsigned int iTime, ClusterCandidate* cls1, ClusterCandidate* cls2)
    {
      if (cls2) {
        if (!cls1 || cls1 == cls2) return cls2;
        cls1->merge(*cls2);
        //Change the pointers to all samples which could have changed
        for (int i = iTime - 1; i >= 0; --i) {
          if (m_clsCur[i] == cls2) m_clsCur[i] = cls1;
        }
        for (unsigned int i = iTime; i < m_maxSamples; i++) {
          if (m_clsTop[i] == cls2) m_clsTop[i] = cls1;
        }
      }
      return cls1;
    }

    void ClusterCache::switchRow(unsigned int iStrip)
    {
      if (iStrip == m_curStrip) return;
      //We skipped a row, forget current row
      if (iStrip > m_curStrip + 1) memset(m_clsCur, 0, m_maxSamples * sizeof(ClusterCandidate*));
      //Clear top row
      memset(m_clsTop, 0, m_maxSamples * sizeof(ClusterCandidate*));

      //reset variables
      m_curStrip = iStrip;
      m_curTime = m_maxSamples + 1;
      //Switch rows, Current row will be top and we reuse memory of last top
      //row as new current row
      swap(m_clsTop, m_clsCur);
    }
  }
} //Belle2 namespace
