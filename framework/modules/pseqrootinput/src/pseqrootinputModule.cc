//+
// File : pseqrootoutput.cc
// Description : Sequential ROOT output module for pbasf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <framework/modules/pseqrootinput/pseqrootinputModule.h>
#include <framework/core/Environment.h>
#include <stdlib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(pSeqRootInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

pSeqRootInputModule::pSeqRootInputModule() : pEventServer()
{
  //Set module properties
  setDescription("SeqROOT output with parallel capability");
  setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  m_nsent = 0;
  m_nrecv = 0;

  m_file = 0;

  //Parameter definition
  addParam("inputFileName"  , m_inputFileName, "SeqRoot file name.", string("pSeqRootInput.root"));
  addParam("compressionLevel", m_compressionLevel, "Compression Level: 0 for no, 1 for low, 9 for high compression.", 1);

  B2INFO("pSeqRootInput: Constructor done.");
}


pSeqRootInputModule::~pSeqRootInputModule()
{
}

void pSeqRootInputModule::initialize()
{

  // get iterators
  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ii++) {
    m_obj_iter[ii]   = DataStore::Instance().getObjectIterator(static_cast<DataStore::EDurability>(ii));
    m_array_iter[ii] = DataStore::Instance().getArrayIterator(static_cast<DataStore::EDurability>(ii));
    //    m_done[ii]     = false;
  }

  // Attach to ring buffer if nprocess > 0
  m_nproc = Environment::Instance().getNumberProcesses();

  //  printf ( "pRootInput : nproc = %d\n", m_nproc );
  B2WARNING("pRootInput : nproc = " << m_nproc)
  if (m_nproc > 0) {
    char temp[] = "PROUTXXXXXX";
    char* rbufname = mktemp(temp);
    m_rbuf = new RingBuffer(rbufname, RINGBUF_SIZE);
    m_rbctl = new RbCtlMgr();
  } else {
    m_file = new SeqFile(m_inputFileName.c_str(), "r");
    m_rbuf = NULL;
    m_rbctl = NULL;
  }
  // Message handler to encode serialized object
  m_msghandler = new MsgHandler(m_compressionLevel);

  B2INFO("pSeqRootInput initialized.");
}


void pSeqRootInputModule::beginRun()
{
  B2INFO("beginRun called.");
}


void pSeqRootInputModule::event()
{
  m_msghandler->clear();

  // Get a SeqRoot record from the file
  char* evtbuf = new char[MAXEVTSIZE];
  EvtMessage* evtmsg;
  int size;
  if (m_nproc == 0) {   // Single process mode
    size = m_file->read(evtbuf, MAXEVTSIZE);
    if (size == 0) {
      delete m_file;
      return;
    } else {
      evtmsg = new EvtMessage(evtbuf);
    }
  } else {
    while ((size = m_rbuf->remq((int*)evtbuf)) == 0) {
      usleep(100);
    }
    evtmsg = new EvtMessage(evtbuf);
    if (evtmsg->type() == MSG_TERMINATE) {
      return;
    }
  }

  // Process synchronization for begin/end run recods (parallel case)
  if (m_nproc > 0 &&
      (evtmsg->type() == MSG_BEGIN_RUN || evtmsg->type() == MSG_END_RUN)) {
    // Notify event server of arrival of begin/end run and synchronize
    m_rbctl->set_flag(ProcHandler::EvtProcID(), 1);
    m_rbctl->sync_wait();
    B2INFO("event process : begin/end run synchronization done");
  }


  // Get number of objects
  DataStore::EDurability durability = (DataStore::EDurability)(evtmsg->header())->reserved[0];
  int nobjs = (evtmsg->header())->reserved[1];
  int narrays = (evtmsg->header())->reserved[2];

  // Decode message
  vector<TObject*> objlist;
  vector<string> namelist;
  m_msghandler->decode_msg(evtmsg, objlist, namelist);

  delete evtbuf;

  // Restore objects in DataStore
  // 1. Objects
  for (int i = 0; i < nobjs; i++) {
    DataStore::Instance().storeObject(
      objlist.at(i), namelist.at(i), durability);
  }
  for (int i = 0; i < narrays; i++) {
    DataStore::Instance().storeArray(
      (TClonesArray*)objlist.at(i + nobjs),
      namelist.at(i + nobjs), durability);
  }
  //  B2INFO ( "Event received : " << m_nrecv++ )
}

void pSeqRootInputModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void pSeqRootInputModule::terminate()
{
  B2INFO("terminate called")
}

// Input Server function
void pSeqRootInputModule::event_server(void)
{
  B2INFO("----> Input Server Invoked");

  // Open output ROOT file
  m_file = new SeqFile(m_inputFileName.c_str(), "r");
  char* evtbuf = new char[MAXEVTSIZE];

  int nevt = 0;
  int nbgr = 0;

  while (1) {
    // Read file
    int size = m_file->read(evtbuf, MAXEVTSIZE);
    if (size == 0) {
      // Send EOF packet
      EvtMessage* term = m_msghandler->encode_msg(MSG_TERMINATE);
      for (int i = 0; i < m_nproc; i++) {
        int sval;
        while ((sval = m_rbuf->insq((int*)(term->buffer()),
                                    (term->size() - 1) / sizeof(int) + 1)) < 0) {
          usleep(200);
        }
      }
      exit(0);
    } else {
      // Put the message in ring buffer
      EvtMessage* msg = new EvtMessage(evtbuf);
      //      B2INFO ( "Message type = " << msg->type() );
      // Begin_run or End_run record
      if (msg->type() == MSG_BEGIN_RUN || msg->type() == MSG_END_RUN) {
        // Wait until ring buffer is cleared
        for (;;) {
          if (m_rbuf->numq() == 0) break;
          usleep(100);
        }
        // Suspend event processes
        m_rbctl->sync_set(1);
        for (int i = 0; i < m_nproc; i++)
          m_rbctl->set_flag(i, 0);
        // Queue begin/end run records up to m_nproc
        B2INFO("event_server : sending begin/end run records");
        for (int i = 0; i < m_nproc; i++) {
          for (;;) {
            int stat = m_rbuf->insq((int*)msg->buffer(), (msg->size() - 1) / 4 + 1);
            if (stat >= 0) break;
            usleep(200);
          }
          m_nsent++;
          nbgr++;
        }
        // Wait until all evprocs receive records
        for (;;) {
          int done = 0;
          for (int i = 0; i < m_nproc; i++)
            if (m_rbctl->get_flag(i) == 1) done++;
          if (done == m_nproc) break;
          usleep(100);
        }
        // Resume event processes
        m_rbctl->sync_done();
      }
      // Event record
      else {
        for (;;) {
          int stat = m_rbuf->insq((int*)msg->buffer(), (msg->size() - 1) / 4 + 1);
          if (stat >= 0) break;
          usleep(200);
        }
        m_nsent++;
        nevt++;
      }
      delete msg;
    }
    //    cout << "Sent records = " << m_nsent
    //       << ", bgr=" << nbgr << ", evt=" << nevt <<endl;
    usleep(200);    // Temporary fix for missing record problem
  }
}

