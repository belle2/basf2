/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/calibration/TOPEventT0OffsetAlgorithm.h>
#include <top/dbobjects/TOPCalEventT0Offset.h>
#include <framework/gearbox/Const.h>
#include <string>
#include <map>
#include <TROOT.h>
#include <TH1F.h>
#include <TF1.h>

using namespace std;

namespace Belle2 {
  namespace TOP {

    TOPEventT0OffsetAlgorithm::TOPEventT0OffsetAlgorithm():
      CalibrationAlgorithm("TOPOffsetCollector")
    {
      setDescription("Calibration algorithm for event T0 offset calibration");
    }

    CalibrationAlgorithm::EResult TOPEventT0OffsetAlgorithm::calibrate()
    {
      gROOT->SetBatch();

      // construct file name and open output root file

      const auto& expRun = getRunList();
      string expNo = to_string(expRun[0].first);
      while (expNo.length() < 4) expNo.insert(0, "0");
      string runNo = to_string(expRun[0].second);
      while (runNo.length() < 5) runNo.insert(0, "0");
      string outputFileName = "evenT0Offset-e" + expNo + "-r" + runNo + ".root";
      auto* file = TFile::Open(outputFileName.c_str(), "recreate");

      // object to store the results

      auto* eventT0Offset = new TOPCalEventT0Offset();

      // fit histograms and store the results

      std::map<Const::EDetector, std::string> names;
      names[Const::SVD] = "svdOffset";
      names[Const::CDC] = "cdcOffset";
      for (const auto& x : names) {
        auto h = getObjectPtr<TH1F>(x.second);
        if (not h) continue;
        double sum = h->Integral();
        if (sum > m_minEntries) {
          auto* func = new TF1("func1g",
                               "[0] + [1]/sqrt(2*pi)/[3]*exp(-0.5*((x-[2])/[3])**2)",
                               h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax());
          func->SetParameter(0, 0);
          func->SetParameter(1, sum * h->GetBinWidth(1));
          func->SetParameter(2, h->GetBinCenter(h->GetMaximumBin()));
          func->SetParameter(3, m_sigma);
          int status = h->Fit(func, "LRSQ");
          if (status == 0) eventT0Offset->set(x.first, func->GetParameter(2), std::abs(func->GetParameter(3)));
        }
        h->Write();
      }

      file->Close();

      if (eventT0Offset->isEmpty()) {
        delete eventT0Offset;
        return c_NotEnoughData;
      }

      saveCalibration(eventT0Offset);

      return c_OK;
    }

  } // end namespace TOP
} // end namespace Belle2
