/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/calibration/TOPCommonT0LLAlgorithm.h>
#include <top/dbobjects/TOPCalCommonT0.h>
#include <top/utilities/Chi2MinimumFinder1D.h>

#include <math.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH1D.h>
#include <TH2F.h>

#include <string>
#include <vector>

using namespace std;

namespace Belle2 {
  namespace TOP {

    TOPCommonT0LLAlgorithm::TOPCommonT0LLAlgorithm():
      CalibrationAlgorithm("TOPCommonT0LLCollector")
    {
      setDescription("Calibration algorithm for common T0 calibration "
                     "with neg. log likelihood minimization (method LL)");
    }

    CalibrationAlgorithm::EResult TOPCommonT0LLAlgorithm::calibrate()
    {

      // get histograms

      auto h1 = getObjectPtr<TH1F>("tracks_per_set");
      if (not h1) {
        B2ERROR("TOPCommonT0LLAlgorithm: histogram 'tracks_per_set' not found");
        return c_NotEnoughData;
      }
      unsigned numSets = h1->GetNbinsX();

      vector<Chi2MinimumFinder1D> finders;
      for (unsigned set = 0; set < numSets; set++) {
        string name = "chi2_set" + to_string(set);
        auto h = getObjectPtr<TH1D>(name);
        if (not h) continue;
        finders.push_back(Chi2MinimumFinder1D(h));
      }
      if (finders.size() != numSets) {
        B2ERROR("TOPCommonT0LLAlgorithm: got number of chi2 scans not as expected"
                << LogVar("expected", numSets)
                << LogVar("found", finders.size()));
        return c_NotEnoughData;
      }

      // bunch separation in time

      auto h4 = getObjectPtr<TH1F>("offset");
      if (not h4) {
        B2ERROR("TOPCommonT0LLAlgorithm: histogram 'offset' not found");
        return c_NotEnoughData;
      }
      double bunchTimeSep = h4->GetXaxis()->GetXmax() - h4->GetXaxis()->GetXmin();

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
      tree->Branch<float>("errorScaling", &m_errorScaling);
      tree->Branch<int>("nTracks", &m_numTracks);
      tree->Branch<int>("nEvt", &m_numEvents);
      tree->Branch<int>("fitStatus", &m_fitStatus);

      m_expNo = expRun[0].first;
      m_runNo = expRun[0].second;
      m_runLast = expRun.back().second;

      // determine scaling factor for the error from statistically independent results

      auto h_pulls = new TH1F("pulls", "Pulls of statistically independent results",
                              200, -15.0, 15.0);
      h_pulls->SetXTitle("pulls");
      std::vector<double> pos, err;
      for (auto& finder : finders) {
        const auto& minimum = finder.getMinimum();
        if (not minimum.valid) continue;
        pos.push_back(minimum.position);
        err.push_back(minimum.error);
      }
      for (unsigned i = 0; i < pos.size(); i++) {
        for (unsigned j = i + 1; j < pos.size(); j++) {
          double pull = (pos[i] - pos[j]) / sqrt(err[i] * err[i] + err[j] * err[j]);
          h_pulls->Fill(pull);
        }
      }
      double scaleError = 1;
      if (h_pulls->GetEntries() > 1) scaleError = h_pulls->GetRMS();

      // merge statistically independent finders and store results into histograms

      auto finder = finders[0];
      for (unsigned i = 1; i < numSets; i++) {
        finder.add(finders[i]);
      }

      auto h_commonT0 = new TH1F("commonT0", "Common T0", 1, 0, 1);
      h_commonT0->SetYTitle("common T0 [ns]");

      const auto& minimum = finder.getMinimum();
      if (minimum.valid) {
        m_fittedOffset = minimum.position;
        m_offset = m_fittedOffset - round(m_fittedOffset / bunchTimeSep) * bunchTimeSep;
        m_offsetError = minimum.error * scaleError;
        m_fitStatus = 0;
        h_commonT0->SetBinContent(1, m_offset);
        h_commonT0->SetBinError(1, m_offsetError);
      } else {
        m_fittedOffset = 0;
        m_offset = 0;
        m_offsetError = 0;
        m_fitStatus = 1;
      }
      m_errorScaling = scaleError;
      m_numTracks = h1->GetSumOfWeights();
      m_numEvents = h4->GetEntries();
      tree->Fill();

      // write the results and close the file

      file->Write();
      auto h = finder.getHistogram("chi2", "chi2 scan; t0 [ns]; -2 logL");
      h.Write();
      h1->Write();
      auto h2 = getObjectPtr<TH1F>("numHits");
      if (h2) h2->Write();
      auto h3 = getObjectPtr<TH2F>("timeHits");
      if (h3) h3->Write();
      h4->Write();
      file->Close();

      // check the results and return if not good enough

      if (m_fitStatus != 0 or m_offsetError > m_minError) return c_NotEnoughData;

      // otherwise create and import payload to DB

      auto* commonT0 = new TOPCalCommonT0(m_offset, m_offsetError);
      saveCalibration(commonT0);

      return c_OK;
    }

  } // end namespace TOP
} // end namespace Belle2
