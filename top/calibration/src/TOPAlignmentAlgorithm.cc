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

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>

#include <string>

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
      alignTree->SetBranchAddress("iterPars", &m_vAlignPars);
      alignTree->SetBranchAddress("iterParsErr", &m_vAlignParsErr);
      alignTree->SetBranchAddress("valid", &m_valid);

      // construct file name, open output root file and book output histograms

      const auto& expRun = getRunList();
      string expNo = to_string(expRun[0].first);
      while (expNo.length() < 4) expNo.insert(0, "0");
      string runNo = to_string(expRun[0].second);
      while (runNo.length() < 5) runNo.insert(0, "0");
      string outputFileName = "alignment-e" + expNo + "-r" + runNo + ".root";
      auto* file = TFile::Open(outputFileName.c_str(), "recreate");

      auto h_iter = new TH1F("iterations", "Number of iterations", 16, 0.5, 16.5);
      h_iter->SetXTitle("slot number");
      h_iter->SetYTitle("iterations");

      auto h_ntrk = new TH1F("ntrk", "Number of tracks", 16, 0.5, 16.5);
      h_ntrk->SetXTitle("slot number");
      h_ntrk->SetYTitle("tracks");

      auto h_valid = new TH1F("valid", "Valid result", 16, 0.5, 16.5);
      h_valid->SetXTitle("slot number");
      h_valid->SetYTitle("valid flag");




      // get last iteration for each module

      std::vector<int> lastEntries(16, -1);
      for (int i = 0; i < alignTree->GetEntries(); i++) {
        alignTree->GetEntry(i);
        lastEntries[m_moduleID - 1] = i;
      }



      return c_OK;
    }

  } // end namespace TOP
} // end namespace Belle2
