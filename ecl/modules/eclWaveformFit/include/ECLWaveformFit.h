/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Savino Longo (longos@uvic.ca)                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLWAVEFORMANALYSISMODULE_H_
#define ECLWAVEFORMANALYSISMODULE_H_

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
  namespace ECL {

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

      /** Name of the ECLEventInformation.*/
      virtual const char* eclEventInformationName() const
      { return "ECLEventInformation" ; }

    protected:
    private:
      std::vector<double> gamma_params_forPSD;
      std::vector<double> psd_params_forPSD;
      //
      std::vector<double> FitWithROOT(double, std::vector<double>, std::vector<double>, int);
      //
      double m_CurrentPulseArray31[EclConfiguration::m_nsmp];
      int m_FitType;
      double m_ChiThreshold;
      double m_EnergyThreshold;
    };
  } // end ECL namespace
} // end Belle2 namespace

#endif
