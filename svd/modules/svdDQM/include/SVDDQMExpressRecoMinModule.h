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

#pragma once

#include <framework/core/HistoModule.h>
#include <vxd/dataobjects/VxdID.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vector>
#include "TH1I.h"
#include "TH1F.h"

namespace Belle2 {

  /** SVD DQM Module */
  class SVDDQMExpressRecoMinModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    SVDDQMExpressRecoMinModule();
    /* Destructor */
    virtual ~SVDDQMExpressRecoMinModule();

    /** Module function initialize */
    virtual void initialize();
    /** Module function beginRun */
    virtual void beginRun();
    /** Module function event */
    virtual void event();
    /** Module function endRun */
    virtual void endRun();
    /** Module function terminate */
    virtual void terminate();

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    virtual void defineHisto();

  private:

    /** cut for accepting to hitmap histogram, using strips only, default = 22 */
    float m_CutSVDCharge = 22.0;

    /** Name of the histogram directory in ROOT file */
    std::string m_histogramDirectoryName;

    /** SVDShaperDigits StoreArray name */
    std::string m_storeSVDShaperDigitsName;
    /** SVDClusters StoreArray name */
    std::string m_storeSVDClustersName;
    /** SVD diagnostics module name */
    std::string m_svdDAQDiagnosticsListName;

    /** Hitmaps u of Digits */
    TH1I* m_hitMapCountsU;
    /** Hitmaps v of Digits */
    TH1I* m_hitMapCountsV;
    /** Hitmaps u of Clusters*/
    TH1I* m_hitMapClCountsU;
    /** Hitmaps v of Clusters*/
    TH1I* m_hitMapClCountsV;
    /** Fired u strips per event */
    TH1F** m_firedU;
    /** Fired v strips per event */
    TH1F** m_firedV;
    /** u clusters per event */
    TH1F** m_clustersU;
    /** v clusters per event */
    TH1F** m_clustersV;
    /** u charge of clusters */
    TH1F** m_clusterChargeU;
    /** v charge of clusters */
    TH1F** m_clusterChargeV;
    /** u charge of strips */
    TH1F** m_stripSignalU;
    /** v charge of strips */
    TH1F** m_stripSignalV;
    /** u size */
    TH1F** m_clusterSizeU;
    /** v size */
    TH1F** m_clusterSizeV;
    /** u time */
    TH1F** m_clusterTimeU;
    /** v time */
    TH1F** m_clusterTimeV;

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
    /** Number of SVD sensors on Belle II */
    int c_nSVDSensors;

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
