//+
// File : DqmHistoManager.cc
// Description : A module to manager histograms/ntuples/ttrees for dqm
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 27 - Feb - 2013
//-

#include <daq/dqm/modules/DqmHistoManagerModule.h>

#include "TText.h"

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DqmHistoManager)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// Implementations
DqmHistoManagerModule::DqmHistoManagerModule() : Module(), m_initmain(false), m_initialized(false)
{
  // Module description
  setDescription("Module to manage histograms/Ntuples/TTrees");
  setPropertyFlags(Module::c_HistogramManager);

  // Parameters
  addParam("HistoFileName", m_histfile, "Name of histogram output file.", string("histofile.root"));
  addParam("HostName", m_hostname, "Name of host to send histograms", string("localhost"));
  addParam("Port", m_port, "Socket port number to connect", DQM_SOCKET);
  addParam("DumpInterval", m_interval, "Interval to dump histos", 1000);
}

DqmHistoManagerModule::~DqmHistoManagerModule()
{
  //  if (m_initmain) {
  //  if (ProcHandler::EvtProcID() == -1) {   // should be called from main proc.
  //    cout << "DqmHistoManager:: destructor called from pid=" << ProcHandler::EvtProcID() << endl;
  //    if (Environment::Instance().getNumberProcesses() > 0 && ProcHandler::EvtProcID() == -1) {
  if (Environment::Instance().getNumberProcesses() > 0) {
    cout << "DqmHistoManager:: adding histogram files" << endl;
    RbTupleManager::Instance().hadd();
    cout << "DqmHistoManager:: adding histogram files done" << endl;
  }
  //  }
}

void DqmHistoManagerModule::initialize()
{
  RbTupleManager::Instance().init(Environment::Instance().getNumberProcesses(), m_histfile.c_str());

  m_initmain = true;
  //  cout << "DqmHistoManager::initialization done" << endl;

  // Connect to Histogram Server
  m_sock = new EvtSocketSend(m_hostname, m_port);
  printf("EvtSocketSend : fd = %d\n", (m_sock->sock())->sock());

  // Message Handler
  m_msg = new MsgHandler(0);    // Compression level = 0

  // Clear event counter
  m_nevent = 0;

}

void DqmHistoManagerModule::beginRun()
{
  if (!m_initialized) {
    //    cout << "DqmHistoManager:: first pass in beginRun() : proc="
    //   << ProcHandler::EvtProcID() << endl;
    RbTupleManager::Instance().begin(ProcHandler::EvtProcID());
    m_initialized = true;
  }
}

void DqmHistoManagerModule::endRun()
{
  if (!m_initialized) {
    //    cout << "DqmHistoManager:: first pass in endRun(): proc="
    //   << ProcHandler::EvtProcID() << endl;
    RbTupleManager::Instance().begin(ProcHandler::EvtProcID());
    m_initialized = true;
  }
}

void DqmHistoManagerModule::event()
{
  if (!m_initialized) {
    //    cout << "DqmHistoManager:: first pass in event() : proc="
    //   << ProcHandler::EvtProcID() << endl;
    RbTupleManager::Instance().begin(ProcHandler::EvtProcID());
    m_initialized = true;
  }
  if (m_nevent % m_interval == 0) {
    //    printf ( "DqmHistoManager: event = %d\n", m_nevent );
    //    printf ( "DqmHistoManger: dumping histos.....\n" );
    m_msg->clear();
    m_nobjs = 0;

    // Stream histograms with directory structure
    StreamHistograms(gDirectory, m_msg);

    EvtMessage* msg = m_msg->encode_msg(MSG_EVENT);
    //    printf ( "Message Size = %d\n", msg->size() );

    (msg->header())->reserved[0] = 0;
    (msg->header())->reserved[1] = m_nobjs;
    (msg->header())->reserved[2] = 0;

    if (m_nobjs > 0) {
      printf("DqmHistoManger: dumping histos.....%d histos\n",
             m_nobjs);
      m_sock->send(msg);
    }

    delete(msg);

  }
  m_nevent++;

}

void DqmHistoManagerModule::terminate()
{
  if (m_initialized) {
    //    cout << "DqmHistoManager::terminating event process : PID=" << ProcHandler::EvtProcID() << endl;
    m_msg->clear();

    m_nobjs = 0;
    StreamHistograms(gDirectory, m_msg);

    printf("terminate : m_nobjs = %d\n", m_nobjs);
    EvtMessage* msg = m_msg->encode_msg(MSG_EVENT);
    (msg->header())->reserved[0] = 0;
    (msg->header())->reserved[1] = m_nobjs;
    (msg->header())->reserved[2] = 0;

    m_sock->send(msg);

    delete(msg);

    //    RbTupleManager::Instance().terminate();
    //    delete m_sock;
    //    delete m_msg;
  }
}

int DqmHistoManagerModule::StreamHistograms(TDirectory* curdir, MsgHandler* msg)
{
  TList* keylist = curdir->GetList();
  //    keylist->ls();

  TIter nextkey(keylist);
  TKey* key = 0;
  int nkeys = 0;
  int nobjs = 0;
  while ((key = (TKey*)nextkey())) {
    nkeys++;
    TObject* obj = curdir->FindObject(key->GetName());
    if (obj->IsA()->InheritsFrom("TH1")) {
      TH1* h1 = (TH1*) obj;
      //      printf ( "Key = %s, entry = %f\n", key->GetName(), h1->GetEntries() );
      m_msg->add(h1, h1->GetName());
      nobjs++;
      m_nobjs++;
    } else if (obj->IsA()->InheritsFrom(TDirectory::Class())) {
      //      printf ( "New directory found  %s, Go into subdir\n", obj->GetName() );
      TDirectory* tdir = (TDirectory*) obj;
      //      m_msg->add(tdir, tdir->GetName());
      TText subdir(0, 0, tdir->GetName());
      m_msg->add(&subdir, "SUBDIR:" + string(obj->GetName())) ;
      nobjs++;
      m_nobjs++;
      tdir->cd();
      StreamHistograms(tdir , msg);
      TText command(0, 0, "COMMAND:EXIT");
      m_msg->add(&command, "SUBDIR:EXIT");
      nobjs++;
      m_nobjs++;
      curdir->cd();
    }
  }
}




