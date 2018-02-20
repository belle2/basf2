/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Phase 2 and Belle II geometry                             *
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
  class SVDDQMExpressRecoModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    SVDDQMExpressRecoModule();
    /* Destructor */
    virtual ~SVDDQMExpressRecoModule();

    /** Module function initialize */
    void initialize() override final;
    /** Module function beginRun */
    void beginRun() override final;
    /** Module function event */
    void event() override final;

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    void defineHisto() override final;

  private:

    /** cut for accepting to hitmap histogram, using strips only, default = 22 ADU */
    float m_CutSVDCharge = 22.0;
    /** cut for accepting clusters to hitmap histogram, default = 12 ke- */
    float m_CutSVDClusterCharge = 12000;

    /** No of FADCs, for Phase2: 5,
     *  TODO add to VXD::GeoCache& geo = VXD::Ge... geo.getFADCs() for
     *  keep universal code for Phase 2 and 3
    */
    int c_nFADC = 5;

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
    /** Hitmaps of digits on chips */
    TH1I* m_hitMapCountsChip;
    /** Hitmaps of clusters on chips */
    TH1I* m_hitMapClCountsChip;
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
    /** u charge of clusters for all sensors */
    TH1F* m_clusterChargeUAll;
    /** v charge of clusters for all sensors */
    TH1F* m_clusterChargeVAll;
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
    /** u time of clusters for all sensors */
    TH1F* m_clusterTimeUAll;
    /** v time of clusters for all sensors */
    TH1F* m_clusterTimeVAll;

    /** Counter of APV errors (16) */
    TH1I** m_CounterAPVErrors;
    /** Counter of FTB errors (256) */
    TH1I** m_CounterFTBErrors;
    /** Counter of apvErrorOR (16) */
    TH1I** m_CounterApvErrorORErrors;
    /** Counter of FTB Flags (32) */
    TH1I** m_CounterFTBFlags;

    /** Number of SVD chips per sensor in u,v in layer 3 (=6) on Belle II */
    int c_nSVDChipsL3 = 6;
    /** Number of SVD chips per sensor in u in layers 4,5,6 (=6) on Belle II */
    int c_nSVDChipsLu = 6;
    /** Number of SVD chips per sensor in v in layers 4,5,6 (=4) on Belle II */
    int c_nSVDChipsLv = 4;
    /** Number of SVD strips per chip on Belle II */
    int c_nSVDChannelsPerChip = 128;
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
    /** Number of SVD chips on Belle II */
    int c_nSVDChips;

    /** Function return index of chip in plots.
       * @param Layer Layer position of sensor
       * @param Ladder Ladder position of sensor
       * @param Sensor Sensor position of sensor
       * @param Chip Chip position on sensor
       * @return Index of sensor in plots.
       */
    int getChipIndex(const int Layer, const int Ladder, const int Sensor, const int Chip, const int IsU) const;
    /** Function return position indexes of chipID in plots.
       * @param Index Index of sensor in plots.
       * @param Layer return Layer position of sensor
       * @param Ladder return Ladder position of sensor
       * @param Sensor return Sensor position of sensor
       * @param Chip return Chip position on sensor
       */
    void getIDsFromChipIndex(const int Index, int& Layer, int& Ladder, int& Sensor, int& Chip, int& IsU) const;

    /** Function return index of sensor in plots.
       * @param Layer Layer position of sensor
       * @param Ladder Ladder position of sensor
       * @param Sensor Sensor position of sensor
       * @return Index of sensor in plots.
       */
    int getSensorIndex(const int Layer, const int Ladder, const int Sensor) const;
    /** Function return position indexes of sensorID in plots.
       * @param Index Index of sensor in plots.
       * @param Layer return Layer position of sensor
       * @param Ladder return Ladder position of sensor
       * @param Sensor return Sensor position of sensor
       */
    void getIDsFromIndex(const int Index, int& Layer, int& Ladder, int& Sensor) const;

  };

}
