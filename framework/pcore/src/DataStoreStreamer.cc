//+
// File : DataStoreStreamer.cc
// Description : Stream/Restore DataStore to/from EvtMessage
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 5 - Sep - 2012
//-

#include <framework/pcore/DataStoreStreamer.h>
#include <framework/pcore/MsgHandler.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <TClonesArray.h>
#include <TClass.h>

using namespace std;
using namespace Belle2;

// Thread related

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_thread[MAXTHREADS];

void* RunDecodeEvtMessage(void* targ)
{
  int* id = (int*)targ;
  DataStoreStreamer::Instance().decodeEvtMessage(*id);
  return NULL;
}

// Instance
DataStoreStreamer& DataStoreStreamer::Instance()
{
  static DataStoreStreamer instance;
  return instance;
}

// Constructor
DataStoreStreamer::DataStoreStreamer(int complevel, int maxthread) : m_compressionLevel(complevel), m_initStatus(0),
  m_maxthread(maxthread),
  m_threadin(0), m_threadout(0)
{
  if (m_maxthread > MAXTHREADS) {
    B2FATAL("DataStoreStreamer : Too many threads " << m_maxthread);
  }
  m_msghandler = new MsgHandler(m_compressionLevel);

  if (maxthread > 0) {
    // Run decoder threads as sustainable detached threads
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    //    pthread_attr_setschedpolicy(&thread_attr , SCHED_FIFO);
    //    pthread_attr_setdetachstate(&thread_attr , PTHREAD_CREATE_DETACHED);
    for (int i = 0; i < maxthread; i++) {
      m_decstat[i] = 0;
      m_pt[i] = (pthread_t)0;
      m_id[i] = i;
      mutex_thread[i] = PTHREAD_MUTEX_INITIALIZER;
      //      args.evtbuf = m_evtbuf[i];
      pthread_create(&m_pt[i], NULL, RunDecodeEvtMessage, (void*)&m_id[i]);
    }
    pthread_attr_destroy(&thread_attr);
  }
}

// Destructor
DataStoreStreamer::~DataStoreStreamer()
{
  delete m_msghandler;
}

// Stream DataStore
EvtMessage* DataStoreStreamer::streamDataStore(DataStore::EDurability durability, bool streamTransientObjects)
{
  // Clear Message Handler
  m_msghandler->clear();

  // Stream objects
  const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(durability);
  int narrays = 0;
  int nobjs = 0;
  for (DataStore::StoreObjConstIter it = map.begin(); it != map.end(); ++it) {
    DataStore::StoreEntry* entry = it->second;

    //skip transient objects/arrays?
    if (!streamTransientObjects and entry->isTransient)
      continue;

    //verify that bits are unused
    if (entry->object->TestBit(c_IsTransient)) {
      B2FATAL("DataStoreStreamer::c_IsTransient bit is set for " << it->first << "!");
    }
    if (entry->object->TestBit(c_IsNull)) {
      B2FATAL("DataStoreStreamer::c_IsNull bit is set for " << it->first << "!");
    }
    //verify TObject bits are serialised
    if (entry->object->IsA()->CanIgnoreTObjectStreamer()) {
      B2FATAL("TObject streamers disabled for " << it->first << "!");
    }
    //store some information in TObject bits to ensure consistent state even if entry->ptr is NULL
    entry->object->SetBit(c_IsTransient, entry->isTransient);
    entry->object->SetBit(c_IsNull, (entry->ptr == NULL));
    if (m_msghandler->add(entry->object, it->first)) {
      B2DEBUG(100, "adding item " << it->first);

      if (entry->isArray)
        narrays++;
      else
        nobjs++;
    }

    //reset bits (are checked to be false when streaming the object)
    entry->object->SetBit(c_IsTransient, false);
    entry->object->SetBit(c_IsNull, false);
  }

  // Encode EvtMessage
  EvtMessage* msg = m_msghandler->encode_msg(MSG_EVENT);
  (msg->header())->reserved[0] = (int)durability;
  (msg->header())->reserved[1] = nobjs;       // No. of objects
  (msg->header())->reserved[2] = narrays;    // No. of arrays

  // Return msg
  // Note : returned EvtMessage has to be deleted later
  return msg;
}

