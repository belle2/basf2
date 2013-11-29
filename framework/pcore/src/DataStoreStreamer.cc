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

static struct DecoderArgs decoder_arg[MAXTHREADS];

void* RunDecodeEvtMessage(void* targ)
{
  int id = ((struct DecoderArgs*)targ)->id;
  EvtMessage* msg = ((struct DecoderArgs*)targ)->msg;
  //  printf ( "DecodeEvtMessage : decoding started for id %d, size = %d\n", id, msg->size() );
  DataStoreStreamer::Instance().decodeEvtMessage(id, msg);
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
  m_navail = m_maxthread;
  for (int i = 0; i < MAXTHREADS; i++) m_done[i] = 1;

  m_msghandler = new MsgHandler(m_compressionLevel);

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
  }
  // Return with normal exit status
  if (m_initStatus == 0) m_initStatus = 1;
  return 0;
}

// Parallel EvtMessage Destreamer implemented using thread

int DataStoreStreamer::queueEvtMessage(EvtMessage* msg)
{
  // If EoF, set EoD in EventMetaData and return with status 1
  if (msg->type() == MSG_TERMINATE) {
    B2INFO("Got termination message. Exitting...");
    //msg doesn't really contain data, set EventMetaData to something equivalent
    StoreObjPtr<EventMetaData> eventMetaData;
    if (m_initStatus == 0)
      eventMetaData.registerAsPersistent();
    eventMetaData.create();
    eventMetaData->setEndOfData();
    return 1;
  }
  // Wait for previous thread to complete if engaged.
  //  pthread_join ( m_pt[m_threadin], NULL );
  while (m_done[m_threadin] != 1) usleep(2);
  m_done[m_threadin] = 0;
  // Start decoding EvtMessage in a thread
  decoder_arg[m_threadin].id = m_threadin;
  decoder_arg[m_threadin].msg = msg;
  pthread_create(&m_pt[m_threadin], NULL, RunDecodeEvtMessage, (void*)&decoder_arg[m_threadin]);
  // Set next thread ID
  m_navail--;
  m_threadin++;
  if (m_threadin >= m_maxthread) m_threadin = 0;
  //  printf ( "queueEvtMessage : event queued!\n" );
  return 0;
}

void* DataStoreStreamer::decodeEvtMessage(int id, EvtMessage* msg)
{
  // Clear Message Handler
  m_msghandler->clear();

  // Decode EvtMessage
  m_msghandler->decode_msg(msg, m_objlist[id], m_namelist[id]);
  m_nobjs[id] = (msg->header())->reserved[1];
  m_narrays[id] = (msg->header())->reserved[2];
  m_durability[id] = (DataStore::EDurability)(msg->header())->reserved[0];

  // End of thread processing, no synchronization in this function required.

  // Delete msg
  delete msg;

  return NULL;
}


int DataStoreStreamer::restoreDataStoreAsync()
{
  // Check for the completion of thread
  pthread_join(m_pt[m_threadout], NULL);

  // Register decoded objects in DataStore
  DataStore::EDurability durability = m_durability[m_threadout];
  int nobjs = m_nobjs[m_threadout];
  int narrays = m_narrays[m_threadout];

  // Restore objects in DataStore
  for (int i = 0; i < nobjs + narrays; i++) {
    bool array = (dynamic_cast<TClonesArray*>(m_objlist[m_threadout].at(i)) != 0);
    if (m_objlist[m_threadout].at(i) != NULL) {
      TObject* obj = m_objlist[m_threadout].at(i);
      const TClass* cl = obj->IsA();
      if (array)
        cl = static_cast<TClonesArray*>(obj)->GetClass();
      if (m_initStatus == 0) { //are we called by the module's initialize() function?
        bool transient = obj->TestBit(c_IsTransient);
        DataStore::Instance().createEntry(m_namelist[m_threadout].at(i), durability, cl, array, transient, false);
      }
      //only restore object if it is valid for current event
      bool ptrIsNULL = obj->TestBit(c_IsNull);
      if (!ptrIsNULL) {
        DataStore::Instance().createObject(obj, true,
                                           StoreAccessorBase(m_namelist[m_threadout].at(i), durability, cl, array));
        B2DEBUG(100, "restoreDS: " << (array ? "Array" : "Object") << ": " << m_namelist[m_threadout].at(i) << " stored");

        //reset bits of object in DataStore (are checked to be false when streaming the object)
        obj->SetBit(c_IsTransient, false);
        obj->SetBit(c_IsNull, false);
      } else {
        //not stored, clean up
        delete obj;
      }
    } else {
      //DataStore always has non-NULL content (wether they're available is a different matter)
      B2ERROR("restoreDS: " << (array ? "Array" : "Object") << ": " << m_namelist[m_threadout].at(i) << " is NULL!");
    }
  }

  // Preparation for Next thread
  //   Clear vectors
  m_objlist[m_threadout].erase(m_objlist[m_threadout].begin(), m_objlist[m_threadout].end());
  m_namelist[m_threadout].erase(m_namelist[m_threadout].begin(), m_namelist[m_threadout].end());
  m_done[m_threadout] = 1;
  m_navail++;
  m_threadout++;
  if (m_threadout >= m_maxthread) m_threadout = 0;
  return 0;
}

void DataStoreStreamer::setMaxThreads(int maxthread)
{
  m_maxthread = maxthread;
  m_navail = maxthread;
}

int DataStoreStreamer::getNumFreeThreads()
{
  return m_navail;
}

