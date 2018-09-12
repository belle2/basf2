/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Combined TB DESY 2016                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDTelDQMOffLineMODULE_H_
#define VXDTelDQMOffLineMODULE_H_

#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <testbeam/vxd/dataobjects/TelDigit.h>
#include <vector>
#include "TH1F.h"
#include "TH2F.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <testbeam/vxd/dataobjects/TelCluster.h>

#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <set>
#include <boost/format.hpp>

#include "TVector3.h"
#include "TDirectory.h"
//---------------------------------------------------------------------------------------
#include <testbeam/vxd/geometry/SensorInfo.h>
#include <sys/time.h>
#include <memory>
#include <string>
#include <map>
#include <tuple>
#include <limits>

#include <pxd/online/PXDIgnoredPixelsMap.h>
#include <svd/online/SVDIgnoredStripsMap.h>

#include <testbeam/vxd/dataobjects/TelDigit.h>

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

namespace Belle2 {

  /** SVD DQM Module */
  class VXDTelDQMOffLineModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Number of SVD planes and their numbers.
     * The actual (layer, ladder, sensor numbers are (i,1,i), i = 3,4,5,6
     */
    enum {
      c_nSVDPlanes = 4,
      c_firstSVDPlane = 3,
      c_lastSVDPlane = 6,
      c_MaxSensorsInSVDPlane = 5,
      c_nPXDPlanes = 2,
      c_firstPXDPlane = 1,
      c_lastPXDPlane = 2,
      c_MaxSensorsInPXDPlane = 2,
      c_nVXDPlanes = 6,
      c_firstVXDPlane = 1,
      c_lastVXDPlane = 6,
      c_nTelPlanes = 6,
      c_firstTelPlane = 1,
      c_lastTelPlane = 6,
      c_nTBPlanes = 12,
      c_firstTBPlane = 1,
      c_lastTBPlane = 12,
    };

    /** Constructor */
    VXDTelDQMOffLineModule();
    /* Destructor */
    virtual ~VXDTelDQMOffLineModule();

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
    /** Utility functions to convert indices to plane numbers and v.v.,
     * and to protect against range errors.
     */
    inline int indexToPlaneTB(int index) const
    {
      return c_firstTBPlane + index;
    }
    inline int planeToIndexTB(int iPlane) const
    {
      return iPlane - c_firstTBPlane;
    }
    inline int indexToPlaneTel(int index) const
    {
      return c_firstTelPlane + index;
    }
    inline int planeToIndexTel(int iPlane) const
    {
      return iPlane - c_firstTelPlane;
    }
    inline int indexToPlane(int index) const
    {
      return c_firstSVDPlane + index;
    }
    inline int planeToIndex(int iPlane) const
    {
      return iPlane - c_firstSVDPlane;
    }
    inline int indexToPlanePXD(int indexPXD) const
    {
      return c_firstPXDPlane + indexPXD;
    }
    inline int planeToIndexPXD(int iPlanePXD) const
    {
      return iPlanePXD - c_firstPXDPlane;
    }
    inline int indexToPlaneVXD(int indexVXD) const
    {
      return c_firstVXDPlane + indexVXD;
    }
    inline int planeToIndexVXD(int iPlaneVXD) const
    {
      return iPlaneVXD - c_firstVXDPlane;
    }

    /** This is a shortcut to getting SVD::SensorInfo from the GeoCache.
     * @param index Index of the sensor (0,1,2,3), _not_ layer number!
     * @param sensor Number of the sensor (1,.. - depend of layer)!
     * @return SensorInfo object for the desired plane.
     */
    inline const SVD::SensorInfo& getInfo(int index, int sensor) const;

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

    int m_SaveOtherHistos = 1;            /**< flag <0,1> for creation of correlation plots for non-neighboar layers */
    int m_SwapPXD = 0;                    /**< flag <0,1> very special case for swap of u-v coordinates */
    int m_SwapTel = 0;                    /**< flag <0,1> very special case for swap of u-v coordinates */
    float m_CorrelationGranulation = 1;   /**< set granulation of histogram plots, default is 1 mm, min = 0.02 mm, max = 1 mm */

    std::string m_storePXDDigitsName;     /**< PXDDigits StoreArray name */
    std::string m_storeDigitsName;        /**< SVDDigits StoreArray name */
    std::string m_storePXDClustersName;   /**< PXDClusters StoreArray name */
    std::string m_storeSVDClustersName;   /**< SVDClusters StoreArray name */
    std::string m_relPXDClusterDigitName; /**< PXDClustersToPXDDigits RelationArray name */
    std::string m_relClusterDigitName;    /**< SVDClustersToSVDDigits RelationArray name */
    std::string m_storeTelDigitsName;     /**< TelDigits StoreArray name */
    std::string m_storeTelClustersName;   /**< TelClusters StoreArray name */
    std::string m_relTelClusterDigitName; /**< TelClustersToPXDDigits RelationArray name */
    std::string m_histogramDirectoryName; /**< Name of the ROOT file directory for these histograms */

    // DQM for every PXD sensor:
    TH1F* m_firedPxdSen[c_nPXDPlanes * c_MaxSensorsInPXDPlane];      /**< Fired pixels per event by plane */
    TH1F* m_clustersPxdSen[c_nPXDPlanes * c_MaxSensorsInPXDPlane];   /**< Pixels per event by plane */
    TH2F* m_hitMapPxdSen[c_nPXDPlanes * c_MaxSensorsInPXDPlane];     /**< Hitmaps for pixels by plane */
    TH1F* m_chargePxdSen[c_nPXDPlanes * c_MaxSensorsInPXDPlane];     /**< Charge by plane */
    TH1F* m_seedPxdSen[c_nPXDPlanes * c_MaxSensorsInPXDPlane];       /**< Seed by plane */
    TH1F* m_sizePxdSen[c_nPXDPlanes * c_MaxSensorsInPXDPlane];       /**< Cluster size by plane */
    TH1F* m_sizeUPxdSen[c_nPXDPlanes * c_MaxSensorsInPXDPlane];      /**< u cluster size by plane */
    TH1F* m_sizeVPxdSen[c_nPXDPlanes * c_MaxSensorsInPXDPlane];      /**< v cluster size by plane */

    // DQM for every SVD sensor:
    TH1F* m_firedUSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];         /**< Fired u strips per event by plane */
    TH1F* m_firedVSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];         /**< Fired v strips per event by plane */
    TH1F* m_clustersUSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];      /**< u clusters per event by plane */
    TH1F* m_clustersVSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];      /**< v clusters per event by plane */
    TH1F* m_hitMapUSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];        /**< Hitmaps for u-strips clusters by plane */
    TH1F* m_hitMapVSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];        /**< Hitmaps for v-strips clusters by plane */
    TH2F* m_hitMapDigitUSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];   /**< Hitmaps for u-strips all timestamps by plane */
    TH2F* m_hitMapDigitVSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];   /**< Hitmaps for v-strips all timestamps by plane */
    TH1F* m_chargeUSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];        /**< u charge by plane */
    TH1F* m_chargeVSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];        /**< v charge by plane */
    TH1F* m_seedUSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];          /**< u seed by plane */
    TH1F* m_seedVSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];          /**< v seed by plane */
    TH1F* m_sizeUSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];          /**< u size by plane */
    TH1F* m_sizeVSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];          /**< v size by plane */
    TH1F* m_timeUSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];          /**< u time by plane */
    TH1F* m_timeVSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];          /**< v time by plane */

    // DQM for every PXD sensor:
    TH1F* m_firedTelSen[c_nTelPlanes];                                /**< Fired pixels per event by plane */
    TH1F* m_clustersTelSen[c_nTelPlanes];                             /**< Pixels per event by plane */
    TH2F* m_hitMapTelSen[c_nTelPlanes];                               /**< Hitmaps for pixels by plane */
    TH1F* m_sizeTelSen[c_nTelPlanes];                                 /**< Cluster size by plane */

    // DQM for correlations for all sensors in rough granulation:
    TH2F* m_correlations[c_nTBPlanes * c_nTBPlanes];                  /**< Correlations and hit maps from space points */

  };

  inline const SVD::SensorInfo& VXDTelDQMOffLineModule::getInfo(int index, int sensor) const
  {
    int iPlane = indexToPlane(index);
    VxdID sensorID(iPlane, 1, sensor);
    return dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }

  inline const PXD::SensorInfo& VXDTelDQMOffLineModule::getInfoPXD(int index, int sensor) const
  {
    int iPlane = indexToPlanePXD(index);
    VxdID sensorID(iPlane, 1, sensor);
    //VxdID sensorID(iPlane, 1, 2);  // TODO very special case for TB2016
    return dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }

  inline const TEL::SensorInfo& VXDTelDQMOffLineModule::getInfoTel(int index) const
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

