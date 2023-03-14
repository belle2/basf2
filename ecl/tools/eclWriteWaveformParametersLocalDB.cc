/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dbobjects/ECLDigitWaveformParameters.h>

/* Basf2 headers. */
#include <framework/database/DBImportObjPtr.h>
#include <framework/logging/Logger.h>

/* ROOT headers. */
#include <TFile.h>
#include <TTree.h>

/* C++ headers. */
#include <iostream>

using namespace Belle2;

//
//See  eclComputePulseTemplates_Step0.cc for README instructions.
///
int main()
{
  //
  TString FileName = "DigitWaveformParameters.root";
  //
  TFile* WaveformParameterFileInput = new TFile(FileName, "READ");
  if (!WaveformParameterFileInput or WaveformParameterFileInput->IsZombie()) {
    B2FATAL("Could not open file " << FileName);
  }
  WaveformParameterFileInput->cd();
  TTree* WaveformParametersTree = (TTree*) WaveformParameterFileInput->Get("ParTree");
  //
  double treePhotonPar11[11];
  double treeHadronPar11[11];
  double treeDiodePar11[11];
  //
  WaveformParametersTree->SetBranchAddress("PhotonPar", &treePhotonPar11);
  WaveformParametersTree->SetBranchAddress("HadronPar", &treeHadronPar11);
  WaveformParametersTree->SetBranchAddress("DiodePar", &treeDiodePar11);
  //

  //
  Belle2::DBImportObjPtr<Belle2::ECLDigitWaveformParameters> importer("ECLDigitWaveformParameters");
  importer.construct();
  for (int ID = 0; ID < ECLElementNumbers::c_NCrystals; ID++) {
    WaveformParametersTree->GetEntry(ID);
    float tempPhotonPar11[11];
    float tempHadronPar11[11];
    float tempDiodePar11[11];
    for (unsigned int k = 0; k < 11; k++) {
      tempPhotonPar11[k] = (float)treePhotonPar11[k];
      tempHadronPar11[k] = (float)treeHadronPar11[k];
      tempDiodePar11[k] = (float)treeDiodePar11[k];
    }
    std::cout << "cellID: " << ID + 1 << " " << tempPhotonPar11[0] << " " << tempHadronPar11[0] << " " << tempDiodePar11[0] <<
              std::endl;
    //
    if (tempPhotonPar11[0] == 0 || tempHadronPar11[0] == 0
        || tempDiodePar11[0] == 0)  std::cout << "WARNING CELLID MISSING: " << ID + 1 << std::endl;
    //
    importer->setTemplateParameters(ID + 1, tempPhotonPar11, tempHadronPar11, tempDiodePar11);
  }
  importer.import(Belle2::IntervalOfValidity::always());
  std::cout << "Successfully wrote payload ECLDigitWaveformParameters" << std::endl;
  //
}
