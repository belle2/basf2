//+
// File : seqrootinput.cc
// Description : Sequential ROOT input module for pbasf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <framework/modules/seqroot/seqrootinputModule.h>
#include <framework/core/Environment.h>

#include <TSystem.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SeqRootInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SeqRootInputModule::SeqRootInputModule() : Module()
{
  //Set module properties
  setDescription("SeqROOT output module");
  setPropertyFlags(c_Input | c_InitializeInProcess);

  m_file = 0;

  //Parameter definition
  addParam("inputFileName"  , m_inputFileName, "SeqRoot file name.", string("SeqRootInput.root"));
  addParam("compressionLevel", m_compressionLevel, "Compression Level: 0 for no, 1 for low, 9 for high compression.", 0);

  B2DEBUG(1, "SeqRootInput: Constructor done.");
}


SeqRootInputModule::~SeqRootInputModule()
{
}

void SeqRootInputModule::initialize()
{
  gSystem->Load("libdataobjects");

  // Open input file
  m_file = new SeqFile(m_inputFileName.c_str(), "r");
  //  m_fd = open ( m_inputFileName.c_str(), O_RDONLY );

  // Message handler to encode serialized object
  m_msghandler = new MsgHandler(m_compressionLevel);

  StoreObjPtr<EventMetaData>::registerPersistent();

  B2INFO("SeqRootInput: initialized.");
}


void SeqRootInputModule::beginRun()
{
  gettimeofday(&m_t0, 0);
  m_size = 0.0;
  m_size2 = 0.0;
  m_nevt = 0;
  B2INFO("SeqRootInput: beginRun called.");
}


void SeqRootInputModule::event()
{
  m_msghandler->clear();

  // Get a SeqRoot record from the file
  char* evtbuf = new char[MAXEVTSIZE];
  EvtMessage* evtmsg = NULL;
  int size;
  size = m_file->read(evtbuf, MAXEVTSIZE);
  if (size == 0) {
    delete m_file;
    return;
  } else {
    //    printf("SeqRootInput : read = %d\n", size);
    evtmsg = new EvtMessage(evtbuf);
  }

  // Statistics
  double dsize = (double)size / 1000.0;
  m_size += dsize;
  m_size2 += dsize * dsize;
  m_nevt++;

  // Get number of objects
  DataStore::EDurability durability = (DataStore::EDurability)(evtmsg->header())->reserved[0];
  int nobjs = (evtmsg->header())->reserved[1];
  int narrays = (evtmsg->header())->reserved[2];

  //  printf ( "nobjs = %d, narrays = %d\n", nobjs, narrays );
  B2INFO("nobjs = " << nobjs << ", narrays = " << narrays);

  // Decode message
  DataStore::Instance().clearMaps();
  vector<TObject*> objlist;
  vector<string> namelist;
  m_msghandler->decode_msg(evtmsg, objlist, namelist);

  delete[] evtbuf;

  //  printf("size of objlist = %d\n", objlist.size());

  // Clear arrays
  //  DataStore::Instance().clearMaps(DataStore::c_Event);

  // Restore objects in DataStore
  // 1. Objects
  for (int i = 0; i < nobjs; i++) {
    if (objlist.at(i) != NULL) {
      DataStore::Instance().createObject(objlist.at(i), false,
                                         namelist.at(i), durability,
                                         objlist.at(i)->IsA(), false);
      B2INFO("Store Object : " << namelist.at(i) << " stored");
    }
  }
  for (int i = 0; i < narrays; i++) {
    if (objlist.at(i + nobjs) != NULL) {
      DataStore::Instance().createObject(objlist.at(i + nobjs), false,
                                         namelist.at(i + nobjs), durability,
                                         ((TClonesArray*)objlist.at(i + nobjs))->GetClass(), true);
      B2INFO("Store Array : " << namelist.at(i) << " stored");
    }
  }
  //  B2INFO ( "Event received : " << m_nrecv++ )

  /*
  // Check objects in DataStore
  // 1. Stored Objects
  const DataStore::StoreObjMap& objmap = DataStore::Instance().getObjectMap(durability);
  for (DataStore::StoreObjConstIter it = objmap.begin(); it != objmap.end(); ++it) {
    printf("Check Object : %s\n", (it->first).c_str());
  }
  // 2. Stored Arrays
  const DataStore::StoreArrayMap& arymap = DataStore::Instance().getArrayMap(durability);
  for (DataStore::StoreObjConstIter it = arymap.begin(); it != arymap.end(); ++it) {
    printf("Check Array : %s\n", (it->first).c_str());
  }
  */
}

void SeqRootInputModule::endRun()
{
  // End time
  gettimeofday(&m_tend, 0);
  double etime = (double)((m_tend.tv_sec - m_t0.tv_sec) * 1000000 +
                          (m_tend.tv_usec - m_t0.tv_usec));

  // Statistics
  // Sigma^2 = Sum(X^2)/n - (Sum(X)/n)^2

  double flowmb = m_size / etime * 1000.0;
  double avesize = m_size / (double)m_nevt;
  double avesize2 = m_size2 / (double)m_nevt;
  double sigma2 = avesize2 - avesize * avesize;
  double sigma = sqrt(sigma2);

  //  printf ( "m_size = %f, m_size2 = %f, m_nevt = %d\n", m_size, m_size2, m_nevt );
  //  printf ( "avesize2 = %f, avesize = %f, avesize*avesize = %f\n", avesize2, avesize, avesize*avesize );
  printf("SeqRootInput :  %d events read with total bytes of %f kB\n",
         m_nevt, m_size);
  printf("SeqRootInput : flow rate = %f (MB/s)\n", flowmb);
  printf("SeqRootInput : event size = %f +- %f (kB)\n", avesize, sigma);

  B2INFO("SeqRootInput: endRun done.");
}


void SeqRootInputModule::terminate()
{
  //  delete m_file;
  B2INFO("SeqRootInput: terminate called")
}

