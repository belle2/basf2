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

#include <top/calibration/TOPAlignmentAlgorithm.h>
#include <top/dbobjects/TOPCalModuleAlignment.h>

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>

#include <string>
#include <map>
#include <algorithm>


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

      // get ntuple

      auto alignTree = getObjectPtr<TTree>("alignTree");
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

      // construct file name, open output root file

      const auto& expRun = getRunList();
      string expNo = to_string(expRun[0].first);
      while (expNo.length() < 4) expNo.insert(0, "0");
      string runNo = to_string(expRun[0].second);
      while (runNo.length() < 5) runNo.insert(0, "0");
      string outputFileName = "alignment-e" + expNo + "-r" + runNo + ".root";
      auto* file = TFile::Open(outputFileName.c_str(), "recreate");

      // book output histograms

      auto h_valid = new TH1F("valid", "Valid results", 16, 0.5, 16.5);
      h_valid->SetXTitle("slot number");
      h_valid->SetYTitle("valid flag");

      auto h_iter = new TH1F("iterations", "Number of iterations", 16, 0.5, 16.5);
      h_iter->SetXTitle("slot number");
      h_iter->SetYTitle("iterations");

      auto h_ntrk = new TH1F("ntrk", "Number of tracks", 16, 0.5, 16.5);
      h_ntrk->SetXTitle("slot number");
      h_ntrk->SetYTitle("tracks");

      std::vector<TH1F*> h_params;

      auto h_x = new TH1F("delta_x", "Displacement in x", 16, 0.5, 16.5);
      h_x->SetXTitle("slot number");
      h_x->SetYTitle("#Delta x [cm]");
      h_params.push_back(h_x);

      auto h_y = new TH1F("delta_y", "Displacement in y", 16, 0.5, 16.5);
      h_y->SetXTitle("slot number");
      h_y->SetYTitle("#Delta y [cm]");
      h_params.push_back(h_y);

      auto h_z = new TH1F("delta_z", "Displacement in z", 16, 0.5, 16.5);
      h_z->SetXTitle("slot number");
      h_z->SetYTitle("#Delta z [cm]");
      h_params.push_back(h_z);

      auto h_alpha = new TH1F("alpha", "Rotation angle around x", 16, 0.5, 16.5);
      h_alpha->SetXTitle("slot number");
      h_alpha->SetYTitle("#alpha [rad]");
      h_params.push_back(h_alpha);

      auto h_beta = new TH1F("beta", "Rotation angle around y", 16, 0.5, 16.5);
      h_beta->SetXTitle("slot number");
      h_beta->SetYTitle("#beta [rad]");
      h_params.push_back(h_beta);

      auto h_gamma = new TH1F("gamma", "Rotation angle around z", 16, 0.5, 16.5);
      h_gamma->SetXTitle("slot number");
      h_gamma->SetYTitle("#gamma [rad]");
      h_params.push_back(h_gamma);

      auto h_t0 = new TH1F("t0", "Module T0", 16, 0.5, 16.5);
      h_t0->SetXTitle("slot number");
      h_t0->SetYTitle("t_0 [ns]");
      h_params.push_back(h_t0);

      auto h_refind = new TH1F("dn_n", "Refractive index scale factor", 16, 0.5, 16.5);
      h_refind->SetXTitle("slot number");
      h_refind->SetYTitle("#Delta n/n");
      h_params.push_back(h_refind);

      // book output ntuple

      auto tree = new TTree("tree", "TOP alignment results (last iteration)");
      tree->Branch("ModuleId", &m_moduleID);
      tree->Branch("iter", &m_iter);
      tree->Branch("ntrk", &m_ntrk);
      tree->Branch("errorCode", &m_errorCode);
      tree->Branch("iterPars", &m_vAlignPars);
      tree->Branch("iterParsErr", &m_vAlignParsErr);
      tree->Branch("valid", &m_valid);

      // create DB object

      auto* alignment = new TOPCalModuleAlignment();

      // get last iteration for each module

      std::map<int, int> lastIterations;
      for (int i = 0; i < alignTree->GetEntries(); i++) {
        alignTree->GetEntry(i);
        lastIterations[m_moduleID] = i;
      }

      // store last iteration in histograms and in DB object

      for (const auto& lastIteration : lastIterations) {
        alignTree->GetEntry(lastIteration.second);
        tree->Fill();
        h_valid->SetBinContent(m_moduleID, m_valid);
        h_iter->SetBinContent(m_moduleID, m_iter);
        h_ntrk->SetBinContent(m_moduleID, m_ntrk);
        if (m_vAlignPars->size() != m_vAlignParsErr->size()) {
          B2ERROR("slot " << m_moduleID <<
                  ": sizes of vectors of alignment parameters and errors differ");
        }
        auto vsize = std::min(m_vAlignPars->size(), m_vAlignParsErr->size());
        for (unsigned i = 0; i < std::min(h_params.size(), vsize); i++) {
          h_params[i]->SetBinContent(m_moduleID, m_vAlignPars->at(i));
          h_params[i]->SetBinError(m_moduleID, m_vAlignParsErr->at(i));
        }
        if (vsize < 6) {
          B2ERROR("slot " << m_moduleID <<
                  "too few alignment parameters found in ntuple");
          continue;
        }
        alignment->setX(m_moduleID, m_vAlignPars->at(0), m_vAlignParsErr->at(0));
        alignment->setY(m_moduleID, m_vAlignPars->at(1), m_vAlignParsErr->at(1));
        alignment->setZ(m_moduleID, m_vAlignPars->at(2), m_vAlignParsErr->at(2));
        alignment->setAlpha(m_moduleID, m_vAlignPars->at(3), m_vAlignParsErr->at(3));
        alignment->setBeta(m_moduleID, m_vAlignPars->at(4), m_vAlignParsErr->at(4));
        alignment->setGamma(m_moduleID, m_vAlignPars->at(5), m_vAlignParsErr->at(5));
        if (m_valid) {
          alignment->setCalibrated(m_moduleID);
        } else {
          alignment->setUnusable(m_moduleID);
        }
      }

      // write the results and close the file

      file->Write();
      file->Close();

      // check the results and return if alignment is not precise

      if (not alignment->areAllCalibrated() or
          not alignment->areAllPrecise(m_spatialPrecision, m_angularPrecision)) {
        delete alignment;
        return c_NotEnoughData;
      }

      // otherwise import calibration to DB

      saveCalibration(alignment);

      return c_OK;
    }

  } // end namespace TOP
} // end namespace Belle2
