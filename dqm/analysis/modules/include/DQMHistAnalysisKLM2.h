/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* DQM headers. */
#include <dqm/analysis/modules/DQMHistAnalysis.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>

/* Belle 2 headers. */
#include <framework/database/DBObjPtr.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/KLMChannelArrayIndex.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMSectorArrayIndex.h>
#include <klm/dbobjects/KLMElectronicsMap.h>

/* ROOT headers. */
#include <TCanvas.h>
#include <TH1.h>
#include <TH2F.h>
#include <TText.h>
#include <TLine.h>

/* C++ headers. */
#include <vector>

namespace Belle2 {

  /**
   * Analysis of KLM DQM histograms.
   */
  class DQMHistAnalysisKLM2Module : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisKLM2Module();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisKLM2Module();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override;

  private:


    /**
     * Process histogram containing the number of hits in plane.
     * @param[in]  histName  Histogram name.
     * @param[in]  histogram Histogram itself.
     */
    void processPlaneHistogram(const std::string& histName, TH1* histogram);


    /** TLine for boundary in plane histograms. */
    TLine m_PlaneLine;

    /** TText for names in plane histograms. */
    TText m_PlaneText;

    /** Histogram for BKLM plane efficiency. */
    TH1* m_eff_bklm = nullptr;

    /** BKLM plane efficiency canvas */
    TCanvas* m_c_eff_bklm = nullptr;

    /** Histogram for EKLM plane efficiency. */
    TH1* m_eff_eklm = nullptr;

    /** EKLM plane efficiency canvas */
    TCanvas* m_c_eff_eklm = nullptr;

    /** Histogram for BKLM sector efficiency. */
    TH1* m_eff_bklm_sector = nullptr;

    /** Histogram for BKLM sector efficiency. */
    TCanvas* m_c_eff_bklm_sector = nullptr;

    /** Histogram for EKLM sector efficiency. */
    TH1* m_eff_eklm_sector = nullptr;

    /** Histogram for EKLM sector efficiency. */
    TCanvas* m_c_eff_eklm_sector = nullptr;


    /** EKLM element numbers. */
    const EKLMElementNumbers* m_EklmElementNumbers;


    /*******************************************/
    //OTHER USEFUL VARIABLES
    /*******************************************/

    /** Number of channel hit histograms per sector for BKLM. */
    const int m_ChannelHitHistogramsBKLM = 2;

    /** Number of channel hit histograms per sector for EKLM. */
    const int m_ChannelHitHistogramsEKLM = 3;


    /** Number of layers/planes for BKLM. */
    const int m_PlaneNumBKLM = BKLMElementNumbers::getMaximalLayerGlobalNumber(); //15 layers per octant, forward and backward

    /** Number of layers/planes for EKLM. */
    const int m_PlaneNumEKLM = EKLMElementNumbers::getMaximalPlaneGlobalNumber();//12 or 14 layers per quadrant, forward and backward

    /** Number of layers/planes for BKLM. */
    const int m_SectorNumBKLM = BKLMElementNumbers::getMaximalSectorGlobalNumber();

    /** Number of layers/planes for EKLM. */
    const int m_SectorNumEKLM = EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder();;



  };

}
