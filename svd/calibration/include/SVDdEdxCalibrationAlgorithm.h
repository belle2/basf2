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
#include <TProfile.h>
#include <TDatabasePDG.h>

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
     * set the number of beta*gamma bins for the fits
     */
    void setNumBGBins(const int& value) { m_numBGBins = value; }

    /**
     * set the upper edge of the dEdx binning for the payloads
     */
    void setDEdxCutoff(const double& value) { m_dedxCutoff = value; }

    /**
     * set the upper edge of the dEdx binning for the payloads
     */
    void setMinEvtsPerTree(const double& value) { m_MinEvtsPerTree = value; }

    /**
     * set the number of events to generate, per momentum bin, for the payloads
     */
    void setNToGenerate(const int& value) { m_toGenerate = value; }

    /**
     * reimplement the profile histogram calculation
     */
    void setCustomProfile(bool value = true) { m_CustomProfile = value; }

    /**
     * In the dEdx:betagamma fit, there is one free parameter that makes fit convergence poor. It is ok to fix it, unless the dEdx behavior changes radically with time.
     */
    void setFixUnstableFitParameter(bool value = true) { m_FixUnstableFitParameter = value; }

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
                                 std::shared_ptr<TTree> ttreeGamma, std::shared_ptr<TTree>
                                 ttreeGeneric);                       /**< generate high-statistics histograms */
    int m_numDEdxBins = 100;                                                 /**< the number of dEdx bins for the payloads */
    int m_numPBins = 69;                                                     /**< the number of momentum bins for the payloads */
    int m_numBGBins = 69;    /**< the number of beta*gamma bins for the profile and fitting */
    double m_dedxCutoff = 5.e6;                                              /**< the upper edge of the dEdx binning for the payloads */
    double m_dedxMaxPossible = 7.e6;  /**< the approximate max possible value of dEdx */
    int m_MinEvtsPerTree =
      100;                                                 /**< number of events in TTree below which we don't try to fit */
    int m_toGenerate =
      500000;                                                     /**< the number of events to be generated in each momentum bin in the new payloads */
    bool m_CustomProfile = 1; /**< reimplement profile histogram calculation instead of the ROOT implementation? */
    bool m_UsePionBGFunctionForEverything =
      0; /**< Assume that the dEdx:betagamma trend is the same for all hadrons; use the pion trend as representative */
    bool m_UseProtonBGFunctionForEverything =
      0; /**< Assume that the dEdx:betagamma trend is the same for all hadrons; use the proton trend as representative */
    bool m_FixUnstableFitParameter =
      1;  /**< In the dEdx:betagamma fit, there is one free parameter that makes fit convergence poor. It is ok to fix it, unless the dEdx behavior changes radically with time. */

    const double m_ElectronPDGMass = TDatabasePDG::Instance()->GetParticle(11)->Mass();  /**< PDG mass for the electron */
    const double m_MuonPDGMass = TDatabasePDG::Instance()->GetParticle(13)->Mass();  /**< PDG mass for the muon */
    const double m_PionPDGMass = TDatabasePDG::Instance()->GetParticle(211)->Mass();  /**< PDG mass for the charged pion */
    const double m_KaonPDGMass = TDatabasePDG::Instance()->GetParticle(321)->Mass();  /**< PDG mass for the charged kaon */
    const double m_ProtonPDGMass = TDatabasePDG::Instance()->GetParticle(2212)->Mass();  /**< PDG mass for the proton */
    const double m_DeuteronPDGMass = TDatabasePDG::Instance()->GetParticle(1000010020)->Mass();  /**< PDG mass for the deuteron */

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
    TH2F* PrepareNewHistogram(TH2F* DataHistogram, TString NewName, TF1* betagamma_function, TF1* ResolutionFunctionOriginal,
                              double bias_correction)
    {
      TF1* ResolutionFunction = (TF1*) ResolutionFunctionOriginal->Clone(Form("%sClone",
                                ResolutionFunctionOriginal->GetName())); // to avoid modifying the resolution function
      ResolutionFunction->SetRange(0, m_dedxMaxPossible); // allow the function to take values outside the histogram range
      TH2F* DataHistogramNew = (TH2F*) DataHistogram->Clone(NewName);

      DataHistogramNew->Reset();

      for (int pbin = 1; pbin <= m_numPBins + 1; pbin++) {
        double mean_dEdx_value = betagamma_function->Eval(DataHistogramNew->GetXaxis()->GetBinCenter(pbin));
        ResolutionFunction->FixParameter(1, mean_dEdx_value + bias_correction);

        // create a projection (1D histogram) in a given momentum bin
        TH1D* MomentumSlice = (TH1D*)DataHistogramNew->ProjectionY("slice", pbin, pbin);

        // fill manually (instead of FillRandom) to also preserve events in the overflow bin
        // this is needed for the correct normalisation
        for (int iEvent = 0; iEvent < m_toGenerate; iEvent++) {
          MomentumSlice->Fill(ResolutionFunction->GetRandom());
        }

        // normalise each momentum slice to unity, but ignore the cases with empty histograms
        if (MomentumSlice->Integral(0, m_numDEdxBins + 1) > 0) {
          MomentumSlice->Scale(1. / MomentumSlice->Integral(0, m_numDEdxBins + 1));
        }
        // fill back the 2D histo with the result
        for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
          DataHistogramNew->SetBinContent(pbin, dedxbin, MomentumSlice->GetBinContent(dedxbin));
          DataHistogramNew->SetBinError(pbin, dedxbin, MomentumSlice->GetBinError(dedxbin));
        }
      }

      return DataHistogramNew;

    }

    /**
    * Reimplement the Profile histogram calculation for a 2D histogram. The standard ROOT implementation takes the mean Y at a given X, which produces biased results in case of rapidly-rising distributions with non-Gaussian resolutions. We fit in slices to extract the mean more precisely.
    */
    TH1F* PrepareProfile(TH2F* DataHistogram, TString NewName)
    {
// define our resolution function: Crystal Ball. Parameter [1] is the mean and [2] is the relative width.
      TF1* ResolutionFunction = new TF1("ResolutionFunction", "[0]*ROOT::Math::crystalball_function(x,[4],[3],[2]*[1],[1])", 100e3,
                                        7000e3);


      ResolutionFunction->SetNpx(1000);

      ResolutionFunction->SetParameters(1000, 6.e5, 0.1, 1, 1);
      ResolutionFunction->SetParLimits(0, 0, 1.e6);
      ResolutionFunction->SetParLimits(1, 3.e5, 7.e6);
      ResolutionFunction->SetParLimits(2, 0, 10);
      ResolutionFunction->SetParLimits(3, 0.01, 100);
      ResolutionFunction->SetParLimits(4, 0.01, 100);

      ResolutionFunction->SetRange(0, m_dedxMaxPossible); // allow the function to take values outside the histogram range

      TH1F* DataHistogramNew = (TH1F*)DataHistogram->ProfileX()->ProjectionX();
      TH1F* DataHistogramClone = (TH1F*)DataHistogramNew->Clone(Form("%sClone",
                                                                DataHistogramNew->GetName())); // preserve the original profile for uncertainty cross-checks
      DataHistogramNew->SetName(NewName);
      DataHistogramNew->SetTitle(NewName);
      DataHistogramNew->Reset();

      for (int pbin = 1; pbin <= m_numBGBins; pbin++) {
        // create a projection (1D histogram) in a given momentum bin
        TH1F* MomentumSlice = (TH1F*)DataHistogram->ProjectionY("slice", pbin, pbin);

        if (MomentumSlice->Integral() < 1) continue;
// guesstimate the starting fit values
        ResolutionFunction->SetParameter(1, MomentumSlice->GetMean());
        ResolutionFunction->SetParameter(2, MomentumSlice->GetStdDev() / MomentumSlice->GetMean());
        ResolutionFunction->SetRange(MomentumSlice->GetMean() * 0.2, MomentumSlice->GetMean() * 1.75);
// fit each slice to extract the mean
        MomentumSlice->Fit(ResolutionFunction, "RQI");

        double stat_error = DataHistogramClone->GetBinError(pbin);
// fill back the 1D histo with the result
        double bincontent = ResolutionFunction->GetParameter(1);
        double binerror = ResolutionFunction->GetParError(1);

        binerror = std::max(binerror, stat_error);

        DataHistogramNew->SetBinContent(pbin, bincontent);
        DataHistogramNew->SetBinError(pbin, binerror);
      }

      return DataHistogramNew;

    }

  };
} // namespace Belle2