// Restore DataStore
int DataStoreStreamer::restoreDataStore(EvtMessage* msg)
{
  if (msg->type() == MSG_TERMINATE) {
    B2INFO("Got termination message. Exitting...");
    //msg doesn't really contain data, set EventMetaData to something equivalent
    StoreObjPtr<EventMetaData> eventMetaData;
    if (m_initStatus == 0)
      eventMetaData.registerAsPersistent();
    eventMetaData.create();
    eventMetaData->setEndOfData();
  } else {
    // Clear Message Handler
    m_msghandler->clear();

    // List of objects to be restored
    vector<TObject*> objlist;
    vector<string> namelist;

    // Decode EvtMessage
    m_msghandler->decode_msg(msg, objlist, namelist);
    DataStore::EDurability durability = (DataStore::EDurability)(msg->header())->reserved[0];
    int nobjs = (msg->header())->reserved[1];
    int narrays = (msg->header())->reserved[2];

    //    if ( m_initStatus != 0 ) return 0;   // Debugging only

    // Restore objects in DataStore
    for (int i = 0; i < nobjs + narrays; i++) {
      bool array = (dynamic_cast<TClonesArray*>(objlist.at(i)) != 0);
      if (objlist.at(i) != NULL) {
        TObject* obj = objlist.at(i);
        const TClass* cl = obj->IsA();
        if (array)
          cl = static_cast<TClonesArray*>(obj)->GetClass();
        if (m_initStatus == 0) { //are we called by the module's initialize() function?
          bool transient = obj->TestBit(c_IsTransient);
          DataStore::Instance().createEntry(namelist.at(i), durability, cl, array, transient, false);
        }
        //only restore object if it is valid for current event
        bool ptrIsNULL = obj->TestBit(c_IsNull);
        if (!ptrIsNULL) {
          //  if (!ptrIsNULL && !array) {
          DataStore::Instance().createObject(obj, true,
                                             StoreAccessorBase(namelist.at(i), durability, cl, array));
          //          B2DEBUG(100, "restoreDS: " << (array ? "Array" : "Object") << ": " << namelist.at(i) << " stored");
          //    B2INFO("restoreDS: " << (array ? "Array" : "Object") << ": " << namelist.at(i) << " stored");

          //reset bits of object in DataStore (are checked to be false when streaming the object)
          obj->SetBit(c_IsTransient, false);
          obj->SetBit(c_IsNull, false);
        } else {
          //not stored, clean up
          delete obj;
        }
      } else {
        //DataStore always has non-NULL content (wether they're available is a different matter)
        B2ERROR("restoreDS: " << (array ? "Array" : "Object") << ": " << namelist.at(i) << " is NULL!");
      }
    }

  }
  // Return with normal exit status
  if (m_initStatus == 0) m_initStatus = 1;
  return 0;
}

// Parallel EvtMessage Destreamer implemented using thread

int DataStoreStreamer::queueEvtMessage(char* evtbuf)
{
  // EOF case
  if (evtbuf == NULL) {
    for (int i = 0; i < m_maxthread; i++) {
      while (m_evtbuf[i].size() >= MAXQUEUEDEPTH) usleep(10);
      m_evtbuf[m_threadin].push(evtbuf);
    }
    return 0;
  }

  // Put the event buffer in the queue of current thread
  struct timeval t_0, t_1;
  gettimeofday(&t_0, NULL);

  int nloop = 0;
  for (;;) {
    if (m_evtbuf[m_threadin].size() < MAXQUEUEDEPTH) {
      m_evtbuf[m_threadin].push(evtbuf);
      break;
    } else {
      m_threadin++;
      if (m_threadin >= m_maxthread) m_threadin = 0;
    }
    nloop++;
    if (nloop == m_maxthread) {
      usleep(20);
      nloop = 0;
    }
  }
  gettimeofday(&t_1, NULL);

  // Statistics
  int flagtime = (t_1.tv_sec * 1000000 + t_1.tv_usec) - (t_0.tv_sec * 1000000 + t_0.tv_usec);
  //  printf ( "queueEvtMessage : thread %d wait took %d usec \n", m_threadin, flagtime );

  // Preparation for next event
  //  printf ( "queueEvtMessage : event queued in thread %d!\n", m_threadin );
  m_decstat[m_threadin] = 1; // Event queued for decoding
  m_threadin++;
  if (m_threadin >= m_maxthread) m_threadin = 0;
  return 1;
}

