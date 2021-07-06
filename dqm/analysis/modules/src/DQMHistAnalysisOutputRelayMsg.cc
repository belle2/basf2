/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisOutputRelayMsg.cc
// Description : DQM Output, send Canvases to jsroot server.
//-


#include <dqm/analysis/modules/DQMHistAnalysisOutputRelayMsg.h>
#include <TROOT.h>
#include <TClass.h>
#include <TObject.h>
#include <TCanvas.h>
#include <TMessage.h>
#include <ctime>

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
  B2DEBUG(20, "DQMHistAnalysisOutputRelayMsg: Constructor done.");
}


DQMHistAnalysisOutputRelayMsgModule::~DQMHistAnalysisOutputRelayMsgModule() { }

void DQMHistAnalysisOutputRelayMsgModule::initialize()
{
  if (m_sock != nullptr) delete m_sock;
  m_sock = new TSocket(m_hostname.c_str(), m_port);
  B2DEBUG(20, "DQMHistAnalysisOutputRelayMsg: initialized.");
}


void DQMHistAnalysisOutputRelayMsgModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisOutputRelayMsg: beginRun called.");
}


void DQMHistAnalysisOutputRelayMsgModule::event()
{
  B2DEBUG(20, "DQMHistAnalysisOutputRelayMsg: event called.");
  TMessage mess(kMESS_OBJECT);

  TSeqCollection* seq = gROOT->GetListOfCanvases();
  TIter nextkey(seq);
  TObject* obj = 0;

  time_t now = time(0);
  char mbstr[100];
  strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));

  B2INFO("[" << mbstr << "] before sending " << seq->GetEntries() << " objects.");
  bool first_try = true;
  while ((obj = (TObject*)nextkey())) {
    if (obj->IsA()->InheritsFrom("TCanvas")) {
      TCanvas* c = (TCanvas*) obj;
      mess.Reset();
      mess.WriteObject(c);     // write object in message buffer
      if (m_sock->Send(mess) < 0) {
        if (!first_try) {
          break;//Only try to reconnect once per event
        } else {
          first_try = false;
        }
        //The plain TSocket can't reconnect, so delete and create a new one
        delete m_sock;
        m_sock = new TSocket(m_hostname.c_str(), m_port);
        //Try to send the failed message again
        if (m_sock->Send(mess) < 0) {
          //If failed a second time stop for this event
          break;
        }
      }
    }
  }
  now = time(0);
  strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));
  B2INFO("[" << mbstr << "] after sending " << seq->GetEntries() << " objects.");
}

void DQMHistAnalysisOutputRelayMsgModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisOutputRelayMsg: endRun called");
}


void DQMHistAnalysisOutputRelayMsgModule::terminate()
{
  B2DEBUG(20, "DQMHistAnalysisOutputRelayMsg: terminate called");
  delete m_sock;
}

