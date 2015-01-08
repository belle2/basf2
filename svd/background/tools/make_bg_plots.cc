#include <svd/background/HistogramFactory.h>

#include <TApplication.h>
#include <TFile.h>
#include <TCanvas.h>

#include <iostream>
#include <string>
#include <map>
#include <set>

using namespace std;
using namespace Belle2::SVD;

set<string> component_names = {
  "Coulomb LER", "Coulomb HER",
  "Touschek LER", "Touschek HER",
  "RBB LER", "RBB HER",
  "twoPhoton", "SR LER", "SR HER"
};

set<string> bar_quantities = {
  "dose", "neutronFlux", "firedU", "firedV", "occupancyU", "occupancyV"
};

set<string> fluence_quantities = {
  "neutronFlux", "neutronFluxNIEL"
};

int main(int argc, char** argv)
{
  if (argc != 2) {
    cout << "Hello!" << endl;
    cout << "Usage: " << argv[0] << " filename.root" << endl;
    cout << "Make plots out of SVDBackground module results file filename.root" << endl;
    return 0;
  }
  // catch name before TApplication devours it!
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
  //We have file open, let's create a HistogramFactory object
  HistogramFactory factory;
  for (auto quantity : bar_quantities) {
    TCanvas* c_bars = factory.PlotStackedBars(f, component_names, quantity);
    string barplot_name(quantity + "_bars.png");
    c_bars->SaveAs(barplot_name.c_str());
  }
  char cStop;
  cout << "Any char to continue" << endl;
  cin >> cStop;
  return 0;
}
