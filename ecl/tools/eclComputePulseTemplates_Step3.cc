#include <TF1.h>
#include <math.h>
#include <TTree.h>
#include <TAxis.h>
#include <TFile.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <vector>
#include <TGraph.h>
#include <iostream>
#include <ecl/digitization/OfflineFitFunction.h>
#include <assert.h>

//
// See eclComputePulseTemplates_Step0.cc for README instructions.
//
//Check Maximum residual for fit to template shape.
double GetMaxRes(TGraph* gin, TF1* fit)
{
  double MaxResidual = 0;
  double MaxData = 0;
  for (int j = 0; j < 1000; j++) {
    if (gin->GetX()[j] < 10) {
      double tempVal = gin->GetY()[j];
      double tempResidual = tempVal - fit->Eval(gin->GetX()[j]);
      if (gin->GetY()[j] > MaxData) MaxData = gin->GetY()[j];
      if (abs(tempResidual) > abs(MaxResidual))  MaxResidual = tempResidual;
    }
  }
  return abs(MaxResidual) / MaxData;
}
//
TF1* FitPulse(TGraph* gin, int ShapeFlag, double* pulseInputPara)
{
  //ShapeFlag = 0 for hadron
  //ShapeFlag = 1 for diode
  //
  double highRange = 10;
  TF1* ShpFloat = new TF1(Form("Shp_%d", ShapeFlag), Belle2::ECL::WaveFuncTwoComponent, 0, highRange, 26);
  ShpFloat->FixParameter(0, 0);
  ShpFloat->FixParameter(1, 0);
  ShpFloat->FixParameter(2, 1);
  ShpFloat->FixParameter(3, 0);
  for (int k = 0; k < 10; k++) {
    ShpFloat->SetParameter(4 + k, pulseInputPara[k + 1]);
    ShpFloat->FixParameter(10 + 4 + k, pulseInputPara[k + 1]);
  }
  ShpFloat->SetParameter(24, pulseInputPara[0]);
  ShpFloat->FixParameter(25, 1);
  ShpFloat->SetNpx(1000);
  gin->Fit(Form("Shp_%d", ShapeFlag), "Q N 0 R", "", 0, 30);
  //
  //Try Fit and Check residual
  double Check = GetMaxRes(gin, ShpFloat);
  double CheckMin = Check;
  double ParMin11[11];
  ParMin11[0] = ShpFloat->GetParameter(24);
  for (int k = 0; k < 10; k++)ParMin11[k + 1] = ShpFloat->GetParameter(k + 4);
  int nFits = 0;
  int Attempt = 0;
  //keep fitting until residual is less than 1%
  std::cout << nFits << " " << Check << std::endl;
  while ((Check > 0.01) && nFits < 20) {
    //
    ShpFloat->SetParameter(24, ParMin11[0] * (1.0 + (0.01 * nFits)));
    for (int k = 0; k < 10; k++)  ShpFloat->SetParameter(k + 4, ParMin11[k + 1] * (1.0 + (0.01 * nFits)));
    //
    gin->Fit(Form("Shp_%d", ShapeFlag), "Q N 0 R W", "", 0.01, highRange);
    nFits++;
    Check = GetMaxRes(gin, ShpFloat);
    //
    if (Check < CheckMin) {
      //save neew best parameters:w
      CheckMin = Check;
      ParMin11[0] = ShpFloat->GetParameter(24);
      for (int k = 0; k < 10; k++)ParMin11[k + 1] = ShpFloat->GetParameter(k + 4);
    }
    std::cout << "nFit=" << nFits << " " << Check << std::endl;;
    //
    if (nFits == 20 && Attempt == 0 && (Check > 0.01)) {
      //Try new initial conditions
      if (ShapeFlag == 1) {
        double ParMin11t[11] = {36.1232, -0.284876, 0.350343, 0.432839, 0.445749, 0.27693, 0.00899611, 6.11111, 0.788569, 0.570159, -0.411252};
        for (int k = 0; k < 11; k++)ParMin11[k] = ParMin11t[k];
      } else {
        double ParMin11t[11] = {10, 0.031, 4.2e-5, 0.74, 0.43, 0.61, 0.03, 3.8, 0.81, 0.77, 0.59};
        for (int k = 0; k < 11; k++)ParMin11[k] = ParMin11t[k];
      }
      Attempt++;
      nFits = 0;
    }
  }
  //set to best parameters
  ShpFloat->SetParameter(24, ParMin11[0]);
  for (int k = 0; k < 10; k++)  ShpFloat->SetParameter(k + 4, ParMin11[k + 1]);
  //
  return ShpFloat;
}
//
int main(int argc, char* argv[])
{
  //
  TString OutputDirectory = "";
  if (OutputDirectory == "") {
    std::cout << "Error set ouput directory" << std::endl;
    return -1;
  }
  //
  assert(argc == 3);
  int LowCellID = atoi(argv[1]);
  int HighCellID = atoi(argv[2]);
  //
  double TimeAll_A[1000];
  double PhotonValues_A[1000];
  double HadronValues_A[1000];
  double DiodeValues_A[1000];
  TFile* HadronShapeFile  = new TFile(OutputDirectory + Form("HadronShapes_Low%d_High%d.root", LowCellID, HighCellID));
  TTree* chain = (TTree*) HadronShapeFile->Get("HadronTree");
  chain->SetBranchAddress("TimeAll_A", &TimeAll_A);
  chain->SetBranchAddress("ValuePhoton_A", &PhotonValues_A);
  chain->SetBranchAddress("ValueHadron_A", &HadronValues_A);
  chain->SetBranchAddress("ValueDiode_A", &DiodeValues_A);
  //
  double TempHadronPar11_A[11];
  double TempDiodePar11_A[11];
  double MaxResDiode_A = 0;
  double MaxValDiode_A = 0;
  double MaxResHadron_A = 0;
  double MaxValHadron_A = 0;
  //
  TFile* f = new TFile(OutputDirectory + Form("HadronPars_Low%d_High%d.root", LowCellID, HighCellID), "RECREATE");
  TTree* WaveformParametersTree = new TTree("HadronWaveformInfo", "");
  WaveformParametersTree->Branch("TempHadronPar11_A", &TempHadronPar11_A, "TempHadronPar11_A[11]/D");
  WaveformParametersTree->Branch("TempDiodePar11_A", &TempDiodePar11_A, "TempDiodePar11_A[11]/D");
  WaveformParametersTree->Branch("MaxResDiode_A", &MaxResDiode_A, "MaxResDiode_A/D");
  WaveformParametersTree->Branch("MaxResHadron_A", &MaxResHadron_A, "MaxResHadron_A/D");
  WaveformParametersTree->Branch("MaxValHadron_A", &MaxValHadron_A, "MaxValHadron_A/D");
  WaveformParametersTree->Branch("MaxValDiode_A", &MaxValDiode_A, "MaxValDiode_A/D");
  std::vector<TCanvas*> AllFits;
  //
  Long64_t nentries = chain->GetEntriesFast();
  for (Long64_t jentry = 0; jentry < nentries; jentry++) {
    chain->GetEntry(jentry);
    //
    for (int l = 0; l < 11; l++) {
      TempDiodePar11_A[l] = 0;
      TempHadronPar11_A[l] = 0;
    }
    int CellID = ((int)jentry) + LowCellID + 1;
    //
    if (TimeAll_A[0] > -100) {
      //
      TGraph* G1_A = new TGraph(1000, TimeAll_A, PhotonValues_A);
      TGraph* G2_A = new TGraph(1000, TimeAll_A, HadronValues_A);
      TGraph* G3_A = new TGraph(1000, TimeAll_A, DiodeValues_A);
      //
      double InputPara_A_H[11] = {1.501407e+01, 0.000000e+00, 4.693325e-03, 7.538656e-01, 4.371189e-01, 1.182754e+00, 3.242071e-03, 6.669907e+00, 8.454105e-01, 7.240517e-01, 4.097126e-01};
      double InputPara_A_D[11] = {1.501407e+01, 0.000000e+00, 4.693325e-03, 7.538656e-01, 4.371189e-01, 1.182754e+00, 3.242071e-03, 6.669907e+00, 8.454105e-01, 7.240517e-01, 4.097126e-01};
      //
      TF1* F2_A =  FitPulse(G2_A, 0, InputPara_A_H);
      TF1* F3_A =  FitPulse(G3_A, 1, InputPara_A_D);
      //
      TempHadronPar11_A[0] = F2_A->GetParameter(24);
      TempDiodePar11_A[0] = F3_A->GetParameter(24);
      for (int k = 0; k < 10; k++) {
        TempHadronPar11_A[k + 1] = F2_A->GetParameter(k + 4);
        TempDiodePar11_A[k + 1] = F3_A->GetParameter(k + 4);
      }
      //
      MaxResHadron_A = GetMaxRes(G2_A, F2_A);
      MaxResDiode_A = GetMaxRes(G3_A, F3_A);
      //
      MaxValHadron_A = F2_A->GetMaximum(1, 10);
      MaxValDiode_A = F3_A->GetMaximum(1, 10);
      //
      std::cout << CellID << " " << MaxResHadron_A << " " << MaxResDiode_A << std::endl;
      WaveformParametersTree->Fill();
      //
      //Set to true to draw fit results in output file
      if (false) {
        G2_A->SetMarkerColor(kBlue);
        G3_A->SetMarkerColor(kRed);
        G1_A->GetYaxis()->SetRangeUser(-0.5, 2);
        //
        AllFits.push_back(new TCanvas(Form("A_all_%lld", jentry), ""));
        G1_A->Draw("AP");
        G2_A->Draw("same P");
        G3_A->Draw("sameP");
        AllFits.push_back(new TCanvas(Form("A_fitHad_%lld", jentry), ""));
        AllFits[AllFits.size() - 1]->cd();
        G2_A->Draw("AP");
        F2_A->Draw("same l");
        AllFits.push_back(new TCanvas(Form("A_fitDio_%lld", jentry), ""));
        AllFits[AllFits.size() - 1]->cd();
        G3_A->Draw("AP");
        F3_A->Draw("same l");
        AllFits.push_back(new TCanvas(Form("MC_all_%lld", jentry), ""));
        G1_A->Draw("AP");
        //TF1*   MCgamam = new TF1(Form("MC_%lld", jentry), Wa, 0, 32, 11);
        //MCgamam->SetParameter(0, 27.7221);
        //MCgamam->SetParameter(1, 0.);
        //MCgamam->SetParameter(2, 0.648324);
        //MCgamam->SetParameter(3, 0.401711);
        //MCgamam->SetParameter(4, 0.374167);
        //MCgamam->SetParameter(5, 0.849417);
        //MCgamam->SetParameter(6, 0.00144548);
        //MCgamam->SetParameter(7, 4.70722);
        //MCgamam->SetParameter(8, 0.815639);
        //MCgamam->SetParameter(9, 0.555605);
        //MCgamam->SetParameter(10, 0.2752);
        //MCgamam->Draw("same l");
      } else {
        G1_A->Delete();
        G2_A->Delete();
        G3_A->Delete();
        F2_A->Delete();
        F3_A->Delete();
      }
    }
  }
  //
  f->cd();
  WaveformParametersTree->Write();
  for (unsigned int k = 0; k < AllFits.size(); k++)  AllFits[k]->Write();
  f->Write();
  //
  return 1;
  //
}
