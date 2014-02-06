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

      /** Struct to hold data of a SVD layer */
      struct LayerData {
        /* and some numbers */
        double m_dose;
        double m_expo;
        double m_firedU;
        double m_firedV;
      };
      /** Struct to hold data of a background component */
      /** FIXME: Toto by sme nevedeli nadrbat do TTree? */
      struct BackgroundData {
        std::string m_componentName;
        double m_componentTime;
        std::map<VxdID, LayerData> m_layerData;
        /* and some numbers */
      };

      /** Constructor */
      SVDBackgroundModule();
      /* Destructor */
      virtual ~SVDBackgroundModule();

      /** Module functions */
      virtual void initialize();
      virtual void beginRun();
      virtual void event();
      virtual void endRun();
      virtual void terminate();

      /**
       * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
       * to be placed in this function.
       */
      virtual void defineHisto();

    private:

      const double c_densitySi = 2.3290 * Unit::g_cm3;
      const double c_smy = 1.0e7 * Unit::s;

      /** This is a shortcut to getting SVD::SensorInfo from the GeoCache.
       * @param sensorID VxdID of the sensor
       * @return SensorInfo object for the desired sensor.
       */
      inline const SVD::SensorInfo& getInfo(VxdID sensorID) const;
      inline double getSensorMass(VxdID sensorID) const;
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
      TTree* m_tree;

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

