/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisInputPVSrv.cc
// Description : DQM input module, convert epics PVs to histograms for analysis
//-


#include <framework/core/ModuleParam.templateDetails.h>
#include <dqm/analysis/modules/DQMHistAnalysisInputPVSrv.h>
#include <TSystem.h>
#include <TDirectory.h>
#include <TH1F.h>
#include <TH2F.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisInputPVSrv);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

#ifdef _BELLE2_EPICS
static void printChidInfo(chid ichid, const char* message)
{
  B2DEBUG(20, message);
  B2DEBUG(20, "pv: " << ca_name(ichid) << " type(" << ca_field_type(ichid) << ") nelements(" << ca_element_count(
            ichid) << ") host(" << ca_host_name(ichid)
          << ") read(" << ca_read_access(ichid) << ") write(" << ca_write_access(ichid) << ") state(" << ca_state(ichid) << ")");
}

static void exceptionCallback(struct exception_handler_args args)
{
  chid  ichid = args.chid;
  long  stat = args.stat; /* Channel access status code*/
  const char*  channel;
  const char* noname = "unknown";

  channel = (ichid ? ca_name(ichid) : noname);


  if (ichid) printChidInfo(ichid, "exceptionCallback");
  printf("exceptionCallback stat %s channel %s\n", ca_message(stat), channel);
}

static void connectionCallback(struct connection_handler_args args)
{
  chid  ichid = args.chid;

  printChidInfo(ichid, "connectionCallback");
}

static void accessRightsCallback(struct access_rights_handler_args args)
{
  chid  ichid = args.chid;

  printChidInfo(ichid, "accessRightsCallback");
}
static void eventCallback(struct event_handler_args eha)
{
  chid  ichid = eha.chid;
  MYNODE* n = (MYNODE*)eha.usr;

  if (eha.status != ECA_NORMAL) {
    printChidInfo(ichid, "eventCallback");
  } else {
//       char *pdata = (char *)eha.dbr;
//       printf("Event Callback: %s = %s (%d,%d)\n",ca_name(eha.chid),pdata,(int)eha.type,(int)eha.count);
//       printf("Event Callback: %s (%ld,%s,%ld)\n",ca_name(n->mychid),ca_field_type(n->mychid),dbr_type_to_text(ca_field_type(n->mychid)),ca_element_count(n->mychid));
    n->changed = true;
  }
}

#endif

DQMHistAnalysisInputPVSrvModule::DQMHistAnalysisInputPVSrvModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("RefreshInterval", m_interval, "Refresh interval of histograms in ms", 2000);
  addParam("HistoList", m_histlist, "pvname, histname, histtitle, (bins,min,max[,bins,min,max])");
  addParam("Callback", m_callback, "Using EPICS callback for changes", true);
  addParam("Server", m_server, "Start http server on port 8082", false);
  B2DEBUG(20, "DQMHistAnalysisInputPVSrv: Constructor done.");
}


DQMHistAnalysisInputPVSrvModule::~DQMHistAnalysisInputPVSrvModule()
{
#ifdef _BELLE2_EPICS
  if (ca_current_context()) ca_context_destroy();
#endif
}

void DQMHistAnalysisInputPVSrvModule::initialize()
{
  m_eventMetaDataPtr.registerInDataStore();
  //if (m_server) m_serv = new THttpServer("http:8082");

#ifdef _BELLE2_EPICS
  if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
  SEVCHK(ca_add_exception_event(exceptionCallback, NULL), "ca_add_exception_event");
  for (auto& it : m_histlist) {
    if (it.size() != 4 && it.size() != 5) {
      B2WARNING("Histolist with wrong nr of parameters " << it.size());
      continue;
    }
    auto n = (MYNODE*) callocMustSucceed(1, sizeof(MYNODE), "caMonitor");
    pmynode.push_back(n);

    {
      TDirectory* oldDir = gDirectory;
      TDirectory* d = oldDir;
      TString myl = it.at(1).c_str();
      TString tok;
      Ssiz_t from = 0;
      while (myl.Tokenize(tok, from, "/")) {
        TString dummy;
        Ssiz_t f;
        f = from;
        if (myl.Tokenize(dummy, f, "/")) { // check if its the last one
          TDirectory* e;
          e = d->GetDirectory(tok);
          if (e) {
            B2DEBUG(20, "Cd Dir " << tok);
            d = e;
          } else {
            B2DEBUG(20, "Create Dir " << tok);
            d = d->mkdir(tok);
          }
          d->cd();
        } else {
          break;
        }
      }

      B2DEBUG(20, "Create Histo " << tok);

      Int_t x;
      Double_t xmin, xmax;
      strncpy(n->name, it.at(0).c_str(), MAX_PV_NAME_LEN - 1);
      istringstream is(it.at(3));
      is >> x;
      is >> xmin;
      is >> xmax;
      if (it.size() == 4) {
        n->histo = (TH1*)new TH1F(tok, it.at(2).c_str(), x, xmin, xmax);
        n->binx = x;
        n->biny = 0;
        n->binmax = x;
      } else {
        Int_t y;
        Double_t ymin, ymax;
        istringstream iss(it.at(4));
        iss >> y;
        iss >> ymin;
        iss >> ymax;
        n->histo = (TH1*)new TH2F(tok, it.at(2).c_str(), x, xmin, xmax, y, ymin, ymax);
        n->binx = x;
        n->biny = y;
        n->binmax = x * y;
      }

      // cd back to root directory
      oldDir->cd();
    }

  }

  for (auto n : pmynode) {
    SEVCHK(ca_create_channel(n->name, connectionCallback, n, 20, &n->mychid), "ca_create_channel");
    SEVCHK(ca_replace_access_rights_event(n->mychid, accessRightsCallback), "ca_replace_access_rights_event");
    if (m_callback) {
      SEVCHK(ca_create_subscription(DBR_STRING, 1, n->mychid, DBE_VALUE, eventCallback, n, &n->myevid), "ca_create_subscription");
    }
  }

#endif
  B2DEBUG(20, "DQMHistAnalysisInputPVSrv: initialized.");
}


void DQMHistAnalysisInputPVSrvModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisInputPVSrv: beginRun called.");
}

void DQMHistAnalysisInputPVSrvModule::event()
{
  m_count++;
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(m_expno);
  m_eventMetaDataPtr->setRun(m_runno);
  m_eventMetaDataPtr->setEvent(m_count);

  TTimer t(m_interval, kFALSE);// in ms

#ifdef _BELLE2_EPICS
  SEVCHK(ca_pend_event(0.0001), "ca_pend_event");

  for (auto n : pmynode) {
    if (m_callback && !n->changed) continue;
    n->changed = false;
    if (ca_field_type(n->mychid) != DBF_LONG && ca_field_type(n->mychid) != DBF_FLOAT) continue;

    // FIXME: dbr_size_n is a preprocessor macro, it would be nice replacing it with something better
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    auto bufferorg = new char[dbr_size_n(ca_field_type(n->mychid), ca_element_count(n->mychid))];
#pragma GCC diagnostic pop

    void* buffer = (void*) bufferorg;
    int status;

    status = ca_array_get(ca_field_type(n->mychid), ca_element_count(n->mychid), n->mychid, buffer);
    SEVCHK(status, "ca_array_get()");
    status = ca_pend_io(15.0);
    if (status != ECA_NORMAL) {
      B2WARNING("EPICS ca_array_get " << ca_name(n->mychid) <<  " didn't return a value.");
    } else {
      if (n->histo) {
        // this should always be the case
        unsigned int bins;
        bins = ca_element_count(n->mychid) < n->binmax ? ca_element_count(n->mychid) : n->binmax;
        TH1* histo = n->histo;
        for (unsigned int j = ca_element_count(n->mychid); j < n->binmax; j++) histo->SetBinContent(j + 1, 0); // zero out undefined bins
        switch (ca_field_type(n->mychid)) {
          case DBF_CHAR: {
            dbr_char_t* b = (dbr_char_t*)buffer;
            for (unsigned int j = 0; j < bins; j++) {
              histo->SetBinContent(j + 1, b[j]);
            }
          }; break;
//         case DBF_INT:
          case DBF_SHORT: { // same as INT
            dbr_short_t* b = (dbr_short_t*)buffer;
            for (unsigned int j = 0; j < bins; j++) {
              histo->SetBinContent(j + 1, b[j]);
            }
          }; break;
          case DBF_LONG: {
            dbr_long_t* b = (dbr_long_t*)buffer;
            for (unsigned int j = 0; j < bins; j++) {
              histo->SetBinContent(j + 1, b[j]);
            }
          }; break;
          case DBF_FLOAT: {
            dbr_float_t* b = (dbr_float_t*)buffer;
            for (unsigned int j = 0; j < bins; j++) {
              histo->SetBinContent(j + 1, b[j]);
            }
          }; break;
          case DBF_DOUBLE: {
            dbr_double_t* b = (dbr_double_t*)buffer;
            for (unsigned int j = 0; j < bins; j++) {
              histo->SetBinContent(j + 1, b[j]);
            }
          }; break;
          default:
            // type not supported
            break;
        }
      }
    }
    delete[] bufferorg;
  }
#endif
  do { // call at least once!
    //if (m_serv) m_serv->ProcessRequests();
    gSystem->Sleep(10);  // 10 ms sleep
  } while (!t.CheckTimer(gSystem->Now()));

}

void DQMHistAnalysisInputPVSrvModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisInputPVSrv: endRun called");
}


void DQMHistAnalysisInputPVSrvModule::terminate()
{
  B2DEBUG(20, "DQMHistAnalysisInputPVSrv: terminate called");
}



