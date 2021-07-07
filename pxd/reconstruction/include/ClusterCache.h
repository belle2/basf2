/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <deque>
#include <pxd/reconstruction/ClusterCandidate.h>

namespace Belle2 {
  namespace PXD {

    /** Class to remember recently assigned clusters
     * This class will remember the current and the last pixel row to allow
     * fast finding of the correct cluster a pixel belongs to.
     *
     * All hit pixels above noise threshold will be processed sorted, row wise with
     * increasing u (column) and v (row) ids. For each pixel we look at the left
     * neighbor in the same row and the three direct neighbors in the last row. If
     * at least one cluster is found in any of these pixels, all those clusters
     * will be merged and the pixel gets assigned to the existing cluster.
     * Otherwise a new cluster is created. At the end, all clusters will be checked
     * if they satisfy the requirements for a valid cluster (seed and cluster
     * charge) and only valid clusters are kept
     *
     * Lets assume that we have a hit in the pixel marked with X. If we process the
     * pixels in a sorted way, than we only have to check the pixels marked O to
     * see if this pixel belongs to a cluster. To do this, we cache the top row and
     * the current row. For each pixel, we just have to check the left cluster and
     * three elements of the top row. If we already found a valid left cluster we
     * only have to check the top right position in addition as we are sure that
     * top left and top are already pointing to the same cluster as the left
     * position due to the sorted processing. If we find more than one adjoining
     * cluster (like the clusters 1 and 2 when looking at 3), all those clusters
     * will be merged. After this step the pixels belonging to 2 and 3 will all
     * belong to cluster 1.
     *
     * We save the row we are currently looking at. Once the v coordinate of
     * the next hit changes we either switch the current row to top row or erase it
     * if the v coordinate changed by more than one (complete row without an hit).
     *
     * With this algorithm we only have to look at each pixel once and otherwise
     * just update the list of clusters. To further improve performance the number
     * of free/malloc calls is kept to a minimum: The list of clusters is kept in a
     * std::deque (to keep pointer valid at all times) and not cleared between
     * events but reused. Each new cluster keeps the pixel information in a
     * std::vector is constructed with a capacity of 10 pixels so that relocation
     * should only occur in rare cases and the current and top row pointers are
     * kept and reused over the lifetime of the Cache.
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
      /** Define iterator type */
      typedef std::deque<ClusterCandidate>::iterator iterator;
      /** Define const iterator type */
      typedef std::deque<ClusterCandidate>::const_iterator const_iterator;

      enum {
        /** Default maximum number of PIXEL columns the cache can handle */
        c_defaultNumberColumns = 250
      };
      /** Create a new cache */
      explicit ClusterCache(unsigned int maxU = c_defaultNumberColumns);

      /** No copy construction. */
      ClusterCache(const ClusterCache&) = delete;

      /** No operator= **/
      ClusterCache& operator=(const ClusterCache&) = delete;

      /** Delete the cache and free the memory */
      ~ClusterCache();
      /** Clear the cache structure */
      void clear();

      /** Find a cluster adjacent to the given coordinates.
       * If no adjacent cluster is found, a new cluster is returned.
       *
       * @param u column id to look for a adjacent clusters
       * @param v row id to look for adjacent clusters
       * @returns cluster reference, either existing, adjacent cluster or empty cluster
       */
      ClusterCandidate& findCluster(unsigned int u, unsigned int v);

      /** Return iterator to the begin of of created clusters.
       * This list also contains clusters which were already merged with
       * other clusters so one has to check that the cluster size is >0
       * To loop over all cluster candidates use something like
       * \code
         for(ClusterCandidate &cls: cache){
           //...
         }
         \endcode
       */
      std::deque<ClusterCandidate>::iterator begin() { return m_clusters.begin(); }
      /** Return iterator to the end of created clusters */
      std::deque<ClusterCandidate>::iterator end() { return m_currCluster; }

      /** Check if there are any clusters */
      bool empty() const { return m_clusters.begin() == m_currCluster; }

    private:

      /** Merge two cluster and update the list of cached clusters */
      ClusterCandidate* mergeCluster(ClusterCandidate* cls1, ClusterCandidate* cls2);

      /** Switch the internal rows.
       * We always keep current and last row. This method switches these rows
       * and resets the new current row
       * @param v current row id
       */
      void switchRow(unsigned int v);

      /** number of columns of the cache. This is the actual number of columns + 2 to get rid of edge effects */
      const unsigned int m_maxU;
      /** current v coordinate, needed to switch top row */
      unsigned int m_curV;
      /** cache of the top row */
      ClusterCandidate** m_clsTop;
      /** cache of the current row */
      ClusterCandidate** m_clsCur;
      /** list of all the clusters created so far */
      std::deque<ClusterCandidate> m_clusters;
      /** iterator to the next free cluster to be used if a new cluster is needed.
       * The List of clusters is kept between clustering calls to save
       * malloc/free calls. On clear, this iterator will point to the
       * beginning of m_clusters and will be incremented every time a cluster
       * is needed. Once it reaches m_clusters.end(), new clusters will be
       * added to the front of the m_clusters container.
       */
      std::deque<ClusterCandidate>::iterator m_currCluster;
    };

  }

}
