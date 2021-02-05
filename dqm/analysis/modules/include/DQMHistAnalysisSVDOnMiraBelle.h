/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Karol Adamczyk (Karol.Adamczyk@ifj.edu.pl)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <vector>

#include <TROOT.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {
  /*! Class for SVD monitoring variables at MiraBelle */
  class DQMHistAnalysisSVDOnMiraBelleModule : public DQMHistAnalysisModule {

    // Public methods
  public:

    //! Constructor / Destructor
    DQMHistAnalysisSVDOnMiraBelleModule();
    virtual ~DQMHistAnalysisSVDOnMiraBelleModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    // Private methods and attributes
  private:

    // List of canvases to be added to MonitoringObject
    TCanvas* m_c_avgEfficiency = nullptr; /**< matched clusters and track clusters */
    TCanvas* m_c_avgOffOccupancy = nullptr; /**< number of ZS5 fired strips */
    TCanvas* m_c_MPVChargeClusterOnTrack = nullptr; /**< charge for clusters on track */
    TCanvas* m_c_MPVSNRClusterOnTrack = nullptr; /**< SNR for clusters on track */
    TCanvas* m_c_MPVTimeClusterOnTrack = nullptr; /**< time for clusters on track */
    TCanvas* m_c_avgMaxBinClusterOnTrack =
      nullptr; /**< average number of the APV sample which corresponds to the maximum amplitude for clusters on track */

    // Monitoring Object
    MonitoringObject* m_monObj = nullptr; /**< MonitoringObject to be produced by this module */

    /**
    * Calculate avg offline occupancy for one specific sensor, especially with high occupancy
    * @param iLayer index of layer
    * @param hU pointer to histogram
    * @param hU pointer to histogram
    * @param iBin index of bin corresponding to sensor (layer, ladder, sensor)
    * @param nEvents number of events
    * @return vector with values for U and V sides
    */
    std::vector<float> highOccupancySensor(int iLayer, TH1F* hU, TH1F* hV, int iBin, int nEvents) const;

    /**
    * Calculate avg offline occupancy for specified sensors
    * @param iLayer index of layer
    * @param hU pointer to histogram
    * @param hU pointer to histogram
    * @param ibin index of bin corresponding to sensor (layer, ladder, sensor)
    * @param nEvents number of events
    * @return vector with values for U and V sides
    */
    std::vector<float> avgOccupancyUV(int iLayer, TH1F* hU, TH1F* hV, int min, int max, int offset, int step, int nEvents) const;

    /**
    * Calculate avg efficiency for specified sensors
    * @param hMCU, hMCV, hFTU, hFTV pointers to histograms
    * @param minX, minY range of ladders
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





