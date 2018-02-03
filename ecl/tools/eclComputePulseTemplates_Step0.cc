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

Sample scripts to compute hadron and diode templates by Savino Longo (longos@uvic.ca)

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

To run template calculation:

First "OutputDirectory" variable in eclComputePulseTemplates_Stepx.cc files should be modified to a temp directory before running scripts.
recompile with "scons ecl"

1) execute eclComputePulseTemplates_Step0 0
2) for i=0 i<873 i++
3)   execute eclComputePulseTemplates_Step1 i*10 (i+1)*10
4)   basf2 eclComputePulseTemplates_Step2.py i*10 (i+1)*10
5)   execute eclComputePulseTemplates_Step3 i*10 (i+1)*10
6) execute eclComputePulseTemplates_Step0 1
7) execute eclWriteWaveformParametersLocalDB
Final step will create local db object.


//
*/
//

struct crystalInfo {
  vector<double> PhotonWaveformPars;
  vector<double> HadronWaveformPars;
  vector<double> DiodeWaveformPars;
  double MaxResHadron;
  double MaxValHadron;
  double MaxResDiode;
  double MaxValDiode;
};

int main(int argc, char* argv[])
{
  //
  TString OutputDirectory = "";
  if (OutputDirectory == "") {
    std::cout << "Error set ouput directory" << std::endl;
    return -1;
  }
  //TString OutputDirectory = "/group/belle2/users/longos/WaveformShapesPars/";
  //
  int Flag = atoi(argv[1]);
  TString InputDirectory = OutputDirectory;
  //
  TString ParameterTreeOutputName = OutputDirectory + "PhotonWaveformParameters.root";
  if (Flag == 1) ParameterTreeOutputName = "DigitWaveformParameters.root";
  TFile* ParameterTreeOutputFile = new TFile(ParameterTreeOutputName, "RECREATE");
  //
  TTree* ParameterTree = new TTree("ParTree", "");
  double PhotonWaveformPar[11];
  double HadronWaveformPar[11];
  double DiodeWaveformPar[11];
  for (int k = 0; k < 11; k++) {
    PhotonWaveformPar[k] = 0;
    HadronWaveformPar[k] = 0;
    DiodeWaveformPar[k] = 0;
  }
  double mHadronRes = 0;
  double mDiodeRes = 0;
  double mHadronMax = 0;
  double mDiodeMax = 0;
  ParameterTree->Branch("PhotonPar", &PhotonWaveformPar, "PhotonWaveformPar[11]/D");
  ParameterTree->Branch("HadronPar", &HadronWaveformPar, "HadronWaveformPar[11]/D");
  ParameterTree->Branch("DiodePar", &DiodeWaveformPar, "DiodeWaveformPar[11]/D");
  ParameterTree->Branch("mHadronRes", &mHadronRes, "mHadronRes/D");
  ParameterTree->Branch("mDiodeRes", &mDiodeRes, "mDiodeRes/D");
  ParameterTree->Branch("mHadronMax", &mHadronMax, "mHadronMax/D");
  ParameterTree->Branch("mDiodeMax", &mDiodeMax, "mDiodeMax/D");
  //
  std::vector<crystalInfo> cellIDcheck(8736);
  ifstream PhotonFile("/home/belle2/longos/WaveformFitting/ecl/tools/params_gamma_shape.dat");
  if (PhotonFile.is_open()) {
    std::vector<double> templine(12);
    for (int k = 0; k < 8736; k++) {
      for (unsigned int j = 0; j < templine.size(); j++)  PhotonFile >> templine[j];
      cellIDcheck[int(templine[0]) - 1].PhotonWaveformPars.resize(11);
      cellIDcheck[int(templine[0]) - 1].PhotonWaveformPars[0] = templine[1];
      for (int j = 0; j < 10; j++)  cellIDcheck[int(templine[0]) - 1].PhotonWaveformPars[j + 1] = templine[j + 2];
      std::cout << int(templine[0]) << " " << templine[1] << endl;
    }
    PhotonFile.close();
  } else {
    std::cout << "ERROR cannont open photon param file." << endl;
    return -1;
  }
  if (Flag == 0) {
    for (unsigned int f = 0; f < cellIDcheck.size(); f++) {
      for (int k = 0; k < 11; k++)  PhotonWaveformPar[k] = cellIDcheck[f].PhotonWaveformPars[k];
      ParameterTree->Fill();
    }
  } else if (Flag == 1) {
    int batch = 10;
    for (int k = 0; k < 874; k++) {
      int low = k * batch;
      int high = (k + 1) * batch;
      //std::cout<<k<<std::endl;
      TFile* TempFile = new TFile(InputDirectory + Form("HadronPars_Low%d_High%d.root", low, high), "READ");
      TTree* TempTree = (TTree*)  TempFile->Get("HadronWaveformInfo");
      double tHadronShapePars_A[11];
      double tDiodeShapePars_A[11];
      double tMaxResidualHadron_A;
      double tMaxResidualDiode_A;
      double tMaxValDiode_A;
      double tMaxValHadron_A;
      TempTree->SetBranchAddress("TempHadronPar11_A", &tHadronShapePars_A);
      TempTree->SetBranchAddress("TempDiodePar11_A", &tDiodeShapePars_A);
      TempTree->SetBranchAddress("MaxResHadron_A", &tMaxResidualHadron_A);
      TempTree->SetBranchAddress("MaxResDiode_A", &tMaxResidualDiode_A);
      TempTree->SetBranchAddress("MaxValDiode_A", &tMaxValDiode_A);
      TempTree->SetBranchAddress("MaxValHadron_A", &tMaxValHadron_A);
      //
      for (int j = 0; j < batch; j++) {
        int tCellID = (k * batch) + j;
        if (tCellID >= 8736)continue;
        TempTree->GetEntry(j);
        cellIDcheck[tCellID].HadronWaveformPars.resize(11);
        cellIDcheck[tCellID].DiodeWaveformPars.resize(11);
        for (int p = 0; p < 11; p++) {
          if (tHadronShapePars_A[p] > 100 || tHadronShapePars_A[p] < -100) {
            std::cout << "Warning  large parameter for: " << tCellID << " " << tHadronShapePars_A[p] << std::endl;
            for (int h = 0; h < 11; h++)std::cout << tHadronShapePars_A[h] << " , ";
            std::cout << std::endl;
          }
          cellIDcheck[tCellID].HadronWaveformPars[p] = tHadronShapePars_A[p];
        }
        //std::cout<<tCellID<<std::endl;
        //for(int h=0;h<11;h++ )std::cout<<tHadronShapePars_A[h]<<" , ";
        //std::cout<<std::endl;
        cellIDcheck[tCellID].MaxResHadron = tMaxResidualHadron_A;
        cellIDcheck[tCellID].MaxValHadron = tMaxValHadron_A;
        for (int p = 0; p < 11; p++)  cellIDcheck[tCellID].DiodeWaveformPars[p] = tDiodeShapePars_A[p];
        cellIDcheck[tCellID].MaxResDiode = tMaxResidualDiode_A;
        cellIDcheck[tCellID].MaxValDiode = tMaxValDiode_A;
      }
      TempFile->Close();
    }
    //
    for (unsigned int f = 0; f < 8736; f++) {
      for (int k = 0; k < 11; k++) {
        PhotonWaveformPar[k] = cellIDcheck[f].PhotonWaveformPars[k];
        HadronWaveformPar[k] = cellIDcheck[f].HadronWaveformPars[k];
        DiodeWaveformPar[k] = cellIDcheck[f].DiodeWaveformPars[k];
      }
      mHadronRes = cellIDcheck[f].MaxResHadron;
      mDiodeRes = cellIDcheck[f].MaxResDiode;
      mHadronMax = cellIDcheck[f].MaxValHadron;
      mDiodeMax = cellIDcheck[f].MaxValDiode;
      ParameterTree->Fill();
    }
  }
  //
  ParameterTreeOutputFile->cd();
  ParameterTree->Write();
  ParameterTreeOutputFile->Write();
  ParameterTreeOutputFile->Close();
  //
  //
  return 0;
}
