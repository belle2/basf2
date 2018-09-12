#ifndef MaskingMODULE_H_
#define MaskingMODULE_H_


#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <svd/geometry/SensorInfo.h>
#include <testbeam/vxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vector>
#include "TH1F.h"
#include "TH2F.h"
#include "TVector3.h"
#include "TDirectory.h"
#include <sys/time.h>
#include <memory>
#include <string>
#include <set>
#include <map>
#include <tuple>
#include <boost/format.hpp>
#include <limits>

#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <pxd/online/PXDIgnoredPixelsMap.h>
#include <svd/online/SVDIgnoredStripsMap.h>

#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <testbeam/vxd/dataobjects/TelDigit.h>

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <framework/dataobjects/FileMetaData.h>
#include <framework/core/InputController.h>

namespace Belle2 {

  /** PXD DQM Module */
  class MaskingModule : public HistoModule {  // <- derived from HistoModule class

  public:

//    /** Simple structure for a pixel, u = map_pixel.first, v = map_pixel.second */
//    typedef std::pair<unsigned short, unsigned short> map_pixel;
    /** Number of PXD planes and their numbers.
     * The actual (layer, ladder, sensor numbers are (i,1,i), i = 1,2
     */
    enum {
      c_nTBPlanes = 12,
      c_firstTBPlane = 1,
      c_lastTBPlane = 12,
      c_nPXDPlanes = 2,
      c_firstPXDPlane = 1,
      c_lastPXDPlane = 2,
      c_MaxSensorsInPXDPlane = 2,
      c_nSVDPlanes = 4,
      c_firstSVDPlane = 3,
      c_lastSVDPlane = 6,
      c_MaxSensorsInSVDPlane = 5,
      c_nTelPlanes = 6,
      c_firstTelPlane = 1,
      c_lastTelPlane = 6,
      c_nTelPlanesFrontGrup = 3,
      c_nVXDPlanes = 6,
      c_firstVXDPlane = 1,
      c_lastVXDPlane = 6,
    };

    /** Constructor */
    MaskingModule();
    /* Destructor */
    virtual ~MaskingModule();

    /** Module functions */
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    int CallSVDFilter(const SVDCluster* cluster);


    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    virtual void defineHisto();

  private:
    /** Utility functions to convert indices to plane numbers and v.v.,
     * and to protect against range errors.
     */
    inline int indexToPlanePXD(int index) const
    {
      return c_firstPXDPlane + index;
    }
    inline int planeToIndexPXD(int iPlane) const
    {
      return iPlane - c_firstPXDPlane;
    }
    inline int indexToPlaneSVD(int index) const
    {
      return c_firstSVDPlane + index;
    }
    inline int planeToIndexSVD(int iPlane) const
    {
      return iPlane - c_firstSVDPlane;
    }
    inline int indexToPlaneVXD(int index) const
    {
      return c_firstVXDPlane + index;
    }
    inline int indexToPlaneTel(int index) const
    {
      return c_firstTelPlane + index;
    }
    inline int planeToIndexTel(int iPlane) const
    {
      return iPlane - c_firstTelPlane;
    }
    // return global TB plane 1 .. 12:
    inline int indexVXDToTBPlane(int index) const      // from local index
    {
      return c_firstTelPlane + c_nTelPlanesFrontGrup + index;
    }
    inline int indexTelToTBPlane(int index) const      // from local index
    {
      return index > c_nTelPlanesFrontGrup - c_firstTelPlane ?
             c_firstTelPlane + index + c_nPXDPlanes + c_nSVDPlanes :
             c_firstTelPlane + index;
    }
    inline int indexToTBPlane(int index) const         // from global index
    {
      return c_firstTelPlane + index;
    }
    // return global TB index 0 .. 11:
    inline int planeToTBIndex(int iPlane) const        // from global plane
    {
      return iPlane - c_firstTelPlane;
    }
    inline int planeVXDToTBIndex(int iPlane) const     // from local plane
    {
      return iPlane - c_firstTelPlane + c_nTelPlanesFrontGrup;
    }
    inline int planeTelToTBIndex(int iPlane) const     // from local plane
    {
      return iPlane > c_nTelPlanesFrontGrup ?
             iPlane - c_firstTelPlane + c_nPXDPlanes + c_nSVDPlanes :
             iPlane - c_firstTelPlane;
    }

