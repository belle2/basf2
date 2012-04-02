/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXD_NOISEMAP_H
#define PXD_NOISEMAP_H

#include <memory>

namespace Belle2 {

  namespace PXD {

    /**
     * Class to represent pixel dependent Noise Map.
     * Currently there is no area dependence but could be used to mask
     * noisy pixels
     */
    class NoiseMap {
    public:
      /** Set the cut values to be used in electrons */
      void setCuts(float adjacent, float seed, float cluster) {
        cut_adjacent = adjacent;
        cut_seed = seed;
        cut_cluster = cluster;
      }
      /** Check if a pixel exceeds the seed cut */
      bool seed(unsigned int, unsigned int, float adc) const { return adc >= cut_seed; };
      /** Check if a pixel exceeds the adjacent cut */
      bool adjacent(unsigned int, unsigned int, float adc) const { return adc >= cut_adjacent; };
      /** Check if a cluster exceeds the cluster cut */
      bool cluster(float adc) const { return adc >= cut_cluster; };

      /** Return a reference to the instance */
      static NoiseMap& getInstance();
    protected:
      /** Constructor hidden, class is singleton */
      NoiseMap(): cut_adjacent(0), cut_seed(0), cut_cluster(0) {}
      /** Copy constructor hidden, class is singleton */
      NoiseMap(const NoiseMap&);
      /** Asignment operator hidden, class is singleton */
      NoiseMap& operator=(const NoiseMap&);
      /** Destructor hidden, class is singleton */
      ~NoiseMap() {}
      /** Allow destruction of singleton instance */
      friend class std::auto_ptr<NoiseMap>;

      /** Noist cut value */
      float cut_adjacent;
      /** Seed cut value */
      float cut_seed;
      /** Cluster cut value */
      float cut_cluster;
    };

  }

}

#endif //PXD_NOISEMAP_H
