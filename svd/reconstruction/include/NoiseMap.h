/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVD_NOISEMAP_H
#define SVD_NOISEMAP_H

#include <vxd/dataobjects/VxdID.h>
#include <svd/geometry/SensorInfo.h>
#include <svd/reconstruction/Sample.h>
#include <vxd/geometry/GeoCache.h>

namespace Belle2 {

  namespace SVD {

    /**
     * Base Class to represent strip-dependent noise map.
     * Currently there is only sensor and side dependence implemented.
     */
    class NoiseMap {
    public:
      /** Constructor  */
      NoiseMap(): m_eInADU(1.0), m_sensorID(0), m_isU(false), m_noiseLevel(0) {}

      /** Destructor */
      virtual ~NoiseMap() {}

      /** Set charge per ADC unit.
       * If signals are in ADC units, the default setting of 1 should be
       * overriden.
       * @param eInADU charge in electrons corresponding to 1 ADU.
       */
      void setADU(double eInADU) { m_eInADU = eInADU; }

      /** Override the noise level */
      void setNoiseLevel(float noise) { m_noiseLevel = noise; }

      /** Set the sensorID currently used */
      virtual void setSensorID(VxdID sensorID, bool side)
      {
        const SensorInfo& info = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorID));
        m_sensorID = sensorID;
        if (side)
          m_noiseLevel = info.getElectronicNoiseU() / m_eInADU;
        else
          m_noiseLevel = info.getElectronicNoiseV() / m_eInADU;
        m_isU = side;
      }

      /** Return the sigma-significance of a signal, that is, signal divided by noise level
       * @param sample Sample of a strip.
       * @return The ratio of maximum strip signal and noise level.
       */
      float getSignificance(const Sample& sample) const { return sample.getCharge() / getNoise(sample); }
      /** Return the noise level for a given strip
       * @param (not used) number of the strip
       * @return noise level
       */
      virtual float getNoise(short) const { return m_noiseLevel; }

      /** Return the noise value for a given strip
       * @param (not used) Sample at whose position we want noise level
       * @return noise level for the Sample's strip
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
      /** Charge corresponding to 1 ADU */
      double m_eInADU;
      /** VxdID of the current sensor */
      VxdID m_sensorID;
      /** Side of current sensor */
      bool m_isU;
      /** Noise level in strips */
      float m_noiseLevel;
    };

  }

}

#endif //SVD_NOISEMAP_H
