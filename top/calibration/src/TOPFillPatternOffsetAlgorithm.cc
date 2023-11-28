/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/calibration/TOPFillPatternOffsetAlgorithm.h>
#include <top/dbobjects/TOPCalFillPatternOffset.h>
#include <string>
#include <TROOT.h>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace TOP {

    TOPFillPatternOffsetAlgorithm::TOPFillPatternOffsetAlgorithm():
      CalibrationAlgorithm("TOPOffsetCollector")
    {
      setDescription("Calibration algorithm for fill pattern offset");
    }

    CalibrationAlgorithm::EResult TOPFillPatternOffsetAlgorithm::calibrate()
    {
      gROOT->SetBatch();

      // construct file name and open output root file

      const auto& expRun = getRunList();
      string expNo = to_string(expRun[0].first);
      while (expNo.length() < 4) expNo.insert(0, "0");
      string runNo = to_string(expRun[0].second);
      while (runNo.length() < 5) runNo.insert(0, "0");
      string outputFileName = "fillPatternOffset-e" + expNo + "-r" + runNo + ".root";
      auto* file = TFile::Open(outputFileName.c_str(), "recreate");

      auto fillPattern = getObjectPtr<TH1F>("fillPattern");
      if (fillPattern) fillPattern->Write();
      auto recBuckets = getObjectPtr<TH1F>("recBuckets");
      if (recBuckets) recBuckets->Write();

      // object to store the results

      auto* fillPatternOffset = new TOPCalFillPatternOffset();

      // find offset

      if (fillPattern and recBuckets and recBuckets->GetEntries() > m_minEntries) {
        auto* chi2 = getChi2Histogram(recBuckets.get(), fillPattern.get());
        int offset = chi2->GetMinimumBin() - 1;
        double fract = getFraction(recBuckets.get(), fillPattern.get(), offset);
        fillPatternOffset->set(offset, fract);
        if (fract < m_minFract) {
          fillPatternOffset->setUnusable();
        }
      }

      file->Write();
      file->Close();

      // save the object in any case (very important!)

      saveCalibration(fillPatternOffset);

      return c_OK;
    }


    double TOPFillPatternOffsetAlgorithm::Chi2(TH1F* recBuckets, TH1F* fillPattern, int i0)
    {
      double chi2 = 0;
      int RFBuckets = fillPattern->GetNbinsX();
      auto* a = recBuckets->GetArray();
      auto* a0 = fillPattern->GetArray();
      for (int i = 0; i < RFBuckets; i++) {
        int k = (i + i0) % RFBuckets;
        chi2 += a[k] * log(m_p * a0[i] + 1 - m_p);
      }
      return -2 * chi2;
    }


    TH1F* TOPFillPatternOffsetAlgorithm::getChi2Histogram(TH1F* recBuckets, TH1F* fillPattern)
    {
      int RFBuckets = fillPattern->GetNbinsX();
      for (int i = 0; i < RFBuckets; i++) {
        if (fillPattern->GetBinContent(i + 1) > 0) fillPattern->SetBinContent(i + 1, 1);
      }
      auto* h_chi2 = new TH1F("chi2", "#chi^{2} vs. circular shift; circular shift; #chi^{2}", RFBuckets, 0, RFBuckets);
      for (int i = 0; i < RFBuckets; i++) {
        h_chi2->SetBinContent(i + 1, Chi2(recBuckets, fillPattern, i));
      }
      return h_chi2;
    }


    double TOPFillPatternOffsetAlgorithm::getFraction(TH1F* recBuckets, TH1F* fillPattern, int offset)
    {
      double n = 0;
      double n0 = 0;
      int RFBuckets = fillPattern->GetNbinsX();
      auto* corrBuckets = new TH1F("corrBuckets", "Offset corrected reconstructed buckets; bucket number",
                                   RFBuckets, 0, RFBuckets);
      auto* a = recBuckets->GetArray();
      auto* a0 = fillPattern->GetArray();
      for (int i = 0; i < RFBuckets; i++) {
        int k = (i + offset) % RFBuckets;
        corrBuckets->SetBinContent(i + 1, a[k]);
        if (a0[i] > 0) n += a[k];
        n0 += a[k];
      }
      double fract = n / n0;
      double err = sqrt(fract * (1 - fract) / n0);

      auto* h_fract = new TH1F("fractions", "Fractions of matched buckets; ; fraction", 2, 0, 2);
      h_fract->GetXaxis()->SetBinLabel(1, "not matched");
      h_fract->GetXaxis()->SetBinLabel(2, "matched");
      h_fract->GetXaxis()->SetLabelSize(0.06);
      h_fract->GetXaxis()->SetAlphanumeric();
      h_fract->SetBinContent(1, 1 - fract);
      h_fract->SetBinContent(2, fract);
      h_fract->SetBinError(1, err);
      h_fract->SetBinError(2, err);
      h_fract->SetMarkerStyle(24);

      return fract;
    }


  } // end namespace TOP
} // end namespace Belle2
