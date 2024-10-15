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
DQMHistAnalysisModule::RefList DQMHistAnalysisModule::s_refList;
#ifdef _BELLE2_EPICS
std::vector <chid>  DQMHistAnalysisModule::m_epicsChID;
#endif

bool DQMHistAnalysisModule::m_useEpics = false; // default to false, to enable EPICS, add special EPICS Module class into chain
bool DQMHistAnalysisModule::m_epicsReadOnly =
  false; // special for second "online" use (reading limits). default to false, to enable EPICS, add special EPICS Module parameter
std::string DQMHistAnalysisModule::m_PVPrefix = "TEST:"; // default to "TEST:", for production, set in EPICS enabler to e.g. "DQM:"

DQMHistAnalysisModule::DQMHistAnalysisModule() : Module()
{
  //Set module properties
  setDescription("Histogram Analysis module base class");
}

void DQMHistAnalysisModule::clearlist()
{
  s_histList.clear();
  s_refList.clear();
// s_monObjList;
  s_deltaList.clear();
  s_canvasUpdatedList.clear();
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
      it->second.update(h); // update
    }
    return true; // histogram changed
  }

  return false; // histogram didn't change
}

// void DQMHistAnalysisModule::addRef(const std::string& dirname, const std::string& histname, TH1* ref)
// {
//   std::string fullname;
//   if (dirname.size() > 0) {
//     fullname = dirname + "/" + histname;
//   } else {
//     fullname = histname;
//   }
//   auto it = s_refList.find(fullname);
//   if (it == s_refList.end()) {
//     B2DEBUG(1, "Did not find histogram " << fullname << "in s_refList, so inserting now.");
//     s_refList.insert({fullname, ref});
//   }
// }

void DQMHistAnalysisModule::addDeltaPar(const std::string& dirname, const std::string& histname, HistDelta::EDeltaType t, int p,
                                        unsigned int a)
{
  std::string fullname;
  if (dirname.size() > 0) {
    fullname = dirname + "/" + histname;
  } else {
    fullname = histname;
  }
  s_deltaList[fullname].set(t, p, a);
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
    return it->second.getDelta(n, onlyIfUpdated);
  }
  B2WARNING("Delta hist " << fullname << " not found");
  return nullptr;
}

