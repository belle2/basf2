#include <iostream>

#include <TApplication.h>
#include <TROOT.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TTree.h>
#include <TH1F.h>
#include <THStack.h>
#include <TLegend.h>

#include <string>
#include <map>
#include <set>

using namespace std;

set<string> component_names = {
  "Coulomb_LER", "Coulomb_HER",
  "Touschek_LER", "Touschek_HER",
  "RBB_LER", "RBB_HER",
  "twoPhoton", "SR_LER", "SR_HER"
};
map<string, int> component_colors = {
  {"Coulomb_LER", 41},  // latte
  {"Coulomb_HER", 42},
  {"Touschek_LER", 45}, // reddish
  {"Touschek_HER", 46},
  {"RBB_LER", 29},      // greenish
  {"RBB_HER", 30},
  {"twoPhoton", 40},      // blue
  {"SR_LER", 25},      // grayish
  {"SR_HER", 28}
};

const char* c_layerlabels[4] = {"3", "4", "5", "6"};
const float c_barwidth = 0.4;

void makeDoseBars(TFile* f);
void makeNeutronFluxBars(TFile* f);

int main(int argc, char** argv)
{
  if (argc != 2) {
    cout << "Hello!" << endl;
    cout << "Usage: " << argv[0] << " filename.root" << endl;
    cout << "Make plots out of SVDBackground module results file filename.root" << endl;
    return 0;
  }
  // catch name before Tapplication devours it!
  string histoFileName(argv[1]);
  TApplication bg_app("make_bg_plots", &argc, argv);
  cout << "Opening input file: " << histoFileName.c_str() << endl;
  TFile* f = new TFile(histoFileName.c_str(), "READ");
  if (!f->IsOpen()) {
    cout << "Input file not found. Exiting." << endl;
    return -1;
  } else {
    cout << "Opened " << argv[1] << ". Processing..." << endl;
  }
  makeDoseBars(f);
  makeNeutronFluxBars(f);
  char cStop;
  cout << "Any char to continue" << endl;
  cin >> cStop;
  return 0;
}

void makeDoseBars(TFile* f)
{
  string quantity("Dose");
  string ylabel("Dose [Gy/smy]");
  cout << "Making stacked bar plot for " << quantity.c_str() << endl;
  // Make canvas
  TString s_barPlotCanvasName(Form("c_%sBars", quantity.c_str()));
  TString s_barPlotCanvasDescription(Form("%s by layer and background type", quantity.c_str()));
  TCanvas* c_barPlot = (TCanvas*)gROOT->GetListOfCanvases()->FindObject(s_barPlotCanvasName.Data());
  if (c_barPlot) delete c_barPlot;
  c_barPlot = new TCanvas(s_barPlotCanvasName.Data(), s_barPlotCanvasDescription.Data());
  // Make stacked bar chart
  THStack hBarStack("hBarStack", s_barPlotCanvasDescription.Data());
  TLegend* hBarLegend = new TLegend(0.6, 0.6, 0.85, 0.85);
  hBarLegend->SetHeader(quantity.c_str());
  hBarLegend->SetBorderSize(0);
  for (auto bg : component_names) {
    TString histoName(Form("hBar_%s_%s", quantity.c_str(), bg.c_str()));
    cout << histoName.Data() << endl;
    TH1F* histoBg = (TH1F*)f->Get(histoName.Data());
    if (!histoBg) {
      cout << "WARNING histogram not found: " << histoName.Data() << endl;
      continue;
    }
    histoBg->SetFillColor(component_colors[bg]);
    histoBg->SetBarWidth(c_barwidth);
    hBarStack.Add(histoBg);
    hBarLegend->AddEntry(histoBg, bg.c_str(), "F");
  }
  c_barPlot->cd();
  hBarStack.Draw("B");
  hBarStack.GetXaxis()->SetTitle("SVD layer");
  hBarStack.GetYaxis()->SetTitle(ylabel.c_str());
  hBarLegend->Draw();
  c_barPlot->Modified(); c_barPlot->Update();
  TString imageName(hBarStack.GetName());
  imageName.Append(".png");
  c_barPlot->SaveAs(imageName.Data());
}

void makeNeutronFluxBars(TFile* f)
{
  string quantity("Neutron_flux");
  string ylabel("Neutron flux [1/cm^2/smy]");
  cout << "Making stacked bar plot for " << quantity.c_str() << endl;
  // Make canvas
  TString s_barPlotCanvasName(Form("c_%sBars", quantity.c_str()));
  TString s_barPlotCanvasDescription(Form("%s by layer and background type", "Neutron flux"));
  TCanvas* c_barPlot = (TCanvas*)gROOT->GetListOfCanvases()->FindObject(s_barPlotCanvasName.Data());
  if (c_barPlot) delete c_barPlot;
  c_barPlot = new TCanvas(s_barPlotCanvasName.Data(), s_barPlotCanvasDescription.Data());
  // Make stacked bar chart
  THStack hBarStack("hBarStack", s_barPlotCanvasDescription.Data());
  TLegend* hBarLegend = new TLegend(0.6, 0.6, 0.85, 0.85);
  hBarLegend->SetHeader(quantity.c_str());
  hBarLegend->SetBorderSize(0);
  for (auto bg : component_names) {
    TString histoName(Form("hBar_%s_%s", quantity.c_str(), bg.c_str()));
    cout << histoName.Data() << endl;
    TH1F* histoBg = (TH1F*)f->Get(histoName.Data());
    if (!histoBg) {
      cout << "WARNING histogram not found: " << histoName.Data() << endl;
      continue;
    }
    histoBg->SetFillColor(component_colors[bg]);
    histoBg->SetBarWidth(c_barwidth);
    hBarStack.Add(histoBg);
    hBarLegend->AddEntry(histoBg, bg.c_str(), "F");
  }
  c_barPlot->cd();
  hBarStack.Draw("B");
  hBarStack.GetXaxis()->SetTitle("SVD layer");
  hBarStack.GetYaxis()->SetTitle(ylabel.c_str());
  hBarLegend->Draw();
  c_barPlot->Modified(); c_barPlot->Update();
  TString imageName(hBarStack.GetName());
  imageName.Append(".png");
  c_barPlot->SaveAs(imageName.Data());
}


