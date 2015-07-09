/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 **************************************************************************/
#include <svd/background/HistogramFactory.h>
#include <framework/logging/Logger.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>

#include <TROOT.h>
#include <TLegend.h>
#include <iostream>
#include <algorithm>
#include <boost/format.hpp>

using namespace std;
using namespace Belle2::SVD;
using boost::format;

HistogramFactory::HistogramFactory()
{
  // Define the BgValues here. If useful, can be read from an xml.
  // Don't know the ranges for some cases.
  m_valueDescriptions.insert(make_pair("dose", BgValue("Dose", "Gy/smy", 0, 1e2, 20)));
  m_valueDescriptions.insert(make_pair("exposition", BgValue("Exposition", "J/cm2/s", 0, 0.05, 20)));
  m_valueDescriptions.insert(make_pair("neutronFlux", BgValue("Neutron flux", "Hz/cm2", 0, 1e10, 20)));
  m_valueDescriptions.insert(make_pair("protonFlux", BgValue("Proton flux", "Hz/cm2", 0, 1e10, 20)));
  m_valueDescriptions.insert(make_pair("pionFlux", BgValue("Pion flux", "Hz/cm2", 0, 1e10, 20)));
  m_valueDescriptions.insert(make_pair("electronFlux", BgValue("Electron flux", "Hz/cm2", 0, 1e10, 20)));
  m_valueDescriptions.insert(make_pair("photonFlux", BgValue("Photon flux", "Hz/cm2", 0, 1e10, 20)));
  m_valueDescriptions.insert(make_pair("NIELFlux", BgValue("NIEL corrected flux", "Hz/cm2", 0, 1e10, 20)));
  m_valueDescriptions.insert(make_pair("neutronFluxNIEL", BgValue("NIEL-corrected neutron flux", "Hz/cm2", 0, 1e10, 20)));
  m_valueDescriptions.insert(make_pair("protonFluxNIEL", BgValue("NIEL-corrected proton flux", "Hz/cm2", 0, 1e10, 20)));
  m_valueDescriptions.insert(make_pair("pionFluxNIEL", BgValue("NIEL-corrected pion flux", "Hz/cm2", 0, 1e10, 20)));
  m_valueDescriptions.insert(make_pair("electronFluxNIEL", BgValue("NIEL-corrected electron flux", "Hz/cm2", 0, 1e10, 20)));
  m_valueDescriptions.insert(make_pair("firedU", BgValue("Fired u-pixels", "Hz/cm2", 0, 1e6, 20)));
  m_valueDescriptions.insert(make_pair("firedV", BgValue("Fired v-pixels", "Hz/cm2", 0, 1e6, 20)));
  m_valueDescriptions.insert(make_pair("occupancyU", BgValue("u-strip occupancy", "%", 0, 10, 10)));
  m_valueDescriptions.insert(make_pair("occupancyV", BgValue("v-strip occupancy", "%", 0, 10, 10)));
}

const std::string& HistogramFactory::getAxisLabel(const std::string& valueName)
{
  static const string emptystring("");

  auto result = m_valueDescriptions.find(valueName);
  if (result != m_valueDescriptions.end()) {
    return result->second.m_axisLabel;
  } else {
    B2FATAL("Incorrect valueName in HistogramFactory::getAxisLabel!");
    return emptystring;
  }
}

const std::string& HistogramFactory::getTitle(const std::string& valueName)
{
  static const string emptystring("");

  auto result = m_valueDescriptions.find(valueName);
  if (result != m_valueDescriptions.end()) {
    return result->second.m_name;
  } else {
    B2FATAL("Incorrect valueName in HistogramFactory::getTitle!");
    return emptystring;
  }
}

std::pair<double, double> HistogramFactory::getRange(const std::string& valueName)
{
  static std::pair<double, double> result_pair(make_pair(0, 0));

  auto result = m_valueDescriptions.find(valueName);
  if (result != m_valueDescriptions.end()) {
    result_pair.first = result->second.m_rangeLow;
    result_pair.second = result->second.m_rangeHigh;
  } else {
    B2FATAL("Incorrect valueName in HistogramFactory::getRange!");
  }
  return result_pair;
}

