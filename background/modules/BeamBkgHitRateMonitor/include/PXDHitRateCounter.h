/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <background/modules/BeamBkgHitRateMonitor/HitRateBase.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <TTree.h>
#include <map>

#include <framework/gearbox/Unit.h>

namespace Belle2 {
  namespace Background {

    /**
     * Class for monitoring beam background hit rates of PXD
     */
    class PXDHitRateCounter: public HitRateBase {

    public:

      /**
       * tree structure
       */
      struct TreeStruct {
        float meanOccupancies[40] = {0}; /**< mean hit occupancy from PXDDigits per sensor [Hits/Channel] */
        float maxOccupancies[40] = {0}; /**< max hit occupancy from PXDDigits per sensor [Hits/Channel] */
        float doseRates[40] = {0}; /**< mean dose rate from PXDDigits per sensor [Gy/s]  */
        float softPhotonFluxes[40] = {0}; /**< mean soft photon flux per sensor (Single pixel cluster <10keV) [clusters/cm2/s]  */
        float hardPhotonFluxes[40] = {0}; /**< mean hard photon flux per sensor (Single pixel cluster >10keV) [clusters/cm2/s]  */
        float chargedFluxes[40] = {0}; /**< mean charged particle flux per sensor (Multi pixel cluster >10keV) [clusters/cm2/s]  */
        float segmentDoseRates[240] = {0};  /**< mean dose rate from PXDDigits per v segment of sensor [Gy/s]  */
        float segmentSoftPhotonFluxes[240] = {0}; /**< mean soft photon flux per v segment of sensor (Single pixel cluster <10keV) [clusters/cm2/s]  */
        float segmentHardPhotonFluxes[240] = {0}; /**< mean hard photon flux per v segment of sensor (Single pixel cluster >10keV) [clusters/cm2/s]  */
        float segmentChargedFluxes[240] = {0}; /**< mean charged particle flux per v segment of sensor (Multi pixel cluster >10keV) [clusters/cm2/s]  */
        float averageRate = 0; /**< total detector average hit rate */
        int numEvents = 0; /**< number of events accumulated */
        bool valid = false;  /**< status: true = rates valid */
      };

      /**
       * Constructor
       */
      PXDHitRateCounter()
      {}

      /**
       * Class initializer: set branch addresses and other staf
       * @param tree a valid TTree pointer
       */
      virtual void initialize(TTree* tree) override;

      /**
       * Clear time-stamp buffer to prepare for 'accumulate'
       */
      virtual void clear() override;

      /**
       * Accumulate hits
       * @param timeStamp time stamp
       */
      virtual void accumulate(unsigned timeStamp) override;

      /**
       * Normalize accumulated hits (e.g. transform to rates)
       * @param timeStamp time stamp
       */
      virtual void normalize(unsigned timeStamp) override;

    private:

      /**
       * Get PXD::SensorInfo
       */
      inline const PXD::SensorInfo& getInfo(VxdID sensorID) const;

      /**
       * Sets fractions of active channels
       */
      void setActivePixels();

      // class parameters: to be set via constructor or setters
      double m_integrationTime = 20 * Unit::us; /**< Integration time of PXD in ns */
      bool m_maskDeadPixels = true; /**< Correct bg rates by taking into account masked pixels */
      int m_nPXDSensors = 0; /**< number of sensors */

      const double c_densitySi = 2.3290 * Unit::g_cm3; /**< Density of crystalline Silicon */

      // tree structure
      TreeStruct m_rates; /**< tree variables */

      // buffer
      std::map<unsigned, TreeStruct> m_buffer; /**< average rates in time stamps */

      // collections
      StoreArray<PXDDigit> m_digits;  /**< collection of digits */
      StoreArray<PXDCluster> m_clusters;  /**< collection of clusters */

      // other
      double m_activePixels[40] = {0}; /**< number  of active pixels in sensor */
      double m_activeAreas[40] = {0}; /**< area of active pixels in sensor */
      double m_segmentActivePixels[240] = {0}; /**< number of active pixels in v segements */
      double m_segmentActiveAreas[240] = {0}; /**< area of active pixels in v segments */
    };

    inline const PXD::SensorInfo& PXDHitRateCounter::getInfo(VxdID sensorID) const
    {
      return dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    }

  } // Background namespace
} // Belle2 namespace
