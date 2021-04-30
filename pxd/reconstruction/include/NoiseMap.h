/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <pxd/reconstruction/Pixel.h>

namespace Belle2 {

  namespace PXD {

    /**
     * Base Class to represent pixel dependent Noise Map.
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
       * Should be utilised later or in derived classes to obtain the correct noiseMap.
       */
      virtual void setSensorID(VxdID) {}

      /** Return the significance of a signal, that is signal divided by noise level */
      float getSignificance(const Pixel& px) const { return px.getCharge() / getNoise(px); }

      /** Return the noise value for a given pixel */
      virtual float getNoise(const Pixel&) const { return m_noiseLevel; }

      /** Check wether a signal exceeds a given significance
       * @param px pixel to check
       * @param significance minimum significance
       */
      bool operator()(const Pixel& px, float significance) const { return getSignificance(px) >= significance; }

      /** Check wether a signal exceeds a given significance using the average noise level
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