void* DataStoreStreamer::decodeEvtMessage(int id)
{
  //  printf ( "decodeEvtMessge : started. Thread ID = %d\n", id );
  // Clear Message Handler
  //  m_msghandler->clear();
  MsgHandler msghandler(m_compressionLevel);

  struct timeval t_0, t_1, t_2;
  for (;;) {
    gettimeofday(&t_0, NULL);
    // Dequeue one event
    int nloop = 0;
    while (m_evtbuf[id].empty()) {
      usleep(10);
      nloop++;
    }
    gettimeofday(&t_1, NULL);
    //    printf ( "Thread processing id=%d, wait %d times\n", id, nloop );

    // Pick up event buffer
    char* evtbuf = m_evtbuf[id].front(); m_evtbuf[id].pop();
    // In case of EOF
    if (evtbuf == NULL) {
      m_nobjs.push(-1);
      return NULL;
    }

    // Construct EvtMessage
    EvtMessage* msg = new EvtMessage(evtbuf);

    /* OLD implementation
    msghandler.decode_msg(msg, m_objlist[id], m_namelist[id]);
    m_nobjs[id] =  (msg->header())->reserved[1];
    m_narrays[id] = (msg->header())->reserved[2];
    m_durability[id] = (DataStore::EDurability)(msg->header())->reserved[0];
    */

    // Decode EvtMessage into Objects
    vector<TObject*> objlist;
    vector<string> namelist;
    msghandler.decode_msg(msg, objlist, namelist);

    // Queue them for the registration in DataStore
    while (m_nobjs.size() >= MAXQUEUEDEPTH) usleep(10);
    pthread_mutex_lock(&mutex);     // Lock queueing
    m_objlist.push(objlist);
    m_namelist.push(namelist);
    m_nobjs.push((msg->header())->reserved[1]);
    m_narrays.push((msg->header())->reserved[2]);
    m_durability.push((DataStore::EDurability)(msg->header())->reserved[0]);
    pthread_mutex_unlock(&mutex);    // Unlock queueing

    // Preparation for next event
    delete msg;
    delete[] evtbuf;
    m_decstat[id]  = 0; // Ready to read next event

    gettimeofday(&t_2, NULL);
    int gettime = (t_1.tv_sec * 1000000 + t_1.tv_usec) - (t_0.tv_sec * 1000000 + t_0.tv_usec);
    int proctime = (t_2.tv_sec * 1000000 + t_2.tv_usec) - (t_1.tv_sec * 1000000 + t_1.tv_usec);
    //    printf ( "Thread processing id=%d, done. took %d + %d = %d usec\n", id, gettime, proctime, gettime+proctime );
  }

  return NULL;
}

int DataStoreStreamer::restoreDataStoreAsync()
{
// Wait for the queue to become ready
  while (m_nobjs.empty()) usleep(10);

  // Register decoded objects in DataStore

  // Pick up event on the top and remove it from the queue
  pthread_mutex_lock(&mutex);
  int nobjs = m_nobjs.front(); m_nobjs.pop();
  if (nobjs == -1) {
    printf("restoreDataStore: EOF detected. exitting with status 0\n");
    pthread_mutex_unlock(&mutex);
    return 0;
  }
  int narrays = m_narrays.front(); m_narrays.pop();
  DataStore::EDurability durability = m_durability.front(); m_durability.pop();
  vector<TObject*> objlist = m_objlist.front(); m_objlist.pop();
  vector<string> namelist = m_namelist.front(); m_namelist.pop();
  pthread_mutex_unlock(&mutex);

  // Restore objects in DataStore
  for (int i = 0; i < nobjs + narrays; i++) {
    //    printf ( "restoring object %d = %s\n", i, namelist.at(i).c_str() );
    bool array = (dynamic_cast<TClonesArray*>(objlist.at(i)) != 0);
    if (objlist.at(i) != NULL) {
      TObject* obj = objlist.at(i);
      const TClass* cl = obj->IsA();
      if (array)
        cl = static_cast<TClonesArray*>(obj)->GetClass();
      if (m_initStatus == 0) { //are we called by the module's initialize() function?
        bool transient = obj->TestBit(c_IsTransient);
        DataStore::Instance().createEntry(namelist.at(i), durability, cl, array, transient, false);
      }
      //only restore object if it is valid for current event
      bool ptrIsNULL = obj->TestBit(c_IsNull);
      if (!ptrIsNULL) {
        DataStore::Instance().createObject(obj, true,
                                           StoreAccessorBase(namelist.at(i), durability, cl, array));
        B2DEBUG(100, "restoreDS: " << (array ? "Array" : "Object") << ": " << namelist.at(i) << " stored");

        //reset bits of object in DataStore (are checked to be false when streaming the object)
        obj->SetBit(c_IsTransient, false);
        obj->SetBit(c_IsNull, false);
      } else {
        //not stored, clean up
        delete obj;
      }
    } else {
      //DataStore always has non-NULL content (wether they're available is a different matter)
      B2ERROR("restoreDS: " << (array ? "Array" : "Object") << ": " << namelist.at(i) << " is NULL!");
    }
  }

  //  printf ( "Objects restored in DataStore\n" );

  return 1;
}

void DataStoreStreamer::setMaxThreads(int maxthread)
{
  m_maxthread = maxthread;
}

int DataStoreStreamer::getMaxThreads()
{
  return m_maxthread;
}

int DataStoreStreamer::getDecoderStatus()
{
  //  printf ( "Decode thread %d = %d\n", m_threadin, m_done_decode[m_threadin] );
  return (m_decstat[m_threadin]);
}

void DataStoreStreamer::setDecoderStatus(int val)
{
  //  printf ( "Decode thread %d = %d\n", m_threadin, m_done_decode[m_threadin] );
  m_decstat[m_threadin] = val;
}


