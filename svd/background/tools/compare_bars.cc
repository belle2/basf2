#include <svd/background/HistogramFactory.h>

#include <TApplication.h>
#include <TFile.h>
#include <TCanvas.h>

#include <iostream>
#include <string>
#include <utility>
#include <map>
#include <set>
#include <algorithm>

// NOTE:
// This is special code for 11th software campaign, and is hardly useful for general
// purposes other than as an example.

using namespace std;
using namespace Belle2::SVD;

pair<string, string> categories("narrow", "wide");

set<string> component_names = {
  "Coulomb LER", "Touschek LER"
};

set<string> bar_quantities = {
  "dose", "neutronFlux", "firedU", "firedV", "occupancyU", "occupancyV"
};

set<string> fluence_quantities = {
  "neutronFlux", "neutronFluxNIEL"
};

int main(int argc, char** argv)
{
  // This is what we are going to compare.
  pair<string, string> categories("narrow", "wide");

  if (argc != 3) {
    cout << "Hello!" << endl;
    cout << "Usage: " << argv[0] << " filename1.root filename2.root" << endl;
    cout << "Make comparison bar plots out of two SVDBackground module results files." << endl;
    return 0;
  }
  // catch names before TApplication devours it!
  string histoFileName1(argv[1]);
  string histoFileName2(argv[2]);
  TApplication bg_app("compare_bg_plots", &argc, argv);
  cout << "Opening input files: " <<
       histoFileName1.c_str() << " " <<
       histoFileName2.c_str() << endl;
  TFile* f1 = new TFile(histoFileName1.c_str(), "READ");
  if (!f1->IsOpen()) {
    cout << "Input file 1 not found. Exiting." << endl;
    return -1;
  } else {
    cout << "Opened " << argv[1] << "." << endl;
  }
  TFile* f2 = new TFile(histoFileName2.c_str(), "READ");
  if (!f2->IsOpen()) {
    cout << "Input file 2 not found. Exiting." << endl;
    return -1;
  } else {
    cout << "Opened " << argv[2] << "." << endl;
  }
  cout << "Plotting..." << endl;
  //We have the two files open, let's create a HistogramFactory object
  HistogramFactory factory;
  for (auto quantity : bar_quantities) {
    for (auto component_name : component_names) {
      TCanvas* c_bars = factory.PlotCompareBars(f1, f2, categories, component_name, quantity);
      string barplot_name(quantity + "_" + component_name + "_compare.png");
      std::replace(barplot_name.begin(), barplot_name.end(), ' ', '_');
      c_bars->SaveAs(barplot_name.c_str());
    }
  }
  char cStop;
  cout << "Any char to continue" << endl;
  cin >> cStop;
  return 0;
}
