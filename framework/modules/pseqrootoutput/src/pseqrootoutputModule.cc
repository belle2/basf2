//+
// File : pseqrootoutput.cc
// Description : Sequential ROOT output module for pbasf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <framework/modules/pseqrootoutput/pseqrootoutputModule.h>
#include <framework/core/Environment.h>
#include <stdlib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(pSeqRootOutput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

pSeqRootOutputModule::pSeqRootOutputModule() : pOutputServer()
{
  //Set module properties
  setDescription("SeqROOT output with parallel capability");
  setPropertyFlags(c_Output | c_ParallelProcessingCertified);

  m_nsent = 0;
  m_nrecv = 0;

  m_file = 0;

  //Parameter definition
  addParam("outputFileName"  , m_outputFileName, "SeqRoot file name.", string("pSeqRootOutput.root"));
  addParam("compressionLevel", m_compressionLevel, "Compression Level: 0 for no, 1 for low, 9 for high compression.", 1);

  B2INFO("pSeqRootOutput: Constructor done.");
}


pSeqRootOutputModule::~pSeqRootOutputModule()
{
}

void pSeqRootOutputModule::initialize()
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
  } else {
    m_file = new SeqFile(m_outputFileName.c_str(), "w");
    m_rbuf = NULL;
  }
  // Message handler to encode serialized object
  m_msghandler = new MsgHandler(m_compressionLevel);

  B2INFO("pSeqRootOutput initialized.");
}


void pSeqRootOutputModule::beginRun()
{
  EvtMessage* msg = buildMessage(MSG_BEGIN_RUN);

  cout << "Begin_Run called!!!" << endl;

  if (m_nproc == 0) {
    // Single process : Store the message if a SeqFile
    m_file->write(msg->buffer());
  } else {
    // Parallel process: Put the message in ring buffer up to m_nproc
    for (;;) {
      int stat = m_rbuf->insq((int*)msg->buffer(), (msg->size() - 1) / 4 + 1);
      if (stat >= 0) break;
      usleep(200);
    }
  }

  B2INFO("beginRun called.");
}

void pSeqRootOutputModule::event()
{
  EvtMessage* msg = buildMessage(MSG_EVENT);

  // Store EvtMessage

  if (m_nproc == 0) {
    // Single process : Store the message if a SeqFile
    m_file->write(msg->buffer());
  } else {
    // Parallel process: Put the message in ring buffer
    for (;;) {
      int stat = m_rbuf->insq((int*)msg->buffer(), (msg->size() - 1) / 4 + 1);
      if (stat >= 0) break;
      usleep(200);
    }
  }
  //  B2INFO ( "Event sent : " << m_nsent++ )
}

void pSeqRootOutputModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void pSeqRootOutputModule::terminate()
{
  // Single process mode
  if (Environment::Instance().getNumberProcesses() == 0)  {
    delete m_file;
  } else { // Parallel processing
    // Send termination packet to output server
    EvtMessage* term = m_msghandler->encode_msg(MSG_TERMINATE);
    int sval;
    while ((sval = m_rbuf->insq((int*)(term->buffer()),
                                (term->size() - 1) / sizeof(int) + 1)) < 0) {
      usleep(200);
    }
  }
  B2INFO("terminate called")
}

// Fill Datastore

EvtMessage* pSeqRootOutputModule::buildMessage(RECORD_TYPE rectype)
{

  m_msghandler->clear();

  DataStore::EDurability durability = DataStore::c_Event;
  if (rectype == MSG_BEGIN_RUN)
    durability = DataStore::c_Run;


  // Collect objects and place them in msghandler
  // 1. Stored Objects
  m_obj_iter[durability]->first();
  int nobjs = 0;
  while (!m_obj_iter[durability]->isDone()) {
    m_msghandler->add(m_obj_iter[durability]->value(), m_obj_iter[durability]->key());
    nobjs++;
    m_obj_iter[durability]->next();
  }
  // 2. Stored Arrays
  m_array_iter[durability]->first();
  int narrays = 0;
  while (!m_array_iter[durability]->isDone()) {
    m_msghandler->add(m_array_iter[durability]->value(), m_array_iter[durability]->key());
    narrays++;
    m_array_iter[durability]->next();
  }

  // Encode EvtMessage
  EvtMessage* msg = m_msghandler->encode_msg(rectype);
  (msg->header())->reserved[0] = (int)durability;
  (msg->header())->reserved[1] = nobjs;       // No. of objects
  (msg->header())->reserved[2] = narrays;    // No. of arrays

  //  printf ( "RecType = %d\n", rectype );
  return msg;

}





// Output Server function
void pSeqRootOutputModule::output_server(void)
{
  B2INFO("----> Output Server Invoked");

  // Open output ROOT file
  m_file = new SeqFile(m_outputFileName.c_str(), "w");
  char* evtbuf = new char[MAXEVTSIZE];

  int size;
  int count_bgr = 0;
  int nbgr = 0;
  int nevt = 0;
  int nwrote = 0;
  while (1) {
    // Pick up one buffer from RingBuffer
    while ((size = m_rbuf->remq((int*)evtbuf)) == 0) {
      usleep(100);
    }
    if (size < 0) {
      B2WARNING("pSeqRootOutput : output server : error in remq")
      exit(-99);
    }
    m_nrecv++;
    //    printf ( "Output Server: got a record = %d\n", size );

    // Form EvtMessage
    EvtMessage* evtmsg = new EvtMessage(evtbuf);

    // Check for termination
    if (evtmsg->type() == MSG_TERMINATE) {
      delete m_file;
      delete[] evtbuf;
      B2INFO("Output Server terminated")
      exit(0);
    }

    // Store EvtMessage in a file
    if (evtmsg->type() == MSG_BEGIN_RUN) {
      nbgr++;
      cout << "====> Begin_Run found. count = " << count_bgr << endl;
      if (count_bgr == 0) {
        m_file->write(evtmsg->buffer());
        count_bgr = m_nproc - 1;
        nwrote++;
      } else {
        B2INFO("skipping extra Begin_Run");
        count_bgr--;
      }
    } else if (evtmsg->type() == MSG_EVENT) {
      m_file->write(evtmsg->buffer());
      nevt++;
      nwrote++;
    }
    //    cout << "---> Received records = " << m_nrecv
    //   << ", bgr=" << nbgr << ", evt=" << nevt
    //   << ", wrote=" << nwrote << endl;
  }
}

