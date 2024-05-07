/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>

#include <TH2F.h>
#include <tuple>

namespace Belle2 {
  /**
   * Class implementing the SVD dEdx calibration algorithm
   */
  class SVDdEdxCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /**
    * Constructor
    */

    SVDdEdxCalibrationAlgorithm();

    /**
    * Destructor
    */

    virtual ~SVDdEdxCalibrationAlgorithm() {}

    /**
    * function to enable plotting
    */
    void setMonitoringPlots(bool value = false) {m_isMakePlots = value;}

  protected:
    /**
    * run algorithm on data
    */
    virtual EResult calibrate() override;

  private:

    bool m_isMakePlots; /**< produce plots for monitoring */
    TH2F LambdaMassFit(std::shared_ptr<TTree> preselTree);  /**< produce histograms for protons */
    std::tuple<TH2F, TH2F, TH2F> DstarMassFit(std::shared_ptr<TTree> preselTree);  /**< produce histograms for K/pi(/mu) */
    TH2F GammaHistogram(std::shared_ptr<TTree> preselTree); /**< produce histograms for e */
  };
} // namespace Belle2
