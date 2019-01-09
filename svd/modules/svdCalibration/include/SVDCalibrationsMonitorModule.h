/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDCALIBRATIONSMONITORMODULE_H
#define SVDCALIBRATIONSMONITORMODULE_H

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>

#include <svd/calibration/SVDFADCMaskedStrips.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDPedestalCalibrations.h>
//#include <svd/calibration/SVDOccupancyCalibrations.h>
//#include <svd/calibration/SVDHotStripsCalibrations.h>
#include <svd/calibration/SVDClusterCalibrations.h>

#include <svd/dataobjects/SVDHistograms.h>

#include <string>
#include <TList.h>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TProfile.h>

namespace Belle2 {
  /**
   * Module to produce a list of histogram showing the uploaded calibration constants
   *
   *    *
   */
  class SVDCalibrationsMonitorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDCalibrationsMonitorModule();

    /**  */
    virtual void initialize() override;

    /**  */
    virtual void beginRun() override;

    /** y */
    virtual void event() override;

    /**  */
    virtual void endRun() override;

    /**  */
    virtual void terminate() override;

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */
    TTree* m_tree; /**<pointer at tree containing the mean and RMS of calibration constants */
    TTree* m_treeDetailed; /**<pointer at tree containing the calibration constants of each strip*/

    //branches
    TBranch* b_run;
    TBranch* b_ladder;
    TBranch* b_layer;
    TBranch* b_sensor;
    TBranch* b_side;
    TBranch* b_strip;
    TBranch* b_mask;
    TBranch* b_gain;
    TBranch* b_pedestal;
    TBranch* b_gainAVE;
    TBranch* b_gainRMS;
    TBranch* b_noise;
    TBranch* b_noiseEl;
    TBranch* b_noiseAVE;
    TBranch* b_noiseRMS;
    TBranch* b_peakTime;
    TBranch* b_peakTimeAVE;
    TBranch* b_peakTimeRMS;
    TBranch* b_pulseWidth;
    TBranch* b_pulseWidthAVE;
    TBranch* b_pulseWidthRMS;

    //branch variables
    int m_run;
    int m_ladder;
    int m_layer;
    int m_sensor;
    int m_side;
    int m_strip;
    float m_mask;
    float m_noise;
    float m_noiseEl;
    float m_noiseAVE;
    float m_noiseRMS;
    float m_pedestal;
    float m_gain;
    float m_gainAVE;
    float m_gainRMS;
    float m_peakTime;
    float m_peakTimeAVE;
    float m_peakTimeRMS;
    float m_pulseWidth;
    float m_pulseWidthAVE;
    float m_pulseWidthRMS;

  private:

    SVDFADCMaskedStrips m_MaskedStr;
    SVDNoiseCalibrations m_NoiseCal;
    SVDPulseShapeCalibrations m_PulseShapeCal;
    SVDPedestalCalibrations m_PedestalCal;
    //    SVDOccupancyCalibrations m_OccCal;
    //    SVDHotStripsCalibrations m_HotStripsCal;
    SVDClusterCalibrations m_ClusterCal;

    static const int m_maxLayers = 6; // 1,2 will not be filled
    static const int m_maxLadders = 16;
    static const int m_maxSensors = 5;
    static const int m_maxSides = 2;


    TList* m_histoList_peakTime;
    TList* m_histoList_pulseWidth;
    TList* m_histoList_timeshift;
    TList* m_histoList_cluster;
    /* the following is currently not needed because this correction is not implemented yet*/
    TList* m_histoList_triggerbin;

    /** MASKS */
    SVDHistograms<TH1F>* m_hMask = NULL; /**< masked strips histo */
    SVDHistograms<TH2F>* m_h2Mask = NULL; /**< mask VS strip 2D histo */
    //    SVDHistograms<TProfile>* m_pMask = NULL; /**< masked strips profile */

    //NOISE
    SVDHistograms<TH1F>* m_hNoise = NULL; /**< noise histo */
    SVDHistograms<TH2F>* m_h2Noise = NULL; /**< noise VS strip 2D histo */
    SVDHistograms<TH1F>* m_hNoiseEl = NULL; /**< noise in e- histo */
    SVDHistograms<TH2F>* m_h2NoiseEl = NULL; /**< noise in e- VS strip 2D histo */

    //PEDESTAL
    SVDHistograms<TH1F>* m_hPedestal = NULL; /**< pedestal histo */
    SVDHistograms<TH2F>* m_h2Pedestal = NULL; /**< pedestal VS strip 2D histo */

    //GAIN
    SVDHistograms<TH1F>* m_hGain = NULL; /**< gain histo */
    SVDHistograms<TH2F>* m_h2Gain = NULL; /**< gain VS strip 2D histo */


    //PULSE PEAK (peak Time and Width)
    TH1F* h_peakTime[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors + 1][m_maxSides]; /**<peakTime in ns*/
    TH1F* h_pulseWidth[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors + 1][m_maxSides]; /**<pulse width in ns*/

    //CoG OLD Corrections (Michael)
    //CoG TIME SHIFT
    TH1F* h_timeshift[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors + 1][m_maxSides]; /**<time shift in ns*/

    //CoG TRIGGER BIN CORRECTION
    TH1F* h_triggerbin[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors +
                                                          1][m_maxSides]; /**< time shift due to the trigger bin correction in ns*/


    //Clusters
    //CLUSTER SNR
    TH1F* h_clsSNR[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors +
                                                      1][m_maxSides]; /**<clusterSNR*/

    //CLUSTER Seed SNR
    TH1F* h_clsSeedSNR[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors +
                                                          1][m_maxSides]; /**<cluster seed SNR*/
    //CLUSTER Adj SNR
    TH1F* h_clsAdjSNR[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors +
                                                         1][m_maxSides]; /**<cluster adj SNR*/
    //CLUSTER Position Error Scale Factor Size 1
    TH1F* h_clsScaleErr1[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors +
                                                            1][m_maxSides]; /**<scale err size 1*/
    //CLUSTER Position Error Scale Factor Size 2
    TH1F* h_clsScaleErr2[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors +
                                                            1][m_maxSides]; /**<scale err size 2*/
    //CLUSTER Position Error Scale Factor Size >2
    TH1F* h_clsScaleErr3[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors +
                                                            1][m_maxSides]; /**< scale err size >2*/


    //CLUSTER TIME FUNCTION VERSION
    TH1F* h_clsTimeFuncVersion[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors +
        1][m_maxSides]; /**<cluster time function version*/

    //CLUSTER MINIMUM TIME
    TH1F* h_clsTimeMin[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors +
                                                          1][m_maxSides]; /**<cluster minimim time*/

    //list of functions to create histograms:
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList = NULL);  /**< thf */
    //list of functions to create profiles:
    TProfile* createProfile(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, const char* ytitle, TList* histoList = NULL); /**< thf */

  };
}

#endif /* SVDCALIBRATIONSMONITORMODULE_H */
