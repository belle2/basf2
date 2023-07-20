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

DQMHistAnalysisModule::HistList DQMHistAnalysisModule::s_histList;
DQMHistAnalysisModule::MonObjList DQMHistAnalysisModule::s_monObjList;
DQMHistAnalysisModule::DeltaList DQMHistAnalysisModule::s_deltaList;
DQMHistAnalysisModule::CanvasUpdatedList DQMHistAnalysisModule::s_canvasUpdatedList;

bool DQMHistAnalysisModule::m_useEpics = false; // default to false, to enable EPICS, add special EPICS Module class into chain
bool DQMHistAnalysisModule::m_epicsReadOnly =
  false; // special for second "online" use (reading limits). default to false, to enable EPICS, add special EPICS Module parameter

DQMHistAnalysisModule::DQMHistAnalysisModule() : Module()
{
  //Set module properties
  setDescription("Histogram Analysis module base class");
}

bool DQMHistAnalysisModule::addHist(const std::string& dirname, const std::string& histname, TH1* h)
{
  std::string fullname;
  if (dirname.size() > 0) {
    fullname = dirname + "/" + histname;
  } else {
    fullname = histname;
  }

  if (s_histList[fullname].update(h)) {
    // only if histogram changed, check if delta histogram update needed
    auto it = s_deltaList.find(fullname);
    if (it != s_deltaList.end()) {
      B2DEBUG(20, "Found Delta" << fullname);
      it->second->update(h); // update
    }
    return true; // histogram changed
  }

  return false; // histogram didnt change
}

void DQMHistAnalysisModule::addDeltaPar(const std::string& dirname, const std::string& histname, HistDelta::EDeltaType t, int p,
                                        unsigned int a)
{
  std::string fullname;
  if (dirname.size() > 0) {
    fullname = dirname + "/" + histname;
  } else {
    fullname = histname;
  }
  s_deltaList[fullname] = new HistDelta(t, p, a);
}

bool DQMHistAnalysisModule::hasDeltaPar(const std::string& dirname, const std::string& histname)
{
  std::string fullname;
  if (dirname.size() > 0) {
    fullname = dirname + "/" + histname;
  } else {
    fullname = histname;
  }
  return s_deltaList.find(fullname) != s_deltaList.end(); // contains() if we switch to C++20
}

TH1* DQMHistAnalysisModule::getDelta(const std::string& dirname, const std::string& histname, int n, bool onlyIfUpdated)
{
  std::string fullname;
  if (dirname.size() > 0) {
    fullname = dirname + "/" + histname;
  } else {
    fullname = histname;
  }
  return getDelta(fullname, n, onlyIfUpdated);
}

TH1* DQMHistAnalysisModule::getDelta(const std::string& fullname, int n, bool onlyIfUpdated)
{
  auto it = s_deltaList.find(fullname);
  if (it != s_deltaList.end()) {
    return it->second->getDelta(n, onlyIfUpdated);
  }
  B2WARNING("Delta hist " << fullname << " not found");
  return nullptr;
}

