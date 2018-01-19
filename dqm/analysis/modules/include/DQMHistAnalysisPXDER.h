/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Description : An example module for DQM histogram analysis             *
 * Author: The Belle II Collaboration                                     *
 * Author : Bjoern Spruck, Peter Kodys                                    *
 *                                                                        *
 * Prepared for Belle II geometry                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <TROOT.h>
#include <vector>
#include "TH1I.h"
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** PXD DQM AnalysisModule */
  class DQMHistAnalysisPXDERModule : public DQMHistAnalysisModule {

  public:

    /** Constructor */
    DQMHistAnalysisPXDERModule();
    /* Destructor */
    virtual ~DQMHistAnalysisPXDERModule();

    /** Module functions */
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  private:

    /** Basic Directory in output file */
    TDirectory* m_oldDir;

    /** Flags of Hitmaps of Digits */
//     TH1I* m_fHitMapCountsFlag;
    /** Flags of Hitmaps of Clusters*/
//     TH1I* m_fHitMapClCountsFlag;
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

    // Name the histograms, we have to find them anyway every event
    /** Hitmaps of Digits */
//     std::string m_hitMapCounts;
    /** Hitmaps of Clusters*/
//     std::string m_hitMapClCounts;

    /** Fired pixels per event */
    std::vector <std::string> m_fired;
    /** Clusters per event */
    std::vector <std::string> m_clusters;
    /** Start row distribution */
    std::vector <std::string> m_startRow;
    /** Cluster seed charge by distance from the start row */
    std::vector <std::string> m_chargStartRow;
    /** counter for Cluster seed charge by distance from the start row */
    std::vector <std::string> m_startRowCount;
    /** Charge of clusters */
    std::vector <std::string> m_clusterCharge;
    /** Charge of pixels */
    std::vector <std::string> m_pixelSignal;
    /** u cluster size */
    std::vector <std::string> m_clusterSizeU;
    /** v cluster size */
    std::vector <std::string> m_clusterSizeV;
    /** Cluster size */
    std::vector <std::string> m_clusterSizeUV;


    /** Fired pixels per event */
    std::vector <std::string> m_ref_fired;
    /** Clusters per event */
    std::vector <std::string> m_ref_clusters;
    /** Start row distribution */
    std::vector <std::string> m_ref_startRow;
    /** Cluster seed charge by distance from the start row */
    std::vector <std::string> m_ref_chargStartRow;
    /** counter for Cluster seed charge by distance from the start row */
    std::vector <std::string> m_ref_startRowCount;
    /** Charge of clusters */
    std::vector <std::string> m_ref_clusterCharge;
    /** Charge of pixels */
    std::vector <std::string> m_ref_pixelSignal;
    /** u cluster size */
    std::vector <std::string> m_ref_clusterSizeU;
    /** v cluster size */
    std::vector <std::string> m_ref_clusterSizeV;
    /** Cluster size */
    std::vector <std::string> m_ref_clusterSizeUV;

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
    int SetFlag(int Type, int bin, double* pars, double ratio, std::string name_hist, std::string name_refhist, TH1I* flaghist);
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
//     int SetFlag(int Type, int bin, double* pars, double ratio, TH1I* hist, TH1I* refhist, TH1I* flaghist);

    /** Reference Histogram Root file name */
    std::string m_refFileName;
    /** The pointer to the reference file */
    TFile* m_refFile;
    //TH1* findHistLocal(TString& a);
    TH1* GetHisto(TString histoname);

  };

}

