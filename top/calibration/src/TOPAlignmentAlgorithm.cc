/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/calibration/TOPAlignmentAlgorithm.h>
#include <top/dbobjects/TOPCalModuleAlignment.h>

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>

#include <string>
#include <algorithm>
#include <set>

using namespace std;

namespace Belle2 {
  namespace TOP {

    TOPAlignmentAlgorithm::TOPAlignmentAlgorithm():
      CalibrationAlgorithm("TOPAlignmentCollector")
    {
      setDescription("Calibration algorithm for geometrcal alignment of TOP module");
    }

    CalibrationAlgorithm::EResult TOPAlignmentAlgorithm::calibrate()
    {

      // get basic histogram

      auto h1 = getObjectPtr<TH2F>("tracks_per_slot");
      if (not h1) {
        B2ERROR("histogram 'tracks_per_slot' not found");
        return c_Failure;
      }
      unsigned numModules = h1->GetNbinsX();
      unsigned numSets = h1->GetNbinsY();

      // clear multimap which will hold alignment data from last iterations

      m_inputData.clear();

      // get alignment data of last iterations

      for (unsigned set = 0; set < numSets; set++) {

        // get ntuple of a given subsample

        std::string name = "alignTree" + to_string(set);
        auto alignTree = getObjectPtr<TTree>(name);
        if (not alignTree) {
          B2ERROR("TOPAlignmentAlgorithm: 'alignTree' not found");
          return c_Failure;
        }

        alignTree->SetBranchAddress("ModuleId", &m_moduleID);
        alignTree->SetBranchAddress("iter", &m_iter);
        alignTree->SetBranchAddress("ntrk", &m_ntrk);
        alignTree->SetBranchAddress("errorCode", &m_errorCode);
        alignTree->SetBranchAddress("iterPars", &m_vAlignPars, &m_bAlignPars);
        alignTree->SetBranchAddress("iterParsErr", &m_vAlignParsErr, &m_bAlignParsErr);
        alignTree->SetBranchAddress("valid", &m_valid);

        // get last iteration for each module

        std::map<int, int> lastIterations;
        std::map<int, int> lastIterationEntries;
        for (int i = 0; i < alignTree->GetEntries(); i++) {
          alignTree->GetEntry(i);
          if (m_iter > lastIterations[m_moduleID]) {
            lastIterations[m_moduleID] = m_iter;
            lastIterationEntries[m_moduleID] = i;
          }
        }

        // store last iteration in a multimap

        for (const auto& lastIterationEntry : lastIterationEntries) {
          alignTree->GetEntry(lastIterationEntry.second);
          if (m_vAlignPars->size() != m_vAlignParsErr->size()) {
            B2ERROR("slot " << m_moduleID << ", set=" << set <<
                    ": sizes of vectors of alignment parameters and errors differ. "
                    "Entry ignored.");
            continue;
          }
          AlignData data;
          data.iter = m_iter;
          data.ntrk = m_ntrk;
          data.alignPars = *m_vAlignPars;
          data.alignErrs = *m_vAlignParsErr;
          data.valid = m_valid;
          m_inputData.emplace(m_moduleID, data);
        }

      }

      // construct file name, open output root file

      const auto& expRun = getRunList();
      string expNo = to_string(expRun[0].first);
      while (expNo.length() < 4) expNo.insert(0, "0");
      string runNo = to_string(expRun[0].second);
      while (runNo.length() < 5) runNo.insert(0, "0");
      string outputFileName = "alignment-e" + expNo + "-r" + runNo + ".root";
      auto* file = TFile::Open(outputFileName.c_str(), "recreate");

      // write control histograms

      h1->Write();
      TDirectory* oldDir = gDirectory;
      for (unsigned slot = 1; slot <= numModules; slot++) {
        std::string slotName = "_s" + to_string(slot);
        auto h2 = getObjectPtr<TH1F>("local_z" + slotName);
        auto h3 = getObjectPtr<TH2F>("cth_vs_p" + slotName);
        auto h4 = getObjectPtr<TH2F>("poca_xy" + slotName);
        auto h5 = getObjectPtr<TH1F>("poca_z" + slotName);
        auto h6 = getObjectPtr<TH1F>("Ecms" + slotName);
        auto h7 = getObjectPtr<TH1F>("charge" + slotName);
        auto h8 = getObjectPtr<TH2F>("timeHits" + slotName);
        auto h9 = getObjectPtr<TH1F>("numPhot" + slotName);

        std::string name = "slot_" + to_string(slot);
        oldDir->mkdir(name.c_str())->cd();
        if (h2) h2->Write();
        if (h3) h3->Write();
        if (h4) h4->Write();
        if (h5) h5->Write();
        if (h6) h6->Write();
        if (h7) h7->Write();
        if (h8) h8->Write();
        if (h9) h9->Write();
        oldDir->cd();
      }

      // book output histograms

      auto h_valid = new TH1F("valid", "Valid results",
                              numModules, 0.5, numModules + 0.5);
      h_valid->SetXTitle("slot number");
      h_valid->SetYTitle("valid flag");

      auto h_iter = new TH1F("iterations", "Number of iterations",
                             numModules, 0.5, numModules + 0.5);
      h_iter->SetXTitle("slot number");
      h_iter->SetYTitle("iterations");

      auto h_ntrk = new TH1F("ntrk", "Number of tracks",
                             numModules, 0.5, numModules + 0.5);
      h_ntrk->SetXTitle("slot number");
      h_ntrk->SetYTitle("tracks");

      std::vector<TH1F*> h_params;

      auto h_x = new TH1F("delta_x", "Displacement in x",
                          numModules, 0.5, numModules + 0.5);
      h_x->SetXTitle("slot number");
      h_x->SetYTitle("#Deltax [cm]");
      h_params.push_back(h_x);

      auto h_y = new TH1F("delta_y", "Displacement in y",
                          numModules, 0.5, numModules + 0.5);
      h_y->SetXTitle("slot number");
      h_y->SetYTitle("#Deltay [cm]");
      h_params.push_back(h_y);

      auto h_z = new TH1F("delta_z", "Displacement in z",
                          numModules, 0.5, numModules + 0.5);
      h_z->SetXTitle("slot number");
      h_z->SetYTitle("#Deltaz [cm]");
      h_params.push_back(h_z);

      auto h_alpha = new TH1F("alpha", "Rotation angle around x",
                              numModules, 0.5, numModules + 0.5);
      h_alpha->SetXTitle("slot number");
      h_alpha->SetYTitle("#alpha [rad]");
      h_params.push_back(h_alpha);

      auto h_beta = new TH1F("beta", "Rotation angle around y",
                             numModules, 0.5, numModules + 0.5);
      h_beta->SetXTitle("slot number");
      h_beta->SetYTitle("#beta [rad]");
      h_params.push_back(h_beta);

      auto h_gamma = new TH1F("gamma", "Rotation angle around z",
                              numModules, 0.5, numModules + 0.5);
      h_gamma->SetXTitle("slot number");
      h_gamma->SetYTitle("#gamma [rad]");
      h_params.push_back(h_gamma);

      auto h_t0 = new TH1F("t0", "Module T0",
                           numModules, 0.5, numModules + 0.5);
      h_t0->SetXTitle("slot number");
      h_t0->SetYTitle("t_{0} [ns]");
      h_params.push_back(h_t0);

      auto h_refind = new TH1F("dn_n", "Refractive index scale factor",
                               numModules, 0.5, numModules + 0.5);
      h_refind->SetXTitle("slot number");
      h_refind->SetYTitle("#Deltan/n");
      h_params.push_back(h_refind);

      // create DB object

      auto* alignment = new TOPCalModuleAlignment();

      // merge subsamples

      mergeData();

      // store merged data in histograms and in DB object

      for (const auto& element : m_mergedData) {
        int moduleID = element.first;
        const auto& data = element.second;
        h_valid->SetBinContent(moduleID, data.valid);
        h_iter->SetBinContent(moduleID, data.iter);
        h_ntrk->SetBinContent(moduleID, data.ntrk);
        auto vsize = std::min(data.alignPars.size(), data.alignErrs.size());
        for (unsigned i = 0; i < std::min(h_params.size(), vsize); i++) {
          h_params[i]->SetBinContent(moduleID, data.alignPars[i]);
          h_params[i]->SetBinError(moduleID, data.alignErrs[i]);
        }
        if (vsize < 6) {
          B2ERROR("slot " << moduleID <<
                  ": too few alignment parameters found in ntuple, npar = " << vsize);
          continue;
        }
        alignment->setX(moduleID, data.alignPars[0], data.alignErrs[0]);
        alignment->setY(moduleID, data.alignPars[1], data.alignErrs[1]);
        alignment->setZ(moduleID, data.alignPars[2], data.alignErrs[2]);
        alignment->setAlpha(moduleID, data.alignPars[3], data.alignErrs[3]);
        alignment->setBeta(moduleID, data.alignPars[4], data.alignErrs[4]);
        alignment->setGamma(moduleID, data.alignPars[5], data.alignErrs[5]);
        if (data.valid) {
          alignment->setCalibrated(moduleID);
        } else {
          alignment->setUnusable(moduleID);
        }
      }

      // write the results and close the file

      file->Write();
      file->Close();

      // check the results and return if alignment precision is not satisfied

      if (not alignment->areAllCalibrated()) {
        B2INFO("Alignment not successful for all slots");
        delete alignment;
        return c_NotEnoughData;
      }
      if (not alignment->areAllPrecise(m_spatialPrecision, m_angularPrecision)) {
        B2INFO("Alignment successful but precision worse than required");
        delete alignment;
        return c_NotEnoughData;
      }

      // otherwise import calibration to DB

      saveCalibration(alignment);

      return c_OK;
    }