int HistogramFactory::getNBins(const std::string& valueName)
{
  auto result = m_valueDescriptions.find(valueName);
  if (result != m_valueDescriptions.end()) {
    return result->second.m_nBins;
  } else {
    B2FATAL("Incorrect valueName in HistogramFactory::getRange!");
    return 0;
  }
}

TH1F* HistogramFactory::MakeBarPlot(const string& componentName, const string& valueName)
{
  // Retrieve full value name from the BgValues map
  const string fullValueName(getTitle(valueName));
  // FIXME: This must be corrected, componentName rather than fullValueName.
  string comp_id(fullValueName + "_" + componentName);
  std::replace(comp_id.begin(), comp_id.end(), ' ', '_');
  string histo_name = "hBar_" + comp_id;
  string histo_title(componentName);  // keep original component name for legend
  TH1F* result = new TH1F(histo_name.c_str(), histo_title.c_str(), 4, 2.5, 6.5);
  for (int i = 1; i <= 4; ++i)
    result->GetXaxis()->SetBinLabel(i, c_layerlabels[i - 1]);
  result->SetBarWidth(c_barwidth);
  result->SetBarOffset(0.5 * (1.0 - c_barwidth));
  // form and set y-axis title
  result->GetYaxis()->SetTitle(getAxisLabel(valueName).c_str());
  result->SetFillColor(component_colors[componentName]);
  return result;
}

TCanvas* HistogramFactory::PlotStackedBars(const component_tuples component_data, const string& valueName, double ymax)
{
  // Retrieve full value name from the BgValues map
  string fullValueName(getTitle(valueName));
  // Set special scaling for percentage plots
  double percent_scale = 1.0;
  if (valueName == "occupancyU" || valueName == "occupancyV")
    percent_scale = 00.0;
  // Make canvas
  string canvasName("c_" + valueName);
  string canvasDescription(fullValueName + " by layer and background type");
  TCanvas* c_barPlot = (TCanvas*)gROOT->GetListOfCanvases()->FindObject(canvasName.c_str());
  if (c_barPlot) delete c_barPlot;
  c_barPlot = new TCanvas(canvasName.c_str(), canvasDescription.c_str());
  //THStack and legend
  THStack* hBarStack = new THStack("hBarStack", canvasDescription.c_str());
  TLegend* hBarLegend = new TLegend(0.6, 0.6, 0.8, 0.85);
  hBarLegend->SetHeader("Background component");
  hBarLegend->SetBorderSize(0);
  for (auto comp_tuple : component_data) {
    const string componentName = std::get<0>(comp_tuple);
    double scale = std::get<1>(comp_tuple);
    TFile* f = std::get<2>(comp_tuple);
    string comp_id(fullValueName + "_" + componentName);
    std::replace(comp_id.begin(), comp_id.end(), ' ', '_');
    string histo_name = "hBar_" + comp_id;
    string histo_title(componentName);  // keep original component name for legend
    f->cd();
    TH1F* histo = (TH1F*)f->Get(histo_name.c_str());
    if (!histo) {
      cout << "WARNING: Histogram " << histo_name.c_str() << " not found. Dropping." << endl;
      continue;
    }
    scale *= percent_scale;
    if (scale != 1.0) histo->Scale(scale);
    histo->SetFillColor(component_colors[componentName]);
    histo->SetBarWidth(c_barwidth);
    hBarStack->Add(histo);
    hBarLegend->AddEntry(histo, componentName.c_str(), "F");
  }
  c_barPlot->cd();
  hBarStack->Draw("B");
  hBarStack->GetXaxis()->SetTitle("SVD layer");
  if (ymax > 0) { // Set fixed scale
    hBarStack->SetMinimum(0);
    hBarStack->SetMaximum(ymax);
  }
  hBarStack->GetYaxis()->SetLimits(0, ymax);
  hBarStack->GetYaxis()->SetRangeUser(0, ymax);
  hBarStack->GetYaxis()->SetTitle(getAxisLabel(valueName).c_str());
  hBarLegend->Draw();
  c_barPlot->Modified(); c_barPlot->Update();
  return c_barPlot;
}

