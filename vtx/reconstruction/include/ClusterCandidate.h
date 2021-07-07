/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef VTX_CLUSTERCANDIDATE_H
#define VTX_CLUSTERCANDIDATE_H

#include <vtx/reconstruction/Pixel.h>
#include <vector>

namespace Belle2 {

  namespace VTX {

    /**
     * Class representing a possible cluster during clustering of the VTX
     * It supports merging of different clusters and keeps track of the highest
     * charge inside the cluster.
     *
     * To save relocation time, each Instance is created with a default capacity of
     * 10 pixels before relocation will occur.
     */
    class ClusterCandidate {
    public:
      enum {
        /** Reserve space for this number of pixels on construction */
        c_defaultCapacity = 10,
        /** If the capacity exceeds this value, shrink the vector back to c_defaultCapacity after clearing */
        c_maxCapacity = 4 * c_defaultCapacity
      };
      /** Constructor to create an empty Cluster */
      ClusterCandidate():
        m_merged(0), m_charge(0), m_seed()
      {
        m_pixels.reserve(c_defaultCapacity);
      }

      /** Clear the Cluster information (to reuse the same cluster instance) */
      void clear()
      {
        m_merged = 0;
        m_charge = 0;
        m_seed = Pixel();
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
       * Increases the charge of the cluster and checks if the seed pixel
       * changes. If this method is called on a cluster which has been
       * merged with another cluster, the add method of the merged cluster
       * will be called,
       * @param pixel Pixel to add to the cluster
       */
      void add(const Pixel& pixel);

      /** get the charge of the cluster */
      float getCharge() const { return m_charge; }
      /** get the seed charge of the cluster */
      float getSeedCharge() const { return m_seed.getCharge(); }
      /** get the seed pixel of the cluster, i.e. the pixel with the highes charge */
      const Pixel& getSeed() const { return m_seed; }
      /** get the cluster size */
      size_t size() const { return m_pixels.size(); }
      /** get a reference to all pixels in the cluster */
      const std::vector<Pixel>& pixels() const { return m_pixels; }

    protected:
      /** Pointer to the cluster this cluster was merged into */
      ClusterCandidate* m_merged;
      /** Charge of the cluster */
      float m_charge;
      /** Seed pixel of the cluster, i.e. the pixel with the highest charge */
      Pixel m_seed;
      /** List of all pixels in the cluster */
      std::vector<Pixel> m_pixels;
    };

  }

}

#endif //VTX_CLUSTERCANDIDATE_H
