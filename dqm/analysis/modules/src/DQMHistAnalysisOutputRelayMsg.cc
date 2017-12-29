//+
// File : DQMHistAnalysisOutputRelayMsg.cc
// Description :
//
// Author : B. Spruck
// Date : 25 - Mar - 2017
// based on wrok from Tomoyuki Konno, Tokyo Metropolitan Univerisity
//-


#include <dqm/analysis/modules/DQMHistAnalysisOutputRelayMsg.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TClass.h>
#include <TObject.h>
#include <TCanvas.h>
#include <TMessage.h>
#include "TKey.h"
#include "TIterator.h"

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisOutputRelayMsg)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisOutputRelayMsgModule::DQMHistAnalysisOutputRelayMsgModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("Hostname", m_hostname, "Hostname of THTTP", std::string("localhost"));
  addParam("Port", m_port, "Port number to THTTP", 9191);
  B2DEBUG(1, "DQMHistAnalysisOutputRelayMsg: Constructor done.");
}


DQMHistAnalysisOutputRelayMsgModule::~DQMHistAnalysisOutputRelayMsgModule() { }

void DQMHistAnalysisOutputRelayMsgModule::initialize()
{
  m_sock = new TSocket(m_hostname.c_str(), m_port);
  B2INFO("DQMHistAnalysisOutputRelayMsg: initialized.");
}


void DQMHistAnalysisOutputRelayMsgModule::beginRun()
{
  B2INFO("DQMHistAnalysisOutputRelayMsg: beginRun called.");
}


void DQMHistAnalysisOutputRelayMsgModule::event()
{
  B2INFO("DQMHistAnalysisOutputRelayMsg: event called.");
  TMessage mess(kMESS_OBJECT);

  TIter nextkey(gROOT->GetListOfCanvases());
  TObject* obj = 0;
  while ((obj = (TObject*)nextkey())) {
    if (obj->IsA()->InheritsFrom("TCanvas")) {
      TCanvas* c = (TCanvas*) obj;
      mess.Reset();
      mess.WriteObject(c);     // write object in message buffer
      m_sock->Send(mess);
    }
  }
}

void DQMHistAnalysisOutputRelayMsgModule::endRun()
{
  B2INFO("DQMHistAnalysisOutputRelayMsg: endRun called");
}


void DQMHistAnalysisOutputRelayMsgModule::terminate()
{
  B2INFO("DQMHistAnalysisOutputRelayMsg: terminate called");
  delete m_sock;
}

