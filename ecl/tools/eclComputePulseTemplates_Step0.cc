#include<iostream>
#include<fstream>
#include<stdlib.h>
#include<string>
#include"TTree.h"
#include"TString.h"
#include"TFile.h"
#include"TGraph.h"
#include"TF1.h"

using namespace std;

/*

hadron and diode code by Savino Longo (longos@uvic.ca)

Sample scripts to compute hadron and diode templates.

Note photon shape calibrations are needed to compute hadron and diode templates.
Photon calibrations should be placed in file named params_gamma_shape.dat in same directiory as this script.
Format of params_gamma_shape.dat:

cellID PhotonScale 0 p0 p1 p2 p3 p4 p5 p6 p7 p8 p9

where p0-p9 are 10 ShaperDSP shaper parameters for basf2 C++ implementation of ShaperDSP function.

eclComputePulseTemplates_Step0.cc - places photon parameters in ntuple
eclComputePulseTemplates_Step1.cc - produce photon array input for eclComputePulseTemplates_Step2.py
eclComputePulseTemplates_Step2.py - produce hadron and diode array input for eclComputePulseTemplates_Step3.py
eclComputePulseTemplates_Step3.cc - computed hadron and diode shape parameters and organizes into temp ntuple
eclComputePulseTemplates_Step4.cc - organizes photon, hadron and diode templates into final ntuple to produce dbobject.

To run calibration :

execute eclComputePulseTemplates_Step0.cc
for i=0 i<873 i++
  execute eclComputePulseTemplates_Step1.cc i*10 (i+1)*10
  basf2 eclComputePulseTemplates_Step2.py i*10 (i+1)*10
  execute eclComputePulseTemplates_Step3.cc i*10 (i+1)*10
execute eclComputePulseTemplates_Step4.py
//
*/
//

struct crystalInfo {
  vector<double> PhotonWaveformPars;
};

int main()
{
  //
  TString ParameterTreeOutputName = "PhotonWaveformParameters.root";
  TFile* ParameterTreeOutputFile = new TFile(ParameterTreeOutputName, "RECREATE");

  TTree* ParameterTree = new TTree("ParTree", "");
  double PhotonWaveformPar[11];
  for (int k = 0; k < 11; k++)  PhotonWaveformPar[k] = 0;
  //
  ParameterTree->Branch("PhotonPar", &PhotonWaveformPar, "PhotonWaveformPar[11]/D");
  //
  vector<crystalInfo> cellIDcheck(8736);
  //
  ifstream PhotonFile("/home/belle2/longos/WaveformFitting/ecl/tools/params_gamma_shape.dat");
  if (PhotonFile.is_open()) {
    vector<double> templine(12);
    for (int k = 0; k < 8736; k++) {
      for (unsigned int j = 0; j < templine.size(); j++)  PhotonFile >> templine[j];
      cellIDcheck[int(templine[0]) - 1].PhotonWaveformPars.resize(11);
      cellIDcheck[int(templine[0]) - 1].PhotonWaveformPars[0] = templine[1];
      for (int j = 0; j < 10; j++)  cellIDcheck[int(templine[0]) - 1].PhotonWaveformPars[j + 1] = templine[j + 2];
      std::cout << int(templine[0]) << " " << templine[1] << endl;
    }
    PhotonFile.close();
  }
  //
  for (unsigned int f = 0; f < cellIDcheck.size(); f++) {
    for (int k = 0; k < 11; k++)  PhotonWaveformPar[k] = cellIDcheck[f].PhotonWaveformPars[k];
    ParameterTree->Fill();
  }
  ParameterTreeOutputFile->cd();
  ParameterTree->Write();
  ParameterTreeOutputFile->Write();
  ParameterTreeOutputFile->Close();
  //
  return 0;
}