MonitoringObject* DQMHistAnalysisModule::getMonitoringObject(const std::string& objName)
{
  auto obj = &s_monObjList[objName];
  obj->SetName(objName.c_str());
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

TH1* DQMHistAnalysisModule::scaleReference(ERefScaling scaling, const TH1* hist, TH1* ref)
{
  // if hist/ref is nullptr, nothing to do
  if (!hist || !ref)
    return ref;

  switch (scaling) {
    // default: do nothing
    // case ERefScaling::c_RefScaleNone: //do nothing
    case ERefScaling::c_RefScaleEntries: // Integral
      // only if we have entries in reference
      if (hist->Integral() != 0 and ref->Integral() != 0) {
        ref->Scale(hist->Integral() / ref->Integral());
      }
      break;
    case ERefScaling::c_RefScaleMax: // Maximum
      // only if we have entries in reference
      if (hist->GetMaximum() != 0 and ref->GetMaximum() != 0) {
        ref->Scale(hist->GetMaximum() / ref->GetMaximum());
      }
      break;
  }
  return ref;
}

TH1* DQMHistAnalysisModule::findRefHist(const std::string& histname, ERefScaling scaling, const TH1* hist)
{
  if (s_refList.find(histname) != s_refList.end()) {
    // get a copy of the reference which we can modify
    // (it is still owned and managed by the framework)
    // then do the scaling
    return scaleReference(scaling, hist, s_refList[histname].getReference());
  }
  B2INFO("Ref Histogram " << histname << " not in list.");
  return nullptr;
}

TH1* DQMHistAnalysisModule::findRefHist(const std::string& dirname, const std::string& histname, ERefScaling scaling,
                                        const TH1* hist)
{
  if (dirname.size() > 0) {
    return findRefHist(dirname + "/" + histname, scaling, hist);
  }
  return findRefHist(histname, scaling, hist);
}

TH1* DQMHistAnalysisModule::findHistInCanvas(const std::string& histo_name, TCanvas** cobj)
{
  TCanvas* cnv = nullptr;
  // try to get canvas from outside
  if (cobj) cnv = *cobj;
  // if no canvas search for it
  if (cnv == nullptr) {
    // parse the dir+histo name and create the corresponding canvas name
    auto s = StringSplit(histo_name, '/');
    if (s.size() != 2) {
      B2ERROR("findHistInCanvas: histoname not valid (missing dir?), should be 'dirname/histname': " << histo_name);
      return nullptr;
    }
    auto dirname = s.at(0);
    auto hname = s.at(1);
    std::string canvas_name = dirname + "/c_" + hname;
    cnv = findCanvas(canvas_name);
    // set canvas pointer for outside
    if (cnv && cobj) *cobj = cnv;
  }

  // get histogram pointer
  if (cnv != nullptr) {
    TIter nextkey(cnv->GetListOfPrimitives());
    TObject* obj{};
    while ((obj = dynamic_cast<TObject*>(nextkey()))) {
      if (obj->IsA()->InheritsFrom("TH1")) {
        if (obj->GetName() == histo_name)
          return  dynamic_cast<TH1*>(obj);
      }
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
    return &s_monObjList[objName];
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

void DQMHistAnalysisModule::clearCanvases(void)
{
  TIter nextckey(gROOT->GetListOfCanvases());
  TObject* cobj = nullptr;

  while ((cobj = dynamic_cast<TObject*>(nextckey()))) {
    if (cobj->IsA()->InheritsFrom("TCanvas")) {
      TCanvas* cnv = dynamic_cast<TCanvas*>(cobj);
      cnv->Clear();
      colorizeCanvas(cnv, c_StatusDefault);
    }
  }
}

void DQMHistAnalysisModule::initHistListBeforeEvent(void)
{
  for (auto& it : s_histList) {
    // attention, we must use reference, otherwise we work on a copy
    it.second.resetBeforeEvent();
  }
  for (auto& it : s_deltaList) {
    // attention, we must use reference, otherwise we work on a copy
    it.second.setNotUpdated();
  }

  s_canvasUpdatedList.clear();
}

void DQMHistAnalysisModule::clearHistList(void)
{
  s_histList.clear();
}

void DQMHistAnalysisModule::clearRefList(void)
{
  s_refList.clear();
}

void DQMHistAnalysisModule::resetDeltaList(void)
{
  for (auto& d : s_deltaList) {
    d.second.reset();
  }
}

void DQMHistAnalysisModule::UpdateCanvas(std::string name, bool updated)
{
  s_canvasUpdatedList[name] = updated;
}

void DQMHistAnalysisModule::UpdateCanvas(TCanvas* c, bool updated)
{
  if (c) UpdateCanvas(c->GetName(), updated);
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

int DQMHistAnalysisModule::registerEpicsPV(std::string pvname, std::string keyname)
{
  return registerEpicsPVwithPrefix(m_PVPrefix, pvname, keyname);
}

int DQMHistAnalysisModule::registerExternalEpicsPV(std::string pvname, std::string keyname)
{
  return registerEpicsPVwithPrefix(std::string(""), pvname, keyname);
}

int DQMHistAnalysisModule::registerEpicsPVwithPrefix(std::string prefix, std::string pvname, std::string keyname)
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
  // the subscribed name includes the prefix, the map below does *not*
  CheckEpicsError(ca_create_channel((prefix + pvname).data(), NULL, NULL, 10, ptr), "ca_create_channel failure", pvname);

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
  CheckEpicsError(ca_put(DBR_DOUBLE, m_epicsNameToChID[keyname], (void*)&value), "ca_set failure", keyname);
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
  CheckEpicsError(ca_put(DBR_SHORT, m_epicsNameToChID[keyname], (void*)&value), "ca_set failure", keyname);
#endif
}

void DQMHistAnalysisModule::setEpicsStringPV(std::string keyname, std::string value)
{
  if (!m_useEpics || m_epicsReadOnly) return;
#ifdef _BELLE2_EPICS
  if (m_epicsNameToChID[keyname] == nullptr) {
    B2ERROR("Epics PV " << keyname << " not registered!");
    return;
  }
  if (value.length() > 40) {
    B2ERROR("Epics string PV " << keyname << " too long (>40 characters)!");
    return;
  }
  char text[40];
  strcpy(text, value.c_str());
  CheckEpicsError(ca_put(DBR_STRING, m_epicsNameToChID[keyname], text), "ca_set failure", keyname);
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
  CheckEpicsError(ca_put(DBR_DOUBLE, m_epicsChID[index], (void*)&value), "ca_set failure", m_epicsChID[index]);
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
  CheckEpicsError(ca_put(DBR_SHORT, m_epicsChID[index], (void*)&value), "ca_set failure", m_epicsChID[index]);
#endif
}

void DQMHistAnalysisModule::setEpicsStringPV(int index, std::string value)
{
  if (!m_useEpics || m_epicsReadOnly) return;
#ifdef _BELLE2_EPICS
  if (index < 0 || index >= (int)m_epicsChID.size()) {
    B2ERROR("Epics PV with " << index << " not registered!");
    return;
  }
  char text[41];
  strncpy(text, value.c_str(), 40);
  text[40] = 0;
  CheckEpicsError(ca_put(DBR_STRING, m_epicsChID[index], text), "ca_set failure", m_epicsChID[index]);
#endif
}

double DQMHistAnalysisModule::getEpicsPV(std::string keyname)
{
  double value{NAN};
  if (!m_useEpics) return value;
#ifdef _BELLE2_EPICS
  if (m_epicsNameToChID[keyname] == nullptr) {
    B2ERROR("Epics PV " << keyname << " not registered!");
    return value;
  }
  // From EPICS doc. When ca_get or ca_array_get are invoked the returned channel value can't be assumed to be stable
  // in the application supplied buffer until after ECA_NORMAL is returned from ca_pend_io. If a connection is lost
  // outstanding get requests are not automatically reissued following reconnect.
  auto r = ca_get(DBR_DOUBLE, m_epicsNameToChID[keyname], (void*)&value);
  if (r == ECA_NORMAL) r = ca_pend_io(5.0); // this is needed!
  if (r == ECA_NORMAL) {
    return value;
  } else {
    CheckEpicsError(r, "Read PV failed in ca_get or ca_pend_io failure", keyname);
  }
#endif
  return NAN;
}

double DQMHistAnalysisModule::getEpicsPV(int index)
{
  double value{NAN};
  if (!m_useEpics) return value;
#ifdef _BELLE2_EPICS
  if (index < 0 || index >= (int)m_epicsChID.size()) {
    B2ERROR("Epics PV with " << index << " not registered!");
    return value;
  }
  // From EPICS doc. When ca_get or ca_array_get are invoked the returned channel value can't be assumed to be stable
  // in the application supplied buffer until after ECA_NORMAL is returned from ca_pend_io. If a connection is lost
  // outstanding get requests are not automatically reissued following reconnect.
  auto r = ca_get(DBR_DOUBLE, m_epicsChID[index], (void*)&value);
  if (r == ECA_NORMAL) r = ca_pend_io(5.0); // this is needed!
  if (r == ECA_NORMAL) {
    return value;
  } else {
    CheckEpicsError(r, "Read PV failed in ca_get or ca_pend_io failure", m_epicsChID[index]);
  }
#endif
  return NAN;
}

std::string DQMHistAnalysisModule::getEpicsStringPV(std::string keyname, bool& status)
{
  status = false;
  char value[40] = "";
  if (!m_useEpics) return std::string(value);
#ifdef _BELLE2_EPICS
  if (m_epicsNameToChID[keyname] == nullptr) {
    B2ERROR("Epics PV " << keyname << " not registered!");
    return std::string(value);
  }
  // From EPICS doc. When ca_get or ca_array_get are invoked the returned channel value can't be assumed to be stable
  // in the application supplied buffer until after ECA_NORMAL is returned from ca_pend_io. If a connection is lost
  // outstanding get requests are not automatically reissued following reconnect.
  auto r = ca_get(DBR_STRING, m_epicsNameToChID[keyname], value);
  if (r == ECA_NORMAL) r = ca_pend_io(5.0); // this is needed!
  if (r == ECA_NORMAL) {
    status = true;
    return std::string(value);
  } else {
    CheckEpicsError(r, "Read PV (string) failed in ca_get or ca_pend_io failure", keyname);
  }
#endif
  return std::string(value);
}

std::string DQMHistAnalysisModule::getEpicsStringPV(int index, bool& status)
{
  status = false;
  char value[40] = "";
  if (!m_useEpics) return std::string(value);
#ifdef _BELLE2_EPICS
  if (index < 0 || index >= (int)m_epicsChID.size()) {
    B2ERROR("Epics PV with " << index << " not registered!");
    return std::string(value);
  }
  // From EPICS doc. When ca_get or ca_array_get are invoked the returned channel value can't be assumed to be stable
  // in the application supplied buffer until after ECA_NORMAL is returned from ca_pend_io. If a connection is lost
  // outstanding get requests are not automatically reissued following reconnect.
  auto r = ca_get(DBR_DOUBLE, m_epicsChID[index], value);
  if (r == ECA_NORMAL) r = ca_pend_io(5.0); // this is needed!
  if (r == ECA_NORMAL) {
    status = true;
    return std::string(value);
  } else {
    CheckEpicsError(r, "Read PV (string) failed in ca_get or ca_pend_io failure", m_epicsChID[index]);
  }
#endif
  return std::string(value);
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

int DQMHistAnalysisModule::updateEpicsPVs(float wait)
{
  int state = ECA_NORMAL;
  if (!m_useEpics) return state;
#ifdef _BELLE2_EPICS
  if (wait > 0.) {
    state = ca_pend_io(wait);
    SEVCHK(state, "ca_pend_io failure");
  }
#endif
  return state;
}

void DQMHistAnalysisModule::cleanupEpicsPVs(void)
{
  // this should be called in terminate function of analysis modules
#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
    for (auto& it : m_epicsChID) CheckEpicsError(ca_clear_channel(it), "ca_clear_channel failure", it);
    updateEpicsPVs(5.0);
    // Make sure we clean up both afterwards!
    m_epicsChID.clear();
    m_epicsNameToChID.clear();
  }
#endif
}

bool DQMHistAnalysisModule::requestLimitsFromEpicsPVs(std::string name, double& lowerAlarm, double& lowerWarn, double& upperWarn,
                                                      double& upperAlarm)
{
  return requestLimitsFromEpicsPVs(getEpicsPVChID(name), lowerAlarm, lowerWarn, upperWarn, upperAlarm);
}

bool DQMHistAnalysisModule::requestLimitsFromEpicsPVs(int index, double& lowerAlarm, double& lowerWarn, double& upperWarn,
                                                      double& upperAlarm)
{
  return requestLimitsFromEpicsPVs(getEpicsPVChID(index), lowerAlarm, lowerWarn, upperWarn, upperAlarm);
}

bool DQMHistAnalysisModule::requestLimitsFromEpicsPVs(chid pv, double& lowerAlarm, double& lowerWarn, double& upperWarn,
                                                      double& upperAlarm)
{
  // get warn and error limit only if pv exists
  // overwrite only if limit is defined (not NaN)
  // user should initialize with NaN before calling, unless
  // some "default" values should be set otherwise
  if (pv != nullptr) {
    struct dbr_ctrl_double tPvData;
    // From EPICS doc. When ca_get or ca_array_get are invoked the returned channel value can't be assumed to be stable
    // in the application supplied buffer until after ECA_NORMAL is returned from ca_pend_io. If a connection is lost
    // outstanding get requests are not automatically reissued following reconnect.
    auto r = ca_get(DBR_CTRL_DOUBLE, pv, &tPvData);
    if (r == ECA_NORMAL) r = ca_pend_io(5.0); // this is needed!
    if (r == ECA_NORMAL) {
      if (!std::isnan(tPvData.lower_alarm_limit)) {
        lowerAlarm = tPvData.lower_alarm_limit;
      }
      if (!std::isnan(tPvData.lower_warning_limit)) {
        lowerWarn = tPvData.lower_warning_limit;
      }
      if (!std::isnan(tPvData.upper_warning_limit)) {
        upperWarn = tPvData.upper_warning_limit;
      }
      if (!std::isnan(tPvData.upper_alarm_limit)) {
        upperAlarm = tPvData.upper_alarm_limit;
      }
      return true;
    } else {
      CheckEpicsError(r, "Reading PV Limits failed in ca_get or ca_pend_io failure", pv);
    }
  }
  return false;
}

DQMHistAnalysisModule::EStatus DQMHistAnalysisModule::makeStatus(bool enough, bool warn_flag, bool error_flag)
{
  // white color is the default, if no colorize
  if (!enough) {
    return (c_StatusTooFew);
  } else {
    if (error_flag) {
      return (c_StatusError);
    } else if (warn_flag) {
      return (c_StatusWarning);
    } else {
      return (c_StatusGood);
    }
  }

  return (c_StatusDefault); // default, but should not be reached
}

DQMHistAnalysisModule::EStatusColor DQMHistAnalysisModule::getStatusColor(EStatus stat)
{
  // white color is the default, if no colorize
  EStatusColor color = c_ColorDefault;
  switch (stat) {
    case c_StatusTooFew:
      color = c_ColorTooFew; // Magenta or Gray
      break;
    case c_StatusDefault:
      color = c_ColorDefault; // default no colors
      break;
    case c_StatusGood:
      color = c_ColorGood; // Good
      break;
    case c_StatusWarning:
      color = c_ColorWarning; // Warning
      break;
    case c_StatusError:
      color = c_ColorError; // Severe
      break;
    default:
      color = c_ColorDefault; // default no colors
      break;
  }
  return color;
}

void DQMHistAnalysisModule::colorizeCanvas(TCanvas* canvas, EStatus stat)
{
  if (!canvas) return;
  auto color = getStatusColor(stat);

  canvas->Pad()->SetFillColor(color);

  canvas->Pad()->SetFrameFillColor(10); // White (kWhite is not used since it results in transparent!)
  canvas->Pad()->SetFrameFillStyle(1001);// White
  canvas->Pad()->Modified();
  canvas->Pad()->Update();
}

void DQMHistAnalysisModule::checkPVStatus(void)
{
  B2INFO("Check PV Connections");

  for (auto& it : m_epicsChID) {
    printPVStatus(it);
  }
  B2INFO("Check PVs done");
}

void DQMHistAnalysisModule::printPVStatus(chid pv, bool onlyError)
{
  if (pv == nullptr) {
    B2WARNING("PV chid was nullptr");
    return;
  }
  auto state = ca_state(pv);
  switch (state) {
    case cs_never_conn: /* valid chid, server not found or unavailable */
      B2WARNING("Channel never connected " << ca_name(pv));
      break;
    case cs_prev_conn:  /* valid chid, previously connected to server */
      B2WARNING("Channel was connected, but now is not " << ca_name(pv));
      break;
    case cs_closed:   /* channel deleted by user */
      B2WARNING("Channel deleted already " << ca_name(pv));
      break;
    case cs_conn:       /* valid chid, connected to server */
      if (!onlyError) B2INFO("Channel connected and OK " << ca_name(pv));
      break;
    default:
      B2WARNING("Undefined status for channel " << ca_name(pv));
      break;
  }
}

void DQMHistAnalysisModule::CheckEpicsError(int state, const std::string& message, const std::string& name)
{
  if (state != ECA_NORMAL) {
    B2WARNING(message << ": " << name);
    printPVStatus(m_epicsNameToChID[name], false);
  }
}

void DQMHistAnalysisModule::CheckEpicsError(int state, const std::string& message, chid id = nullptr)
{
  if (state != ECA_NORMAL) {
    std::string name;
    if (id) name =  ca_name(id);
    B2WARNING(message << ": " << name);
    printPVStatus(id, false);
  }
}

