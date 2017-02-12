#ifndef SVDDQM2MODULE_H_
#define SVDDQM2MODULE_H_

#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif
#include <vxd/dataobjects/VxdID.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vector>
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** SVD DQM Module */
  class SVDDQM2Module : public HistoModule {  // <- derived from HistoModule class

    static const char* const type[]; /**< array of strings used to name type of sensor in histo */

  public:

    /** Enum with some basic SVD parameters for plots */
    enum {
      c_nSVDPlanes = 4, /**< Number of SVD planes */
      c_firstSVDPlane = 3, /**< Layer number of the first SVD plane */
      c_lastSVDPlane = 6, /**< Layer number of the last SVD plane */
      c_nTypes = 2, /**< Number of sensor types */
    };

    /** Constructor */
    SVDDQM2Module();
    /* Destructor */
    virtual ~SVDDQM2Module();

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
    /** Utility function to convert index to plane number
      * and to protect against range errors.
      */
    inline int indexToPlane(int index) const
    {
      return c_firstSVDPlane + index;
    }

    /** Utility function to convert plane number to index
     * and to protect against range errors.
     */
    inline int planeToIndex(int iPlane) const
    {
      return iPlane - c_firstSVDPlane;
    }

    /** Utility function to convert index (for loops in source code) to sensor number
      * and to protect against range errors.
      **/
    inline int indexToSensor(int sensorIndex) const
    {
      return (1 + sensorIndex);
    }

    /** Utility function to convert sensor number to index (for loops in source code)
     * and to protect against range errors.
     */
    inline int sensorToIndex(int iSensor, int iPlane) const
    {
      int index;
      if (iPlane == 3) {
        index = 1;
      } else {
        if (iSensor == 1) {
          index = 0;
        } else {
          index = 1;
        }
      }
      return index;
    }

    /** This is a shortcut to getting SVD::SensorInfo from the GeoCache.
     * @param index Index of the sensor (0,1), _not_ layer number!
     * @return SensorInfo object for the desired plane.
     */
    inline const SVD::SensorInfo& getInfo(int layerIndex, int sensorIndex) const;

    std::string m_storeDigitsName;        /**< SVDDigits StoreArray name */
    std::string m_storeClustersName;      /**< SVDClusters StoreArray name */
    std::string m_relClusterDigitName;    /**< SVDClustersToSVDDigits RelationArray name */
    std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */
    //float m_SVDCutSeedL;                  /**< SVD - lower seed cut for acceptans of cluster */
    //float m_SVDCutSeedH;                  /**< SVD - Higher seed cut for acceptans of cluster */

    // +1 in dimensions to protect against noisy VXDID values.
    TH1F* m_fired[c_nSVDPlanes];          /**< Fired strips per event by plane */
    TH1F* m_clusters[c_nSVDPlanes];       /**< clusters per event by plane */
    TH1F* m_hitMapUSlanted[c_nSVDPlanes];        /**< Hitmaps for u-strips by plane */
    TH1F* m_hitMapUBarrel[c_nSVDPlanes];        /**< Hitmaps for u-strips by plane */
    TH1F* m_hitMapVSlanted[c_nSVDPlanes];        /**< Hitmaps for v-strips by plane */
    TH1F* m_hitMapVBarrel[c_nSVDPlanes];        /**< Hitmaps for v-strips by plane */
    TH1F* m_digitCharge[c_nSVDPlanes];    /**< Digit charge by plane */
    TH1F* m_clusterCharge[c_nSVDPlanes];  /**< Cluster charge by plane */
    TH1F* m_seedCharge[c_nSVDPlanes];     /**<  seed by plane */
    TH1F* m_sizeU[c_nSVDPlanes];          /**< u size by plane */
    TH1F* m_sizeV[c_nSVDPlanes];          /**< v size by plane */
  };


  inline const SVD::SensorInfo& SVDDQM2Module::getInfo(int layerIndex, int sensorIndex) const
  {
    int iPlane = indexToPlane(layerIndex);
    int iSensor = indexToSensor(sensorIndex);
    VxdID sensorID(iPlane, 1, iSensor);
    return dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }


}
#endif

