/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TEL_CLUSTERCANDIDATE_H
#define TEL_CLUSTERCANDIDATE_H

#include <testbeam/vxd/reconstruction/Pixel.h>
#include <vector>

namespace Belle2 {

  namespace TEL {

    /**
     * Class representing a possible cluster during clustering of telescopes.
     * It supports merging of different clusters.
     *
     * To save relocation time, each Instance is created with a default capacity of
     * 10 pixels before relocation will occur.
     */
    class ClusterCandidate {
    public:
      enum {
        /** Reserve space for this number of pixels on construction */
        c_defaultCapacity = 100,
        /** If the capacity exceeds this value, shrink the vector back to c_defaultCapacity after clearing */
        c_maxCapacity = 4 * c_defaultCapacity
      };
      /** Constructor to create an empty Cluster */
      ClusterCandidate(): m_merged(0) {
        m_pixels.reserve(c_defaultCapacity);
      }

      /** Clear the Cluster information (to reuse the same cluster instance) */
      void clear() {
        m_merged = 0;
        m_pixels.clear();
      }

      /** Merge the given cluster with this one.
       * This method will remove all pixels from a given cluster and add it
       * to this one. In addition we set a pointer to this instance in the
       * other cluster so that new pixels will be directly added to the
       * correct cluster when add is called on an already merged cluster
       *
       * @param cls Cluster to merge into this one
       * @returns the address of the cluster containing all pixels
       */
      ClusterCandidate* merge(ClusterCandidate& cls);

      /** Add a Pixel to the current cluster.
       * If this method is called on a cluster which has been merged with
       * another cluster, the add method of the merged cluster will be called,
       * @param pixel Pixel to add to the cluster
       */
      void add(const Pixel& pixel);

      /** get the cluster size */
      size_t size() const { return m_pixels.size(); }
      /** get a reference to all pixels in the cluster */
      const std::vector<Pixel>& pixels() const { return m_pixels; }

    protected:
      /** Pointer to the cluster this cluster was merged into */
      ClusterCandidate* m_merged;
      /** List of all pixels in the cluster */
      std::vector<Pixel> m_pixels;
    };

  }

}

#endif //PXD_CLUSTERCANDIDATE_H