MonitoringObject* DQMHistAnalysisModule::getMonitoringObject(const std::string& objName)
{
  if (s_monObjList.find(objName) != s_monObjList.end()) {
    if (s_monObjList[objName]) {
      return s_monObjList[objName];
    } else {
      B2WARNING("MonitoringObject " << objName << " listed as being in memfile but points to nowhere. New Object will be made.");
      s_monObjList.erase(objName);
    }
  }

  MonitoringObject* obj = new MonitoringObject(objName);
  s_monObjList.insert(MonObjList::value_type(objName, obj));
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

TH1* DQMHistAnalysisModule::findHist(const std::string& histname, bool was_updated)
{
  if (s_histList.find(histname) != s_histList.end()) {
    if (was_updated && !s_histList[histname].isUpdated()) return nullptr;
    if (s_histList[histname].getHist()) {
      return s_histList[histname].getHist();
    } else {
      B2ERROR("Histogram " << histname << " in histogram list but nullptr.");
    }
  }
  B2INFO("Histogram " << histname << " not in list.");
  return nullptr;
}

TH1* DQMHistAnalysisModule::findHist(const std::string& dirname, const std::string& histname, bool updated)
{
  if (dirname.size() > 0) {
    return findHist(dirname + "/" + histname, updated);
  }
  return findHist(histname, updated);
}

TH1* DQMHistAnalysisModule::findHistInCanvas(const std::string& histo_name)
{
  // parse the dir+histo name and create the corresponding canvas name
  auto s = StringSplit(histo_name, '/');
  if (s.size() != 2) {
    B2ERROR("findHistInCanvas: histoname not valid (missing dir?), should be 'dirname/histname': " << histo_name);
    return nullptr;
  }
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
  if (s_monObjList.find(objName) != s_monObjList.end()) {
    if (s_monObjList[objName]) {
      //Want to search elsewhere if null-pointer saved in map
      return s_monObjList[objName];
    } else {
      B2ERROR("MonitoringObject " << objName << " listed as being in memfile but points to nowhere.");
    }
  }
  B2INFO("MonitoringObject " << objName << " not in memfile.");
  return NULL;
}

double DQMHistAnalysisModule::getSigma68(TH1* h) const
{
  double probs[2] = {0.16, 1 - 0.16};
  double quant[2] = {0, 0};
  h->GetQuantiles(2, quant, probs);
  const double sigma68 = (-quant[0] + quant[1]) / 2;
  return sigma68;
}

std::vector <std::string> DQMHistAnalysisModule::StringSplit(const std::string& in, const char delim)
{
  std::vector <std::string> out;
  boost::split(out, in, [delim](char c) {return c == delim;});
  return out;
}

void DQMHistAnalysisModule::initHistListBeforeEvent(void)
{
  for (auto& h : s_histList) {
    // attention, we need the reference, otherwise we work on a copy
    h.second.resetBeforeEvent();
  }
  for (auto d : s_deltaList) {
    d.second->setNotUpdated();
  }

  s_canvasUpdatedList.clear();
}

void DQMHistAnalysisModule::clearHistList(void)
{
  s_histList.clear();
}

void DQMHistAnalysisModule::UpdateCanvas(std::string name, bool updated)
{
  s_canvasUpdatedList[name] = updated;
}

void DQMHistAnalysisModule::ExtractRunType(std::vector <TH1*>& hs)
{
  s_runType = "";
  for (size_t i = 0; i < hs.size(); i++) {
    if (hs[i]->GetName() == std::string("DQMInfo/rtype")) {
      s_runType = hs[i]->GetTitle();
      return;
    }
  }
  B2ERROR("ExtractRunType: Histogram \"DQMInfo/rtype\" missing");
}

void DQMHistAnalysisModule::ExtractEvent(std::vector <TH1*>& hs)
{
  s_eventProcessed = 0;
  for (size_t i = 0; i < hs.size(); i++) {
    if (hs[i]->GetName() == std::string("DAQ/Nevent")) {
      s_eventProcessed = hs[i]->GetEntries();
      return;
    }
  }
  B2ERROR("ExtractEvent: Histogram \"DAQ/Nevent\" missing");
}


int DQMHistAnalysisModule::registerEpicsPV(std::string pvname, std::string keyname, bool update_pvs)
{
  if (!m_useEpics) return -1;
#ifdef _BELLE2_EPICS
  if (m_epicsNameToChID[pvname] != nullptr) {
    B2ERROR("Epics PV " << pvname << " already registered!");
    return -1;
  }
  if (keyname != "" && m_epicsNameToChID[keyname] != nullptr) {
    B2ERROR("Epics PV with key " << keyname << " already registered!");
    return -1;
  }

  m_epicsChID.emplace_back();
  auto ptr = &m_epicsChID.back();
  if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
  SEVCHK(ca_create_channel(pvname.data(), NULL, NULL, 10, ptr), "ca_create_channel failure");

  if (update_pvs) SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");

  m_epicsNameToChID[pvname] =  *ptr;
  if (keyname != "") m_epicsNameToChID[keyname] =  *ptr;
  return m_epicsChID.size() - 1; // return index to last added item
#else
  return -1;
#endif
}

void DQMHistAnalysisModule::setEpicsPV(std::string keyname, double value)
{
  if (!m_useEpics || m_epicsReadOnly) return;
#ifdef _BELLE2_EPICS
  if (m_epicsNameToChID[keyname] == nullptr) {
    B2ERROR("Epics PV " << keyname << " not registered!");
    return;
  }
  SEVCHK(ca_put(DBR_DOUBLE, m_epicsNameToChID[keyname], (void*)&value), "ca_set failure");
#endif
}

void DQMHistAnalysisModule::setEpicsPV(std::string keyname, int value)
{
  if (!m_useEpics || m_epicsReadOnly) return;
#ifdef _BELLE2_EPICS
  if (m_epicsNameToChID[keyname] == nullptr) {
    B2ERROR("Epics PV " << keyname << " not registered!");
    return;
  }
  SEVCHK(ca_put(DBR_SHORT, m_epicsNameToChID[keyname], (void*)&value), "ca_set failure");
#endif
}

void DQMHistAnalysisModule::setEpicsPV(int index, double value)
{
  if (!m_useEpics || m_epicsReadOnly) return;
#ifdef _BELLE2_EPICS
  if (index < 0 || index >= (int)m_epicsChID.size()) {
    B2ERROR("Epics PV with " << index << " not registered!");
    return;
  }
  SEVCHK(ca_put(DBR_DOUBLE, m_epicsChID[index], (void*)&value), "ca_set failure");
#endif
}

void DQMHistAnalysisModule::setEpicsPV(int index, int value)
{
  if (!m_useEpics || m_epicsReadOnly) return;
#ifdef _BELLE2_EPICS
  if (index < 0 || index >= (int)m_epicsChID.size()) {
    B2ERROR("Epics PV with " << index << " not registered!");
    return;
  }
  SEVCHK(ca_put(DBR_SHORT, m_epicsChID[index], (void*)&value), "ca_set failure");
#endif
}

chid DQMHistAnalysisModule::getEpicsPVChID(std::string keyname)
{
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (m_epicsNameToChID[keyname] != nullptr) {
      return m_epicsNameToChID[keyname];
    } else {
      B2ERROR("Epics PV " << keyname << " not registered!");
    }
  }
#endif
  return nullptr;
}

chid DQMHistAnalysisModule::getEpicsPVChID(int index)
{
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (index >= 0 && index < (int)m_epicsChID.size()) {
      return m_epicsChID[index];
    } else {
      B2ERROR("Epics PV with " << index << " not registered!");
    }
  }
#endif
  return nullptr;
}

void DQMHistAnalysisModule::updateEpicsPVs(float wait)
{
  if (!m_useEpics) return;
#ifdef _BELLE2_EPICS
  if (wait > 0.) SEVCHK(ca_pend_io(wait), "ca_pend_io failure");
#endif
}

void DQMHistAnalysisModule::cleanupEpicsPVs(void)
{
  // this should be called in terminate function of analysis modules
#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
    for (auto& it : m_epicsChID) SEVCHK(ca_clear_channel(it), "ca_clear_channel failure");
    updateEpicsPVs(5.0);
    // Make sure we clean up both afterwards!
    m_epicsChID.clear();
    m_epicsNameToChID.clear();
  }
#endif
}
