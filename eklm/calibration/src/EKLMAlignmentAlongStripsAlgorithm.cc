/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TFile.h>
#include <TTree.h>

/* Belle2 headers. */
#include <eklm/calibration/EKLMAlignmentAlongStripsAlgorithm.h>

using namespace Belle2;

EKLMAlignmentAlongStripsAlgorithm::EKLMAlignmentAlongStripsAlgorithm() :
  CalibrationAlgorithm("EKLMAlignmentAlongStripsCollector")
{
}

EKLMAlignmentAlongStripsAlgorithm::~EKLMAlignmentAlongStripsAlgorithm()
{
}

CalibrationAlgorithm::EResult EKLMAlignmentAlongStripsAlgorithm::calibrate()
{
  return CalibrationAlgorithm::c_OK;
}

void EKLMAlignmentAlongStripsAlgorithm::dumpData(const char* fname)
{
  int i, n;
  struct Event* event = NULL;
  TFile* f_out;
  TTree* t_in, *t_out;
  t_in = &getObject<TTree>("calibration_data");
  getObject<TTree>("calibration_data").Print();
  t_in->Print();
  t_in->SetBranchAddress("event", &event);
  f_out = new TFile(fname, "recreate");
  t_out = new TTree("tree", "");
  t_out->Branch("event", event);
  n = t_in->GetEntries();
  for (i = 0; i < n; i++) {
    t_in->GetEntry(i);
    t_out->Fill();
  }
  f_out->cd();
  t_out->Write();
  delete t_out;
  delete f_out;
}

