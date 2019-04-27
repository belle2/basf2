//+
// File : DQMHistOutputToEPICS.cc
// Description : Write Histogram Content to EPICS Arrays
//
// Author : Bjoern Spruck, University Mainz
// Date : 2019
//-


#include <dqm/analysis/modules/DQMHistOutputToEPICS.h>
#include <TROOT.h>
#include <TClass.h>

using namespace std;
using namespace Belle2;


using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistOutputToEPICS)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistOutputToEPICSModule::DQMHistOutputToEPICSModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

  //Parameter definition
  addParam("HistoList", m_histlist, "histname, pvname");
  B2DEBUG(99, "DQMHistOutputToEPICS: Constructor done.");
}

DQMHistOutputToEPICSModule::~DQMHistOutputToEPICSModule()
{
#ifdef _BELLE2_EPICS
  if (ca_current_context()) ca_context_destroy();
#endif
}

void DQMHistOutputToEPICSModule::initialize()
{
#ifdef _BELLE2_EPICS
  if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
#endif
  for (auto& it : m_histlist) {
    if (it.size() < 2) {
      B2WARNING("Histolist with wrong nr of parameters " << it.size());
      continue;
    }
#ifdef _BELLE2_EPICS
    auto n = new MYNODE;
    n->histoname = it.at(0);
    SEVCHK(ca_create_channel(it.at(1).c_str(), NULL, NULL, 10, &n->mychid), "ca_create_channel failure");
    if (it.size() >= 3) {
      SEVCHK(ca_create_channel(it.at(2).c_str(), NULL, NULL, 10, &n->mychid_last), "ca_create_channel failure");
    } else {
      n->mychid_last = 0;
    }
    pmynode.push_back(n);
#endif
  }

#ifdef _BELLE2_EPICS
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
  B2DEBUG(99, "DQMHistOutputToEPICS: initialized.");
}

void DQMHistOutputToEPICSModule::cleanPVs(void)
{
#ifdef _BELLE2_EPICS
  for (auto* n : pmynode) {
    if (!n->mychid) continue;
    int length = int(ca_element_count(n->mychid));
    if (length > 0) {
      std::vector <double> data(length, 0.0);
      SEVCHK(ca_array_put(DBR_DOUBLE, length, n->mychid, (void*)(data.data())), "ca_put failure");
      if (n->mychid_last) {
        if (length == int(ca_element_count(n->mychid_last))) {
          SEVCHK(ca_array_put(DBR_DOUBLE, length, n->mychid_last, (void*)(data.data())), "ca_put failure");
        }
      }
    }
  }
#endif
}

void DQMHistOutputToEPICSModule::beginRun()
{
  B2DEBUG(99, "DQMHistOutputToEPICS: beginRun called.");
  cleanPVs();
  m_dirty = true;
}

void DQMHistOutputToEPICSModule::event()
{
#ifdef _BELLE2_EPICS
  for (auto& it : pmynode) {
    if (!it->mychid) continue;
    TH1* hh1 = findHist(it->histoname);
    if (hh1) {
      int length = int(ca_element_count(it->mychid));
      if (length > 0 && hh1->GetNcells() > 2) {
        std::vector <double> data(length, 0.0);
        // If bin count doesnt match, we loose bins but otherwise ca_array_put will complain
        // We fill up the array with ZEROs otherwise
        if (hh1->GetDimension() == 1) {
          int i = 0;
          int nx = hh1->GetNbinsX() - 1;
          for (int x = 1; x < nx && i < length ; x++) {
            data[i++] = hh1->GetBinContent(x);
          }

        } else if (hh1->GetDimension() == 2) {
          int i = 0;
          int nx = hh1->GetNbinsX() - 1;
          int ny = hh1->GetNbinsY() - 1;
          for (int y = 1; y < ny && i < length; y++) {
            for (int x = 1; x < nx && i < length ; x++) {
              data[i++] = hh1->GetBinContent(x, y);
            }
          }
        }
        SEVCHK(ca_array_put(DBR_DOUBLE, length, it->mychid, (void*)data.data()), "ca_set failure");
      }
    }
  }
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
}

void DQMHistOutputToEPICSModule::copyToLast(void)
{
  if (!m_dirty) return;
#ifdef _BELLE2_EPICS
  for (auto* n : pmynode) {
    if (n->mychid && n->mychid_last) {
      // copy PVs to last-run-PV if existing
      int length = int(ca_element_count(n->mychid));
      if (length > 0 && length == int(ca_element_count(n->mychid_last))) {
        std::vector <double> data(length, 0.0);
        SEVCHK(ca_array_get(DBR_DOUBLE, length, n->mychid, (void*)(data.data())), "ca_get failure");
        SEVCHK(ca_array_put(DBR_DOUBLE, length, n->mychid_last, (void*)(data.data())), "ca_put failure");
      }
    }
  }
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif

  m_dirty = false;
}

void DQMHistOutputToEPICSModule::endRun()
{
  B2DEBUG(99, "DQMHistOutputToEPICS: endRun called");
  copyToLast();
}

void DQMHistOutputToEPICSModule::terminate()
{
  B2DEBUG(99, "DQMHistOutputToEPICS: terminate called");
  copyToLast();
  // the following belongs to terminate
#ifdef _BELLE2_EPICS
  for (auto* n : pmynode) {
    if (n->mychid) SEVCHK(ca_clear_channel(n->mychid), "ca_clear_channel failure");
    if (n->mychid_last) SEVCHK(ca_clear_channel(n->mychid_last), "ca_clear_channel failure");
  }
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
}

