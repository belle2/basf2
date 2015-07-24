#include <TFile.h>
#include <TChain.h>
#include <TH2.h>
#include <TH1.h>
#include <TF1.h>
#include <TMath.h>
#include <iostream>
#include <fstream>
#include <cassert>

TH2F* Box = new TH2F("Box", "box", 8736, 0., 8736., 2900, 2000, 4900);
TH1F* Ms = new TH1F("Ms", "ev", 2900, 2000., 4900.);

using namespace std;

Double_t glog(Double_t* x, Double_t* par)
{
//      par(1)  -   normalization constant
//      par(2)  -   maximum of lg-function
//      par(3)  -   sigma=fwhm/2.355 {=2*sqrt(2*alog(2))}
//      par(4)  -   assymetry parameter


  Double_t cc = TMath::Sqrt(2.*TMath::Log(2.));
  Double_t sig = TMath::Log(cc * par[3] + TMath::Sqrt(1. + cc * cc * par[3] * par[3])) / cc;
  Double_t N = par[0] * par[3] / par[2] / TMath::Sqrt(2.*3.1415) / TMath::Exp(sig * sig / 2) / sig;
  Double_t arg = TMath::Log(1. + (x[0] - par[1]) * par[3] / par[2]) / sig;

  Double_t fitval = 0;
  if (x[0] > par[1] - par[2] / par[3]) {
    fitval = N * TMath::Exp(-arg * arg / 2);
  }
  return fitval;
}

void cut(const char* inputRootFilename, const char* outputCutFilename)
{

  TChain fChain("m_tree");
  Int_t poq;
  cout << "!!! file for calibration:" << inputRootFilename << endl;
  fChain.Add(inputRootFilename);

  Int_t           nhits;
  Int_t          hitA[8736][31];


  TBranch*        b_nhits;
  TBranch*        b_hitA;

  fChain.SetBranchAddress("nhits", &nhits, &b_nhits);
  fChain.SetBranchAddress("hitA", hitA, &b_hitA);

  Int_t k;
  Int_t u;

  Double_t InT;
  Double_t mug;
  Double_t rg;

  Double_t Mu[8736];
  Double_t Sg[8736];
  Double_t As[8736];
  Double_t Sr[8736];
  Double_t Sl[8736];
  Double_t Chi[8736]
  ;
  Double_t cc = TMath::Sqrt(2.*TMath::Log(2.));

  Int_t nevent = fChain.GetEntries();
  std::cout << "! nevent=" << nevent << std::endl;
  //fill 2D historgamm
  //         for (Int_t i=0;i<nevent;i++) {

  for (Int_t i = 0; i < 250; i++) {
    fChain.GetEntry(i);
    for (k = 0; k < nhits; k++) {
      for (u = 0; u < 31; u++) {
        Box->Fill(k, hitA[k][u]);
      }
    }
    if (i % 100 == 0) {std::cout << " event=" << i << std::endl;}
  }  //event cicle

  //fill 1D histigramm and fit
  //       for(k=0;k<8736;k++){
  for (k = 0; k < 86; k++) {
    InT = 0.;
    for (u = 1; u < 2901; u++) {
      Double_t Ty = Box->GetBinContent(k + 1, u);
      if (u < 3001) {
        Ms->SetBinContent(u, Ty);
        InT = InT + Ty;
      }
    }
    rg = Ms->GetRMS();
    mug = Ms->GetMean();

    TF1* func = new TF1("glog", glog, 2990., 3500., 4);
    func->SetParNames("Nornamisation", "Maximum", "Sigma", "Assimetry");
    func->SetParameters(InT, mug, rg, 0.1);
    Ms->Fit(func, "l");

    Double_t par[4];
    func->GetParameters(par);
    Mu[k] = par[1];
    Sg[k] = par[2];
    As[k] = par[3];
    Double_t chi2 = func->GetChisquare();
    Double_t ndf = func->GetNDF();

    if (ndf > 0) {
      Chi[k] = chi2 / ndf;
    } else {Chi[k] = -1.;}

    Sr[k] = (-1. + As[k] * cc + TMath::Sqrt(1. + cc * cc * As[k] * As[k])) * Sg[k] / As[k] / cc;
    Sl[k] = (1. - 1. / (As[k] * cc + TMath::Sqrt(1. + cc * cc * As[k] * As[k]))) * Sg[k] / As[k] / cc;
  }   // channels cicle


  // write data

  ofstream outputCutFile(outputCutFilename);
  for (poq = 0; poq < 8736; poq++) {
    outputCutFile << poq << " " <<  Mu[poq] << " "
                  << Sg[poq] << " " <<  Sl[poq]  << " "
                  <<  Sr[poq]  << " " <<  As[poq] << endl;
    // fprintf(McoIN, "%d %f %f %f %f %f \n  ", poq, Mu[poq], Sg[poq], Sl[poq], Sr[poq], As[poq]);
  }
  outputCutFile.close();
}

int main(int argc, char** argv)

{
  assert(argc == 3);
  // first argument is input root file
  // second argument is output cut text file
  cut(argv[1], argv[2]);
  //cut("/gpfs/home/belle/avbobrov/belle2/j15/ecl/examples/rootfile.txt");
  // sprintf(Min, "/home/belle/avbobrov/binp2/chdatuXX.txt");
}
