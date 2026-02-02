/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* DQM headers. */
#include <dqm/core/DQMHistAnalysis.h>

/* ROOT headers. */
#include <TH2.h>

/* C++ headers. */
#include <string>

/* KLM headers */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/bklm/geometry/GeometryPar.h>

namespace Belle2 {

  /**
   * Analysis of HLT histograms for KLM Monitoring Object on Mirabelle
   */
  class DQMHistAnalysisKLMMonObjModule final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisKLMMonObjModule();

    /** Destructor. */
    ~DQMHistAnalysisKLMMonObjModule();

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

    /**
     * This method is called if the current run ends.
     */
    void endRun() override final;


  private:

    /**
     * Calculate KLM hit rate for a specific layer from TH2 histogram.
     * Formula: hitRate = entries / (totalEvents * lookbackWindow * layerArea)
     * @param[in]  hist2D         TH2 histogram containing occupancy data
     * @param[in]  layer          Layer number to calculate hit rate for
     * @param[in]  totalEvents    Total number of events passed delayed bhabha timing decision (TTYP_DPHY == 1)
     * @param[in]  layerArea      Area of the KLM layer in cm^2
     * @param[out] hitRate        Calculated KLM hit rate for the specific layer (Hz/cm^2)
     * @param[out] hitRateErr     Error on KLM hit rate for the specific layer (Hz/cm^2)
     */
    void CalculateKLMHitRate(TH2* hist2D, int layer, Double_t totalEvents, Double_t layerArea,
                             Double_t& hitRate, Double_t& hitRateErr);


    /** Run type flag for physics runs. */
    bool m_IsPhysicsRun = false;

    /** Run type flag for null runs. */
    bool m_IsNullRun;

    /** BKLM element numbers. */
    const BKLMElementNumbers* m_BklmElementNumbers;

    /** KLM Monitoring object. */
    MonitoringObject* m_klmMonObj {};

    /** Name of histogram directory */
    std::string m_histogramDirectoryName;

    /** Lookback window in seconds for hit rate calculation (default: 10.4 microseconds) */
    Double_t m_lookbackWindow;

    /** Tag suffix based on injection veto */
    const std::string m_tag[2] = {"OUT", "IN"};

    /** Name of histogram suffix based on tag */
    const std::string m_title[2] = {"[Outside Active Veto Window]", "[Inside Active Veto Window]"};

    /** BKLM Geometry data. */
    const bklm::GeometryPar* m_bklmGeoPar;

  };//class definition end

}//namespace end
