/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Program for calculation some parameters that needs for                 *
 * covariance matrices calculation                                        *
 *                                                                        *
 * Contributors: Alexander Bobrov (a.v.bobrov@inp.nsk.su) ,               *
 * Guglielmo De Nardo                                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <TFile.h>
#include <TChain.h>
#include <TH2.h>
#include <TH1.h>
#include <TF1.h>
#include <TMath.h>
#include <TCanvas.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <ecl/digitization/WrapArray2D.h>


using namespace std;
using namespace Belle2;
using namespace ECL;

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

void cut(const char* inputRootFilename, const char* outputCutFilename,
         const char* outputpdfdir, int crystalMin, int crystalMax,
         int ampMin, int ampMax)
{

  int nbins = ampMax - ampMin;
  TH2F* Box = new TH2F("Box", "box", 8736, 0., 8736., nbins, float(ampMin), float(ampMax));
  TH1F* Ms = new TH1F("Ms", "ev", nbins, float(ampMin), float(ampMax));
  TChain fChain("m_tree");
  Int_t poq;
  cout << "!!! file for calibration:" << inputRootFilename << endl;
  fChain.Add(inputRootFilename);


  string outputpdffile(outputpdfdir);
  bool doplot = !(outputpdffile == "noplot");
  outputpdffile += "/amplitudefits.pdf";

  Int_t           nhits;
  WrapArray2D<Int_t>  hitA(8736, 31);

  TBranch*        b_nhits;
  TBranch*        b_hitA;

  fChain.SetBranchAddress("nhits", &nhits, &b_nhits);
  fChain.SetBranchAddress("hitA", hitA, &b_hitA);

  Int_t k;
  Int_t u;

  Double_t InT;
  Double_t mug;
  Double_t rg;

  vector<Double_t> Mu(8736, 0);
  vector<Double_t> Sg(8736, 0);
  vector<Double_t> As(8736, 0);
  vector<Double_t> Sr(8736, 0);
  vector<Double_t> Sl(8736, 0);
  vector<Double_t> Chi(8736, 0)
  ;
  Double_t cc = TMath::Sqrt(2.*TMath::Log(2.));

  Int_t nevent = fChain.GetEntries();
  std::cout << "! nevent=" << nevent << std::endl;
  //fill 2D historgamm
  //         for (Int_t i=0;i<250;i++) {

  for (Int_t i = 0; i < nevent; i++) {
    fChain.GetEntry(i);
    for (k = 0; k < nhits; k++) {
      for (u = 0; u < 31; u++) {
        Box->Fill(k, hitA[k][u]);
      }
    }
    if (i % 100 == 0) {std::cout << " event=" << i << std::endl;}
  }  //event cicle

  // open output pdf file

  TCanvas* plot = nullptr;
  if (doplot) {
    plot = new TCanvas;
    plot->SaveAs((outputpdffile + string("[")).c_str());
  }
  //fill 1D histigramm and fit
  for (k = crystalMin ; k <= crystalMax; k++) {
    cout << "Crystal " << k << endl;

    //  for (k = 0; k < 86; k++) {
    InT = 0.;
    for (u = 1; u < 2901; u++) {
      Double_t Ty = Box->GetBinContent(k + 1, u);
      if (u < 3001) {
        Ms->SetBinContent(u, Ty);
        InT = InT + Ty;
        //cout << Ty << endl;
      }
    }
    rg = Ms->GetRMS();
    // both choices do not converge
    mug = Ms->GetMean();
    // mug = Ms->GetXaxis()->GetBinCenter(Ms->GetMaximumBin());
    TF1* func = new TF1("glog", glog, 2990., 3500., 4);
    func->SetParNames("Nornamisation", "Maximum", "Sigma", "Assimetry");
    func->SetParameters(InT, mug, rg, 0.1);
    Ms->Fit(func, "l");
    Ms->Draw();

    if (doplot) plot->SaveAs(outputpdffile.c_str());
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

    cout << "Chi[ " << k << "] = " << Chi[k] << "calculated but not used." << endl;

    Sr[k] = (-1. + As[k] * cc + TMath::Sqrt(1. + cc * cc * As[k] * As[k])) * Sg[k] / As[k] / cc;
    Sl[k] = (1. - 1. / (As[k] * cc + TMath::Sqrt(1. + cc * cc * As[k] * As[k]))) * Sg[k] / As[k] / cc;
  }   // channels cicle

  // close output pdf file
  if (doplot) plot->SaveAs((outputpdffile + string("]")).c_str());
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
  assert(argc > 3 && argc < 7);
  // first argument is input root file
  // second argument is output cut text file
  // third argument output plot file or "noplot"
  // 4th argument crystal min = 0
  // 5th argument crystal max = 8735
  int crystalMin = 0;
  int crystalMax = 8735;
  int ampMin = 2000;
  int ampMax = 4900;
  if (argc >= 5) crystalMin = stoi(argv[4]);
  if (argc >= 6) crystalMax = stoi(argv[5]);
  if (argc >= 7) ampMin = stoi(argv[6]);
  if (argc >= 8) ampMax = stoi(argv[7]);
  cut(argv[1], argv[2], argv[3], crystalMin, crystalMax, ampMin, ampMax);
  //cut("/gpfs/home/belle/avbobrov/belle2/j15/ecl/examples/rootfile.txt");
  // sprintf(Min, "/home/belle/avbobrov/binp2/chdatuXX.txt");
}
