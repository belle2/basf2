/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/calibration/TOPCommonT0BFAlgorithm.h>
#include <top/dbobjects/TOPCalCommonT0.h>
#include <math.h>
#include <TROOT.h>
#include <TF1.h>
#include <TFile.h>
#include <TTree.h>
#include <string>

using namespace std;

namespace Belle2 {
  namespace TOP {

    TOPCommonT0BFAlgorithm::TOPCommonT0BFAlgorithm():
      CalibrationAlgorithm("TOPCommonT0BFCollector")
    {
      setDescription("Calibration algorithm for common T0 calibration "
                     "with a fit of bunch finder residuals (method BF)");

    }

    CalibrationAlgorithm::EResult TOPCommonT0BFAlgorithm::calibrate()
    {
      gROOT->SetBatch();

      // get histogram to fit and check if statistics is sufficient

      auto h = getHistogram();
      if (not h) {
        B2ERROR("TOPCommonT0BFAlgorithm: no histogram to fit");
        return c_NotEnoughData;
      }

      int numEntries = h->GetSumOfWeights();
      if (numEntries < m_minEntries) {
        B2ERROR("TOPCommonT0BFAlgorithm: too few entries to fit the histogram");
        return c_NotEnoughData;
      }

      m_bunchTimeSep = h->GetXaxis()->GetXmax() - h->GetXaxis()->GetXmin();

      // construct file name, open output root file and book output tree

      const auto& expRun = getRunList();
      string expNo = to_string(expRun[0].first);
      while (expNo.length() < 4) expNo.insert(0, "0");
      string runNo = to_string(expRun[0].second);
      while (runNo.length() < 5) runNo.insert(0, "0");
      string outputFileName = "commonT0-e" + expNo + "-r" + runNo + ".root";
      auto* file = TFile::Open(outputFileName.c_str(), "recreate");

      auto* tree = new TTree("tree", "common T0 calibration results");
      tree->Branch<int>("expNum", &m_expNo);
      tree->Branch<int>("runNum", &m_runNo);
      tree->Branch<int>("runLast", &m_runLast);
      tree->Branch<float>("fitted_offset", &m_fittedOffset);
      tree->Branch<float>("offset", &m_offset);
      tree->Branch<float>("offsetErr", &m_offsetError);
      tree->Branch<float>("sigma", &m_sigmaCore);
      tree->Branch<float>("sigmaTail", &m_sigmaTail);
      tree->Branch<float>("tailFract", &m_tailFract);
      tree->Branch<float>("chi2", &m_chi2);
      tree->Branch<float>("integral", &m_integral);
      tree->Branch<int>("nEvt", &m_numEvents);
      tree->Branch<int>("fitStatus", &m_fitStatus);

      m_expNo = expRun[0].first;
      m_runNo = expRun[0].second;
      m_runLast = expRun.back().second;

      // fit histogram

      if (numEntries > m_cutoffEntries and m_tailFractInit > 0) {
        int status = fitDoubleGaus(h);
        if (status != 0) fitSingleGaus(h);
      } else {
        fitSingleGaus(h);
      }
      tree->Fill();

      // write the results and close the file

      h->Write();
      file->Write();
      file->Close();

      // check the results and return if not good enough

      if (m_fitStatus != 0 or m_offsetError > m_minError) return c_NotEnoughData;

      // otherwise create and import payload to DB

      auto* commonT0 = new TOPCalCommonT0(m_offset, m_offsetError);
      saveCalibration(commonT0);

      return c_OK;
    }

    std::shared_ptr<TH1F> TOPCommonT0BFAlgorithm::getHistogram()
    {
      auto h1a = getObjectPtr<TH1F>("offset_a");
      auto h1b = getObjectPtr<TH1F>("offset_b");
      if (not h1a) {
        B2ERROR("TOPCommonT0BFAlgorithm: histogram 'offset_a' not found");
        return 0;
      }
      if (not h1b) {
        B2ERROR("TOPCommonT0BFAlgorithm: histogram 'offset_b' not found");
        return 0;
      }

      int halfbins = h1a->GetNbinsX() / 2;
      if (abs(h1a->GetMaximumBin() - halfbins) < abs(h1b->GetMaximumBin() - halfbins)) {
        return h1a;
      } else {
        return h1b;
      }
    }

    int TOPCommonT0BFAlgorithm::fitSingleGaus(std::shared_ptr<TH1F> h)
    {
      double sum = h->GetSumOfWeights();
      if (sum < 5) return 5;
      double maxPosition = h->GetBinCenter(h->GetMaximumBin());
      double binWidth = h->GetBinWidth(1);
      double xmin = h->GetXaxis()->GetXmin();
      double xmax = h->GetXaxis()->GetXmax();

      auto* func = new TF1("func1g",
                           "[0] + [1]/sqrt(2*pi)/[3]*exp(-0.5*((x-[2])/[3])**2)",
                           xmin, xmax);
      func->SetParameter(0, 0);
      func->SetParameter(1, sum * binWidth);
      func->SetParameter(2, maxPosition);
      func->SetParameter(3, m_sigmaCoreInit);

      int status = h->Fit(func, "LRSQ");

      m_fittedOffset = func->GetParameter(2);
      m_offset = m_fittedOffset - round(m_fittedOffset / m_bunchTimeSep) * m_bunchTimeSep;
      m_offsetError = func->GetParError(2);
      m_sigmaCore = func->GetParameter(3);
      m_sigmaTail = 0;
      m_tailFract = 0;
      m_chi2 = func->GetChisquare() / func->GetNDF();
      m_integral = func->GetParameter(1) / binWidth;
      m_numEvents = sum;
      m_fitStatus = status;

      return status;
    }


    int TOPCommonT0BFAlgorithm::fitDoubleGaus(std::shared_ptr<TH1F> h)
    {
      double sum = h->GetSumOfWeights();
      if (sum < 7) return 7;
      double maxPosition = h->GetBinCenter(h->GetMaximumBin());
      double binWidth = h->GetBinWidth(1);
      double xmin = h->GetXaxis()->GetXmin();
      double xmax = h->GetXaxis()->GetXmax();

      auto* func = new TF1("func2g",
                           "[0] + [1]*((1-[4])/sqrt(2*pi)/[3]*exp(-0.5*((x-[2])/[3])**2)"
                           "+ [4]/sqrt(2*pi)/[5]*exp(-0.5*((x-[2])/[5])**2))",
                           xmin, xmax);
      func->SetParameter(0, 0);
      func->SetParameter(1, sum * binWidth);
      func->SetParameter(2, maxPosition);
      func->SetParameter(3, m_sigmaCoreInit);
      func->SetParameter(4, m_tailFractInit);
      func->SetParameter(5, m_sigmaTailInit);

      int status = h->Fit(func, "LRSQ");

      m_fittedOffset = func->GetParameter(2);
      m_offset = m_fittedOffset - round(m_fittedOffset / m_bunchTimeSep) * m_bunchTimeSep;
      m_offsetError = func->GetParError(2);
      m_sigmaCore = func->GetParameter(3);
      m_sigmaTail = func->GetParameter(5);
      m_tailFract = func->GetParameter(4);
      m_chi2 = func->GetChisquare() / func->GetNDF();
      m_integral = func->GetParameter(1) / binWidth;
      m_numEvents = sum;
      m_fitStatus = status;

      return status;
    }


  } // end namespace TOP
} // end namespace Belle2
