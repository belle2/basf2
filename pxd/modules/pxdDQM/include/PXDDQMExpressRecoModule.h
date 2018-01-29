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
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vector>
#include "TH1I.h"
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** PXD DQM Module */
  class PXDDQMExpressRecoModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    PXDDQMExpressRecoModule();
    /* Destructor */
    virtual ~PXDDQMExpressRecoModule();

  private:
    /** Module functions */
    void initialize() override final;
    void beginRun() override final;
    void event() override final;
    void endRun() override final;
    void terminate() override final;

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    void defineHisto() override final;

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

    /** Basic Directory in output file */
    TDirectory* m_oldDir;

    /** Name of file contain reference histograms, default=VXD-ReferenceHistos */
    std::string m_RefHistFileName = "vxd/data/VXD-DQMReferenceHistos.root";
    /** Number of events */
    int m_NoOfEvents;
    /** Number of events in reference histogram */
    int m_NoOfEventsRef;

    /** Using local files instead of DataBase for reference histogram, default=0 */
    int m_NotUseDB = 0;
    /** Create and fill reference histograms in DataBase, default=0 */
    int m_CreateDB = 0;

    /** Flags of Hitmaps of Digits */
    TH1I* m_fHitMapCountsFlag;
    /** Flags of Hitmaps of Clusters*/
    TH1I* m_fHitMapClCountsFlag;
    /** Flags of Fired Digits */
    TH1I* m_fFiredFlag;
    /** Flags of Clusters per event */
    TH1I* m_fClustersFlag;
    /** Flags of Start row distribution */
    TH1I* m_fStartRowFlag;
    /** Flags of Cluster seed charge by distance from the start row */
    TH1I* m_fChargStartRowFlag;
    /** Flags of counter for Cluster seed charge by distance from the start row */
    TH1I* m_fStartRowCountFlag;
    /** Flags of Charge of clusters */
    TH1I* m_fClusterChargeFlag;
    /** Flags of Charge of pixels */
    TH1I* m_fPixelSignalFlag;
    /** Flags of u cluster size */
    TH1I* m_fClusterSizeUFlag;
    /** Flags of v cluster size */
    TH1I* m_fClusterSizeVFlag;
    /** Flags of Cluster size */
    TH1I* m_fClusterSizeUVFlag;

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

    /** Number of pixels on PXD v direction */
    int m_nPixels;
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
    void getIDsFromIndex(int Index, int* Layer, int* Ladder, int* Sensor);
    /** Function return flag histogram filled based on condition from TH1F source.
       * Flag values:
       * -3: nonexisting Type
       * -2: histogram is missing or masked
       * -1: less than 100 samles, skip comparition
       *  0: good much with reference
       *  1: warning level = diff > 6 * sigma and < error level
       *  2: error level = diff > 10 * sigma
       * @param Type Set type of condition for flag calculation.
       * 1: use counts, mean and RMS.
       * 2: use counts only.
       * 3: use mean only.
       * 4: use RMS only.
       * 5: use counts and mean.
       * 9: use bin content only.
       * 10: use Chi2 condition and pars[0] and pars[1].
       * 100: nothing do just fill flags as OK.
       * @param bin bin which is fill in flag histogram.
       * @param pars array of parameters need for condition.
       * @param ratio Ratio of acquired events to reference events.
       * @param hist Histogram of sources.
       * @param refhist Reference histogram.
       * @param flag Histogram of flags.
       * @return Indication of succes of realizing of condition, 1: OK.
       */
    int SetFlag(int Type, int bin, double* pars, double ratio, TH1F* hist, TH1F* refhist, TH1I* flaghist);
    /** Function return flag histogram filled based on condition from TH1I source.
       * Flag values:
       * -3: nonexisting Type
       * -2: histogram is missing or masked
       * -1: less than 100 samles, skip comparition
       *  0: good much with reference
       *  1: warning level = diff > 6 * sigma and < error level
       *  2: error level = diff > 10 * sigma
       * @param Type Set type of condition for flag calculation.
       * 1: use counts, mean and RMS.
       * 2: use counts only.
       * 3: use mean only.
       * 4: use RMS only.
       * 5: use counts and mean.
       * 9: use bin content only.
       * 10: use Chi2 condition and pars[0] and pars[1].
       * 100: nothing do just fill flags as OK.
       * @param bin bin which is fill in flag histogram.
       * @param pars array of parameters need for condition.
       * @param ratio Ratio of acquired events to reference events.
       * @param hist Histogram of sources.
       * @param refhist Reference histogram.
       * @param flag Histogram of flags.
       * @return Indication of succes of realizing of condition, 1: OK.
       */
    int SetFlag(int Type, int bin, double* pars, double ratio, TH1I* hist, TH1I* refhist, TH1I* flaghist);

    /** Function for filling of TH1F histogram to database.
       * @param HistoBD Histogram for DB.
       */
    void CreateDBHisto(TH1F* HistoBD);
    /** Function for filling of TH1I histogram to database.
       * @param HistoBD Histogram for DB.
       */
    void CreateDBHisto(TH1I* HistoBD);

    /** Function for filling of group of TH1F histogram to database.
       * @param HistoBD Histogram for DB.
       * @param Number Number of histograms to glue to one.
       */
    void CreateDBHistoGroup(TH1F** HistoBD, int Number);
    /** Function for filling of group of TH1I histogram to database.
       * @param HistoBD Histogram for DB.
       * @param Number Number of histograms to glue to one.
       */
    void CreateDBHistoGroup(TH1I** HistoBD, int Nomber);

    /** Function for loading of TH1F histogram from database.
       * @param HistoBD Histogram for DB.
       * @return Indication of succes of realizing of condition, 1: OK.
       */
    int LoadDBHisto(TH1F* HistoBD);
    /** Function for loading of TH1I histogram from database.
       * @param HistoBD Histogram for DB.
       * @return Indication of succes of realizing of condition, 1: OK.
       */
    int LoadDBHisto(TH1I* HistoBD);

    /** Function for loading of group of TH1F histogram from database.
       * @param HistoBD Histogram for DB.
       * @param Number Number of histograms to extract from DB.
       * @return Indication of succes of realizing of condition, 1: OK.
       */
    int LoadDBHistoGroup(TH1F** HistoBD, int Number);
    /** Function for loading of group of TH1I histogram from database.
       * @param HistoBD Histogram for DB.
       * @param Number Number of histograms to extract from DB.
       * @return Indication of succes of realizing of condition, 1: OK.
       */
    int LoadDBHistoGroup(TH1I** HistoBD, int Nomber);

  };

}

