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

#include <svd/calibration/SVDDetectorConfiguration.h>
#include <svd/calibration/SVDFADCMaskedStrips.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDPedestalCalibrations.h>
//#include <svd/calibration/SVDOccupancyCalibrations.h>
#include <svd/calibration/SVDHotStripsCalibrations.h>

#include <svd/dataobjects/SVDHistograms.h>
#include <svd/dataobjects/SVDSummaryPlots.h>

#include <string>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {
  /**
   * Module to produce a list of histogram showing the uploaded local calibration constants
   */
  class SVDLocalCalibrationsMonitorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDLocalCalibrationsMonitorModule();

    /** initialize the TTrees and check validities of payloads*/
    virtual void beginRun() override;

    /** fill trees and histograms */
    virtual void event() override;

    /** print the payloads uniqueID and write trees and histograms to the rootfile  */
    virtual void endRun() override;

    /* ROOT file related parameters */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */
    TTree* m_tree = nullptr; /**<pointer at tree containing the mean and RMS of calibration constants */
    TTree* m_treeDetailed = nullptr; /**<pointer at tree containing the calibration constants of each strip*/

    //branches
    TBranch* b_exp = nullptr; /**< exp number*/
    TBranch* b_run = nullptr; /**< run number*/
    TBranch* b_date = nullptr; /**< date of the noise local run in yyyy-mm-dd format*/
    TBranch* b_hv = nullptr; /**< HV*/
    TBranch* b_ladder = nullptr; /**< ladder number*/
    TBranch* b_layer = nullptr; /**< layer number*/
    TBranch* b_sensor = nullptr; /**< sensor number*/
    TBranch* b_side = nullptr; /**< sensor side */
    TBranch* b_strip = nullptr; /**< strip number*/
    TBranch* b_occupancy = nullptr; /**< strip occupancy*/
    TBranch* b_mask = nullptr; /**< strip mask 0/1*/
    TBranch* b_maskAVE = nullptr; /**< average sensor mask*/
    TBranch* b_hotstrips = nullptr; /**< strip hotstrips 0/1*/
    TBranch* b_hotstripsAVE = nullptr; /**< average sensor hotstrips*/
    TBranch* b_pedestal = nullptr; /**< strip pedestal*/
    TBranch* b_pedestalAVE = nullptr; /**< average sensor pedestal*/
    TBranch* b_pedestalRMS = nullptr; /**< rms sensor pedestal*/
    TBranch* b_gain = nullptr; /**< strip gain*/
    TBranch* b_gainAVE = nullptr; /**< sensor gain average*/
    TBranch* b_gainRMS = nullptr; /**< sensor gain rms*/
    TBranch* b_noise = nullptr; /**< strip noise (ADC)*/
    TBranch* b_noiseEl = nullptr; /**< strip noise (e-)*/
    TBranch* b_noiseAVE = nullptr; /**< sensor noise average (ADC)*/
    TBranch* b_noiseRMS = nullptr; /**< sensor noise rms (ADC)*/
    TBranch* b_noiseElAVE = nullptr; /**< sensor noise average (e-)*/
    TBranch* b_noiseElRMS = nullptr; /**< sensor noise rms (e-)*/
    TBranch* b_occupancyAVE = nullptr; /**< sensor occupancy average (ADC)*/
    TBranch* b_occupancyRMS = nullptr; /**< sensor occupancy rms (ADC)*/
    TBranch* b_calPeakADC = nullptr; /**< strip calPeakADC*/
    TBranch* b_calPeakADCAVE = nullptr; /**< sensor calPeakADC average*/
    TBranch* b_calPeakADCRMS = nullptr; /**< sensor calPeakADC arm*/
    TBranch* b_calPeakTime = nullptr; /**< strip calPeakTime*/
    TBranch* b_calPeakTimeAVE = nullptr; /**< sensor calPeakTime average*/
    TBranch* b_calPeakTimeRMS = nullptr; /**< sensor calPeakTime arm*/
    TBranch* b_pulseWidth = nullptr; /**< strip pulse width*/
    TBranch* b_pulseWidthAVE = nullptr; /**< sensor pulse width average*/
    TBranch* b_pulseWidthRMS = nullptr; /**< sensor pulse width rms*/

    //branch variables
    int m_exp = -1; /**< exp number*/
    int m_run = -1; /**< run number*/
    char m_date[11] = ""; /**< date of the noise local run in yyyy-mm-dd format*/
    float m_hv = -1; /**< applied hv=Vbias/2*/
    int m_layer = -1; /**< layer number*/
    int m_ladder = -1; /**< ladder number */
    int m_sensor = -1; /**< sensor number*/
    int m_side = -1; /**< sensor side*/
    int m_strip = -1; /**< strip number*/
    float m_mask = -1; /**< strip mask 0/1*/
    float m_maskAVE = -1; /**< sensor mask average*/
    float m_hotstrips = -1; /**< strip hotstrips 0/1*/
    float m_hotstripsAVE = -1; /**< sensor hotstrips average*/
    float m_occupancy = -1; /**< strip occupancy (ADC) */
    float m_noise = -1; /**< strip noise (ADC) */
    float m_noiseEl = -1; /**< strip noise (e-)*/
    float m_occupancyAVE = -1; /**< sensor occupancy average*/
    float m_occupancyRMS = -1; /**< sensor occupancy rms*/
    float m_noiseAVE = -1; /**< sensor noise average (ADC)*/
    float m_noiseRMS = -1; /**< sensor noise rms (ADC)*/
    float m_noiseElAVE = -1; /**< sensor noise average (e-)*/
    float m_noiseElRMS = -1; /**< sensor noise rms (e-)*/
    float m_pedestal = -1; /**< strip pedestal*/
    float m_pedestalAVE = -1; /**< sensor pedestal average*/
    float m_pedestalRMS = -1; /**< sensor pedestal rms*/
    float m_gain = -1; /**< strip gain*/
    float m_gainAVE = -1; /**< sensor gain average*/
    float m_gainRMS = -1; /**< sensor gain rms*/
    float m_calPeakADC = -1; /**< strip peak time*/
    float m_calPeakADCAVE = -1; /**< sensor peak time average */
    float m_calPeakADCRMS = -1; /**< sensor peak time rms*/
    float m_calPeakTime = -1; /**< strip peak time*/
    float m_calPeakTimeAVE = -1; /**< sensor peak time average */
    float m_calPeakTimeRMS = -1; /**< sensor peak time rms*/
    float m_pulseWidth = -1; /**< strip pulse width */
    float m_pulseWidthAVE = -1; /**< sensor pulse width average*/
    float m_pulseWidthRMS = -1; /**< sensor pulse width rms*/

    std::string m_rootFileName = "SVDLocalCalibrationMonitor_output.root";   /**< root file name */

  private:
    SVDDetectorConfiguration m_DetectorConf; /**< Detector Configuration Payload */

    SVDFADCMaskedStrips m_MaskedStr; /**< FADC masked strip payload*/
    SVDNoiseCalibrations m_NoiseCal; /**< noise payload*/
    SVDPulseShapeCalibrations m_PulseShapeCal; /**< pulse shape payload*/
    SVDPedestalCalibrations m_PedestalCal; /**< pedestal payload*/
    //    SVDOccupancyCalibrations m_OccupancyCal; /**< occupancy payload*/
    //    SVDHotStripsCalibrations m_HotStripsCal; /**< hot strips calibration*/

    /** MASKS */
    SVDHistograms<TH1F>* m_hMask = nullptr; /**< masked strips histo */
    SVDHistograms<TH2F>* m_h2Mask = nullptr; /**< mask VS strip 2D histo */
    //    SVDHistograms<TProfile>* m_pMask = nullptr; /**< masked strips profile */

    //NOISE
    SVDHistograms<TH1F>* m_hNoise = nullptr; /**< noise (ADC) histo */
    SVDHistograms<TH2F>* m_h2Noise = nullptr; /**< noise (ADC) VS strip 2D histo */
    SVDHistograms<TH1F>* m_hNoiseEl = nullptr; /**< noise in e- histo */
    SVDHistograms<TH2F>* m_h2NoiseEl = nullptr; /**< noise in e- VS strip 2D histo */

    //PEDESTAL
    SVDHistograms<TH1F>* m_hPedestal = nullptr; /**< pedestal (ADC) histo */
    SVDHistograms<TH2F>* m_h2Pedestal = nullptr; /**< pedestal (ADC) VS strip 2D histo */

    //GAIN
    SVDHistograms<TH1F>* m_hGain = nullptr; /**< gain (e-/ADC) histo */
    SVDHistograms<TH2F>* m_h2Gain = nullptr; /**< gain (e-/ADC) VS strip 2D histo */

    //PEAKTIME
    SVDHistograms<TH1F>* m_hCalPeakTime = nullptr; /**< calPeakTime (ns) histo */
    SVDHistograms<TH2F>* m_h2CalPeakTime = nullptr; /**< calPeakTime (ns) VS strip 2D histo */

    //PEAKADC
    SVDHistograms<TH1F>* m_hCalPeakADC = nullptr; /**< calPeakADC (ns) histo */
    SVDHistograms<TH2F>* m_h2CalPeakADC = nullptr; /**< calPeakADC (ns) VS strip 2D histo */

    //PULSEWIDTH
    SVDHistograms<TH1F>* m_hPulseWidth = nullptr; /**< calPeakTime (ns) histo */
    SVDHistograms<TH2F>* m_h2PulseWidth = nullptr; /**< calPeakTime (ns) VS strip 2D histo */

    //OCCUPANCY
    SVDHistograms<TH1F>* m_hOccupancy = nullptr; /**< occupancy (hits/evt) histo */
    SVDHistograms<TH2F>* m_h2Occupancy = nullptr; /**< occupancy (hits/evt) VS strip 2D histo */

    //HOT STRIPS
    SVDHistograms<TH1F>* m_hHotstrips = nullptr; /**< hot strips histo */
    SVDHistograms<TH2F>* m_h2Hotstrips = nullptr; /**< hotstrips VS strip 2D histo */
    SVDHistograms<TH1F>* hm_hot_strips = nullptr; /**< hot strips per sensor*/    SVDSummaryPlots* m_hHotStripsSummary =
      nullptr; /**< hot strip summary  histo */
  };
}

