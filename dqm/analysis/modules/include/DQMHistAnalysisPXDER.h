/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <dqm/core/DQMHistAnalysis.h>
#include <TFile.h>

namespace Belle2 {
  /*! PXD DQM AnalysisModule */

  class DQMHistAnalysisPXDERModule final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisPXDERModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisPXDERModule();

    /**
     * Initializer.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     */
    void beginRun() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;


  private:

    /** Basic Directory in output file */
    //TDirectory* m_oldDir;

    /** Flags of Hitmaps of Digits */
//     TH1I* m_fHitMapCountsFlag;
    /** Flags of Hitmaps of Clusters*/
//     TH1I* m_fHitMapClCountsFlag;
    /** Flags of Fired Digits */
    TH1I* m_fFiredFlag = nullptr;
    /** Flags of Clusters per event */
    TH1I* m_fClustersFlag = nullptr;
    /** Flags of Start row distribution */
    TH1I* m_fStartRowFlag = nullptr;
    /** Flags of Cluster seed charge by distance from the start row */
    TH1I* m_fChargStartRowFlag = nullptr;
    /** Flags of counter for Cluster seed charge by distance from the start row */
    TH1I* m_fStartRowCountFlag = nullptr;
    /** Flags of Charge of clusters */
    TH1I* m_fClusterChargeFlag = nullptr;
    /** Flags of Charge of pixels */
    TH1I* m_fPixelSignalFlag = nullptr;
    /** Flags of u cluster size */
    TH1I* m_fClusterSizeUFlag = nullptr;
    /** Flags of v cluster size */
    TH1I* m_fClusterSizeVFlag = nullptr;
    /** Flags of Cluster size */
    TH1I* m_fClusterSizeUVFlag = nullptr;

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
    //int m_nPixels;
    /** Number of VXD layers on Belle II */
    int c_nVXDLayers = 0;
    /** Number of PXD layers on Belle II */
    int c_nPXDLayers = 0;
    /** Number of SVD layers on Belle II */
    int c_nSVDLayers = 0;
    /** First VXD layer on Belle II */
    int c_firstVXDLayer = 0;
    /** Last VXD layer on Belle II */
    int c_lastVXDLayer = 0;
    /** First PXD layer on Belle II */
    int c_firstPXDLayer = 0;
    /** Last PXD layer on Belle II */
    int c_lastPXDLayer = 0;
    /** First SVD layer on Belle II */
    int c_firstSVDLayer = 0;
    /** Last SVD layer on Belle II */
    int c_lastSVDLayer = 0;
    /** Number of PXD sensors on Belle II */
    int c_nPXDSensors = 0;

    /** Function return index of sensor in plots.
       * @param Index Index of sensor in plots.
       * @param Layer return Layer position of sensor
       * @param Ladder return Ladder position of sensor
       * @param Sensor return Sensor position of sensor
       */
    void getIDsFromIndex(const int Index, int& Layer, int& Ladder, int& Sensor)  const;
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
       * @param name_hist Histogram of sources.
       * @param name_refhist Reference histogram.
       * @param flaghist Histogram of flags.
       * @return Indication of succes of realizing of condition, 1: OK.
       */
    int SetFlag(int Type, int bin, const double* pars, double ratio, const std::string& name_hist, const std::string& name_refhist,
                TH1I* flaghist);
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

    /** Histogram Directory name */
    std::string m_histogramDirectoryName;
    /** Reference Histogram Root file name */
    std::string m_refFileName;
    /** The pointer to the reference file */
    TFile* m_refFile = nullptr;

  };

}

