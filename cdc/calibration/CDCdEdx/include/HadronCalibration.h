/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <math.h>
#include <iostream>
#include <stdlib.h>
#include <vector>

#include <TFile.h>
#include <TTree.h>
#include <TGraphErrors.h>
#include <TLine.h>
#include <TMultiGraph.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TF1.h>

#include <TStyle.h>
#include <TPaveText.h>
#include <framework/gearbox/Const.h>
#include <cdc/utilities/CDCDedxMeanPred.h>
#include <cdc/utilities/CDCDedxWidgetCurve.h>
#include <cdc/utilities/CDCDedxSigmaPred.h>

#include <cdc/calibration/CDCdEdx/HadronBgPrep.h>

namespace Belle2 {

  /**
  * Class to perform the fitting in beta gamma bins
  */
  class HadronCalibration {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the algorithm.
    */
    HadronCalibration();

    /**
    * Destructor
    */
    virtual ~HadronCalibration() {};

    /**
    * fit the beta-gamma curve
    */
    void fitBGCurve(std::vector< std::string > particles, std::string filename, std::string paramfile, std::string suffx);

    /**
    * fit sigma vs. ionzation
    */
    void fitSigmavsIonz(std::vector< std::string > particles, std::string filename, std::string paramfile, std::string suffix);

    /**
    * fit sigma vs. nhit
    */
    void fitSigmaVsNHit(std::vector< std::string > particles, std::string filename, std::string paramsigma, std::string suffx);

    /**
    * fit sigma vs. cos(theta)
    */
    void fitSigmaVsCos(std::vector< std::string > particles, std::string filename, std::string paramfile, std::string suffx);

    /**
    * plots mean and width after fitting
    */
    void plotBGMonitoring(std::vector< std::string > particles, std::string filename, std::string suffix);

    /**
    * plots chi and width after fitting - main function
    */
    void plotMonitoring(std::vector< std::string > particles, std::string filename, std::string sname, std::string title,
                        std::string sx, std::string sy);

    /**
    * function to set grpah cosmetics
    */
    void setGraphStyle(TGraphErrors& gr, const int ic)
    {
      gr.SetMarkerColor(ic);
      gr.SetMarkerStyle(4);
      gr.SetMarkerSize(0.5);
    };

    /**
    * function to set fitter cosmetics
    */
    void setFitterStyle(TF1*& fitt, const int ic, const int il)
    {
      fitt->SetLineColor(ic);
      fitt->SetLineWidth(1);
      fitt->SetLineStyle(il);
    };

  private:

    HadronBgPrep m_prep; /**< object for dE/dx to prepare sample */
  };

} // namespace Belle2
