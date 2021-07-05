/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/calibration/TOPModuleT0LLAlgorithm.h>
#include <top/dbobjects/TOPCalModuleT0.h>
#include <top/utilities/Chi2MinimumFinder1D.h>

#include <math.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH1D.h>
#include <TH2F.h>

#include <string>
#include <vector>

using namespace std;

namespace Belle2 {
  namespace TOP {

    TOPModuleT0LLAlgorithm::TOPModuleT0LLAlgorithm():
      CalibrationAlgorithm("TOPModuleT0LLCollector")
    {
      setDescription("Calibration algorithm for method LL");
    }

    CalibrationAlgorithm::EResult TOPModuleT0LLAlgorithm::calibrate()
    {
      // get basic histogram

      auto h1 = getObjectPtr<TH2F>("tracks_per_slot");
      if (not h1) {
        B2ERROR("TOPModuleT0LLAlgorithm: histogram 'tracks_per_slot' not found");
        return c_Failure;
      }
      unsigned numModules = h1->GetNbinsX();
      unsigned numSets = h1->GetNbinsY();

      // construct file name, open output root file

      const auto& expRun = getRunList();
      string expNo = to_string(expRun[0].first);
      while (expNo.length() < 4) expNo.insert(0, "0");
      string runNo = to_string(expRun[0].second);
      while (runNo.length() < 5) runNo.insert(0, "0");
      string outputFileName = "moduleT0_final-e" + expNo + "-r" + runNo + ".root";
      auto* file = TFile::Open(outputFileName.c_str(), "recreate");

      // write-out input histograms

      h1->Write();
      auto h2 = getObjectPtr<TH1F>("numHits");
      if (h2) h2->Write();
      auto h3 = getObjectPtr<TH2F>("timeHits");
      if (h3) h3->Write();
      auto h4 = getObjectPtr<TH1F>("offset");
      if (h4) h4->Write();

      // create histogram of pulls which is needed for error rescaling

      auto h_pulls = new TH1F("pulls", "Pulls of statistically independent results",
                              200, -15.0, 15.0);
      h_pulls->SetXTitle("pulls");

      // create vectors for keeping the results

      std::vector<double> T0(numModules, 0);
      std::vector<double> errT0(numModules, 0);

      // find minimum for each slot

      bool ok = true;
      for (unsigned slot = 1; slot <= numModules; slot++) {

        Chi2MinimumFinder1D slotFinder;
        std::vector<double> pos, err;

        // loop over statistically independent subsamples, add them
        for (unsigned set = 0; set < numSets; set++) {

          if (h1->GetBinContent(slot, set + 1) == 0) continue;

          string name = "chi2_set" + to_string(set) + "_slot" + to_string(slot);
          auto h = getObjectPtr<TH1D>(name);
          if (not h) {
            B2ERROR("TOPModuleT0LLAlgorithm: histogram '" << name << "' not found");
            continue;
          }
          Chi2MinimumFinder1D setFinder(h);
          if (slotFinder.getNbins() == 0) {
            slotFinder = setFinder;
          } else {
            slotFinder.add(setFinder);
          }
          const auto& minimum = setFinder.getMinimum();
          if (not minimum.valid) continue;
          pos.push_back(minimum.position);
          err.push_back(minimum.error);
        }

        // fill pulls
        for (unsigned i = 0; i < pos.size(); i++) {
          for (unsigned j = i + 1; j < pos.size(); j++) {
            double pull = (pos[i] - pos[j]) / sqrt(err[i] * err[i] + err[j] * err[j]);
            h_pulls->Fill(pull);
          }
        }

        // determine minimum and store it
        const auto& minimum = slotFinder.getMinimum();
        if (minimum.valid) {
          T0[slot - 1] = minimum.position;
          errT0[slot - 1] = minimum.error;
        } else {
          B2ERROR("TOPModuleT0Algorithm: no minimum found for slot " << slot);
          ok = false;
        }

        // write-out histogram of a chi2 scan
        string name = "chi2_slot_" + to_string(slot);
        string title = "chi2 scan, slot " + to_string(slot) + "; t0 [ns]; -2 logL";
        auto h = slotFinder.getHistogram(name, title);
        h.Write();
      }

      // rescale the errors

      if (h_pulls->GetEntries() > 1) {
        double scaleError = h_pulls->GetRMS();
        for (auto& err : errT0) err *= scaleError;
      }

      // store the results in a histogram

      auto* h_moduleT0 = new TH1F("moduleT0", "Module T0", 16, 0.5, 16.5);
      h_moduleT0->SetXTitle("slot number");
      h_moduleT0->SetYTitle("module T0 [ns]");
      for (unsigned i = 0; i < T0.size(); i++) {
        h_moduleT0->SetBinContent(i + 1, T0[i]);
        h_moduleT0->SetBinError(i + 1, errT0[i]);
      }

      // write the results and close the file

      file->Write();
      file->Close();

      // check the results and return if not good enough

      if (not ok) return c_Failure;
      for (auto err : errT0) {
        if (err > m_minError) return c_NotEnoughData;
      }

      // otherwise create and import payload to DB

      auto* moduleT0 = new TOPCalModuleT0();
      for (unsigned i = 0; i < T0.size(); i++) {
        moduleT0->setT0(i + 1, T0[i], errT0[i]);
      }
      moduleT0->suppressAverage();
      saveCalibration(moduleT0);

      return c_OK;
    }

  } // end namespace TOP
} // end namespace Belle2
