/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_CLUSTERCACHE_H
#define SVD_CLUSTERCACHE_H

#include <cstring>
#include <svd/reconstruction/Sample.h>
#include <svd/reconstruction/ClusterCandidate.h>

namespace Belle2 {
  namespace SVD {

    /** Class to remember recently assigned clusters
     * This class will remember the samples of the current and the last strip
     * to allow fast finding of the correct cluster that a sample belongs to.
     *
     * All samples above noise threshold will be processed sorted, strip-wise with
     * with increasing strip number and time. For each sample we only look at the
     * preceding sample in the same strip and at the three samples around this sample's
     * time in the preceding strip. If any of these samples belongs to a cluster,
     * the clusters will be merged and the sample gets assigned to the existing
     * cluster. Otherwise a new cluster is created. In the end, all clusters will
     * be checked if they satisfiy the requirements on a valid cluster (seed
     * and cluster charge, minimum number of samples per strip) and only valid
     * clusters are kept.
     *
     * Lets assume that we have a hit in the strip/time position  marked with X.
     * If we process the samples in a sorted way, than we only have to check the
     * positions marked O to see if this sample belongs to a cluster. To do this,
     * we cache the samples of the preceding strip and the preceding sample of
     * the current strip. We have to check the preceding strip and see if the
     * left cluster is really to the left of the checked sample. If there are
     * different clusters adjacent to a given strip/time position (like 1 and 2
     * are adjacent to 3) we merge all together and update the cache. After this
     * step the samples belonging to 2 and 3 will all belong to 1.
     *
     * We save the strip data we are currently looking at as the next top strip
     * data. Once the strip coordinate changes we switch the top strip or erase
     * it completely if the strip has no hit at all.
     *
     * This way we only have to look at each sample once and otherwise just
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
        /** Default maximum number of samples per strip that the cache can handle */
        MAX_SAMPLES = 50
      };
      /** Create a new cache */
      ClusterCache(unsigned int maxSamples = MAX_SAMPLES);
      /** No copy construction. */
      ClusterCache(const ClusterCache& other) = delete;
      /** Delete the cache and free the memory */
      ~ClusterCache();
      /** Clear the cache structure */
      void clear();
      /** update the cluster for a given coordinate and remember the last updated position */
      void setLast(unsigned int iTime, unsigned int iStrip, ClusterCandidate* cls);
      /** Find a cluster adjacent to the given coordinates. If no cluster is found, 0 is returned */
      ClusterCandidate* findCluster(unsigned int iTime, unsigned int iStrip);

    protected:

      /** Merge two cluster and update the list of cached clusters */
      ClusterCandidate* mergeCluster(unsigned int istrip, ClusterCandidate* cls1, ClusterCandidate* cls2);

      /** Switch the internal strip data.
       * We always keep the data of the current and the last strip. This method
       * switches these strip data and resets the new current strip.
       */
      void switchRow(unsigned int istrip);

      /** the maximum number of samples to be considered */
      unsigned int m_maxSamples;
      /** current sample coordinate, needed for the left neighbour */
      unsigned int m_curTime;
      /** current strip coordinate, needed to switch top strip */
      unsigned int m_curStrip;
      /** cache of the top strip. */
      ClusterCandidate** m_clsTop;
      /** cache of the current strip. */
      ClusterCandidate** m_clsCur;

    private:

    };

  }

}

#endif //SVD_CLUSTERCACHE_H
