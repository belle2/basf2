/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <string>
#include <memory>
#include <map>
#include <vxd/background/niel_fun.h>

namespace Belle2 {

  namespace PXD {

    /** PXD Background module.
     * This module collects data from Belle II background simulation and produces a report
     * on:
     * 1. PXD radiation exposure: Deposited energy per unit mass of material.
     * 2. PXD neutron flux: flux of non-ionizing damage normalized to 1 MeV neutron in perpendicular
     *    direction.
     * 3. Occupancy:
     *    - total : pixels fired per unit area and time
     *    - occupancy distribution.
     *
     * For each item, we tabulate depending on background source and layer.
     */
    class PXDBackgroundModule: public Module {

    public:

      // Reporting levels
      static const unsigned short c_reportNone = 0; /**< No reporting */
      static const unsigned short c_reportSummary = 1; /**< Summary only */
      static const unsigned short c_reportNTuple = 2; /**< Summary and NTuple */

      /** Struct to hold data of an PXD sensor */
      struct SensorData {
        /** Exposition (energy deposited per cm2 and 1 second) */
        double m_expo;
        /** Dose (Gy/smy) */
        double m_dose;
        /** Neutron flux */
        double m_neutronFlux;
        /** Fired pixels per cm2 and second, zero-suppression threshold */
        double m_fired;
        /** Occupancy*/
        double m_occupancy;
      };

      /** Constructor */
      PXDBackgroundModule();
      /** Destructor */
      virtual ~PXDBackgroundModule();

      /** Initialize module */
      virtual void initialize() override;
      /** Start-of-run initializations */
      virtual void beginRun() override;
      /** Event processing */
      virtual void event() override;
      /** End-of-run tasks */
      virtual void endRun() override;
      /** Final summary and cleanup */
      virtual void terminate() override;

    private:

      // General
      const double c_densitySi = 2.3290 * Unit::g_cm3; /**< Density of crystalline Silicon */
      const double c_smy = 1.0e7 * Unit::s;             /**< Seconds in snowmass year */
      /** NIEL-correction file for neutrons */
      const std::string c_niel_neutronFile = "/vxd/data/neutrons.csv";
      /** NIEL-correction file for protons */
      const std::string c_niel_protonFile = "/vxd/data/protons.csv";
      /** NIEL-correction file for pions */
      const std::string c_niel_pionFile = "/vxd/data/pions.csv";
      /** NIEL-correction file for electrons */
      const std::string c_niel_electronFile = "/vxd/data/electrons.csv";
      // No NIEL for photons.

      /** This is a shortcut to getting PXD::SensorInfo from the GeoCache.
       * @param sensorID VxdID of the sensor
       * @return SensorInfo object for the desired sensor.
       */
      inline const PXD::SensorInfo& getInfo(VxdID sensorID) const;
      /** Return thickness of the sensor with the given sensor ID */
      inline double getSensorThickness(VxdID sensorID) const;
      /** Return mass of the sensor with the given sensor ID */
      inline double getSensorMass(VxdID sensorID) const;
      /** Return area of the sensor with the given sensor ID */
      inline double getSensorArea(VxdID sensorID) const;

      /** Convert local sensor coordinates to global */
      const TVector3& pointToGlobal(VxdID sensorID, const TVector3& local);
      /** Convert local vector coordinates to global */
      const TVector3& vectorToGlobal(VxdID sensorID, const TVector3& local);

      /** Get number of sensors in a layer */
      inline int getNumSensors(int layerNum);
      /** Get total number of sensors */
      inline int getTotalSensors();

      // Output directory
      std::string m_outputDirectoryName; /**< Path to directory where output data will be stored */
      std::string m_storeFileMetaDataName; /**< Name of the persistent FileMetaData object */
      std::string m_storeBgMetaDataName; /**< Name of the persistent BackgroundMetaDta object */
      // StoreArrays
      std::string m_storeMCParticlesName; /**< MCParticles StoreArray name */
      std::string m_storeSimHitsName; /**< PXDSimHits StoreArray name */
      std::string m_storeTrueHitsName; /**< PXDTrueHits StoreArray name */
      std::string m_relParticlesTrueHitsName; /**< MCParticlesToPXDTrueHits RelationArray name */
      std::string m_storeDigitsName; /**< PXDDigits StoreArray name */
      std::string m_relDigitsMCParticlesName; /**< StoreArray name of PXDDigits to MCParticles relation */
      std::string m_relDigitsTrueHitsName; /**< StoreArray name of PXDDigits to PXDTrueHits relation */
      std::string m_storeClustersName; /**< PXDClusters StoreArray name */
      std::string m_relClusterDigitName; /**< PXDClustersToPXDDigits RelationArray name */
      std::string m_relTrueHitsSimHitsName;  /**< PXDTrueHitsToPXDSimHits RelationArray name */

      std::string m_storeEnergyDepositsName; /**< PXDEnergyDepositEvents StoreArray name */
      std::string m_storeNeutronFluxesName; /**< PXDNeutronFluxEvents StoreArray name */
      std::string m_storeOccupancyEventsName; /**< PXDOccupancyEvents StoreArray name */

      unsigned short m_doseReportingLevel; /**< 0 - no data, 1 - summary only, 2 - ntuple */
      unsigned short m_nfluxReportingLevel; /**< 0 - no data, 1 - summary only, 2 - ntuple */
      unsigned short m_occupancyReportingLevel; /**< 0 - no data, 1 - summary only, 2 - ntuple */

      std::string m_componentName; /**< Name of the current bg component. */
      double m_componentTime; /**< Time of current component. */
      double m_integrationTime; /**< Integration time of PXD. */

      std::map<VxdID, SensorData> m_sensorData; /**< Struct to hold sensor-wise background data. */

      // NIEL tables
      std::unique_ptr<TNiel> m_nielNeutrons;  /**< Pointer to Niel table for neutrons */
      std::unique_ptr<TNiel> m_nielProtons;   /**< Pointer to Niel table for protons */
      std::unique_ptr<TNiel> m_nielPions;     /**< Pointer to Niel table for pions */
      std::unique_ptr<TNiel> m_nielElectrons; /**< Pointer to Niel table for electrons */

    };

    inline const PXD::SensorInfo& PXDBackgroundModule::getInfo(VxdID sensorID) const
    {
      return dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    }

    inline  double PXDBackgroundModule::getSensorThickness(VxdID sensorID) const
    {
      return getInfo(sensorID).getThickness();
    }

    inline  double PXDBackgroundModule::getSensorMass(VxdID sensorID) const
    {
      const PXD::SensorInfo& info = getInfo(sensorID);
      return info.getWidth() * info.getLength() * info.getThickness() * c_densitySi;
    }

    inline  double PXDBackgroundModule::getSensorArea(VxdID sensorID) const
    {
      const PXD::SensorInfo& info = getInfo(sensorID);
      return info.getWidth() * info.getLength();
    }

    inline int PXDBackgroundModule::getNumSensors(int layerNum)
    {
      VxdID layerID;
      layerID.setLayerNumber(layerNum);
      int result = 0;
      for (auto ladderID : VXD::GeoCache::getInstance().getLadders(layerID))
        result += VXD::GeoCache::getInstance().getSensors(ladderID).size();
      return result;
    }
  } // namespace PXD
} // namespace Belle2

