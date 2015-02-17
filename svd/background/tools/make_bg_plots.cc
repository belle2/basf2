#include <svd/background/HistogramFactory.h>

#include <TApplication.h>
#include <TFile.h>
#include <TCanvas.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>
#include <set>
#include <map>
#include <algorithm>

using namespace std;
using namespace Belle2::SVD;

/** Data to hold pointers to already open files */
typedef map<string, TFile*> filemap;
/** Pairs (short name, ymax) to list quantities to plot and (optional) plot scales.*/
set<tuple<string, double>> bar_quantities = {
  tuple<string, double>("dose", 0.0),
  tuple<string, double>("neutronFlux", 0.0),
  tuple<string, double>("firedU", 0.0),
  tuple<string, double>("firedV", 0.0),
  tuple<string, double>("occupancyU", 0.0),
  tuple<string, double>("occupancyV", 0.0)
};

/** Not used so far. */
set<string> fluence_quantities = {
  "neutronFlux", "neutronFluxNIEL"
};

//FIXME: Would be useful to have more command-line options, like output dir for plots.
int main(int argc, char** argv)
{
  if (argc != 2) {
    cout << "Hello!" << endl;
    cout << "Usage: " << argv[0] << " filename.def" << endl;
    cout << "Plot SVDBackground module results using the def file." << endl;
    cout << "The .def file is a text file containing the following:" << endl;
    cout << "component_name scaling_factor y_max root_filename" << endl;
    cout << "for each component to be included in the stacked bar plot." << endl;
    cout << endl;
    return 0;
  }
  // Get the filename before TApplication devours it.
  string defFileName(argv[1]);
  TApplication bg_app("make_bg_plots", &argc, argv);

  // Parse the def file
  ifstream def(defFileName);
  if (!def.good()) {
    cout << "Couldn't open .def file " << defFileName << ". Quitting. " << endl;
    return -1;
  }
  filemap files;
  HistogramFactory::component_tuples component_data;
  for (;;) {
    string line;
    getline(def, line);
    if (!def) break;
    istringstream defline(line);
    string comp_name, file_name;
    double scale;
    defline >> comp_name >> scale >> file_name;
    //The Histogram factory method wants component names without hyphens.
    std::replace(comp_name.begin(), comp_name.end(), '_', ' ');
    //Check if the file is open; if not, open it.
    if (files.count(file_name))
      component_data.insert(make_tuple(comp_name, scale, files[file_name]));
    else { // first open file
      cout << "Opening input file: " << file_name << ". ";
      TFile* f = new TFile(file_name.c_str(), "READ");
      if (!f || !f->IsOpen())
        cout << "Input file not found. Skipping. " << endl;
      else {
        cout << "Opened. " << endl;
        files.insert(make_pair(file_name, f));
        component_data.insert(make_tuple(comp_name, scale, f));
      } // if !f
    } // if has_key
  } // for lines
  //We have files open, let's create a HistogramFactory object
  HistogramFactory factory;
  for (auto quantity_tuple : bar_quantities) {
    string quantity_name = get<0>(quantity_tuple);
    double ymax = get<1>(quantity_tuple);
    TCanvas* c_bars = factory.PlotStackedBars(component_data, quantity_name, ymax);
    string barplot_name(quantity_name + "_bars.png");
    c_bars->SaveAs(barplot_name.c_str());
  }
  char cStop;
  cout << "Any char to continue" << endl;
  cin >> cStop;
  return 0;
}
