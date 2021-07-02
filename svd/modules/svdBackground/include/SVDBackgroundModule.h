/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef SVDBackgroundMODULE_H_
#define SVDBackgroundMODULE_H_

#include <framework/core/Module.h>
#include <framework/gearbox/Unit.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <string>
#include <memory>
#include <map>
#include <vxd/background/niel_fun.h>

namespace Belle2 {

  namespace SVD {

    /** SVD Background module.
     * This module collects data from Belle II background simulation and produces a report
     * on:
     * 1. SVD radiation exposure: Deposited energy per unit mass of material.
     * 2. SVD neutron flux: flux of non-ionizing damage normalized to 1 MeV neutron in perpendicular
     * direction.
     * 3. Occupancy:
     * - total : strips fired per unit area and time
     * - effective: corrected for acceptance time winodow of the SVD
     * - occupancy distribution.
     *
     * For each item, we tabulate depending on background source and layer.
     * TODO:
     * Only show things extrapolated to smy in tables or bar charts.
     * Otherwise use sensible units, such as Hz, that is, extrapolate to 1 second
     * rather than 1 smy.
     * Much of the statistics about doses is corrupt, it is sums or means of Landauian
     * contributions. Not speaking about error bounds.
     */

    /* Modification Jan 2016:
     * 1. Delete all histogramming, only produce n-tuples and text file
     * with summaries. The HistogramFactory class is not used any longer.
     * All drawing will be done in pandas.
     * 2. Only process one background component at a time.
     * 3. Add occupancy estimates and a separate ntuple to hold the data.
     */
    /* Modifications Feb 2016:
     * Add more information on fired strips rate and related occupancy estmates:
     * 1. Two strip firing rates:
     * - one with threshold equal to threshold cut, to provide check for occupancy estimates
     * - another with threshold equal to elNoise
     * 2. Two occupancies:
     * - one for time of 1 APV cycle, no SNR adjustment
     * - one for trigger jitter of 5 ns + testbeam-based hit time error
     *
     * Make module functionality switchable:
     * - Add module parameters to turn on/off dose, neutron flux, and occupancy data collection.
     */

    class SVDBackgroundModule: public Module {

    public:
      /** Number of VXD layers */

      static const int c_nVXDLayers = 6;

      // Reporting levels
      static const unsigned short c_reportNone = 0; /**< No reporting */
      static const unsigned short c_reportSummary = 1; /**< Summary only */
      static const unsigned short c_reportNTuple = 2; /**< Summary and NTuple */

      /** Struct to hold data of an SVD sensor */
      struct SensorData {
        /** Exposition (energy deposited per cm2 and 1 second) */
        double m_expo;
        /** Dose (Gy/smy) */
        double m_dose;
        /** Neutron flux */
        double m_neutronFlux;
        /** Fired pixels in U, per cm2 and second, zero-suppression threshold */
        double m_firedU;
        /** Fired pixels in V, per cm2 and second, zero-suppression threshold */
        double m_firedV;
        /** Fired pixels in U, occupied time per cm2 and second */
        double m_firedU_t;
        /** Fired pixels in V, occupied time per cm2 and second */
        double m_firedV_t;
        /** Occupancy in U */
        double m_occupancyU;
        /** Occupancy in V */
        double m_occupancyV;
        /** Occupancy in U, for 1 APV cycle */
        double m_occupancyU_APV;
        /** Occupancy in V, for 1 APV cycle */
        double m_occupancyV_APV;
      };

      /** Constructor */
      SVDBackgroundModule();
      /** Destructor */
      virtual ~SVDBackgroundModule();

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
      const double c_APVCycleTime = 31.44 * Unit::ns;   /**< APV cycle time */
      // NIEL file names - placed in vxd/data, so needn't be module parameters.
      /** NIEL-correction file for neutrons */
      const std::string c_niel_neutronFile = "/vxd/data/neutrons.csv";
      /** NIEL-correction file for protons */
      const std::string c_niel_protonFile = "/vxd/data/protons.csv";
      /** NIEL-correction file for pions */
      const std::string c_niel_pionFile = "/vxd/data/pions.csv";
      /** NIEL-correction file for electrons */
      const std::string c_niel_electronFile = "/vxd/data/electrons.csv";
      // No NIEL for photons.

      /** This is a shortcut to getting SVD::SensorInfo from the GeoCache.
       * @param sensorID VxdID of the sensor
       * @return SensorInfo object for the desired sensor.
       */
      inline const SVD::SensorInfo& getInfo(VxdID sensorID) const;
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
      // cppcheck-suppress unusedPrivateFunction
      inline int getTotalSensors();

