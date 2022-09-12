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
#include <cmath>

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
      // open output root file

      auto* file = TFile::Open("TOPCalValidation.root", "recreate");

      // write-out the histograms that are ready

      B2INFO("TOPValidationAlgorithm: get basic histograms");

      double scaleFactor = 1; // scaling factor for module and common T0 uncertainties
      auto h1 = getObjectPtr<TH1F>("moduleT0_pulls");
      if (h1) {
        if (h1->GetEntries() > 1) scaleFactor = h1->GetRMS();
        h1->Write();
      }

      std::vector<string> slotNames;
      for (unsigned slot = 1; slot <= c_numModules; slot++) {
        string slotName = "slot";
        if (slot < 10) slotName += "0";
        slotName += to_string(slot);
        slotNames.push_back(slotName);
      }

      for (unsigned slot = 0; slot < c_numModules; slot++) {
        string name = "hits_" + slotNames[slot];
        auto h = getObjectPtr<TH2F>(name);
        if (h) h->Write();
      }

      // channelT0 residuals: determine Chi2 minima and save the results into histograms

      B2INFO("TOPValidationAlgorithm: determine channel T0 residuals");

      for (unsigned slot = 0; slot < c_numModules; slot++) {
        string name = "chi2_" + slotNames[slot];
        auto h = getObjectPtr<TH2F>(name);
        if (not h) continue;

        int nx = h->GetNbinsX();
        string nameOut = "channelT0_" + slotNames[slot];
        string title = "ChannelT0, " + slotNames[slot] + "; channel; residuals [ns]";
        auto* h0 = new TH1F(nameOut.c_str(), title.c_str(), nx, 0, nx);

        for (int ibin = 1; ibin <= nx; ibin++) {
          Chi2MinimumFinder1D finder(std::shared_ptr<TH1D>(h->ProjectionY("py", ibin, ibin)));
          const auto& minimum = finder.getMinimum();
          if (not minimum.valid) continue;
          h0->SetBinContent(ibin, minimum.position);
          h0->SetBinError(ibin, minimum.error);
        }
      }

      // moduleT0, commonT0 and others: get input tree and set branch addresses

      B2INFO("TOPValidationAlgorithm: get input tree");

      auto inputTree = getObjectPtr<TTree>("tree");
      if (not inputTree) {
        B2ERROR("TOPValidationAlgorithm: input tree not found");
        file->Write();
        file->Close();
        return c_Failure;
      }
      inputTree->SetBranchAddress("expNo", &m_treeEntry.expNo);
      inputTree->SetBranchAddress("runNo", &m_treeEntry.runNo);
      inputTree->SetBranchAddress("numTracks", &m_treeEntry.numTracks);
      inputTree->SetBranchAddress("commonT0", &m_treeEntry.commonT0);
      inputTree->SetBranchAddress("commonT0Err", &m_treeEntry.commonT0Err);
      inputTree->SetBranchAddress("moduleT0", &m_treeEntry.moduleT0);
      inputTree->SetBranchAddress("moduleT0Err", &m_treeEntry.moduleT0Err);
      inputTree->SetBranchAddress("numTBCalibrated", &m_treeEntry.numTBCalibrated);
      inputTree->SetBranchAddress("numT0Calibrated", &m_treeEntry.numT0Calibrated);
      inputTree->SetBranchAddress("numActive", &m_treeEntry.numActive);
      inputTree->SetBranchAddress("numActiveCalibrated", &m_treeEntry.numActiveCalibrated);
      inputTree->SetBranchAddress("thrEffi", &m_treeEntry.thrEffi);
      inputTree->SetBranchAddress("asicShifts", &m_treeEntry.asicShifts);
      inputTree->SetBranchAddress("svdOffset", &m_treeEntry.svdOffset);
      inputTree->SetBranchAddress("svdSigma", &m_treeEntry.svdSigma);
      inputTree->SetBranchAddress("cdcOffset", &m_treeEntry.cdcOffset);
      inputTree->SetBranchAddress("cdcSigma", &m_treeEntry.cdcSigma);
      inputTree->SetBranchAddress("fillPatternOffset", &m_treeEntry.fillPatternOffset);
      inputTree->SetBranchAddress("fillPatternFraction", &m_treeEntry.fillPatternFraction);

      // sort input tree entries according to exp/run

      B2INFO("TOPValidationAlgorithm: sort input tree entries");

      std::multimap<int, ValidationTreeStruct> sortedEntries;
      std::set<int> sortedRuns;
      for (int iev = 0; iev < inputTree->GetEntries(); iev++) {
        inputTree->GetEntry(iev);
        int expRun = (m_treeEntry.expNo << 16) + m_treeEntry.runNo;
        sortedEntries.emplace(expRun, m_treeEntry);
        sortedRuns.insert(expRun);
      }

      // merge input entries of the same exp/run and rescale the errors

      B2INFO("TOPValidationAlgorithm: merge input tree entries");

      std::vector<ValidationTreeStruct> mergedEntries;
      for (auto expRun : sortedRuns) {
        ValidationTreeStruct mergedEntry;
        const auto range = sortedEntries.equal_range(expRun);
        for (auto it = range.first; it != range.second; ++it) {
          mergedEntry.merge(it->second);
        }
        mergedEntry.rescaleErrors(scaleFactor);
        mergedEntries.push_back(mergedEntry);
      }

      // make histograms vs. run index

      int nx = mergedEntries.size();
      if (nx == 0) {
        B2ERROR("TOPValidationAlgorithm: input tree is empty");
        file->Write();
        file->Close();
        return c_Failure;
      }

      file->cd();

      std::set<int> sortedExps;
      for (auto& entry : mergedEntries) sortedExps.insert(entry.expNo);
      std::string titleIndex = "Experiment";
      if (sortedExps.size() > 1) titleIndex += "s";
      for (auto expNo : sortedExps) titleIndex += " " + to_string(expNo) + ",";
      titleIndex.pop_back();
      auto* h_index = new TH1F("runIndex", (titleIndex + "; run index; run number").c_str(), nx, 0, nx);
      for (int i = 0; i < nx; i++) {
        h_index->SetBinContent(i + 1, mergedEntries[i].runNo);
      }

      auto* h_numTracks = new TH1F("numTracks", "Number of tracks; run index; number of tracks", nx, 0, nx);
      for (int i = 0; i < nx; i++) {
        h_numTracks->SetBinContent(i + 1, mergedEntries[i].numTracks);
      }

      auto* h_numMerged = new TH1F("numMerged", "Number of merged entries; run index; number of merged", nx, 0, nx);
      for (int i = 0; i < nx; i++) {
        h_numMerged->SetBinContent(i + 1, mergedEntries[i].numMerged);
      }

      auto* h_commonT0 = new TH1F("commonT0", "commonT0; run index; residuals [ns]", nx, 0, nx);
      for (int i = 0; i < nx; i++) {
        h_commonT0->SetBinContent(i + 1, mergedEntries[i].commonT0);
        h_commonT0->SetBinError(i + 1, mergedEntries[i].commonT0Err);
      }

      for (unsigned slot = 0; slot < c_numModules; slot++) {
        string name = "moduleT0_" + slotNames[slot];
        string title = "moduleT0, " + slotNames[slot] + "; run index; residuals [ns]";
        auto* h = new TH1F(name.c_str(), title.c_str(), nx, 0, nx);
        for (int i = 0; i < nx; i++) {
          h->SetBinContent(i + 1, mergedEntries[i].moduleT0[slot]);
          h->SetBinError(i + 1, mergedEntries[i].moduleT0Err[slot]);
        }
      }

      for (unsigned slot = 0; slot < c_numModules; slot++) {
        string name = "numTBCalibrated_" + slotNames[slot];
        string title = "Time base calibrated, " + slotNames[slot] + "; run index; fraction";
        auto* h = new TH1F(name.c_str(), title.c_str(), nx, 0, nx);
        for (int i = 0; i < nx; i++) {
          h->SetBinContent(i + 1, mergedEntries[i].numTBCalibrated[slot] / 512.0);
        }
      }

      for (unsigned slot = 0; slot < c_numModules; slot++) {
        string name = "numT0Calibrated_" + slotNames[slot];
        string title = "channel T0 calibrated, " + slotNames[slot] + "; run index; fraction";
        auto* h = new TH1F(name.c_str(), title.c_str(), nx, 0, nx);
        for (int i = 0; i < nx; i++) {
          h->SetBinContent(i + 1, mergedEntries[i].numT0Calibrated[slot] / 512.0);
        }
      }

      for (unsigned slot = 0; slot < c_numModules; slot++) {
        string name = "numActive_" + slotNames[slot];
        string title = "Active, " + slotNames[slot] + "; run index; fraction";
        auto* h = new TH1F(name.c_str(), title.c_str(), nx, 0, nx);
        for (int i = 0; i < nx; i++) {
          h->SetBinContent(i + 1, mergedEntries[i].numActive[slot] / 512.0);
        }
      }

      for (unsigned slot = 0; slot < c_numModules; slot++) {
        string name = "numActiveCalibrated_" + slotNames[slot];
        string title = "Active and calibrated, " + slotNames[slot] + "; run index; fraction";
        auto* h = new TH1F(name.c_str(), title.c_str(), nx, 0, nx);
        for (int i = 0; i < nx; i++) {
          h->SetBinContent(i + 1, mergedEntries[i].numActiveCalibrated[slot] / 512.0);
        }
      }

      for (unsigned slot = 0; slot < c_numModules; slot++) {
        string name = "thrEffi_" + slotNames[slot];
        string title = "Threshold efficiency, " + slotNames[slot] + "; run index; efficiency";
        auto* h = new TH1F(name.c_str(), title.c_str(), nx, 0, nx);
        for (int i = 0; i < nx; i++) {
          h->SetBinContent(i + 1, mergedEntries[i].thrEffi[slot]);
        }
      }

      for (unsigned carrier = 0; carrier < 4; carrier++) {
        string name = "asicShifts_" + to_string(carrier);
        string title = "BS13d, carrier " + to_string(carrier) + "; run index; shift [ns]";
        auto* h = new TH1F(name.c_str(), title.c_str(), nx, 0, nx);
        for (int i = 0; i < nx; i++) {
          auto y = mergedEntries[i].asicShifts[carrier];
          if (isnan(y)) {
            h->SetBinError(i + 1, 0);
          } else {
            h->SetBinContent(i + 1, y);
            h->SetBinError(i + 1, 0.001);
          }
        }
      }

      auto* h_svdOffset = new TH1F("svdOffset", "Event T0 offset w.r.t TOP; run index; offset [ns]", nx, 0, nx);
      auto* h_svdSigma = new TH1F("svdSigma", "Event T0 resolution; run index; sigma [ns]", nx, 0, nx);
      for (int i = 0; i < nx; i++) {
        h_svdOffset->SetBinContent(i + 1, mergedEntries[i].svdOffset);
        h_svdOffset->SetBinError(i + 1, mergedEntries[i].svdSigma / 1000);
        h_svdSigma->SetBinContent(i + 1, mergedEntries[i].svdSigma);
        h_svdSigma->SetBinError(i + 1, 0.001);
      }

      auto* h_cdcOffset = new TH1F("cdcOffset", "Event T0 offset w.r.t TOP; run index; offset [ns]", nx, 0, nx);
      auto* h_cdcSigma = new TH1F("cdcSigma", "Event T0 resolution; run index; sigma [ns]", nx, 0, nx);
      for (int i = 0; i < nx; i++) {
        h_cdcOffset->SetBinContent(i + 1, mergedEntries[i].cdcOffset);
        h_cdcOffset->SetBinError(i + 1, mergedEntries[i].cdcSigma / 1000);
        h_cdcSigma->SetBinContent(i + 1, mergedEntries[i].cdcSigma);
        h_cdcSigma->SetBinError(i + 1, 0.001);
      }

      auto* h_fillPatternOffset = new TH1F("fillPatternOffset", "Fill pattern offset; run index; offset [RF clk]", nx, 0, nx);
      auto* h_fillPatternFract = new TH1F("fillPatternFract",
                                          "Fraction of buckets matched with filled ones; run index; fraction", nx, 0, nx);
      for (int i = 0; i < nx; i++) {
        if (isnan(mergedEntries[i].fillPatternOffset)) {
          h_fillPatternOffset->SetBinError(i + 1, 0);
        } else {
          h_fillPatternOffset->SetBinContent(i + 1, mergedEntries[i].fillPatternOffset);
          h_fillPatternOffset->SetBinError(i + 1, 0.01);
        }
        h_fillPatternFract->SetBinContent(i + 1, mergedEntries[i].fillPatternFraction);
      }

      // write-out the results

      B2INFO("TOPValidationAlgorithm: write the results");

      file->Write();
      file->Close();

      return c_OK;
    }

  } // end namespace TOP
} // end namespace Belle2