    /** This is a shortcut to getting SVD::SensorInfo from the GeoCache.
     * @param index Index of the sensor (0,1,2,3), _not_ layer number!
     * @param sensor Number of the sensor (1,.. - depend of layer)!
     * @return SensorInfo object for the desired plane.
     */
    inline const SVD::SensorInfo& getInfoSVD(int index, int sensor) const;

    /** This is a shortcut to getting PXD::SensorInfo from the GeoCache.
     * @param index Index of the sensor (0,1), _not_ layer number!
     * @param sensor Number of the sensor (1,2)!
     * @return SensorInfo object for the desired plane.
     */
    inline const PXD::SensorInfo& getInfoPXD(int index, int sensor) const;

    /** This is a shortcut to getting TEL::SensorInfo from the GeoCache.
     * @param index Index of the sensor (0,1,2,9,10,11), _not_ layer number!
     * @return SensorInfo object for the desired plane.
     */
    inline const TEL::SensorInfo& getInfoTel(int index) const;

    /** This is a shortcut to getting number of sensors for PXD and SVD layers.
    * @param layer Index of sensor layer (1,6)
    * @return Number of sensors in layer.
    */
    inline int getSensorsInLayer(int layer) const
    {
      int nSensors = 0;
      if ((layer >= 1) && (layer <= 3)) nSensors = 2;
      if (layer == 4) nSensors = 3;
      if (layer == 5) nSensors = 4;
      if (layer == 6) nSensors = 5;
      //if (layer == 1) nSensors = 1;  // TODO very special case for TB2016
      //if (layer == 2) nSensors = 1;  // TODO very special case for TB2016
      return nSensors;
    }

    std::string m_storePXDDigitsName;        /**< PXDDigits StoreArray name */
    std::string m_storePXDClustersName;      /**< PXDClusters StoreArray name */
    std::string m_relPXDClusterDigitName;    /**< PXDClustersToPXDDigits RelationArray name */
    std::string m_storeSVDDigitsName;        /**< SVDDigits StoreArray name */
    std::string m_storeSVDClustersName;      /**< SVDClusters StoreArray name */
    std::string m_relSVDClusterDigitName;    /**< SVDClustersToPXDDigits RelationArray name */
    std::string m_storeTelDigitsName;        /**< TelDigits StoreArray name */
    std::string m_storeTelClustersName;      /**< TelClusters StoreArray name */
    std::string m_relTelClusterDigitName;    /**< TelClustersToPXDDigits RelationArray name */
    std::string m_histogramDirectoryName;    /**< Name of the histogram directory in ROOT file */

    std::string m_storeFileMetaDataName;     /**< Name of the persistent FileMetaData object */

    std::string m_PXDMaskFileBasicName;      /**< Name of file with list of masked channels */
    std::string m_SVDMaskFileBasicName;      /**< Name of file with list of masked channels */
    std::string m_TelMaskFileBasicName;      /**< Name of file with list of masked channels */
    std::string m_PXDMaskFileRunName;        /**< Name of file with list of masked channels */
    std::string m_SVDMaskFileRunName;        /**< Name of file with list of masked channels */
    std::string m_TelMaskFileRunName;        /**< Name of file with list of masked channels */

