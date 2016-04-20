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

#ifndef VXDDQMOnLineMODULE_H_
#define VXDDQMOnLineMODULE_H_

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
#include <vector>
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** SVD DQM Module */
  class VXDDQMOnLineModule : public HistoModule {  // <- derived from HistoModule class

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
    };

    /** Constructor */
    VXDDQMOnLineModule();
    /* Destructor */
    virtual ~VXDDQMOnLineModule();

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

    int m_Reduce1DCorrelHistos = 0;       /**< flag <0,1> for removing of 1D correlation plots from output */
    int m_SaveOtherHistos = 0;            /**< flag <0,1> for creation of correlation plots for non-neighboar layers */
    int m_SwapPXD = 0;                    /**< flag <0,1> very special case for swap of u-v coordinates */

    std::string m_storePXDDigitsName;     /**< PXDDigits StoreArray name */
    std::string m_storeDigitsName;        /**< SVDDigits StoreArray name */
    std::string m_storePXDClustersName;   /**< PXDClusters StoreArray name */
    std::string m_storeSVDClustersName;   /**< SVDClusters StoreArray name */
    std::string m_relPXDClusterDigitName; /**< PXDClustersToPXDDigits RelationArray name */
    std::string m_relClusterDigitName;    /**< SVDClustersToSVDDigits RelationArray name */
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
    TH1F* m_hitMapUSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];        /**< Hitmaps for u-strips by plane */
    TH1F* m_hitMapVSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];        /**< Hitmaps for v-strips by plane */
    TH1F* m_chargeUSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];        /**< u charge by plane */
    TH1F* m_chargeVSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];        /**< v charge by plane */
    TH1F* m_seedUSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];          /**< u seed by plane */
    TH1F* m_seedVSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];          /**< v seed by plane */
    TH1F* m_sizeUSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];          /**< u size by plane */
    TH1F* m_sizeVSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];          /**< v size by plane */
    TH1F* m_timeUSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];          /**< u time by plane */
    TH1F* m_timeVSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];          /**< v time by plane */

    // DQM for correlations for all sensors in rough granulation:
    TH2F* m_correlationsHitMapsSPGlob[c_nVXDPlanes * c_nVXDPlanes];   /**< Correlations and hit maps from space points */
    TH1F* m_correlationsHitMapsSPGlob1Du[c_nVXDPlanes *
                                         c_nVXDPlanes];/**< Correlations and hit maps from space points - differencies in u*/
    TH1F* m_correlationsHitMapsSPGlob1Dv[c_nVXDPlanes *
                                         c_nVXDPlanes];/**< Correlations and hit maps from space points - differencies in v*/

  };

  inline const SVD::SensorInfo& VXDDQMOnLineModule::getInfo(int index, int sensor) const
  {
    int iPlane = indexToPlane(index);
    VxdID sensorID(iPlane, 1, sensor);
    return dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }

  inline const PXD::SensorInfo& VXDDQMOnLineModule::getInfoPXD(int index, int sensor) const
  {
    int iPlane = indexToPlanePXD(index);
    VxdID sensorID(iPlane, 1, sensor);
    //VxdID sensorID(iPlane, 1, 2);  // TODO very special case for TB2016
    return dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }

}
#endif

