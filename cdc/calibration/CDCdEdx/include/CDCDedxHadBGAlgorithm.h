/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <map>
#include <vector>
#include <iostream>
#include <fstream>

#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include <framework/gearbox/Const.h>

#include <cdc/utilities/CDCDedxMeanPred.h>
#include <cdc/utilities/CDCDedxSigmaPred.h>
#include <cdc/utilities/CDCDedxHadSat.h>

#include <cdc/dbobjects/CDCDedxMeanPars.h>
#include <cdc/dbobjects/CDCDedxSigmaPars.h>

#include <cdc/calibration/CDCdEdx/HadronBgPrep.h>
#include <cdc/calibration/CDCdEdx/HadronCalibration.h>

namespace Belle2 {

  /**
   * A calibration algorithm for beta gamma curve and resolution fitting and save payloads
   */
  class CDCDedxHadBGAlgorithm : public CalibrationAlgorithm {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the algorithm.
    */
    CDCDedxHadBGAlgorithm();

    /**
    * Destructor
    */
    virtual ~CDCDedxHadBGAlgorithm() {}

    /**
    * function to enable monitoring plots
    */
    void setMonitoringPlots(bool value) {m_ismakePlots = value;}

    /**
    * function to set number of iteration
    */
    void setNumIterations(int value) {m_iter = value;}

    /**
    * function to set the cut to clean protons
    */
    void setProtonCut(double value) {m_cut = value;}

    /**
    * function to set beta gamma parameters for particle
    */
    void setBGPars(std::string particle, double nbin, double min, double max)
    {
      m_bgpar[particle.data()][0] = nbin;
      m_bgpar[particle.data()][1] = min;
      m_bgpar[particle.data()][2] = max;
    }

    /**
    * function to set inection time parameters for particle
    */
    void setInjectionPars(std::string particle, double nbin, double min, double max)
    {
      m_injpar[particle.data()][0] = nbin;
      m_injpar[particle.data()][1] = min;
      m_injpar[particle.data()][2] = max;
    }

    /**
    * function to set cosine bins for particle
    */
    void setCosBin(std::string particle, double nbin) { m_cospar[particle.data()] = nbin; }

    /**
    * function to set cos parameters for particle
    */
    void setCosPars(double min, double max) { m_cosMin = min; m_cosMax = max; }

    /**
    * function to get exp/run information (payload object, plotting)
    */
    void getExpRunInfo();

    /**
    * function to store payloads after full calibration
    */
    void createPayload();

    /**
    * function to do the sigma vs nhit or cos fits and store parameters
    */
    void SigmaFits(std::vector< std::string > particles, std::string sfx, std::string svar);

    /**
    * function to prepare sample for bgcurve fitting, sigma vs ionzation fitting and monitoring plots
    */
    void prepareSample(std::vector< std::string > particles, std::string filename, std::string sfx);

  protected:

    /**
    * CDC dE/dx Beta Gamma curve and resolution algorithm
    */
    virtual EResult calibrate() override;

  private:

    std::map<std::string, std::array<double, 3>> m_bgpar = {
      {"muon", {12, 2.85, 28.85}},
      {"pion", {12, 0.9, 14.90}},
      {"kaon", {11, 1.0, 6.5}},
      {"proton", {20, 0.33, 0.85}},
      {"electron", {60, 500.0, 12500.0}}
    };  /**< bg bins, min, max for different particles */

    std::map<std::string, std::array<double, 3>> m_injpar = {
      {"electron", {40, 0, 80000}},
      {"pion", {10, 0, 48000}},
      {"kaon", {6, 0, 48000}},
      {"muon", {40, 0, 80000}},
      {"proton", {20, 0, 80000}}
    };  /**< injection time bins, min, max for different particles */

    std::map<std::string, double> m_cospar = {
      {"electron", 24},
      {"pion", 18},
      {"kaon", 18},
      {"muon", 24},
      {"proton", 20}
    };  /**< cos bins for different particles */

    double m_cosMin = -0.84; /**< min range of cosine */
    double m_cosMax = 0.96; /**< max range of cosine */

    int m_nhitBins = 10; /**< bins for nhits */
    double m_nhitMin = 7; /**< min range of nhits */
    double m_nhitMax = 39; /**< max range of nhits */

    double m_cut = 0.5;    /**< cut to clean protons */

    bool m_ismakePlots; /**< produce plots for monitoring */

    std::string m_bgsigma;  /**< string for sigma parameter file names */
    std::string m_bgcurve;  /**< string for mean parameter file names */
    std::string m_suffix; /**< string suffix for object names */

    int m_iter = 5;  /**< set number of iteration */

    DBObjPtr<CDCDedxMeanPars> m_DBMeanPars; /**< db object for dE/dx mean parameters */
    DBObjPtr<CDCDedxSigmaPars> m_DBSigmaPars; /**< db object for dE/dx resolution parameters */

  };
} // namespace Belle2