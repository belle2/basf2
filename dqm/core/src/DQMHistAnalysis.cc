/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisModule.cc
// Description : Baseclass for DQM histogram analysis module
//-

#include <dqm/core/DQMHistAnalysis.h>
#include <boost/algorithm/string.hpp>
#include <TROOT.h>
#include <TClass.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysis);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisModule::HistList DQMHistAnalysisModule::g_hist;
DQMHistAnalysisModule::MonObjList DQMHistAnalysisModule::g_monObj;

DQMHistAnalysisModule::DQMHistAnalysisModule() : Module()
{
  //Set module properties
  setDescription("Histogram Analysis module");
}


DQMHistAnalysisModule::~DQMHistAnalysisModule()
{

}

void DQMHistAnalysisModule::addHist(const std::string& dirname, const std::string& histname, TH1* h)
{
  if (dirname.size() > 0) {
    g_hist.insert(HistList::value_type(dirname + "/" + histname, h));
  } else {
    g_hist.insert(HistList::value_type(histname, h));
  }
}

MonitoringObject* DQMHistAnalysisModule::getMonitoringObject(const std::string& objName)
{
  if (g_monObj.find(objName) != g_monObj.end()) {
    if (g_monObj[objName]) {
      return g_monObj[objName];
    } else {
      B2WARNING("MonitoringObject " << objName << " listed as being in memfile but points to nowhere. New Object will be made.");
      g_monObj.erase(objName);
    }
  }

  MonitoringObject* obj = new MonitoringObject(objName);
  g_monObj.insert(MonObjList::value_type(objName, obj));
  return obj;
}

TCanvas* DQMHistAnalysisModule::findCanvas(TString canvas_name)
{
  TIter nextkey(gROOT->GetListOfCanvases());
  TObject* obj{};

  while ((obj = dynamic_cast<TObject*>(nextkey()))) {
    if (obj->IsA()->InheritsFrom("TCanvas")) {
      if (obj->GetName() == canvas_name)
        return dynamic_cast<TCanvas*>(obj);
    }
  }
  return nullptr;
}

TH1* DQMHistAnalysisModule::findHist(const std::string& histname)
{
  if (g_hist.find(histname) != g_hist.end()) {
    if (g_hist[histname]) {
      return g_hist[histname];
    } else {
      B2ERROR("Histogram " << histname << " listed as being in memfile but points to nowhere.");
    }
  }
  B2INFO("Histogram " << histname << " not in memfile.");
  return nullptr;
}

TH1* DQMHistAnalysisModule::findHist(const std::string& dirname, const std::string& histname)
{
  if (dirname.size() > 0) {
    return findHist(dirname + "/" + histname);
  }
  return findHist(histname);
}

TH1* DQMHistAnalysisModule::findHistInCanvas(const std::string& histo_name)
{
  // parse the dir+histo name and create the corresponding canvas name
  auto s = StringSplit(histo_name, '/');
  auto dirname = s.at(0);
  auto hname = s.at(1);
  std::string canvas_name = dirname + "/c_" + hname;

  auto cobj = findCanvas(canvas_name);
  if (cobj == nullptr) return nullptr;

  TIter nextkey(((TCanvas*)cobj)->GetListOfPrimitives());
  TObject* obj{};
  while ((obj = dynamic_cast<TObject*>(nextkey()))) {
    if (obj->IsA()->InheritsFrom("TH1")) {
      if (obj->GetName() == histo_name)
        return  dynamic_cast<TH1*>(obj);
    }
  }
  return nullptr;
}

TH1* DQMHistAnalysisModule::findHistInFile(TFile* file, const std::string& histname)
{
  // find histogram by name in file, histname CAN contain directory!
  // will return nullptr if file is zeroptr, not found or not correct type
  if (file && file->IsOpen()) {
    auto obj = file->Get(histname.data());
    if (obj != nullptr) {
      // check class type
      if (obj->IsA()->InheritsFrom("TH1")) {
        B2DEBUG(20, "Histogram " << histname << " found in file");
        return dynamic_cast<TH1*>(obj);
      } else {
        B2INFO("Found Object " << histname << " in file is not a histogram");
      }
    } else {
      B2INFO("Histogram " << histname << " not found in file");
    }
  }
  return nullptr;
}

MonitoringObject* DQMHistAnalysisModule::findMonitoringObject(const std::string& objName)
{
  if (g_monObj.find(objName) != g_monObj.end()) {
    if (g_monObj[objName]) {
      //Want to search elsewhere if null-pointer saved in map
      return g_monObj[objName];
    } else {
      B2ERROR("MonitoringObject " << objName << " listed as being in memfile but points to nowhere.");
    }
  }
  B2INFO("MonitoringObject " << objName << " not in memfile.");
  return NULL;
}

std::vector <std::string> DQMHistAnalysisModule::StringSplit(const std::string& in, const char delim)
{
  std::vector <std::string> out;
  boost::split(out, in, [delim](char c) {return c == delim;});
  return out;
}