    void TOPAlignmentAlgorithm::mergeData()
    {
      // clear the map

      m_mergedData.clear();

      // make a list of module ID's

      std::set<int> slotIDs;
      for (const auto& element : m_inputData) {
        slotIDs.insert(element.first);
      }

      // book a histogram of pulls

      auto h_pulls = new TH1F("pulls", "Pulls of statistically independent results",
                              200, -20.0, 20.0);
      h_pulls->SetXTitle("pulls");

      // fill pulls

      for (auto slot : slotIDs) {
        const auto range = m_inputData.equal_range(slot);
        std::vector<AlignData> data;
        for (auto it = range.first; it != range.second; ++it) {
          data.push_back(it->second);
        }
        for (size_t i = 0; i < data.size(); i++) {
          const auto& pars1 = data[i].alignPars;
          const auto& errs1 = data[i].alignErrs;
          for (size_t j = i + 1; j < data.size(); j++) {
            const auto& pars2 = data[j].alignPars;
            const auto& errs2 = data[j].alignErrs;
            for (size_t k = 0; k < std::min(pars1.size(), pars2.size()); k++) {
              double e1 = errs1[k];
              double e2 = errs2[k];
              if (e1 == 0 or e2 == 0) continue;
              double pull = (pars1[k] - pars2[k]) / sqrt(e1 * e1 + e2 * e2);
              h_pulls->Fill(pull);
            }
          }
        }
      }

      // determine scale factor for errors

      double scaleFact = 1;
      if (h_pulls->GetEntries() > 1) scaleFact = h_pulls->GetRMS();

      // merge data

      for (auto slot : slotIDs) {
        const auto range = m_inputData.equal_range(slot);
        if (range.first == range.second) continue;
        AlignData mergedData;
        for (auto it = range.first; it != range.second; ++it) {
          mergedData.add(it->second);
        }
        mergedData.finalize(scaleFact);
        m_mergedData[slot] = mergedData;
      }

    }


    void TOPAlignmentAlgorithm::AlignData::add(const AlignData& data)
    {
      iter = std::max(iter, data.iter);
      ntrk += data.ntrk;
      if (not data.valid) return;

      for (size_t i = alignPars.size(); i < data.alignPars.size(); i++) {
        alignPars.push_back(0);
        alignErrs.push_back(0);
      }

      for (size_t i = 0; i < data.alignPars.size(); i++) {
        auto e = data.alignErrs[i];
        if (e == 0) continue;
        auto wt = 1 / (e * e);
        alignPars[i] += data.alignPars[i] * wt ;
        alignErrs[i] += wt ;
      }
      valid = true;
    }


    void TOPAlignmentAlgorithm::AlignData::finalize(double scaleFact)
    {
      for (size_t i = 0; i < alignPars.size(); i++) {
        auto wt = alignErrs[i];
        if (wt == 0) continue;
        alignPars[i] /= wt;
        alignErrs[i] = 1 / sqrt(wt) * scaleFact;
      }
    }


  } // end namespace TOP
} // end namespace Belle2
