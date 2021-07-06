/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

    /** initialize the TTrees and create SVDHistograms and SVDAPVHistograms*/
    virtual void beginRun() override;

    /** perform analysis and Draw pdf Canvas */
    virtual void event() override;

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
    TBranch* b_calPeakADCREF = nullptr; /**< strip calPeakADC (ADC of max pulse)*/
    TBranch* b_calPeakTimeREF = nullptr; /**< strip calPeakTime (time of max pulse)*/
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
    TBranch* b_calPeakTimeCHECK = nullptr; /**< strip calPeakTime (time of max pulse)*/
    TBranch* b_calPeakADCCHECK = nullptr; /**< strip calPeakADC (ADC of max pulse)*/
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
    float m_calPeakTimeREF = -1; /**< strip peak time*/
    float m_calPeakADCREF = -1; /**< strip max peak ADC*/
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
    float m_calPeakTimeCHECK = -1; /**< strip peak time*/
    float m_calPeakADCCHECK = -1; /**< strip max peak ADC*/
    float m_pulseWidthCHECK = -1; /**< strip pulse width */

    std::string m_rootFileNameREF = "SVDLocalCalibrationMonitor_experiment5_run92.root";   /**< root file name REFERENCE*/
    std::string m_rootFileNameCHECK = "SVDLocalCalibrationMonitor_experiment5_run408.root";   /**< root file name CHECK*/

    std::string m_idFileNameREF = "refID";   /**< ID of the xml file name REFERENCE*/
    std::string m_idFileNameCHECK = "checkID";   /**< ID of the xml file name CHECK*/

    std::string m_outputPdfName = "SVDLocalCalibrationCheck.pdf"; /**< output pdf filename*/

    bool m_plotGoodAPVs = false; /**< if true also the good APVs are plotted on the DIFF canvas*/

    //analsyis parameters
    int m_cutN_out = -1; /**< maximum number of allowed outliers */
    float m_cutNoise_ave = -1; /**< maximum relative deviation APV-average (noise)*/
    float m_cutNoise_out = -1; /**< maximum relative deviation strip  (noise)*/
    float m_cutCalpeakADC_ave = -1; /**< maximum relative deviation APV-average (calpeakADC)*/
    float m_cutCalpeakADC_out = -1; /**< maximum relative deviation strip  (calpeakADC)*/
    float m_cutPedestal_ave = -1; /**< maximum relative deviation APV-average (pedestal)*/
    float m_cutPedestal_out = -1; /**< maximum relative deviation strip  (pedestal)*/

  private:

    void printConfiguration(); /**< print the configuration of the check of the calibration VS a reference calibration*/

    const int m_apvColors[6] = { 1, 2, 8 , kBlue, 6, 28}; /**< color palette*/
    void   setAPVHistoStyles(SVDAPVHistograms<TH1F>* m_APVhistos); /**< set style of APV histograms*/

    void createLegends(); /**< create the TLegends for the plot*/
    TLegend* m_leg2D = nullptr; /**< legend of the 2D plot*/
    TLegend* m_legU = nullptr;  /**< legend of U-side plot*/
    TLegend* m_legV = nullptr;  /**< legend of V-side plot*/

    void printFirstPage();  /**< print the first page of the output pdf*/
    void printLayerPage(int layer);  /**< print layer-number page*/
    void printPage(VxdID theVxdID, TList* listUBAD, TList* listVBAD, TList* listUGOOD, TList* listVGOOD, TString variable,
                   bool isL3);  /**< print the page relative to a sensor with problematic APVs*/
    void printSummaryPages();  /**< summary page with 2D summary plot*/
    void printLastPage();  /**< print last empty page*/

    int hasAnyProblem(TH1F* h, float cutAve, float cutCOUNT);  /**< return True if the APV has a problem, given a variable*/

    /** NOISES */
    SVDHistograms<TH2F>* m_h2NoiseREF = nullptr; /**< noise VS strip 2D histo */
    SVDHistograms<TH2F>* m_h2NoiseCHECK = nullptr; /**< noise VS strip 2D histo */
    SVDAPVHistograms<TH1F>* m_hNoiseDIFF = nullptr; /**< noise histo */
    SVDSummaryPlots* m_hNoiseSummary = nullptr; /**< noise summary  histo */

    /** CALPEAKS ADC*/
    SVDHistograms<TH2F>* m_h2CalpeakADCREF = nullptr; /**< calpeakADC VS strip 2D histo */
    SVDHistograms<TH2F>* m_h2CalpeakADCCHECK = nullptr; /**< calpeakADC VS strip 2D histo */
    SVDAPVHistograms<TH1F>* m_hCalpeakADCDIFF = nullptr; /**< calpeakADC histo */
    SVDSummaryPlots* m_hCalpeakADCSummary = nullptr; /**< calpeakADC summary  histo */

    /** CALPEAKS TIME*/
    SVDHistograms<TH2F>* m_h2CalpeakTimeREF = nullptr; /**< calpeakTime VS strip 2D histo */
    SVDHistograms<TH2F>* m_h2CalpeakTimeCHECK = nullptr; /**< calpeakTime VS strip 2D histo */
    SVDAPVHistograms<TH1F>* m_hCalpeakTimeDIFF = nullptr; /**< calpeakTime histo */
    SVDSummaryPlots* m_hCalpeakTimeSummary = nullptr; /**< calpeakTime summary  histo */

    /** PEDESTALS */
    SVDHistograms<TH2F>* m_h2PedestalREF = nullptr; /**< pedestal VS strip 2D histo */
    SVDHistograms<TH2F>* m_h2PedestalCHECK = nullptr; /**< pedestal VS strip 2D histo */
    SVDAPVHistograms<TH1F>* m_hPedestalDIFF = nullptr; /**< pedestal histo */
    SVDSummaryPlots* m_hPedestalSummary = nullptr; /**< pedestal summary  histo */

  };
}

