//+
// File : DQMHistOutputToEPICS.cc
// Description : Write Histogram Content to EPICS Arrays
//
// Author : Bjoern Spruck, Univerisity Mainz
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

void DQMHistOutputToEPICSModule::initialize()
{
#ifdef _BELLE2_EPICS
  SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
#endif
  for (auto& it : m_histlist) {
    if (it.size() != 2) {
      B2WARNING("Histolist with wrong nr of parameters " << it.size());
      continue;
    }
#ifdef _BELLE2_EPICS
    auto n = new MYNODE;
    n->histoname = it.at(0);
    SEVCHK(ca_create_channel(it.at(1).c_str(), NULL, NULL, 10, &n->mychid), "ca_create_channel failure");
    pmynode.push_back(n);
#endif
  }

#ifdef _BELLE2_EPICS
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
  B2DEBUG(99, "DQMHistOutputToEPICS: initialized.");
}

void DQMHistOutputToEPICSModule::beginRun()
{
  B2DEBUG(99, "DQMHistOutputToEPICS: beginRun called.");
}

void DQMHistOutputToEPICSModule::event()
{
#ifdef _BELLE2_EPICS
  for (auto& it : pmynode) {
    TH1* hh1 = findHist(it->histoname);
    if (hh1) {
      int length = int(ca_element_count(it->mychid));
      if (length > 0 && hh1->GetNcells() > 2) {
        double* data = new double[length];
        // If bin count doesnt match, we loose bins but otherwise ca_array_put will complain
        // We fill up the array with ZEROs otherwise
        for (int i = 0; i < length ; i++) {
          if (i < hh1->GetNcells() - 2) { // minus under/overflow bin
            data[i] = hh1->GetBinContent(i + 1);
          } else {
            data[i] = 0.0;
          }
        }

        SEVCHK(ca_array_put(DBR_DOUBLE, length, it->mychid, (void*)&data), "ca_set failure");
        delete []data;
      }
    }
  }
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
}

void DQMHistOutputToEPICSModule::terminate()
{
  B2DEBUG(99, "DQMHistOutputToEPICS: terminate called");
}

