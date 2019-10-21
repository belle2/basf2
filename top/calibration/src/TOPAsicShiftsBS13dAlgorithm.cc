/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/calibration/TOPAsicShiftsBS13dAlgorithm.h>
#include <top/dbobjects/TOPCalAsicShift.h>
#include <string>
#include <algorithm>
#include <math.h>
#include <TH2F.h>
#include <TFile.h>

using namespace std;

namespace Belle2 {
  namespace TOP {

    TOPAsicShiftsBS13dAlgorithm::TOPAsicShiftsBS13dAlgorithm():
      CalibrationAlgorithm("TOPAsicShiftsBS13dCollector")
    {
      setDescription("Calibration algorithm for carrier shifts of BS13d");
    }


    CalibrationAlgorithm::EResult TOPAsicShiftsBS13dAlgorithm::calibrate()
    {

      // get collected histograms

      auto timeReference = getObjectPtr<TH1F>("time_reference");
      if (not timeReference) {
        B2ERROR("TOPAsicShiftsBS13dAlgorithm: histogram 'time_reference' not found");
        return c_Failure;
      }
      if (timeReference->GetSumOfWeights() == 0) {
        B2ERROR("TOPAsicShiftsBS13dAlgorithm: histogram 'time_reference' is empty");
        return c_NotEnoughData;
      }
      m_timeReference.clear();
      for (int i = 0; i < timeReference->GetNbinsX(); i++) {
        m_timeReference.push_back(timeReference->GetBinContent(i + 1));
      }

      std::vector<std::shared_ptr<TH1F> > timeCarriers;
      for (unsigned i = 0; i < 4; i++) {
        string name = "time_carr_" + to_string(i);
        auto h = getObjectPtr<TH1F>(name);
        if (not h) {
          B2ERROR("TOPAsicShiftsBS13dAlgorithm: histogram '" << name << "' not found");
          return c_Failure;
        }
        timeCarriers.push_back(h);
      }

      // construct file name, open output root file and book output histograms

      const auto& expRun = getRunList();
      string expNo = to_string(expRun[0].first);
      while (expNo.length() < 4) expNo.insert(0, "0");
      string runNo = to_string(expRun[0].second);
      while (runNo.length() < 5) runNo.insert(0, "0");
      string outputFileName = "calibrateBS13d-e" + expNo + "-r" + runNo + ".root";
      auto* file = TFile::Open(outputFileName.c_str(), "recreate");

      int nx = m_shiftEnd - m_shiftBegin;
      double xmi = m_shiftBegin - 0.5;
      double xma = m_shiftEnd - 0.5;

      std::vector<TH1F*> chi2Carriers;
      for (unsigned i = 0; i < 4; i++) {
        string name = "chi2_carr_" + to_string(i);
        string title = "chi2 of carrier " + to_string(i);
        auto* h = new TH1F(name.c_str(), title.c_str(), nx, xmi, xma);
        h->SetXTitle("shift [127 MHz clk cycles]");
        h->SetYTitle("-2*logL [arbitrary]");
        chi2Carriers.push_back(h);
      }

      auto* shiftClk = new TH1F("shift_clk", "shift in clk units", 4, -0.5, 3.5);
      shiftClk->SetXTitle("boardstack number");
      shiftClk->SetYTitle("shift [127 MHz clk cycles]");

      auto* shiftTime = new TH1F("shift_time", "shift in time units", 4, -0.5, 3.5);
      shiftTime->SetXTitle("boardstack number");
      shiftTime->SetYTitle("shift [ns]");

      auto* signif = new TH1F("significance", "significance of the result", 4, -0.5, 3.5);
      signif->SetXTitle("boardstack number");
      signif->SetYTitle("significance [sigma]");

      // find carrier shifts

      double timeStep = timeReference->GetBinWidth(1);
      for (unsigned cb = 0; cb < 4; cb++) {
        auto& h_chi = chi2Carriers[cb];
        auto& h_time = timeCarriers[cb];
        for (int shift = m_shiftBegin; shift < m_shiftEnd; shift++) {
          double chi2 = -2.0 * logL(h_time, shift);
          h_chi->SetBinContent(shift - m_shiftBegin + 1, chi2);
        }
        double hmin = h_chi->GetMinimum();
        for (int i = 0; i < h_chi->GetNbinsX(); i++) {
          h_chi->SetBinContent(i + 1, h_chi->GetBinContent(i + 1) - hmin);
        }
        int i0 = h_chi->GetMinimumBin();
        double x = h_chi->GetBinCenter(i0);
        shiftClk->SetBinContent(cb + 1, x);
        shiftTime->SetBinContent(cb + 1, x * timeStep);
        double s = std::min(h_chi->GetBinContent(i0 - 1), h_chi->GetBinContent(i0 + 1));
        signif->SetBinContent(cb + 1, sqrt(s));
      }

      // save results in vectors since histograms are destroyed after file is closed

      std::vector<double> shifts;
      std::vector<double> significances;
      for (unsigned cb = 0; cb < 4; cb++) {
        shifts.push_back(shiftTime->GetBinContent(cb + 1));
        significances.push_back(signif->GetBinContent(cb + 1));
      }

      // write the results and close the file

      auto time_vs_BS = getObjectPtr<TH2F>("time_vs_BS");
      if (time_vs_BS) {
        time_vs_BS->Write();
        // make histogram corrected for shifts - for visual validation
        int Nx = time_vs_BS->GetNbinsX();
        int Ny = time_vs_BS->GetNbinsY();
        int step = Nx / 16;
        if (step * 16 == Nx) {
          auto* time_vs_BS_corr = (TH2F*) time_vs_BS->Clone("time_vs_BS_corrected");
          int i0 = (Nx / 4) * 3;
          for (int i = i0; i < Nx; i++) {
            int cb = (i - i0) / step;
            int shift = shiftClk->GetBinContent(cb + 1);
            if (shift != 0) {
              for (int k = 0; k < Ny; k++) {
                double val = 0;
                int k0 = k + shift;
                if (k0 < Ny) val = time_vs_BS->GetBinContent(i + 1, k0 + 1);
                time_vs_BS_corr->SetBinContent(i + 1, k + 1, val);
              }
            }
          }
        } else {
          B2ERROR("TOPAsicShiftsBS13dAlgorithm: can't make corrected histogram");
        }
      }
      timeReference->Write();
      for (auto& h : timeCarriers) h->Write();
      file->Write();
      file->Close();

      // check result significances and return if too low

      for (auto significance : significances) {
        if (significance < m_minSignificance) return c_NotEnoughData;
      }

      // otherwise create and import payload to DB

      auto* asicShift = new TOPCalAsicShift();
      int moduleID = 13;
      unsigned bs = 3;
      for (unsigned cb = 0; cb < 4; cb++) {
        for (unsigned a = 0; a < 4; a++) {
          unsigned asic = a + cb * 4 + bs * 16;
          asicShift->setT0(moduleID, asic, shifts[cb]);
        }
      }
      saveCalibration(asicShift);

      return c_OK;
    }


    double TOPAsicShiftsBS13dAlgorithm::fun(int x)
    {
      int nx = m_timeReference.size();
      if (x < 0 or x >= nx) return m_minVal;
      return std::max(m_timeReference[x], m_minVal);
    }


    std::vector<double> TOPAsicShiftsBS13dAlgorithm::getPDF(int ishft)
    {
      std::vector<double> p;
      for (size_t i = 0; i < m_timeReference.size(); i++) {
        p.push_back(fun(i - ishft));
      }
      double s = 0;
      for (auto x : p) s += x;
      for (auto& x : p) x /= s;
      return p;
    }


    double TOPAsicShiftsBS13dAlgorithm::logL(std::shared_ptr<TH1F> h, int shift)
    {
      double logl = 0;
      auto pdf = getPDF(shift);
      for (unsigned i = 0; i < pdf.size(); i++) {
        logl += h->GetBinContent(i + 1) * log(pdf[i]) - pdf[i];
      }
      return logl;
    }

  } // end namespace TOP
} // end namespace Belle2