    long m_nEventsProcess;                   /**< Number of events to process */
    long m_nRealEventsProcess;               /**< Real Number of events to process */
    float m_nEventsProcessFraction;          /**< Fraction of events to process to expected No. */
    long m_nEventsPlane[c_nTBPlanes];        /**< How many events in plane */
    int m_AppendMaskFile;                    /**< Set option for append of existing file or recreate new list */
    float m_PXDCutSeedL;                     /**< PXD - lower seed cut for acceptans of cluster */
    float m_PXDCutSeedH;                     /**< PXD - higher seed cut for acceptans of cluster */
    int m_StatAllEvents;                     /**< Statistics: all events */
    int m_StatSelEvents;                     /**< Statistics: selected events */
    float m_StatEverageOccupancy[c_nTBPlanes * 2]; /**< Statistics: average occupancy for every plane and direction */

    float m_CutSVDCharge = 10;               /**< Cut to show on plot signal over this border  TODO set it more sofisticaly */

    float m_PXDCut = 22.0;                   /**< Cut for masking of PXD pixel - prreset for 10 kEvents */
    float m_TelCut = 12.0;                   /**< Cut for masking of Tel pixel - prreset for 10 kEvents */
    float m_SVDuCut = 500.0;                 /**< Cut for masking of SVD u strip - prreset for 10 kEvents */
    float m_SVDvCut = 1000.0;                /**< Cut for masking of SVD v strip - prreset for 10 kEvents */

    TH2F* m_PXDHitMapUV[c_nPXDPlanes * c_MaxSensorsInPXDPlane];       /**< Hitmaps for pixels by PXD plane */
    TH2F* m_SVDHitMapU[c_nSVDPlanes * c_MaxSensorsInSVDPlane];        /**< SVD hitmaps for u strips and timestamp by plane */
    TH2F* m_SVDHitMapV[c_nSVDPlanes * c_MaxSensorsInSVDPlane];        /**< SVD hitmaps for v strips and timestamp by plane */
    TH2F* m_TelHitMapUV[c_nTelPlanes];                                /**< Hitmaps for pixels by Tel plane */
    TH2F* m_PXDMaskUV[c_nPXDPlanes * c_MaxSensorsInPXDPlane];         /**< mask for pixels by PXD plane */
    TH1F* m_SVDMaskU[c_nSVDPlanes * c_MaxSensorsInSVDPlane];          /**< SVD mask for u strips by plane */
    TH1F* m_SVDMaskV[c_nSVDPlanes * c_MaxSensorsInSVDPlane];          /**< SVD mask for v strips by plane */
    TH2F* m_TelMaskUV[c_nTelPlanes];                                  /**< mask for pixels by Tel plane */
    TH1F* m_SVDSignalU[c_nSVDPlanes * c_MaxSensorsInSVDPlane];        /**< SVD signal for u strips by plane */
    TH1F* m_SVDSignalV[c_nSVDPlanes * c_MaxSensorsInSVDPlane];        /**< SVD signal for v strips by plane */
    TH1F* m_PXDSignal[c_nPXDPlanes * c_MaxSensorsInPXDPlane];         /**< PXD signal for pixels by plane */
  };


  inline const PXD::SensorInfo& MaskingModule::getInfoPXD(int index, int sensor) const
  {
    int iPlane = indexToPlanePXD(index);
    if (sensor != 2) sensor = 2;  // TODO seems very unusual condition ...
    VxdID sensorID(iPlane, 1, sensor);
    return dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }

  inline const SVD::SensorInfo& MaskingModule::getInfoSVD(int index, int sensor) const
  {
    int iPlane = indexToPlaneSVD(index);
    VxdID sensorID(iPlane, 1, sensor);
    return dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }

  inline const TEL::SensorInfo& MaskingModule::getInfoTel(int index) const
  {
    // for only index: 0,1,2,3,4,5.
    int iPlane = indexToPlaneTel(index);
    VxdID sensorID;
    if (iPlane <= 3)
      sensorID = VxdID(7 , 2, iPlane);
    else if (iPlane >= 4)
      sensorID = VxdID(7 , 3, iPlane);
//    sensorID = VxdID(iPlane , 1, iPlane);
    return dynamic_cast<const TEL::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }

}
#endif

