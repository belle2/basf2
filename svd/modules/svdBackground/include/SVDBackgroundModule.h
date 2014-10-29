#ifndef SVDBackgroundMODULE_H_
#define SVDBackgroundMODULE_H_

#include <framework/core/HistoModule.h>
#include <framework/gearbox/Unit.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/background/HistogramFactory.h>
#include <vector>
#include <memory>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>

#include <svd/background/niel_fun.h>

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
    class SVDBackgroundModule: public HistoModule { // <- derived from HistoModule class

    public:
      /** Number of VXD layers */

      static const int c_nVXDLayers = 6;

      /** Struct to hold data of an SVD sensor */
      struct SensorData {
        /** Exposition (energy deposited per cm2 and 1 second) */
        double m_expo;
        /** Dose (Gy/smy) */
        double m_dose;
        /** Neutron flux */
        double m_neutronFlux;
        /** Fired pixels in U, per cm2 and second */
        double m_firedU;
        /** Fired pixels in V, per cm2 and second */
        double m_firedV;
        /** Occupancy in U */
        double m_occupancyU;
        /** Occupancy in V */
        double m_occupancyV;
      };

      /** Struct to hold data of a background component */
      struct BackgroundData {
        /** Name of the component */
        std::string m_componentName;
        /** Time equivalent of background sample */
        double m_componentTime;
        /** Data by layer */
        std::map<VxdID, SensorData> m_sensorData;
        // Histograms by component: Fkuencies vs. particle kinetic energy
        /** Raw neutron flux */
        TH1F* m_neutronFlux[c_nVXDLayers];
        /** NIEL-corrected neutron flux */
        TH1F* m_neutronFluxNIEL[c_nVXDLayers];
        /** Raw proton flux */
        TH1F* m_protonFlux[c_nVXDLayers];
        /** NIEL-corrected proton flux */
        TH1F* m_protonFluxNIEL[c_nVXDLayers];
        /** Raw pion flux */
        TH1F* m_pionFlux[c_nVXDLayers];
        /** NIEL-corrected pion flux */
        TH1F* m_pionFluxNIEL[c_nVXDLayers];
        /** Raw electron flux */
        TH1F* m_electronFlux[c_nVXDLayers];
        /** NIEL-corrected electron flux */
        TH1F* m_electronFluxNIEL[c_nVXDLayers];
        /** Raw photon flux */
        TH1F* m_photonFlux[c_nVXDLayers];
        /** NIEL-corrected summary flux */
        TH1F* m_NIELFlux[c_nVXDLayers];
        // Histograms for bar charts (bar per layer)
        /** Bar chart for doses */
        TH1F* m_doseBars;
        /** Bar chart for NIEL flux */
        TH1F* m_neutronFluxBars;
        /** Bar chart for fired u-strips */
        TH1F* m_uFiredBars;
        /** Bar chart for fired v-strips */
        TH1F* m_vFiredBars;
        /** Bar chart for u-strip occupancy */
        TH1F* m_uOccupancyBars;
        /** Bar chart for v-strip occupancy */
        TH1F* m_vOccupancyBars;
      };
      /** Constructor */
      SVDBackgroundModule();
      /** Destructor */
      virtual ~SVDBackgroundModule();

      /* Initialize module */
      virtual void initialize();
      /* Start-of-run initializations */
      virtual void beginRun();
      /* Event processing */
      virtual void event();
      /* End-of-run tasks */
      virtual void endRun();
      /* Final summary and cleanup */
      virtual void terminate();

      /**
       * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
       * to be placed in this function.
       */
      virtual void defineHisto();

    private:

      // General
      const double c_densitySi = 2.3290 * Unit::g_cm3; /**< Density of crystalline Silicon */
      const double c_smy = 1.0e7 * Unit::s;             /**< Seconds in snowmass year */
      // APV sampling
      const double c_APVSampleTime = 31.4 * Unit::ns;      /**< APV sampling time */
      const int c_nAPVSamples = 6;                       /**< Number of APV samples */

      // NIEL file names - placed in svd/data, so needn't be module parameters.
      /** NIEL-correction file for neutrons */
      const std::string c_niel_neutronFile = "svd/data/neutrons.csv";
      /** NIEL-correction file for protons */
      const std::string c_niel_protonFile = "svd/data/protons.csv";
      /** NIEL-correction file for pions */
      const std::string c_niel_pionFile = "svd/data/pions.csv";
      /** NIEL-correction file for electrons */
      const std::string c_niel_electronFile = "svd/data/electrons.csv";
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

      /** Get number of sensors in a layer */
      inline int getNumSensors(int layerNum);
      /** Get layer area */
      inline double getLayerArea(int layerNum);
      /** Get layer mass */
      inline double getLayerMass(int layerNum);
      /** Get total number of sensors */
      inline int getTotalSensors();
      /** Get total area of sensors */
      inline double getTotalArea();
      /** Get total mass of sensors */
      inline double getTotalMass();

      // Output directory
      std::string m_outputDirectoryName; /**< Path to directory where output data will be stored */

      std::string m_storeBgMetaDataName; /**< Name of the persistent BackgroundMetaDta object */
      // StoreArrays
      std::string m_storeMCParticlesName; /**< MCParticles StoreArray name */
      std::string m_storeSimHitsName; /**< SVDSimHits StoreArray name */
      std::string m_storeTrueHitsName; /**< SVDTrueHits StoreArray name */
      std::string m_relParticlesTrueHitsName; /** MCParticlesToSVDTrueHits RelationArray name */
      std::string m_storeDigitsName; /**< SVDDigits StoreArray name */
      std::string m_relDigitsMCParticlesName; /**< StoreArray name of SVDDigits to MCParticles relation */
      std::string m_relDigitsTrueHitsName; /**< StoreArray name of SVDDigits to SVDTrueHits relation */
      std::string m_storeClustersName; /**< SVDClusters StoreArray name */
      std::string m_relClusterDigitName; /**< SVDClustersToSVDDigits RelationArray name */
      std::string m_relTrueHitsSimHitsName;  /**< SVDTrueHitsToSVDSimHits RelationArray name */

      std::vector<std::string> m_componentNames; /**< List of component names.*/
      std::vector<unsigned long> m_componentTimes; /**< List of equivalent times for individual components */
      std::string m_currentComponentName; /**< Name of the current component. */
      std::string m_currentComponentTime; /**< Time of current component. */

      std::map<std::string, BackgroundData> m_data; /**< Container for collected data and histograms by component */
      TTree* m_treeSummary; /**< TTree containing aggregated background data */

      // NIEL tables
      std::unique_ptr<TNiel> m_nielNeutrons;  /**< Pointer to Niel table for neutrons */
      std::unique_ptr<TNiel> m_nielProtons;   /**< Pointer to Niel table for protons */
      std::unique_ptr<TNiel> m_nielPions;     /**< Pointer to Niel table for pions */
      std::unique_ptr<TNiel> m_nielElectrons; /**< Pointer to Niel table for electrons */

      // Histogram factory
      std::unique_ptr<HistogramFactory> m_histogramFactory; /**< Pointer to HistogramFactory class */
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

    /** Get layer area */
    inline double SVDBackgroundModule::getLayerArea(int layerNum)
    {
      VxdID layerID;
      layerID.setLayerNumber(layerNum);
      double result = 0;
      for (auto ladderID : VXD::GeoCache::getInstance().getLadders(layerID))
        for (auto sensorID : VXD::GeoCache::getInstance().getSensors(ladderID))
          result += getSensorArea(sensorID);
      return result;
    }

    /** Get layer mass */
    inline double SVDBackgroundModule::getLayerMass(int layerNum)
    {
      VxdID layerID;
      layerID.setLayerNumber(layerNum);
      double result = 0;
      for (auto ladderID : VXD::GeoCache::getInstance().getLadders(layerID))
        for (auto sensorID : VXD::GeoCache::getInstance().getSensors(ladderID))
          result += getSensorMass(sensorID);
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

    /** Get total area of sensors */
    inline double SVDBackgroundModule::getTotalArea()
    {
      int result = 0;
      for (auto layerID : VXD::GeoCache::getInstance().getLayers(VXD::SensorInfoBase::SVD))
        result += getLayerArea(layerID.getLayerNumber());
      return result;
    }
    /** Get total mass of sensors */
    inline double SVDBackgroundModule::getTotalMass()
    {
      int result = 0;
      for (auto layerID : VXD::GeoCache::getInstance().getLayers(VXD::SensorInfoBase::SVD))
        result += getLayerMass(layerID.getLayerNumber());
      return result;
    }

  } // namespace SVD
} // namespace Belle2
#endif

