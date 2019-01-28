/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani, Giulia Casarosa                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>

#include <svd/dataobjects/SVDHistograms.h>
#include <svd/dataobjects/SVDAPVHistograms.h>
#include <svd/dataobjects/SVDSummaryPlots.h>

#include <string>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TList.h>
#include <TCanvas.h>
#include <TLegend.h>

namespace Belle2 {
  /**
   * Module to produce a list of histogram showing the uploaded local calibration constants
   */
  class SVDLocalCalibrationsCheckModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDLocalCalibrationsCheckModule();

    /** initialize the TTrees and check validities of payloads*/
    virtual void beginRun() override;

    /** fill trees and histograms */
    virtual void event() override;

    /** print the payloads uniqueID and write trees and histograms to the rootfile  */
    virtual void endRun() override;

    /* ROOT file related parameters */
    TFile* m_rootFilePtrREF = nullptr; /**< pointer at the REFERENCE root file*/
    TFile* m_rootFilePtrCHECK = nullptr; /**< pointer at the CHECK root file*/
    TTree* m_treeREF = nullptr; /**<pointer at REF tree */
    TTree* m_treeCHECK = nullptr; /**<pointer at CHECK tree*/

    //branches REF
    TBranch* b_runREF = nullptr; /**< run number*/
    TBranch* b_ladderREF = nullptr; /**< ladder number*/
    TBranch* b_layerREF = nullptr; /**< layer number*/
    TBranch* b_sensorREF = nullptr; /**< sensor number*/
    TBranch* b_sideREF = nullptr; /**< sensor side */
    TBranch* b_stripREF = nullptr; /**< strip number*/
    TBranch* b_maskREF = nullptr; /**< strip mask 0/1*/
    TBranch* b_pedestalREF = nullptr; /**< strip pedestal*/
    TBranch* b_gainREF = nullptr; /**< strip gain*/
    TBranch* b_noiseREF = nullptr; /**< strip noise (ADC)*/
    TBranch* b_noiseElREF = nullptr; /**< strip noise (e-)*/
    TBranch* b_peakTimeREF = nullptr; /**< strip peakTime*/
    TBranch* b_pulseWidthREF = nullptr; /**< strip pulse width*/

    //branches CHECK
    TBranch* b_runCHECK = nullptr; /**< run number*/
    TBranch* b_ladderCHECK = nullptr; /**< ladder number*/
    TBranch* b_layerCHECK = nullptr; /**< layer number*/
    TBranch* b_sensorCHECK = nullptr; /**< sensor number*/
    TBranch* b_sideCHECK = nullptr; /**< sensor side */
    TBranch* b_stripCHECK = nullptr; /**< strip number*/
    TBranch* b_maskCHECK = nullptr; /**< strip mask 0/1*/
    TBranch* b_pedestalCHECK = nullptr; /**< strip pedestal*/
    TBranch* b_gainCHECK = nullptr; /**< strip gain*/
    TBranch* b_noiseCHECK = nullptr; /**< strip noise (ADC)*/
    TBranch* b_noiseElCHECK = nullptr; /**< strip noise (e-)*/
    TBranch* b_peakTimeCHECK = nullptr; /**< strip peakTime*/
    TBranch* b_pulseWidthCHECK = nullptr; /**< strip pulse width*/


    //branch variables
    UInt_t m_runREF = -1; /**< run number*/
    UInt_t m_layerREF = -1; /**< layer number*/
    UInt_t m_ladderREF = -1; /**< ladder number */
    UInt_t m_sensorREF = -1; /**< sensor number*/
    UInt_t m_sideREF = -1; /**< sensor side*/
    UInt_t m_stripREF = -1; /**< strip number*/
    float m_maskREF = -1; /**< strip mask 0/1*/
    float m_noiseREF = -1; /**< strip noise (ADC) */
    float m_noiseElREF = -1; /**< strip noise (e-)*/
    float m_pedestalREF = -1; /**< strip pedestal*/
    float m_gainREF = -1; /**< strip gain*/
    float m_peakTimeREF = -1; /**< strip peak time*/
    float m_pulseWidthREF = -1; /**< strip pulse width */


