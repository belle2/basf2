/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani, Giulia Casarosa, Michael De Nuccio           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDCOGCALIBRATORMODULE_H
#define SVDCOGCALIBRATORMODULE_H

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
  class SVDCoGCalibratorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDCoGCalibratorModule();

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
    bool m_useTracks;   /**< use tracks or not */
    bool m_fillAreas;   /**< fill DB with area-related values or not */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */

    /* Variables used in the program */
    float m_CoGTime;
    int m_Side;
    int m_Layer;
    int m_Ladder;
    int m_Sensor;

    int m_firstRun;
    int m_lastRun;
    int m_firstExperiment;
    int m_lastExperiment;

  private:

    SVDPulseShapeCalibrations m_PulseShapeCal;

    static const int m_maxLayers = 6; // 1,2 will not be filled
    static const int m_maxLadders = 16;
    static const int m_maxSensors = 5;
    static const int m_maxSides = 2;
    static const int m_Nsets = 6;

    TList* m_histoList_CoGTime;
    TList* m_histoList_CoGMean_all;
    TList* m_histoList_CoGMean_areas;
    TList* m_histoList_CoGMean_phi;
    TList* m_histoList_CoGTime_all;
    TList* m_histoList_CoGTime_areas;
    TList* m_histoList_CoGTime_phi;

    //HISTOS
    TH1F* h_CoGTime[m_maxLayers + 1][m_maxLadders + 1][m_maxSensors + 1][m_maxSides]; //CoGTime histos
    TH1F* h_CoGMean_all[m_maxSides]; //CoGMeanTime all
    TH1F* h_CoGMean_areas[m_Nsets]; //CoGMeanTime areas
    TH1F* h_CoGMean_phi[m_maxLayers + 1][m_maxSensors + 1][m_maxSides]; //CoGMeanTime phi
    TH1F* h_CoGTime_all[m_maxSides]; //CoGTime all
    TH1F* h_CoGTime_areas[m_Nsets]; //CoGTime areas
    TH1F* h_CoGTime_phi[m_maxLayers + 1][m_maxSensors + 1][m_maxSides]; //CoGTime phi

    /** function to create the histos */
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList = NULL);  /**< thf */

    /** Function returning the index used for Histos */
    int indexFromLayerSensorSide(int LayerNumber, int SensorNumber, int UVNumber);

    /** Function returning "Internal" or "External" depending on the index */
    TString IntExtFromIndex(int idx);

    /** Function returning "Forward" or "Backword" depending on the index */
    TString FWFromIndex(int idx);

    /** Function returning "U" or "V" depending on the index */
    TString UVFromIndex(int idx);

  };
}

#endif /* SVDCoGCalibratorMODULE_H */






















