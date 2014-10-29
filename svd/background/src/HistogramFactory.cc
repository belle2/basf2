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

#include <algorithm>
#include <boost/format.hpp>

using namespace std;
using namespace Belle2;
using namespace SVD;
using boost::format;

HistogramFactory::HistogramFactory()
{
  // Define the BgValues here. If useful, can be read from an xml.
  // Don't know the ranges for some cases.
  m_valueDescriptions.insert(make_pair("dose", BgValue("Dose", "Gy/smy", 0, 1e3, 20)));
  m_valueDescriptions.insert(make_pair("exposition", BgValue("Exposition", "J/cm2/s", 0, 1, 20)));
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

TH1F* HistogramFactory::MakeFluencePlot(const string& componentName, const string& valueName, int layer)
{
  // Retrieve full value name from the BgValues map
  const string fullValueName(getTitle(valueName));
  string comp_id(fullValueName + "_" + componentName + str(format("_%i") % layer));
  std::replace(comp_id.begin(), comp_id.end(), ' ', '_');
  string histo_name = "hStack_" + comp_id;
  string histo_title(componentName);  // keep original component name for legend; will not be used in actual plot anyway.
  auto range = getRange(valueName);
  int nBins = getNBins(valueName);
  TH1F* result = new TH1F(histo_name.c_str(), histo_title.c_str(), nBins, range.first, range.second);
  //form and set y-axis title
  result->GetYaxis()->SetTitle(getAxisLabel(valueName).c_str());
  result->SetFillColor(component_colors[componentName]);
  return result;
}

