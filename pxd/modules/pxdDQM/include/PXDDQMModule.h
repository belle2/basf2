/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Belle II geometry                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDDQMMODULE_H_
#define PXDDQMMODULE_H_

#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vector>
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** PXD DQM Module */
  class PXDDQMModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    PXDDQMModule();
    /* Destructor */
    virtual ~PXDDQMModule();

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

    float m_CutPXDCharge = 0.0;            /**< cut for accepting to hitmap histogram, using strips only, default = 0 */
    int m_UsePixels = 0;                   /**< flag <0,1> for using pixels only, no clusters will be required, default = 0 */
    int m_SaveOtherHistos = 1;             /**< flag <0,1> for creation of more plots for experts mostly, default = 1 */

    std::string m_storePXDDigitsName;      /**< PXDDigits StoreArray name */
    std::string m_storePXDClustersName;    /**< PXDClusters StoreArray name */
    std::string m_relPXDClusterDigitName;  /**< PXDClustersToPXDDigits RelationArray name */
    std::string m_storeFramesName;         /**< Frames StoreArray name */

    TH1F** m_fired;          /**< Fired pixels per event */
    TH1F** m_clusters;       /**< Clusters per event */
    TH1F** m_hitMapU;        /**< Hitmaps pixels for u */
    TH1F** m_hitMapV;        /**< Hitmaps pixels for v */
    TH2F** m_hitMap;         /**< Hitmaps pixels */
    TH1F** m_hitMapUCl;      /**< Hitmaps clusters for u */
    TH1F** m_hitMapVCl;      /**< Hitmaps clusters for v */
    TH2F** m_hitMapCl;       /**< Hitmaps clusters */
    TH1F** m_charge;         /**< Charge of clusters */
    TH1F** m_chargePix;      /**< Charge of pixels */
    TH1F** m_seed;           /**< Seed */
    TH1F** m_sizeU;          /**< u cluster size */
    TH1F** m_sizeV;          /**< v cluster size */
    TH1F** m_size;           /**< Cluster size */
    TH1F** m_startRow;       /**< Start row distribution */
    TH1F** m_chargStartRow;  /**< Cluster seed charge by distance from the start row */
    TH1F** m_StartRowCount;  /**< counter for Cluster seed charge by distance from the start row */

    int c_nVXDLayers;                /**< Number of VXD layers on Belle II */
    int c_nPXDLayers;                /**< Number of PXD layers on Belle II */
    int c_nSVDLayers;                /**< Number of SVD layers on Belle II */
    int c_firstVXDLayer;             /**< First VXD layer on Belle II */
    int c_lastVXDLayer;              /**< Last VXD layer on Belle II */
    int c_firstPXDLayer;             /**< First PXD layer on Belle II */
    int c_lastPXDLayer;              /**< Last PXD layer on Belle II */
    int c_firstSVDLayer;             /**< First SVD layer on Belle II */
    int c_lastSVDLayer;              /**< Last SVD layer on Belle II */
    int c_nPXDSensors;               /**< Number of PXD sensors on Belle II */

    /**< Function return index of sensor in plots.
       * @param Layer Layer position of sensor
       * @param Ladder Ladder position of sensor
       * @param Sensor Sensor position of sensor
       * @return Index of sensor in plots.
       */
    int getSensorIndex(int Layer, int Ladder, int Sensor);
    /**< Function return index of sensor in plots.
       * @param Index Index of sensor in plots.
       * @param Layer return Layer position of sensor
       * @param Ladder return Ladder position of sensor
       * @param Sensor return Sensor position of sensor
       */
    void getIDsFromIndex(int Index, int* Layer, int* Ladder, int* Sensor);

  };

}
#endif

