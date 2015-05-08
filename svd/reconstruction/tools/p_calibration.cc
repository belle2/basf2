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

// This script creates calibration plots for signal probabilities
// estimated by the SVD time fitter.
// It uses the results of a toy simulation to calculate signal probability
// for a given signal prior based on simulation statistics, and compares
// them with signal probabilities reported by the time fitter.

using namespace std;

/** Returns an approximation to integral over a range of a histogram
 * @param h Histogram to integrate
 * @xmin lower bound of integration range
 * @xmax upper bound of integration range
 * @return integral of histogram from xmin to xmax
 */
double histogram_integral(const TH1F* h, double xmin, double xmax)
{
  if (xmin > xmax) {
    double z = xmin;
    xmin = xmax;
    xmax = z;
  }
  TAxis* axis = h->GetXaxis();
  int bmin = axis->FindBin(xmin);
  int bmax = axis->FindBin(xmax);
  double integral = h->Integral(bmin, bmax);
  integral -= h->GetBinContent(bmin) * (xmin - axis->GetBinLowEdge(bmin)) /
              axis->GetBinWidth(bmin);
  integral -= h->GetBinContent(bmax) * (axis->GetBinUpEdge(bmax) - xmax) /
              axis->GetBinWidth(bmax);
  return integral;
}

const double inner = 5.0;
const double outer = 31.44;

const double p_signal_inner = 1.0 / inner / (1.0 / inner + 1.0 / outer);
const double w_inner = outer / inner;

double p_signal(double t)
{
  if (abs(t) < inner) return p_signal_inner;
  else return 0;
}

int main(int argc, char** argv)
{
  TApplication pCalApp("pCalApp", &argc, argv);
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
    tree->Draw("p_signal:fit_t0", critProf.Data(), "profs");
    TProfile* htemp = (TProfile*)cCalib->GetPrimitive("htemp");
    htemp->GetXaxis()->SetTitle("fitted time [ns]");
    htemp->GetYaxis()->SetTitle("signal probability");
    TString histoname(Form("Signal probabilities, S/N = %2.0f", amplitude));
    htemp->SetTitle(histoname.Data());
    // Now calculate probabilities from simulation
    TString critHisto(Form("%f*(abs(true_t0)<=5 && true_amp==%2.0f) + 1.0*(abs(true_t0)>5 && true_amp==%2.0f)", w_inner, amplitude,
                           amplitude));
    TString formHisto(Form("(abs(true_t0)<=5)*%f + (abs(true_t0)>5)*0.0 : fit_t0 >> hp", p_signal_inner));
    tree->Draw(formHisto.Data(), critHisto.Data(), "prof same");
    TH1F* h = (TH1F*)gDirectory->Get("hp");
    h->SetMarkerColor(kRed);
    h->SetLineColor(kRed);
    // Save the canvas
    TString canvasName(Form("pCalib_%2.0f.png", amplitude));
    cCalib->SaveAs(canvasName.Data());
    char c;
    cout << "Any char to continue" << endl;
    cin >> c;
  }
  return 0;
}
