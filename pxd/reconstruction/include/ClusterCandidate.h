/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXD_CLUSTERCANDIDATE_H
#define PXD_CLUSTERCANDIDATE_H

#include <pxd/reconstruction/NoiseMap.h>
#include <pxd/reconstruction/Pixel.h>
#include <vector>

namespace Belle2 {

  namespace PXD {

    /**
     * Class representing a possible cluster during clustering of the PXD
     * It supports merging of different clusters and keeps track of the highest
     * charge inside the cluster
     */
    class ClusterCandidate {
    public:
      /** Constructor to create an empty Cluster */
      ClusterCandidate(): m_charge(0), m_seed(0, 0) {};

      /** Merge the given cluster with this one.
       * Remove all pixels from the given cluster and set its charge to 0
       * @param cls Cluster to merge into this one
       */
      void merge(ClusterCandidate& cls);

      /** Add a Pixel to the current cluster.
       * Increasing the charge of the cluster and check if the seed pixel changes
       * @param pixel Pixel to add to the cluster
       */
      void add(const Pixel& pixel);

      /** Check wether this cluster fullfils all constrains like noise cuts.
       * @return true if cluster is valid and should be kept, false otherwise
       */
      bool valid() const {
        return (m_charge > 0) && (m_seed.getCharge() > 0) && (NoiseMap::getInstance().cluster(m_charge));
      }

      /** get the charge of the cluster */
      float getCharge() const { return m_charge; }
      /** get the seed charge of the cluster */
      float getSeedCharge() const { return m_seed.getCharge(); }
      /** get the seed pixel of the cluster */
      const Pixel& getSeed() const { return m_seed; }
      /** get the cluster size */
      size_t size() const { return m_pixels.size(); }
      /** get a reference to all pixels in the cluster */
      const std::vector<Pixel>& pixels() const { return m_pixels; }

    protected:
      /** Charge of the cluster */
      float m_charge;
      /** Seed pixel of the cluster */
      Pixel m_seed;
      /** List of all pixels in the cluster */
      std::vector<Pixel> m_pixels;
    };

  }

}

#endif //PXD_CLUSTERCANDIDATE_H