      // Output directory
      std::string m_outputDirectoryName; /**< Path to directory where output data will be stored */
      std::string m_storeFileMetaDataName; /**< Name of the persistent FileMetaData object */
      std::string m_storeBgMetaDataName; /**< Name of the persistent BackgroundMetaDta object */
      // StoreArrays
      std::string m_storeMCParticlesName; /**< MCParticles StoreArray name */
      std::string m_storeSimHitsName; /**< SVDSimHits StoreArray name */
      std::string m_storeTrueHitsName; /**< SVDTrueHits StoreArray name */
      std::string m_relParticlesTrueHitsName; /**< MCParticlesToSVDTrueHits RelationArray name */
      std::string m_storeDigitsName; /**< SVDDigits StoreArray name */
      std::string m_relDigitsMCParticlesName; /**< StoreArray name of SVDDigits to MCParticles relation */
      std::string m_relDigitsTrueHitsName; /**< StoreArray name of SVDDigits to SVDTrueHits relation */
      std::string m_storeClustersName; /**< SVDClusters StoreArray name */
      std::string m_relClusterDigitName; /**< SVDClustersToSVDDigits RelationArray name */
      std::string m_relTrueHitsSimHitsName;  /**< SVDTrueHitsToSVDSimHits RelationArray name */

      std::string m_storeEnergyDepositsName; /**< SVDEnergyDepositEvents StoreArray name */
      std::string m_storeNeutronFluxesName; /**< SVDNeutronFluxEvents StoreArray name */
      std::string m_storeOccupancyEventsName; /**< SVDOccupancyEvents StoreArray name */

      unsigned short m_doseReportingLevel; /**< 0 - no data, 1 - summary only, 2 - ntuple */
      unsigned short m_nfluxReportingLevel; /**< 0 - no data, 1 - summary only, 2 - ntuple */
      unsigned short m_occupancyReportingLevel; /**< 0 - no data, 1 - summary only, 2 - ntuple */

      std::string m_componentName; /**< Name of the current component. */
      double m_componentTime; /**< Time of current component. */
      double m_triggerWidth; /**< RMS of trigger time measurement.*/
      double m_acceptanceWidth; /**< A hit is accepted if arrived within +/- m_acceptanceWidth * RMS(hit time - trigger time). */

      std::map<VxdID, SensorData> m_sensorData; /**< Struct to hold sensor-wise background data. */

      // NIEL tables
      std::unique_ptr<TNiel> m_nielNeutrons;  /**< Pointer to Niel table for neutrons */
      std::unique_ptr<TNiel> m_nielProtons;   /**< Pointer to Niel table for protons */
      std::unique_ptr<TNiel> m_nielPions;     /**< Pointer to Niel table for pions */
      std::unique_ptr<TNiel> m_nielElectrons; /**< Pointer to Niel table for electrons */

    };

    inline const SVD::SensorInfo& SVDBackgroundModule::getInfo(VxdID sensorID) const
    {
      return dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    }

    inline  double SVDBackgroundModule::getSensorThickness(VxdID sensorID) const
    {
      return getInfo(sensorID).getThickness();
    }

    inline  double SVDBackgroundModule::getSensorMass(VxdID sensorID) const
    {
      const SVD::SensorInfo& info = getInfo(sensorID);
      return info.getWidth() * info.getLength() * info.getThickness() * c_densitySi;
    }

    inline  double SVDBackgroundModule::getSensorArea(VxdID sensorID) const
    {
      const SVD::SensorInfo& info = getInfo(sensorID);
      return info.getWidth() * info.getLength();
    }

    inline int SVDBackgroundModule::getNumSensors(int layerNum)
    {
      VxdID layerID;
      layerID.setLayerNumber(layerNum);
      int result = 0;
      for (auto ladderID : VXD::GeoCache::getInstance().getLadders(layerID))
        result += VXD::GeoCache::getInstance().getSensors(ladderID).size();
      return result;
    }

    /** Get total number of sensors */
    inline int SVDBackgroundModule::getTotalSensors()
    {
      int result = 0;
      for (auto layerID : VXD::GeoCache::getInstance().getLayers(VXD::SensorInfoBase::SVD))
        result += getNumSensors(layerID.getLayerNumber());
      return result;
    }
  } // namespace SVD
} // namespace Belle2
#endif

