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
#include <TF1.h>
#include <TList.h>
#include <TTree.h>
// #include <tuple>

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
    TTree* LambdaMassFit(std::shared_ptr<TTree> preselTree);                        /**< Mass fit for Lambda->ppi */
    TList* LambdaHistogramming(TTree* inputTree);                        /**< produce histograms for protons */
    TTree* DstarMassFit(std::shared_ptr<TTree> preselTree); /**< Mass fit for D*->Dpi */
    TList* DstarHistogramming(TTree* inputTree);                        /**< produce histograms for K/pi */
    TList* GammaHistogramming(std::shared_ptr<TTree> preselTree);                       /**< produce histograms for e */
    TList* GenerateNewHistograms(std::shared_ptr<TTree> ttreeLambda, std::shared_ptr<TTree> ttreeDstar,
                                 std::shared_ptr<TTree> ttreeGamma);                       /**< generate high-statistics histograms */
    int m_numDEdxBins = 100;                                                 /**< the number of dEdx bins for the payloads */
    int m_numPBins = 69;                                                     /**< the number of momentum bins for the payloads */
    double m_dedxCutoff = 5.e6;                                              /**< the upper edge of the dEdx binning for the payloads */
    int m_MinEvtsPerTree =
      100;                                                 /**< number of events in TTree below which we don't try to fit */
    int m_toGenerate =
      500000;                                                     /**< the number of events to be generated in each momentum bin in the new payloads */
    /**
    * build the binning scheme for the momentum
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

    /**
    * Normalise an dEdx:momentum histogram in each momentum bin, so that sum of entries in each momentum bin is 1.
    * Note that this accounts for entries in the underflow/overflow bins.
    */
    TH2F* Normalise2DHisto(TH2F* HistoToNormalise)
    {
      for (int pbin = 0; pbin <= m_numPBins + 1; pbin++) {
        for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
          // get rid of the bins with negative weights
          if (HistoToNormalise->GetBinContent(pbin, dedxbin) <= 1) {
            HistoToNormalise->SetBinContent(pbin, dedxbin, 0);
          };
        }
        // create a projection (1D histogram) in a given momentum bin

        TH1D* MomentumSlice = (TH1D*)HistoToNormalise->ProjectionY("slice_tr", pbin, pbin);
        // normalise, but ignore the cases with empty histograms
        if (MomentumSlice->Integral(0, m_numDEdxBins + 1) > 0) {
          MomentumSlice->Scale(1. / MomentumSlice->Integral(0, m_numDEdxBins + 1));
        }
        // fill back the 2D histogram with the result
        for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
          HistoToNormalise->SetBinContent(pbin, dedxbin, MomentumSlice->GetBinContent(dedxbin));
          HistoToNormalise->SetBinError(pbin, dedxbin, MomentumSlice->GetBinError(dedxbin));
        }
      }
      return HistoToNormalise;
    }

    /**
    * Generate a new dEdx:momentum histogram from a function that encodes dEdx:momentum trend and a function that encodes dEdx resolution.
    */
    TH2D* prepare_new_histogram(TH2F* data_histogram, TString new_name, TF1* betagamma_function, TF1* resolution_function,
                                double bias_correction)
    {

      resolution_function->SetRange(0, m_dedxCutoff * 2); // allow the function to take values outside the histogram range
      TH2D* data_histogram_new = (TH2D*) data_histogram->Clone(new_name);//Form("%s_new", data_histogram->GetName()));

      data_histogram_new->Reset();

      for (int pbin = 1; pbin <= m_numPBins + 1; pbin++) {
        double mean_dEdx_value = betagamma_function->Eval(data_histogram_new->GetXaxis()->GetBinCenter(pbin));
        resolution_function->FixParameter(1, mean_dEdx_value + bias_correction);

        // create a projection (1D histogram) in a given momentum bin
        TH1D* momentum_slice = (TH1D*)data_histogram_new->ProjectionY("slice", pbin, pbin);

        // fill manually (instead of FillRandom) to also preserve events in the overflow bin
        // this is needed for the correct normalisation
        for (int iEvent = 0; iEvent < m_toGenerate; iEvent++) {
          momentum_slice->Fill(resolution_function->GetRandom());
        }

        // normalise each momentum slice to unity, but ignore the cases with empty histograms
        if (momentum_slice->Integral(0, m_numDEdxBins + 1) > 0) {
          momentum_slice->Scale(1. / momentum_slice->Integral(0, m_numDEdxBins + 1));
        }
        // fill back the 2D histo with the result
        for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
          data_histogram_new->SetBinContent(pbin, dedxbin, momentum_slice->GetBinContent(dedxbin));
        }
      }

      return data_histogram_new;

    }

  };
} // namespace Belle2
