/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_CLUSTERCANDIDATE_H
#define SVD_CLUSTERCANDIDATE_H

#include <svd/reconstruction/Sample.h>
#include <vector>
#include <map>

namespace Belle2 {

  namespace SVD {

    /**
     * Class representing a potential cluster during clustering of the SVD
     * It supports merging of different clusters and keeps track of the highest
     * charge inside the cluster
     */
    class ClusterCandidate {
    public:
      /** Constructor to create an empty Cluster */
      ClusterCandidate(): m_charge(0), m_seed() {};

      /** Merge a cluster with this one.
       * Discard all samples of the other cluster and set its charge to 0
       * @param cls Cluster to merge into this one
       */
      void merge(ClusterCandidate& cls);

      /** Add a Sample to the current cluster.
       * Update the charge of the cluster and its seed.
       * @param sample Sample to add to the cluster
       */
      void add(const Sample& sample);
      /** get the strip-wise charge of the cluster (sum of maximum strip charges). */
      float getCharge() const { return m_charge; }
      /** get the seed charge of the cluster */
      float getSeedCharge() const { return m_seed.getCharge(); }
      /** get the seed sample of the cluster */
      const Sample& getSeed() const { return m_seed; }
      /** get the cluster size */
      size_t size() const { return m_samples.size(); }
      /** get a reference to all samples in the cluster */
      const std::vector<Sample>& samples() const { return m_samples; }
      /** get the reference to the map of strip charges */
      const std::map<unsigned int, float>& getStripCharges() const { return m_charges; }
      /** get the reference to the map of indices of maximum strip signals */
      const std::map<unsigned int, unsigned int>& getMaxima() const { return m_maxima; }
      /** get the reference to the map of sample counts per strip */
      const std::map<unsigned int, unsigned int>& getCounts() const { return m_counts; }


    protected:
      /** Charge of the cluster */
      float m_charge;
      /** Seed pixel of the cluster */
      Sample m_seed;
      /** List of all pixels in the cluster */
      std::vector<Sample> m_samples;
      /** Samples with maximum signal by strip */
      std::map<unsigned int, float> m_charges;
      /** Samples with maximum signal indices */
      std::map<unsigned int, unsigned int> m_maxima;
      /** Number of samples per strip */
      std::map<unsigned int, unsigned int> m_counts;
    };

  }

}

#endif //SVD_CLUSTERCANDIDATE_H
