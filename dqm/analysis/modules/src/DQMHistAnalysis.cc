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

#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <TROOT.h>
#include <TClass.h>



using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysis)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisModule::HistList DQMHistAnalysisModule::g_hist;
DQMHistAnalysisModule::MonObjList DQMHistAnalysisModule::g_monObj;

DQMHistAnalysisModule::DQMHistAnalysisModule() : Module()
{
  //Set module properties
  setDescription("Histgram Analysis module");
}


DQMHistAnalysisModule::~DQMHistAnalysisModule()
{

}

TCanvas* DQMHistAnalysisModule::find_canvas(TString canvas_name)
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


const DQMHistAnalysisModule::HistList& DQMHistAnalysisModule::getHistList()
{
  return g_hist;
}

const DQMHistAnalysisModule::MonObjList& DQMHistAnalysisModule::getMonObjList()
{
  return g_monObj;
}


TH1* DQMHistAnalysisModule::findHist(const std::string& histname)
{
  if (g_hist.find(histname) != g_hist.end()) {
    if (g_hist[histname]) {
      //Want to search elsewhere if null-pointer saved in map
      return g_hist[histname];
    } else {
      B2ERROR("Histogram " << histname << " listed as being in memfile but points to nowhere.");
    }
  }
  B2INFO("Histogram " << histname << " not in memfile.");

  //Histogram not in list, search in memory for it
  gROOT->cd();

  //Following the path to the histogram
  TDirectory* d = gROOT;
  TString myl = histname;
  TString tok;
  Ssiz_t from = 0;
  while (myl.Tokenize(tok, from, "/")) {
    TString dummy;
    Ssiz_t f;
    f = from;
    if (myl.Tokenize(dummy, f, "/")) { // check if its the last one
      auto e = d->GetDirectory(tok);
      if (e) {
        B2INFO("Cd Dir " << tok);
        d = e;
      }
      d->cd();
    } else {
      break;
    }
  }

  // This code assumes that the histograms address does NOT change between initialization and any later event
  // This assumption seems to be reasonable for TFiles and in-memory objects
  // BUT this means => Analysis moules MUST NEVER create a histogram with already existing name NOR delete any histogram
  TH1* found_hist = findHist(d, tok);
  if (found_hist) {
    g_hist[histname] = found_hist;//Can't use addHist as we want to overwrite invalid entries
  }
  return found_hist;

}

TH1* DQMHistAnalysisModule::findHist(const std::string& dirname, const std::string& histname)
{
  if (dirname.size() > 0) {
    return findHist(dirname + "/" + histname);
  }
  return findHist(histname);
}

TH1* DQMHistAnalysisModule::findHist(const TDirectory* histdir, const TString& histname)
{
  TObject* obj = histdir->FindObject(histname);
  if (obj != NULL) {
    if (obj->IsA()->InheritsFrom("TH1")) {
      B2INFO("Histogram " << histname << " found in mem");
      return (TH1*)obj;
    }
  } else {
    B2INFO("Histogram " << histname << " NOT found in mem");
  }
  return NULL;
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
