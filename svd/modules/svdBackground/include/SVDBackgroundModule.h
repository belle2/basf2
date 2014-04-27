#ifndef SVDBackgroundMODULE_H_
#define SVDBackgroundMODULE_H_

#include <framework/core/HistoModule.h>
#include <framework/gearbox/Unit.h>
#include <vxd/dataobjects/VxdID.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vector>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>

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
     */
    class SVDBackgroundModule: public HistoModule { // <- derived from HistoModule class

    public:

      /** Struct to hold data of an SVD layer */
      struct LayerData {
        /** Dose (Gy) */
        double m_dose;
        /** Exposition */
        double m_expo;
        /** Fired pixels in U */
        double m_firedU;
        /** Fired pixels in V */
        double m_firedV;
      };
      /** Struct to hold data of a background component */
      struct BackgroundData {
        /** Name of the component */
        std::string m_componentName;
        /** Time equivalent of background sample */
        double m_componentTime;
        /** Data by layer */
        std::map<VxdID, LayerData> m_layerData;
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

      const double c_densitySi = 2.3290 * Unit::g_cm3;  /**< Density of crystalline Silicon */
      const double c_smy = 1.0e7 * Unit::s;             /**< Seconds in snowmass year */

      /** This is a shortcut to getting SVD::SensorInfo from the GeoCache.
       * @param sensorID VxdID of the sensor
       * @return SensorInfo object for the desired sensor.
       */
      inline const SVD::SensorInfo& getInfo(VxdID sensorID) const;
      /** Return mass of the sensor with the given sensor ID */
      inline double getSensorMass(VxdID sensorID) const;
      /** Return area of the sensor with the given sensor ID */
      inline double getSensorArea(VxdID sensorID) const;

      std::string m_storeMCParticlesName; /**< MCParticles StoreArray name */
      std::string m_storeSimHitsName; /**< SVDSimHits StoreArray name */
      std::string m_storeTrueHitsName; /**< SVDTrueHits StoreArray name */
      std::string m_storeDigitsName; /**< SVDDigits StoreArray name */
      std::string m_relDigitsMCParticlesName; /**< StoreArray name of SVDDigits to MCParticles relation */
      std::string m_relDigitsTrueHitsName; /**< StoreArray name of SVDDigits to SVDTrueHits relation */
      std::string m_storeClustersName; /**< SVDClusters StoreArray name */
      std::string m_relClusterDigitName; /**< SVDClustersToSVDDigits RelationArray name */

      std::vector<std::string> m_componentNames; /**< List of component names. Don't know if I use this.*/
      std::vector<unsigned long> m_componentTimes; /**< List of equivalent times for individual components */
      std::string m_currentComponentName; /**< Name of the current component. */
      std::string m_currentComponentTime; /**< Time of current component. */
      bool m_componentChanged; /**< True if the component name changed in current event */

      std::map<std::string, BackgroundData> m_data; /**< containter for collected data and histograms. */
      // TTree* m_tree; /**< TTree containing aggregated background data */

    };

    inline const SVD::SensorInfo& SVDBackgroundModule::getInfo(VxdID sensorID) const
    {
      return dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
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



  } // namespace SVD
} // namespace Belle2
#endif

