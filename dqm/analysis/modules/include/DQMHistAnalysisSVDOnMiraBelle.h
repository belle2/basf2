/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <dqm/core/DQMHistAnalysis.h>
#include <vxd/dataobjects/VxdID.h>

#include <vector>

#include <TROOT.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {

  /** Class derived from HistoModule, for SVD monitoring variables at MiraBelle */
  class DQMHistAnalysisSVDOnMiraBelleModule final : public DQMHistAnalysisModule {

  public:

    /** Constructor */
    DQMHistAnalysisSVDOnMiraBelleModule();
    /** Destructor */
    ~DQMHistAnalysisSVDOnMiraBelleModule();
    /** Module function initialize */
    void initialize() override final;
    /** Module function beginRun */
    void beginRun() override final;
    /** Module function event */
    void event() override final;
    /** Module function endRun */
    void endRun() override final;
    /** Module function terminate */
    void terminate() override final;

  private:

    /** List of canvases to be added to MonitoringObject */
    TCanvas* m_c_avgEfficiency = nullptr; /**< matched clusters and track clusters */
    TCanvas* m_c_avgOffOccupancy = nullptr; /**< number of ZS5 fired strips */
    TCanvas* m_c_MPVChargeClusterOnTrack = nullptr; /**< charge for clusters on track */
    TCanvas* m_c_MPVSNRClusterOnTrack = nullptr; /**< SNR for clusters on track */
    TCanvas* m_c_MPVTimeClusterOnTrack = nullptr; /**< time for clusters on track */
    TCanvas* m_c_avgMaxBinClusterOnTrack =
      nullptr; /**< average number of the APV sample which corresponds to the maximum amplitude for clusters on track */
    TCanvas* m_c_MeanSVDEventT0 = nullptr; /**< Mean Event T0 from SVD */

    /** Monitoring Object to be produced by this module, which contain defined canvases and monitoring variables */
    MonitoringObject* m_monObj = nullptr;

    //! IDs of all SVD Modules to iterate over
    std::vector<VxdID> m_SVDModules;


    /**
    * Calculate avg offline occupancy for one specific sensor, especially with high occupancy
    * @param iLayer index of layer
    * @param hU, hV pointers to histograms
    * @param iBin index of bin corresponding to sensor (layer, ladder, sensor)
    * @param nEvents number of events
    * @return vector with values for U and V sides
    */
    std::vector<float> highOccupancySensor(int iLayer, TH1F* hU, TH1F* hV, int iBin, int nEvents) const;

    /**
    * Calculate avg offline occupancy for specified sensors
    * @param iLayer index of layer
    * @param hU, hV pointers to histograms
    * @param min, max range of ladders
    * @param offset first bin of relevant sensor
    * @param step depends on number of sensor in each layer
    * @param nEvents number of events
    * @return vector with values for U and V sides
    */
    std::vector<float> avgOccupancyUV(int iLayer, TH1F* hU, TH1F* hV, int min, int max, int offset, int step, int nEvents) const;


    /**
     * Calculate avg offline occupancy for specified layer for time group id = 0
     * @param iLayer index of layer
     * @param nEvents number of events
     * @return vector with values for U and V sides
     */
    std::vector<float>  avgOccupancyGrpId0UV(int iLayer, int nEvents) const;


    /**
    * Calculate avg efficiency for specified sensors
    * @param hMCU, hMCV, hFTU, hFTV pointers to histograms
    * @param minX, maxX range of ladders
    * @param minY, maxY range of sensors
    * @return vector with values for U and V sides
    */
    std::vector<float> avgEfficiencyUV(TH2F* hMCU, TH2F* hMCV, TH2F* hFTU, TH2F* hFTV, int minX, int maxX, int minY, int maxY) const;

    /**
    * Calculate abscissa of max Y bin
    * @param h pointer to histogram
    * @return x value for max Y bin
    */
    float xForMaxY(TH1F* h) const;

    /**
    * Calculate full width at half maximum of histogram
    * @param h pointer to histogram
    * @return x value for fwhm
    */
    float histFWHM(TH1F* h) const;

  };
} // end namespace Belle2





