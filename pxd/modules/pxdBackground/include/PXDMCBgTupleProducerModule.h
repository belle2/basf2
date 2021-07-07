/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/gearbox/Unit.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <string>
#include <vector>
#include <map>


namespace Belle2 {

  namespace PXD {

    /** PXD MC Background Tuple Producer.
     *
     * This module produces PXD tuples for BEAST PXD background studies. The output is a
     * root TFile with a TTree containing one second time stamps together with
     * different PXD observables like occupancy, charged particle flux, photon flux, dose
     * and exposition.
     */
    class PXDMCBgTupleProducerModule: public Module {

    public:

      /** Struct to hold data of an PXD sensor */
      struct SensorData {
        /** Average occupancy of all events inside one second block */
        double m_meanOccupancy;
        /** Exposition (energy deposited per cm2 and second) */
        double m_expo;
        /** Dose (Gy per second) */
        double m_dose;
        /** Soft photon flux (selected clusters per cm and second) */
        double m_softPhotonFlux;
        /** Charged particle flux (selected clusters per cm and second) */
        double m_chargedParticleFlux;
        /** Hard photon flux (selected clusters per cm and second) */
        double m_hardPhotonFlux;

        /** Expositions (energy deposited per cm2 and second) for sensor regions */
        std::vector<double> m_regionExpoMap;
        /** Dose (Gy per second) for sensor regions */
        std::vector<double> m_regionDoseMap;
        /** Soft photon flux (selected clusters per cm and second) for sensor regions */
        std::vector<double> m_regionSoftPhotonFluxMap;
        /** Charged particle flux (selected clusters per cm and second) for sensor regions */
        std::vector<double> m_regionChargedParticleFluxMap;
        /** Hard photon flux (selected clusters per cm and second) for sensor regions */
        std::vector<double> m_regionHardPhotonFluxMap;
      };

      /** Constructor */
      PXDMCBgTupleProducerModule();

      /** Initialize module */
      void initialize() override final;
      /** Start-of-run initializations */
      void beginRun() override final;
      /** Event processing */
      void event() override final;
      /** Final summary and cleanup */
      void terminate() override final;

    private:

      // General
      const double c_densitySi = 2.3290 * Unit::g_cm3; /**< Density of crystalline Silicon */

      /** This is a shortcut to getting PXD::SensorInfo from the GeoCache.
       * @param sensorID VxdID of the sensor
       * @return SensorInfo object for the desired sensor.
       */
      inline const PXD::SensorInfo& getInfo(VxdID sensorID) const;
      /** Return area of the sensor with the given sensor ID */
      inline double getSensorArea(VxdID sensorID) const;
      /** Get region id from region uBin and vBin */
      inline int getRegionID(int uBin, int vBin) const;
      /** Return area of the region with the given sensor ID and region vBin*/
      inline double getRegionArea(VxdID sensorID, int vBin) const;

      // Output directory
      std::string m_outputFileName; /**< output tuple file name */

      // StoreArrays
      std::string m_storeClustersName; /**< PXDClusters StoreArray name */
      std::string m_storeDigitsName; /**< PXDDigits StoreArray name */
      std::string m_storeBgMetaDataName; /**< Name of the persistent BackgroundMetaDta object */
      double m_integrationTime; /**< Integration time of PXD. */
      bool m_maskDeadPixels; /**< Correct bg rates by taking into account masked pixels */
      int m_nBinsU; /**< Number of regions per sensor along u side */
      int m_nBinsV; /**< Number of regions per sensor along v side */

      std::map<VxdID, SensorData> m_sensorData; /**< Struct to hold sensor-wise background data. */
      std::map<VxdID, int> m_sensitivePixelMap; /**< Struct to hold sensor-wise number of sensitive pixels */
      std::map<VxdID, double> m_sensitiveAreaMap; /**< Struct to hold sensor-wise sensitive area */
      std::map<std::pair<VxdID, int>, int> m_regionSensitivePixelMap; /**< Struct to hold region-wise number of sensitive pixels */
      std::map<std::pair<VxdID, int>, double> m_regionSensitiveAreaMap; /**< Struct to hold region-wise sensitive area */
      bool m_hasPXDData; /**< Flag to indicate there was at least one PXDDigit in the run */
      double m_componentTime; /**< Time of current component. */
      double m_overrideComponentTime; /**< Time of current component given by user. */
    };

    inline int PXDMCBgTupleProducerModule::getRegionID(int uBin, int vBin) const
    {
      return uBin * m_nBinsV + vBin;
    }

    inline const PXD::SensorInfo& PXDMCBgTupleProducerModule::getInfo(VxdID sensorID) const
    {
      return dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    }

    inline  double PXDMCBgTupleProducerModule::getSensorArea(VxdID sensorID) const
    {
      const PXD::SensorInfo& info = getInfo(sensorID);
      return info.getWidth() * info.getLength();
    }

    inline  double PXDMCBgTupleProducerModule::getRegionArea(VxdID sensorID, int vBin) const
    {
      const PXD::SensorInfo& info = getInfo(sensorID);
      int vi = vBin * info.getVCells() / m_nBinsV;
      double length = info.getVPitch(info.getVCellPosition(vi)) * info.getVCells() / m_nBinsV;
      return length * info.getWidth() / m_nBinsU;
    }

  } // namespace PXD
} // namespace Belle2

