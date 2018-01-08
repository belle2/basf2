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

#ifndef PXDDQMExpressRecoMinMODULE_H_
#define PXDDQMExpressRecoMinMODULE_H_

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
#include "TH1I.h"
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** PXD DQM Module */
  class PXDDQMExpressRecoMinModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    PXDDQMExpressRecoMinModule();
    /* Destructor */
    virtual ~PXDDQMExpressRecoMinModule();

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

    /** cut for accepting to hitmap histogram, using strips only, default = 0 */
    float m_CutPXDCharge = 0.0;

    /** PXDDigits StoreArray name */
    std::string m_storePXDDigitsName;
    /** PXDClusters StoreArray name */
    std::string m_storePXDClustersName;
    /** PXDClustersToPXDDigits RelationArray name */
    std::string m_relPXDClusterDigitName;
    /** Frames StoreArray name */
    std::string m_storeFramesName;

    /** Hitmaps of Digits */
    TH1I* m_hitMapCounts;
    /** Hitmaps of Clusters*/
    TH1I* m_hitMapClCounts;
    /** Fired pixels per event */
    TH1F** m_fired;
    /** Clusters per event */
    TH1F** m_clusters;
    /** Start row distribution */
    TH1F** m_startRow;
    /** Cluster seed charge by distance from the start row */
    TH1F** m_chargStartRow;
    /** counter for Cluster seed charge by distance from the start row */
    TH1F** m_startRowCount;
    /** Charge of clusters */
    TH1F** m_clusterCharge;
    /** Charge of pixels */
    TH1F** m_pixelSignal;
    /** u cluster size */
    TH1F** m_clusterSizeU;
    /** v cluster size */
    TH1F** m_clusterSizeV;
    /** Cluster size */
    TH1F** m_clusterSizeUV;

    /** Number of VXD layers on Belle II */
    int c_nVXDLayers;
    /** Number of PXD layers on Belle II */
    int c_nPXDLayers;
    /** Number of SVD layers on Belle II */
    int c_nSVDLayers;
    /** First VXD layer on Belle II */
    int c_firstVXDLayer;
    /** Last VXD layer on Belle II */
    int c_lastVXDLayer;
    /** First PXD layer on Belle II */
    int c_firstPXDLayer;
    /** Last PXD layer on Belle II */
    int c_lastPXDLayer;
    /** First SVD layer on Belle II */
    int c_firstSVDLayer;
    /** Last SVD layer on Belle II */
    int c_lastSVDLayer;
    /** Number of PXD sensors on Belle II */
    int c_nPXDSensors;

    /** Function return index of sensor in plots.
       * @param Layer Layer position of sensor
       * @param Ladder Ladder position of sensor
       * @param Sensor Sensor position of sensor
       * @return Index of sensor in plots.
       */
    int getSensorIndex(int Layer, int Ladder, int Sensor);
    /** Function return index of sensor in plots.
       * @param Index Index of sensor in plots.
       * @param Layer return Layer position of sensor
       * @param Ladder return Ladder position of sensor
       * @param Sensor return Sensor position of sensor
       */
    void getIDsFromIndex(int Index, int& Layer, int& Ladder, int& Sensor);

  };

}
#endif

