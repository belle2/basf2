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
#include <TMultiGraph.h>
#include <TMath.h>
#include <TLegend.h>
#include <TLatex.h>
#include <cmath>

#include <iostream>
#include <vector>

// This script creates sensitivity/selectivity plots for signal selection
// based on the integrated likelihood ratio method in the SVD time fitter.
// It uses the results of a toy simulation to calculate sensitivity and s
// electivity for a given acceptance level.

using namespace std;

const double inner = 5.0;

/** String for TTree::Draw to select positive cases */
TString test_string(double level)
{
  return TString(Form("(lr_signal > %f)", level));
}

/** String for TTree::Draw to select true cases */
TString true_string()
{
  return TString(Form("(true_t0>%f && true_t0<%f)", -inner, +inner));
}

/** Negate a criterion in a string */
TString negate(const TString& criterion)
{
  return "!(" + criterion + ")";
}

const int nPoints = 8;
const std::vector<double> levels = {0.01, 0.03, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5};


int main(int argc, char** argv)
{
  TApplication lrCalApp("lrCalApp", &argc, argv);
  TFile* f = new TFile("time-stats.root", "READ");
  TTree* tree = (TTree*)f->Get("tree");

  const std::vector<double> sn_values = {3, 5, 7, 10, 15, 20};
  const std::vector<int> sn_colors = {22, 25, 28, 31, 34, 37, 40};

  TMultiGraph* mg = new TMultiGraph();
  TCanvas* cCalib = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("cCalib");
  if (cCalib) {
    delete cCalib;
    cCalib = nullptr;
  }
  cCalib = new TCanvas("cCalib");
  // Create an array of graphs for level lines
  TGraph** g_levels = new TGraph*[nPoints];
  for (int i = 0; i < nPoints; ++i) {
    g_levels[i] = new TGraph(sn_values.size());
    g_levels[i]->SetLineColor(kBlack);
    g_levels[i]->SetLineStyle(3);
  }
  for (int iamp = 0; iamp < sn_values.size(); ++iamp) {
    double amplitude = sn_values[iamp];
    TString amp_string(Form("(true_amp==%f)", amplitude));
    TGraph* g = new TGraph(nPoints);
    for (int i = 0; i < nPoints; ++i) {
      double level = levels[i];
      // Total true cases
      TString total_true_crit(true_string() + " && " + amp_string);
      tree->Draw("1", total_true_crit.Data(), "goff");
      int total_true = tree->GetSelectedRows();
      // Total selected cases
      TString total_selected_crit(test_string(level) + " && " + amp_string);
      tree->Draw("1", total_selected_crit.Data(), "goff");
      int total_selected = tree->GetSelectedRows();
      // True selected cases
      TString true_positive_crit(true_string() + " && " + test_string(level) + " && " + amp_string);
      tree->Draw("1", true_positive_crit.Data(), "goff");
      int true_selected = tree->GetSelectedRows();
      g->SetPoint(i, 1.0 * true_selected / total_true, 1.0 * true_selected / total_selected);
      g_levels[i]->SetPoint(iamp, 1.0 * true_selected / total_true, 1.0 * true_selected / total_selected);
      //cout << "Amplitude: " << amplitude << " level: " << level << " Total selected: " << total_selected << " True selected: " << true_selected << " Total true: " << total_true << " Sens: " << 1.0*true_selected/total_true << " Sel: " << 1.0*true_selected/total_selected << endl;
      // Set labels on the plot
      if (amplitude == 20) {
        TString level_label(Form("#alpha=%4.2f", level));
        TLatex* label = new TLatex(g->GetX()[i] + 0.01, g->GetY()[i], level_label.Data());
        label->SetTextSize(0.03);
        g->GetListOfFunctions()->Add(label);
      }
    }
    // add s/n values
    TString sn_string(Form("s/n=%2.0f", amplitude));
    TLatex* sn_label = new TLatex(g->GetX()[nPoints - 1] - 0.08, g->GetY()[nPoints - 1], sn_string.Data());
    sn_label->SetTextColor(sn_colors[iamp]);
    sn_label->SetTextSize(0.03);
    g->GetListOfFunctions()->Add(sn_label);
    // set plot
    g->SetLineColor(sn_colors[iamp]);
    g->SetLineWidth(2);
    g->SetMarkerColor(sn_colors[iamp]);
    g->SetMarkerStyle(kFullCircle);
    mg->Add(g, "PL");
  }
  for (int i = 0; i < nPoints; ++i) mg->Add(g_levels[i], "L");
  // Add line to show original purity of the sample
  tree->Draw("1", true_string().Data(), "goff");
  int total_true = tree->GetSelectedRows();
  double original_purity = 1.0 * total_true / tree->GetEntries();
  TGraph* g_low = new TGraph(2);
  for (int i = 0; i < 2; ++i) g_low->SetPoint(i, i, original_purity);
  g_low -> SetLineColor(kBlack);
  g_low -> SetLineStyle(3);
  TLatex* low_label = new TLatex(0.05, original_purity + 0.01, "dilution in base sample");
  low_label->SetTextSize(0.03);
  g_low->GetListOfFunctions()->Add(low_label);
  mg->Add(g_low, "L");
  mg->Draw("A");
  mg->GetXaxis()->SetTitle("sensitivity");
  mg->GetXaxis()->SetLimits(0.0, 1.0);
  mg->GetXaxis()->SetRangeUser(0.0, 1.0);
  mg->GetYaxis()->SetTitle("selectivity");
  mg->GetYaxis()->SetLimits(0.0, 1.0);
  mg->GetYaxis()->SetRangeUser(0.0, 1.0);
  mg->SetTitle("Selectivity/sensitivity of the time fitter");
  cCalib->Modified();
  // Save the canvas
  cCalib->SaveAs("SensitivitySelectivity.png");
  char c;
  cout << "Any char to continue" << endl;
  cin >> c;
  return 0;
}
