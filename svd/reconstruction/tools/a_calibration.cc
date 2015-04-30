#include <TApplication.h>
#include <TROOT.h>
#include <TString.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TTree.h>
#include <TAxis.h>
#include <TBranch.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TProfile.h>
#include <TGraph.h>
#include <TMath.h>
#include <TLegend.h>
#include <cmath>

#include <iostream>
#include <vector>

// This script creates calibration plots for the CI acceptance criterion.
// It uses the results of a toy simulation to sample data and acceptances.

using namespace std;

int main(int argc, char** argv)
{
  TApplication aCalApp("aCalApp", &argc, argv);
  TFile* f = new TFile("time-stats.root", "READ");
  TTree* tree = (TTree*)f->Get("tree");

  const std::vector<double> sn_values = {2, 3, 5, 7, 10, 15, 20};
  //const std::vector<double> sn_values = {20};

  for (auto amplitude : sn_values) {
    TCanvas* cCalib = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("cCalib");
    if (cCalib) {
      delete cCalib;
      cCalib = nullptr;
    }
    cCalib = new TCanvas("cCalib");
    // first plot profile for the given amplitude
    TString critProf(Form("true_amp==%f", amplitude));
    tree->Draw("accept*(abs(true_t0)<5) + (1-accept)*(abs(true_t0)>5):true_t0", critProf.Data(), "prof");
    TProfile* htemp = (TProfile*)cCalib->GetPrimitive("htemp");
    htemp->GetXaxis()->SetTitle("fitted time [ns]");
    htemp->GetYaxis()->SetTitle("acceptance");
    htemp->GetYaxis()->SetLimits(0.0, 1.0);
    htemp->GetYaxis()->SetRangeUser(0.0, 1.0);
    TString histoname(Form("Acceptance, S/N = %2.0f", amplitude));
    htemp->SetTitle(histoname.Data());
    // Now calculate probabilities from simulation
    TString critHisto(Form("true_amp==%2.0f", amplitude));
    TString formHisto("(1-accept)*(abs(true_t0)<5) + accept * (abs(true_t0)>5):true_t0 >> hp");
    tree->Draw(formHisto.Data(), critHisto.Data(), "prof same");
    TH1F* h = (TH1F*)gDirectory->Get("hp");
    h->SetMarkerColor(kRed);
    h->SetLineColor(kRed);
    // Save the canvas
    TString canvasName(Form("aCalib_%2.0f.png", amplitude));
    cCalib->SaveAs(canvasName.Data());
    char c;
    cout << "Any char to continue" << endl;
    cin >> c;
  }
  return 0;
}
