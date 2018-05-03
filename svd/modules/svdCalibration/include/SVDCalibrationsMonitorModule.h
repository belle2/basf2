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

#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDPedestalCalibrations.h>
#include <svd/calibration/SVDOccupancyCalibrations.h>
#include <svd/calibration/SVDHotStripsCalibrations.h>

#include <string>
#include <TList.h>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TH1F.h>
#include <TH2F.h>

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
    virtual void initialize();

    /**  */
    virtual void beginRun();

    /** y */
    virtual void event();

    /**  */
    virtual void endRun();

    /**  */
    virtual void terminate();

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */
    TTree* m_tree; /**<pointer at tree containing the mean and RMS of calibration constants */

    //branches
    TBranch* b_run;
    TBranch* b_ladder;
    TBranch* b_layer;
    TBranch* b_sensor;
    TBranch* b_side;
    TBranch* b_gainAVE;
    TBranch* b_gainRMS;
    TBranch* b_noiseAVE;
    TBranch* b_noiseRMS;
    TBranch* b_peakTimeAVE;
    TBranch* b_peakTimeRMS;
    TBranch* b_pulseWidthAVE;
    TBranch* b_pulseWidthRMS;

    //branch variables
    int m_run;
    int m_ladder;
    int m_layer;
    int m_sensor;
    int m_side;
    float m_noiseAVE;
    float m_noiseRMS;
    float m_gainAVE;
    float m_gainRMS;
    float m_peakTimeAVE;
    float m_peakTimeRMS;
    float m_pulseWidthAVE;
    float m_pulseWidthRMS;

  private:

    SVDNoiseCalibrations m_NoiseCal;
    SVDPulseShapeCalibrations m_PulseShapeCal;
    //    SVDPedestalCalibrations m_PedCal;
    //    SVDOccupancyCalibrations m_OccCal;
    //    SVDHotStripsCalibrations m_HotStripsCal;

    static const int m_maxLayers = 6; // 1,2 will not be filled
    static const int m_maxLadders = 16;
    static const int m_maxSensors = 5;
    static const int m_maxSides = 2;


    TList* m_histoList_noise;
    TList* m_histoList_noiseInElectrons;
    TList* m_histoList_gainInElectrons;
    TList* m_histoList_peakTime;
    TList* m_histoList_pulseWidth;
    TList* m_histoList_timeshift;
    /* the following is currently not needed because this correction is not implemented yet*/
    TList* m_histoList_triggerbin;

    //NOISES

    TH1F* h_noise[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors + 1][m_maxSides]; // noise in ADC units
    TH1F* h_noiseInElectrons[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors + 1][m_maxSides]; // noise in electrons

    //PULSE PEAK (gain)
    TH1F* h_gainInElectrons[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors + 1][m_maxSides]; // gain in electrons

    //PULSE PEAK (peak Time and Width)
    TH1F* h_peakTime[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors + 1][m_maxSides]; // peakTime in ns
    TH1F* h_pulseWidth[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors + 1][m_maxSides]; // pulse width in ns

    //CoG TIME SHIFT
    TH1F* h_timeshift[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors + 1][m_maxSides]; // time shift in ns


    //CoG TRIGGER BIN CORRECTION
    TH1F* h_triggerbin[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors +
                                                          1][m_maxSides]; // thime shift due to the trigger bin correction in ns

    //list of functions to create histograms:
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList = NULL);  /**< thf */

  };
}

#endif /* SVDCALIBRATIONSMONITORMODULE_H */
