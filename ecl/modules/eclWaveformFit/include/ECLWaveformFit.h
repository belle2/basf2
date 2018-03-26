/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Savino Longo (longos@uvic.ca)                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <ecl/digitization/EclConfiguration.h>

// OTHER
#include <vector>

#include <ecl/dataobjects/ECLWaveformData.h>
// ROOT
#include <TRandom3.h>
#include <TMatrixFSym.h>
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TGraph.h"
#include "TCanvas.h"

namespace Belle2 {

  class ECLWaveformFitModule : public Module {

  public:

    /** Constructor.
     */
    ECLWaveformFitModule();

    /** Destructor.
     */
    ~ECLWaveformFitModule();

    /** Initialize variables. */
    virtual void initialize();

    /** begin run.*/
    virtual void beginRun();

    /** event per event.
     */
    virtual void event();

    /** end run. */
    virtual void endRun();

    /** terminate.*/
    virtual void terminate();

    virtual const char* eclDigitArrayName() const
    { return "ECLDigits" ; }

    virtual const char* eclDspArrayName() const
    { return "ECLDsps" ; }

  private:
    std::vector<double> FitWithROOT(double, std::vector<double>, std::vector<double>, int);  /** Fit with ROOT::Fit function.*/
    double m_CurrentPulseArray31[Belle2::ECL::EclConfiguration::m_nsmp];  /** Current waveform adc values.*/
    int m_FitType;  /**0 = photon + hadron, 1 = photon + diode*/
    double m_EnergyThreshold;  /**energy threshold to fit pulse offline*/
    double m_TriggerThreshold;  /**energy threshold for waveform trigger.*/
    std::vector<double> m_ADCtoEnergy;  /**calibration vector form adc to energy*/
    std::vector< std::vector<double> > m_PhotonTemplates; /**photon templates*/
    std::vector< std::vector<double> > m_SecondComponentTemplates; /**hadron or diode templates*/
  };
} // end Belle2 namespace
