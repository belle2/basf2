/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/calibration/TOPValidationAlgorithm.h>
#include <top/utilities/Chi2MinimumFinder1D.h>

#include <TFile.h>
#include <TH1F.h>
#include <TH1D.h>
#include <TH2F.h>

#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;

namespace Belle2 {
  namespace TOP {

    TOPValidationAlgorithm::TOPValidationAlgorithm():
      CalibrationAlgorithm("TOPValidationCollector")
    {
      setDescription("Calibration algorithm for automatic validation of the calibration");
    }


    CalibrationAlgorithm::EResult TOPValidationAlgorithm::calibrate()
    {
      // get basic histogram

      auto h1 = getObjectPtr<TH2F>("tracks_per_slot");
      if (not h1) {
        B2ERROR("TOPValidationAlgorithm: histogram 'tracks_per_slot' not found");
        return c_Failure;
      }
      unsigned numModules = h1->GetNbinsX();
      unsigned numSets = h1->GetNbinsY();
      double scaleFactor = 1; // scaling factor for module and common T0 uncertainties

      // open output root file

      auto* file = TFile::Open("TOPCalValidation.root", "recreate");

      // write-out the histograms that are ready

      h1->Write();
      auto h2 = getObjectPtr<TH1F>("moduleT0_pulls");
      if (h2) {
        if (h2->GetEntries() > 1) scaleFactor = h2->GetRMS();
        h2->Write();
      }
      for (unsigned slot = 1; slot <= numModules; slot++) {
        string slotName = to_string(slot);
        if (slot < 10) slotName.insert(0, "0");
        string name = "hits_slot" + slotName;
        auto h = getObjectPtr<TH2F>(name);
        if (h) h->Write();
      }

      // channelT0 residuals: determine Chi2 minima and the pulls; save results into histograms

      std::vector<TH1F*> h_slots;
      auto h_pulls = new TH1F("channelT0_pulls", "Channel T0 pulls; pulls", 200, -15.0, 15.0);

      for (unsigned slot = 1; slot <= numModules; slot++) {
        string slotName = to_string(slot);
        if (slot < 10) slotName.insert(0, "0");
        std::vector<std::shared_ptr<TH2F> > histos;
        for (unsigned set = 0; set < numSets; set++) {
          string name = "chi2_set" + to_string(set) + "_slot" + slotName;
          auto h = getObjectPtr<TH2F>(name);
          if (h) histos.push_back(h);
        }
        if (histos.empty()) continue;

        int nx = histos[0]->GetNbinsX();
        string name = "channelT0_slot" + slotName;
        string title = "ChannelT0 slot" + slotName + "; channel; T0 [ns]";
        auto* h0 = new TH1F(name.c_str(), title.c_str(), nx, 0, nx);
        h_slots.push_back(h0);

        for (int ibin = 1; ibin <= nx; ibin++) {
          Chi2MinimumFinder1D finder;
          std::vector<double> pos, err;
          for (auto& h : histos) {
            Chi2MinimumFinder1D finder1(std::shared_ptr<TH1D>(h->ProjectionY("py", ibin, ibin)));
            finder.add(finder1);
            const auto& minimum = finder1.getMinimum();
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
          const auto& minimum = finder.getMinimum();
          if (not minimum.valid) continue;
          h0->SetBinContent(ibin, minimum.position);
          h0->SetBinError(ibin, minimum.error);
        }
      }

      // channelT0: rescale the errors

      if (h_pulls->GetEntries() > 1) {
        double scaleError = h_pulls->GetRMS();
        for (auto& h : h_slots) {
          for (int ibin = 1; ibin <= h->GetNbinsX(); ibin++) h->SetBinError(ibin, h->GetBinError(ibin) * scaleError);
        }
      }

      // moduleT0, commonT0 and others: get input tree and set branch addresses

      auto inputTree = getObjectPtr<TTree>("tree");
      if (not inputTree) {
        B2ERROR("TOPValidationAlgorithm: input tree 'tree' not found");
        return c_Failure;
      }
      inputTree->SetBranchAddress("exp", &m_inputEntry.expNo);
      inputTree->SetBranchAddress("run", &m_inputEntry.runNo);
      inputTree->SetBranchAddress("numTracks", &m_inputEntry.numTracks);
      inputTree->SetBranchAddress("commonT0", &m_inputEntry.commonT0);
      inputTree->SetBranchAddress("commonT0Err", &m_inputEntry.commonT0Err);
      inputTree->SetBranchAddress("moduleT0", &m_inputEntry.moduleT0);
      inputTree->SetBranchAddress("moduleT0Err", &m_inputEntry.moduleT0Err);
      inputTree->SetBranchAddress("numTBCalibrated", &m_inputEntry.numTBCalibrated);
      inputTree->SetBranchAddress("numT0Calibrated", &m_inputEntry.numT0Calibrated);
      inputTree->SetBranchAddress("numActive", &m_inputEntry.numActive);
      inputTree->SetBranchAddress("numActiveCalibrated", &m_inputEntry.numActiveCalibrated);
      inputTree->SetBranchAddress("thrEffi", &m_inputEntry.thrEffi);
      inputTree->SetBranchAddress("asicShifts", &m_inputEntry.shifts);

      // moduleT0, commonT0 and others: define output tree

      file->cd();
      auto* outputTree = new TTree("tree", "Validation tree, merged per run");
      outputTree->Branch("exp", &m_outputEntry.expNo, "exp/I");
      outputTree->Branch("run", &m_outputEntry.runNo, "run/I");
      outputTree->Branch("numTracks", &m_outputEntry.numTracks, "numTracks/I");
      outputTree->Branch("commonT0", &m_outputEntry.commonT0, "commonT0/F");
      outputTree->Branch("commonT0Err", &m_outputEntry.commonT0Err, "commonT0Err/F");
      outputTree->Branch("moduleT0", &m_outputEntry.moduleT0, "moduleT0[16]/F");
      outputTree->Branch("moduleT0Err", &m_outputEntry.moduleT0Err, "moduleT0Err[16]/F");
      outputTree->Branch("numTBCalibrated", &m_outputEntry.numTBCalibrated, "numTBCalibrated[16]/I");
      outputTree->Branch("numT0Calibrated", &m_outputEntry.numT0Calibrated, "numT0Calibrated[16]/I");
      outputTree->Branch("numActive", &m_outputEntry.numActive, "numActive[16]/I");
      outputTree->Branch("numActiveCalibrated", &m_outputEntry.numActiveCalibrated, "numActiveCalibrated[16]/I");
      outputTree->Branch("thrEffi", &m_outputEntry.thrEffi, "thrEffi[16]/F");
      outputTree->Branch("asicShifts", &m_outputEntry.shifts, "asicShifts[4]/F");
      outputTree->Branch("numMerged", &m_outputEntry.numMerged, "numMerged/I");

      // sort input tree entries according to exp/run

      std::multimap<int, int> sortedEntries;
      std::set<int> sortedRuns;
      for (int iev = 0; iev < inputTree->GetEntries(); iev++) {
        inputTree->GetEntry(iev);
        int expRun = (m_inputEntry.expNo << 16) + m_inputEntry.runNo;
        sortedEntries.emplace(expRun, iev);
        sortedRuns.insert(expRun);
      }

      // merge input entries of the same exp/run, rescale the errors and fill the output tree

      for (auto expRun : sortedRuns) {
        m_outputEntry.clearNumMerged();
        const auto range = sortedEntries.equal_range(expRun);
        for (auto it = range.first; it != range.second; ++it) {
          inputTree->GetEntry(it->second);
          m_outputEntry.merge(m_inputEntry);
        }
        m_outputEntry.rescaleErrors(scaleFactor);
        outputTree->Fill();
      }

      // write-out the results

      file->Write();
      file->Close();

      return c_OK;
    }

  } // end namespace TOP
} // end namespace Belle2
