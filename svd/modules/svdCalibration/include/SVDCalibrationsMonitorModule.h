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

#include <string>
#include <TList.h>
#include <TFile.h>
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


  private:

    SVDNoiseCalibrations m_NoiseCal;
    SVDPulseShapeCalibrations m_PulseShapeCal;

    static const int m_maxLayers = 6; // 1,2 will not be filled
    static const int m_maxLadders = 16;
    static const int m_maxSensors = 5;
    static const int m_maxSides = 2;


    TList* m_histoList_noise;
    TList* m_histoList_noiseInElectrons;

    //NOISES

    TH1F* h_noise[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors + 1][m_maxSides]; // noise in ADC units
    TH1F* h_noiseInElectrons[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors + 1][m_maxSides]; // noise in electrons

    //list of functions to create histograms:
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList = NULL);  /**< thf */

  };
}

#endif /* SVDCALIBRATIONSMONITORMODULE_H */
