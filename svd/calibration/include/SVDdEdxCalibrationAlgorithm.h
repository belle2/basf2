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
    void setMonitoringPlots(bool value = false) { m_isMakePlots = value; }

    /**
     * set the number of dEdx bins for the payloads
     */
    void setNumDEdxBins(const int& value) { m_numDEdxBins = value; }

    /**
     * set the number of momentum bins for the payloads
     */
    void setNumPBins(const int& value) { m_numPBins = value; }

    /**
     * set the upper edge of the dEdx binning for the payloads
     */
    void setDEdxCutoff(const double& value) { m_dedxCutoff = value; }

    /**
     * set the upper edge of the dEdx binning for the payloads
     */
    void setMinEvtsPerTree(const double& value) { m_MinEvtsPerTree = value; }

  protected:
    /**
     * run algorithm on data
     */
    virtual EResult calibrate() override;

  private:
    bool m_isMakePlots;                                                           /**< produce plots for monitoring */
    TH2F LambdaMassFit(std::shared_ptr<TTree> preselTree);                        /**< produce histograms for protons */
    std::tuple<TH2F, TH2F, TH2F> DstarMassFit(std::shared_ptr<TTree> preselTree); /**< produce histograms for K/pi(/mu) */
    TH2F GammaHistogram(std::shared_ptr<TTree> preselTree);                       /**< produce histograms for e */
    int m_numDEdxBins = 100;                                                 /**< the number of dEdx bins for the payloads */
    int m_numPBins = 69;                                                     /**< the number of momentum bins for the payloads */
    double m_dedxCutoff = 5.e6;                                              /**< the upper edge of the dEdx binning for the payloads */
    int m_MinEvtsPerTree =
      100;                                                 /**< number of events in TTree below which we don't try to fit */
    /**
     * build the binning scheme
     */
    std::vector<double> CreatePBinningScheme()
    {
      std::vector<double> pbins;
      pbins.reserve(m_numPBins + 1);
      pbins.push_back(0.0);
      pbins.push_back(0.05);

      for (int iBin = 2; iBin <= m_numPBins; iBin++) {
        if (iBin <= 19)
          pbins.push_back(0.025 + 0.025 * iBin);
        else if (iBin <= 59)
          pbins.push_back(pbins.at(19) + 0.05 * (iBin - 19));
        else
          pbins.push_back(pbins.at(59) + 0.3 * (iBin - 59));
      }

      return pbins;
    }
  };
} // namespace Belle2
