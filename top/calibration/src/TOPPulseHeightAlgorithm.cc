/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/calibration/TOPPulseHeightAlgorithm.h>
#include "TROOT.h"
#include <TDirectory.h>
#include <TF1.h>
#include <TMath.h>
#include <string>
#include <algorithm>

using namespace std;

namespace Belle2 {
  namespace TOP {

    TOPPulseHeightAlgorithm::TOPPulseHeightAlgorithm():
      CalibrationAlgorithm("TOPPulseHeightCollector")
    {
      setDescription("Calibration algorithm for pulse-height and threshold efficiency calibration");
    }

    CalibrationAlgorithm::EResult TOPPulseHeightAlgorithm::calibrate()
    {
      gROOT->SetBatch();

      // construct file name, open output root file and book output tree

      const auto& expRun = getRunList();
      string expNo = to_string(expRun[0].first);
      while (expNo.length() < 4) expNo.insert(0, "0");
      string runNo = to_string(expRun[0].second);
      while (runNo.length() < 5) runNo.insert(0, "0");
      string outputFileName = "pulseHeight-e" + expNo + "-r" + runNo + ".root";
      m_file = TFile::Open(outputFileName.c_str(), "recreate");

      m_tree = new TTree("tree", "pulse-height calibration results");
      m_tree->Branch<int>("slot", &m_slot);
      m_tree->Branch<unsigned>("channel", &m_channel);
      m_tree->Branch<int>("numPhot", &m_numPhot);
      m_tree->Branch<float>("x0", &m_x0);
      m_tree->Branch<float>("p1", &m_p1);
      m_tree->Branch<float>("p2", &m_p2);
      m_tree->Branch<float>("x0err", &m_x0err);
      m_tree->Branch<float>("p1err", &m_p1err);
      m_tree->Branch<float>("p2err", &m_p2err);
      m_tree->Branch<float>("effi", &m_effi);
      m_tree->Branch<float>("meanHist", &m_meanHist);
      m_tree->Branch<float>("meanFunc", &m_meanFunc);
      m_tree->Branch<float>("chi2", &m_chi2);
      m_tree->Branch<int>("ndf", &m_ndf);
      m_tree->Branch<int>("fitStatus", &m_fitStatus);
      m_tree->Branch<bool>("good", &m_good);

      // write-out control histograms

      auto h1a = getObjectPtr<TH1F>("time");
      if (h1a) h1a->Write();
      auto h1b = getObjectPtr<TH1F>("time_sel");
      if (h1b) h1b->Write();
      auto h2a = getObjectPtr<TH2F>("ph_vs_width");
      if (h2a) h2a->Write();
      auto h2b = getObjectPtr<TH2F>("ph_vs_width_sel");
      if (h2b) h2b->Write();

      // create payloads for storing results

      m_pulseHeight = new TOPCalChannelPulseHeight();
      m_thresholdEffi = new TOPCalChannelThresholdEff();

      // fit histograms

      TDirectory* oldDir = gDirectory;
      int nsucc = 0;
      for (int slot = 1; slot <= 16; slot++) {
        m_slot = slot;
        string name = "ph_slot_" + to_string(slot);
        auto h = getObjectPtr<TH2F>(name);
        if (not h) continue;

        name = "slot_" + to_string(slot);
        oldDir->mkdir(name.c_str())->cd();
        h->Write();

        int n = fitChannels(h);
        B2INFO("slot " << slot << ": " << n << "/512 channels successfully fitted");
        nsucc += n;
      }

      // write the results and close the file

      m_file->Write();
      m_file->Close();

      // check the results and return if fraction of well fitted too small

      if (nsucc / 8192.0 < m_minCalibrated) {
        delete m_pulseHeight;
        delete m_thresholdEffi;
        return c_NotEnoughData;
      }

      // otherwise import payloads to DB

      saveCalibration(m_pulseHeight);
      saveCalibration(m_thresholdEffi);

      return c_OK;
    }


    int TOPPulseHeightAlgorithm::fitChannels(std::shared_ptr<TH2F> h2d)
    {
      int n = 0;

      for (int ch = 0; ch < h2d->GetNbinsX(); ch++) {
        m_channel =  ch;
        string chan = to_string(ch);
        while (chan.length() < 3) chan.insert(0, "0");
        string name = "chan_" + chan;
        auto* h = h2d->ProjectionY(name.c_str(), ch + 1, ch + 1);
        string title = "slot " + to_string(m_slot) + " channel " + to_string(ch);
        h->SetTitle(title.c_str());

        auto status = fitPulseHeight(h);

        if (status == 0 and m_good) {
          m_pulseHeight->setParameters(m_slot, m_channel, m_x0, m_p1, m_p2);
          m_thresholdEffi->setThrEff(m_slot, m_channel, std::min(m_effi, 1.0f), m_xmin);
          n++;
        }
      }

      return n;
    }


    int TOPPulseHeightAlgorithm::fitPulseHeight(TH1D* h)
    {
      m_numPhot = h->GetSumOfWeights();
      if (m_numPhot < std::max(m_minEntries, 5)) return -1;

      double xmin = h->GetXaxis()->GetXmin();
      double xmax = h->GetXaxis()->GetXmax();

      auto* func = new TF1("func", "[0]*x/[1]*exp(-pow(x/[1],[2]))", xmin, xmax);

      double p2 = 1.0;
      m_meanHist = h->GetMean();
      double x0 = m_meanHist / 3;            // for p2 = 1 only
      func->SetParameter(0, m_numPhot / x0); // for p2 = 1 only
      func->SetParameter(1, x0);
      func->SetParameter(2, p2);

      int status = h->Fit(func, "LRSQ", "", m_xmin, xmax);

      m_x0 = func->GetParameter(1);
      m_p1 = 1.0;
      m_p2 = func->GetParameter(2);
      m_x0err = func->GetParError(1);
      m_p1err = 0;
      m_p2err = func->GetParError(2);
      m_chi2 = func->GetChisquare();
      m_effi = getEfficiency(h, func);
      m_meanFunc = TMath::Gamma((m_p1 + 2) / m_p2) / TMath::Gamma((m_p1 + 1) / m_p2) * m_x0;
      m_fitStatus = status;
      m_good = (m_chi2 / m_ndf < 10 and m_p2 > 0.5 and m_p2 < 5);
      m_tree->Fill();

      return status;
    }


    double TOPPulseHeightAlgorithm::getEfficiency(TH1D* h, TF1* func)
    {
      double fbelow = 0;
      double fabove = 0;
      double count = 0;
      m_ndf = -func->GetNumberFreeParameters();
      for (int i = 0; i < h->GetNbinsX(); i++) {
        double x = h->GetBinCenter(i + 1);
        if (x > m_xmin) {
          fabove += func->Eval(x);
          count += h->GetBinContent(i);
          if (h->GetBinContent(i) > 0) m_ndf++;
        } else {
          fbelow += func->Eval(x);
        }
      }
      double integral = (fbelow + fabove) * count / fabove;
      return h->GetSumOfWeights() / integral;
    }


  } // end namespace TOP
} // end namespace Belle2
