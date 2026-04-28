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

#include <cdc/dbobjects/CDCDedxHadronCor.h>
#include <cdc/calibration/CDCdEdx/HadronSaturation.h>
#include <cdc/calibration/CDCdEdx/HadronPrep.h>

namespace Belle2 {

  /**
   * A calibration algorithm for hadron saturation
   */
  class CDCDedxHadSatAlgorithm : public CalibrationAlgorithm {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the algorithm.
    */
    CDCDedxHadSatAlgorithm();

    /**
    * Destructor
    */
    virtual ~CDCDedxHadSatAlgorithm() {}

    /**
    * function to enable monitoring plots
    */
    void setMonitoringPlots(bool value = false) {m_ismakePlots = value;}

    /**
    * function to set the cut to clean protons
    */
    void setProtonCut(double value) {m_cut = value;}

    /**
    * function to set beta gamma parameters for particle
    */
    void setBGPars(const std::string& particle, double nbin, double min, double max)
    {
      m_bgpar[particle][0] = nbin;
      m_bgpar[particle][1] = min;
      m_bgpar[particle][2] = max;
    }

    /**
    * function to cos parameters for particle
    */
    void setCosPars(int bin, double min, double max) { m_cosBins = bin; m_cosMin = min; m_cosMax = max; }

    /**
    * function to get exp/run information (payload object, plotting)
    */
    void getExpRunInfo();

    /**
    * function to store payloads after full calibration
    */
    void createPayload();

    /**
    * function to prepare sample for bgcurve hadron saturation and monitoring plots
    */
    void prepareSample(std::vector< std::string >& particles, const std::string& filename, const std::string& sfx, bool correct);

  protected:

    /**
    * CDC dE/dx Hadron saturation algorithm
    */
    virtual EResult calibrate() override;

  private:

    std::map<std::string, std::array<double, 3>> m_bgpar = {
      {"electron", {18, 600, 9600}},
      {"pion", {4, 2.0, 12.94}},
      {"kaon", {4, 1.10, 3.6}},
      {"muon", {8, 2.83, 28.83}},
      {"proton", {6, 0.44, 0.85}}
    };  /**< bg bins, min, max for different particles */

    int m_cosBins = 8; /**< bins for cosine */
    double m_cosMin = 0; /**< min range of cosine */
    double m_cosMax = 0.95; /**< max range of cosine */

    double m_cut = 0.5;    /**< cut to clean protons */

    bool m_ismakePlots; /**< produce plots for monitoring */

    std::string m_suffix; /**< string suffix for object names */

    DBObjPtr<CDCDedxHadronCor> m_DBHadronCor; /**< db object for dE/dx hadron saturation parameters */

  };
} // namespace Belle2