    //branch variables
    UInt_t m_runCHECK = -1; /**< run number*/
    UInt_t m_layerCHECK = -1; /**< layer number*/
    UInt_t m_ladderCHECK = -1; /**< ladder number */
    UInt_t m_sensorCHECK = -1; /**< sensor number*/
    UInt_t m_sideCHECK = -1; /**< sensor side*/
    UInt_t m_stripCHECK = -1; /**< strip number*/
    float m_maskCHECK = -1; /**< strip mask 0/1*/
    float m_noiseCHECK = -1; /**< strip noise (ADC) */
    float m_noiseElCHECK = -1; /**< strip noise (e-)*/
    float m_pedestalCHECK = -1; /**< strip pedestal*/
    float m_gainCHECK = -1; /**< strip gain*/
    float m_peakTimeCHECK = -1; /**< strip peak time*/
    float m_pulseWidthCHECK = -1; /**< strip pulse width */

    std::string m_rootFileNameREF = "SVDLocalCalibrationMonitor_experiment5_run92.root";   /**< root file name */
    std::string m_rootFileNameCHECK = "SVDLocalCalibrationMonitor_experiment5_run408.root";   /**< root file name */

    std::string m_outputPdfName = "SVDLocalCalibrationCheck.pdf";

    bool m_plotGoodAPVs = false;

    //analsyis parameters
    int m_cutN_out = -1; /**< maximum number of allowed outliers */
    float m_cutNoise_ave = -1; /**< maximum relative deviation APV-average (noise)*/
    float m_cutNoise_out = -1; /**< maximum relative deviation strip  (noise)*/
    float m_cutGain_ave = -1; /**< maximum relative deviation APV-average (gain)*/
    float m_cutGain_out = -1; /**< maximum relative deviation strip  (gain)*/
    float m_cutPedestal_ave = -1; /**< maximum relative deviation APV-average (pedestal)*/
    float m_cutPedestal_out = -1; /**< maximum relative deviation strip  (pedestal)*/

  private:

    const int m_apvColors[6] = { 1, 2, 8 , kBlue, 6, 28};
    void   setAPVHistoStyles(SVDAPVHistograms<TH1F>* m_APVhistos);

    void createLegends();
    TLegend* m_leg2D = nullptr;
    TLegend* m_legU = nullptr;
    TLegend* m_legV = nullptr;

    void printFirstPage();
    void printLayerPage(int layer);
    void printPage(VxdID theVxdID, TList* listUBAD, TList* listVBAD, TList* listUGOOD, TList* listVGOOD, TString variable, bool isL3);
    void printSummaryPages();
    void printLastPage();

    int hasAnyProblem(TH1F* h, float cutAve, float cutCOUNT);

    /** MASKS */
    //    SVDHistograms<TH2F>* m_h2MaskREF = nullptr; /**< mask VS strip 2D histo */
    //    SVDHistograms<TH2F>* m_h2MaskCHECK = nullptr; /**< mask VS strip 2D histo */
    //    SVDAPVHistograms<TH1F>* m_hMaskDIFF = nullptr; /**< mask histo */

    /** NOISES */
    SVDHistograms<TH2F>* m_h2NoiseREF = nullptr; /**< noise VS strip 2D histo */
    SVDHistograms<TH2F>* m_h2NoiseCHECK = nullptr; /**< noise VS strip 2D histo */
    SVDAPVHistograms<TH1F>* m_hNoiseDIFF = nullptr; /**< noise histo */
    SVDSummaryPlots* m_hNoiseSummary = nullptr; /**< noise summary  histo */

    /** GAINS */
    SVDHistograms<TH2F>* m_h2GainREF = nullptr; /**< gain VS strip 2D histo */
    SVDHistograms<TH2F>* m_h2GainCHECK = nullptr; /**< gain VS strip 2D histo */
    SVDAPVHistograms<TH1F>* m_hGainDIFF = nullptr; /**< gain histo */
    SVDSummaryPlots* m_hGainSummary = nullptr; /**< gain summary  histo */

    /** PEDESTALS */
    SVDHistograms<TH2F>* m_h2PedestalREF = nullptr; /**< pedestal VS strip 2D histo */
    SVDHistograms<TH2F>* m_h2PedestalCHECK = nullptr; /**< pedestal VS strip 2D histo */
    SVDAPVHistograms<TH1F>* m_hPedestalDIFF = nullptr; /**< pedestal histo */
    SVDSummaryPlots* m_hPedestalSummary = nullptr; /**< pedestal summary  histo */

  };
}

