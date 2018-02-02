#include <framework/database/DBImportObjPtr.h>
#include <ecl/dbobjects/ECLDigitWaveformParameters.h>
#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TTree.h>

int main()
{
  TString FileName = "/home/belle2/longos/TestStandard/workdir/ComputeHadronShapes/WaveformParametersALE.root";
  TFile* WaveformParameterFileInput = new TFile(FileName, "READ");
  if (!WaveformParameterFileInput or WaveformParameterFileInput->IsZombie()) {
    std::cout << "Could not open file " << FileName << std::endl;;
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
  Belle2::DBImportObjPtr<Belle2::ECLDigitWaveformParameters> importer("ECLDigitWaveformParameters");
  importer.construct();
  for (int ID = 0; ID <= 8736; ID++) {
    WaveformParametersTree->GetEntry(ID);
    std::vector<double> tempPhotonPar11(11);
    std::vector<double> tempHadronPar11(11);
    std::vector<double> tempDiodePar11(11);
    for (unsigned int k = 0; k < tempPhotonPar11.size(); k++) {
      tempPhotonPar11[k] = treePhotonPar11[k];
      tempHadronPar11[k] = treeHadronPar11[k];
      tempDiodePar11[k] = treeDiodePar11[k];
    }
    std::cout << "cellID: " << ID + 1 << " " << tempPhotonPar11[0] << " " << tempHadronPar11[0] << " " << tempDiodePar11[0] <<
              std::endl;
    importer->setTemplateParameters(ID + 1, tempPhotonPar11, tempHadronPar11, tempDiodePar11);
  }
  importer.import(Belle2::IntervalOfValidity::always());
  std::cout << "Successfully wrote payload ECLDigitWaveformParameters" << std::endl;
  //
}
