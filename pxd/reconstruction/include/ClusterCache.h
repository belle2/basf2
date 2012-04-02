/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXD_CLUSTERCACHE_H
#define PXD_CLUSTERCACHE_H

#include <cstring>
#include <pxd/reconstruction/Pixel.h>
#include <pxd/reconstruction/ClusterCandidate.h>

namespace Belle2 {
  namespace PXD {

    /** Class to remember recently assigned clusters
     * This class will remember the current and the last pixel row to allow
     * fast finding of the correct cluster a pixel belongs to.
     *
     * All hit pixels above noise threshold will be processed sorted, rowwise
     * with increasing u and v ids. For each pixel we look at the left neihbour
     * in the same row and the three direkt neihbours in the last row. If a
     * cluster is found in any of these pixels, all clusters will be merged and
     * the pixel gets assigned the existing cluster. Otherwise a new cluster is
     * created. At the end, all clusters will be checked if they satisfiy the
     * requirements for a valid cluster (seed and cluster charge) and only
     * valid clusters are kept
     *
     * Lets assume that we have a hit in the pixel marked with X. If we process
     * the pixels in a sorted way, than we only have to check the pixels marked
     * O to see if this pixel belongs to a cluster. To do this, we cache the
     * top row and the left cluster. For each pixel, we just have to check the
     * top row and see if the left cluster is really on left to the checked
     * pixel. If there are different clusters adjacent to a given pixel (like 1
     * and 2 are adjacent to 3) we merge all found clusters and update the
     * cache. after This step the pixels belonging to 2 and 3 will all belong
     * to 1.
     *
     * We save the row we are currently looking at as the next top row. Once
     * the row coordinate changes we switch the top row or erase it completley
     * if one row has no hit at all.
     *
     * this way we only have to look at each pixel once and otherwise just
     * update the list of clusters.
     *
     * @verbatim
     *   u → → → → → → → → → →
     * v┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐
     * ↓│ │ │O│O│O│ │ │ │ │ │ │
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * ↓│ │ │O│X│ │ │ │ │ │ │ │
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * ↓│ │ │ │ │ │ │ │ │ │ │ │
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * ↓│ │ │1│ │2│2│ │ │ │ │ │
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * ↓│ │ │ │3│ │ │ │ │ │ │ │
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * ↓│ │ │ │ │ │ │ │ │ │ │ │
     *  └─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
     * @endverbatim
     *
     */
    class ClusterCache {
    public:
      enum {
        /** Default maximum number of PIXEL columns the cache can handle */
        MAX_PIXELS_U = 250
      };
      /** Create a new cache */
      ClusterCache(unsigned int maxU = MAX_PIXELS_U);
      /** Delete the cache and free the memory */
      ~ClusterCache();
      /** Clear the cache structure */
      void clear();
      /** update the cluster for a given coordinate and remember the last updated position */
      void setLast(unsigned int u, unsigned int v, ClusterCandidate* cls);
      /** Find a cluster adjacent to the given coordinates. If no cluster is found, 0 is returned */
      ClusterCandidate* findCluster(unsigned int u, unsigned int v);

    protected:

      /** Merge two cluster and update the list of cached clusters */
      ClusterCandidate* mergeCluster(unsigned int u, ClusterCandidate* cls1, ClusterCandidate* cls2);

      /** Switch the internal rows.
       * We always keep current and last row. This method switches these
       * rows and resets the new current row
       */
      void switchRow(unsigned int v);

      /** the maximum number of columns to be considered */
      unsigned int m_maxU;
      /** current u coordinate, needed for left neighbour */
      unsigned int m_curU;
      /** current v coordinate, needed to switch top row */
      unsigned int m_curV;
      /** cache of the top row */
      ClusterCandidate** m_clsTop;
      /** cache of the current row */
      ClusterCandidate** m_clsCur;
    };

  }

}

#endif //PXD_CLUSTERCACHE_H