TCanvas* HistogramFactory::PlotCompareBars(TFile* f1, TFile* f2, const pair<string, string>& categories,
                                           const string& componentName, const string& valueName)
{
  // Retrieve full value name from the BgValues map
  string fullValueName(getTitle(valueName));
  // Make canvas
  string canvasName("c_comp_" + componentName + valueName);
  string canvasDescription(fullValueName + ", comparison by layer");
  TCanvas* c_barPlot = (TCanvas*)gROOT->GetListOfCanvases()->FindObject(canvasName.c_str());
  if (c_barPlot) delete c_barPlot;
  c_barPlot = new TCanvas(canvasName.c_str(), canvasDescription.c_str());
  //TLegend
  TLegend* hBarLegend = new TLegend(0.6, 0.6, 0.8, 0.75);
  hBarLegend->SetHeader("Setting:");
  hBarLegend->SetBorderSize(0);
  //FIXME: Correct as soon as it is corrected in the MakeBarPlot method.
  string comp_id(fullValueName + "_" + componentName);
  std::replace(comp_id.begin(), comp_id.end(), ' ', '_');
  string histo_name = "hBar_" + comp_id;
  string histo_title(fullValueName + " for " + componentName);  // keep original component name for legend
  TH1F* histo1 = (TH1F*)f1->Get(histo_name.c_str());
  if (!histo1) {
    cout << "WARNING: First histogram " << histo_name.c_str() << " not found." << endl;
    return c_barPlot;
  }

  histo1->SetFillColor(component_colors[componentName]);
  histo1->SetFillStyle(3001);
  histo1->SetLineColor(kBlack);
  histo1->SetBarWidth(c_barwidth);
  histo1->SetBarOffset(0.5 * (1 - c_barwidth) - 0.5 * c_barwidth);
  hBarLegend->AddEntry(histo1, categories.first.c_str(), "F");

  TH1F* histo2 = (TH1F*)f2->Get(histo_name.c_str());
  if (!histo2) {
    cout << "WARNING: Second histogram " << histo_name.c_str() << " not found." << endl;
    return c_barPlot;
  }
  histo2->SetFillColor(component_colors[componentName]);
  histo2->SetBarWidth(c_barwidth);
  histo2->SetLineColor(kBlack);
  histo2->SetBarOffset(0.5 * (1 - c_barwidth) + 0.5 * c_barwidth);
  hBarLegend->AddEntry(histo2, categories.second.c_str(), "F");

  c_barPlot->cd();
  histo1->Draw("B");
  histo1->GetXaxis()->SetTitle("SVD layer");
  histo1->GetYaxis()->SetTitle(getAxisLabel(valueName).c_str());
  double max1 = histo1->GetMaximum();
  double max2 = histo2->GetMaximum();
  histo1->GetYaxis()->SetLimits(0, 1.1 * max(max1, max2));
  histo1->GetYaxis()->SetRangeUser(0, 1.1 * max(max1, max2));

  histo2->Draw("B same");
  hBarLegend->Draw();
  string plotTitle(fullValueName + " for " + componentName);
  histo1->SetTitle(plotTitle.c_str());
  histo2->SetTitle(plotTitle.c_str());
  c_barPlot->SetTitle(plotTitle.c_str());
  c_barPlot->Modified(); c_barPlot->Update();
  return c_barPlot;
}


TH1F* HistogramFactory::MakeFluencePlot(const string& componentName, const string& valueName, int layer)
{
  // Retrieve full value name from the BgValues map
  const string fullValueName(getTitle(valueName));
  string comp_id(fullValueName + " for " + componentName + str(format("_%i") % layer));
  std::replace(comp_id.begin(), comp_id.end(), ' ', '_');
  string histo_name = "hStack_" + comp_id;
  string histo_title(componentName);  // keep original component name for legend; will not be used in actual plot anyway.
  // auto range = getRange(valueName);
  int nBins = getNBins(valueName);
  TH1F* result = new TH1F(histo_name.c_str(), histo_title.c_str(), nBins, 0, 5);
  //form and set y-axis title
  result->GetYaxis()->SetTitle(getAxisLabel(valueName).c_str());
  result->SetFillColor(component_colors[componentName]);
  return result;
}

