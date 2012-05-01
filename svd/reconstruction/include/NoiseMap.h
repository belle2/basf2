/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Peter Kvasnicka                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_NOISEMAP_H
#define SVD_NOISEMAP_H

#include <svd/reconstruction/Sample.h>

namespace Belle2 {

  namespace SVD {

    /**
     * Base Class to represent strip-dependent noise map.
     * Currently there is no area dependence implemented.
     */
    class NoiseMap {
    public:
      /** Constructor  */
      NoiseMap(): m_noiseLevel(0) {}

      /** Destructor */
      virtual ~NoiseMap() {}

      /** Set the noise level */
      void setNoiseLevel(float noise) { m_noiseLevel = noise; }

      /** Set the sensorID currently used.
       * May be used later or in derived classes to retrieve the correct noiseMap.
       */
      virtual void setSensorID(VxdID) {}

      /** Return the sigma-significance of a signal, that is, signal divided by noise level
       * @param sample Sample of the a strip.
       * @return The ratio of maximum strip signal and noise level.
       */
      float getSignificance(const Sample& sample) const { return sample.getCharge() / getNoise(sample); }

      /** Return the noise value for a given strip
       * @return noise level for a given strip.
       */
      virtual float getNoise(const Sample&) const { return m_noiseLevel; }

      /** Check whether a signal exceeds a given significance
       * @param sample strip to check
       * @param significance minimum significance
       * @true if the maximum signal of the strip exceeds the given significance.
       */
      bool operator()(const Sample& sample, float significance) const
      { return getSignificance(sample) >= significance; }

      /** Check whether a signal exceeds a given significance using the average noise level
       * @param signal signal to check
       * @param significance minimum significance
       */
      bool operator()(float signal, float significance) const { return signal / m_noiseLevel >= significance; }

    protected:
      /** Noise level */
      float m_noiseLevel;
    };

  }

}

#endif //SVD_NOISEMAP_H
