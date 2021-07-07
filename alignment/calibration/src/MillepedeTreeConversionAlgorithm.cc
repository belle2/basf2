/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/calibration/MillepedeTreeConversionAlgorithm.h>

#include <genfit/GblData.h>

#include <TFile.h>

using namespace Belle2;

MillepedeTreeConversionAlgorithm::MillepedeTreeConversionAlgorithm() :
  CalibrationAlgorithm("MillepedeCollector"), m_OutputFile("millepede_data.root") {}

MillepedeTreeConversionAlgorithm::~MillepedeTreeConversionAlgorithm()
{
}

void MillepedeTreeConversionAlgorithm::setOutputFile(const char* outputFile)
{
  m_OutputFile = outputFile;
}

CalibrationAlgorithm::EResult MillepedeTreeConversionAlgorithm::calibrate()
{
  const int max_entries = 100;
  int i, j, n;
  std::vector<gbl::GblData>* dat = NULL;
  std::vector<gbl::GblData>::iterator it;
  double aValue, aErr;
  std::vector<unsigned int>* indLocal;
  std::vector<int>* labGlobal;
  std::vector<double>* derLocal, *derGlobal;
  float value, error, der[max_entries];
  int nlab, label[max_entries];
  auto gblData = getObjectPtr<TTree>("GblDataTree");
  gblData->SetBranchAddress("GblData", &dat);
  TFile* f_out = new TFile(m_OutputFile.c_str(), "recreate");
  TTree* t_out = new TTree("mille", "");
  t_out->Branch("value", &value, "value/F");
  t_out->Branch("error", &error, "error/F");
  t_out->Branch("nlab", &nlab, "nlab/I");
  t_out->Branch("label", label, "label[nlab]/I");
  t_out->Branch("der", der, "der[nlab]/F");
  n = gblData->GetEntries();
  for (i = 0; i < n; i++) {
    gblData->GetEntry(i);
    for (it = dat->begin(); it != dat->end(); ++it) {
      it->getAllData(aValue, aErr, indLocal, derLocal, labGlobal, derGlobal);
      if (labGlobal->size() == 0)
        continue;
      value = aValue;
      error = aErr;
      nlab = std::min((int)labGlobal->size(), max_entries);
      for (j = 0; j < nlab; j++) {
        label[j] = (*labGlobal)[j];
        der[j] = (*derGlobal)[j];
      }
      t_out->Fill();
    }
  }
  f_out->cd();
  t_out->Write();
  delete t_out;
  delete f_out;
  return CalibrationAlgorithm::c_OK;
}

