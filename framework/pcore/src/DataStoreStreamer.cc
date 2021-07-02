/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/pcore/DataStoreStreamer.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/Mergeable.h>

#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <TClonesArray.h>
#include <TClass.h>
#include <TStreamerInfo.h>
#include <TList.h>

#include <unistd.h>
#include <cstdio>                      // for NULL, printf

#include <algorithm>
#include <queue>

using namespace Belle2;

namespace {
// Thread related
  static DataStoreStreamer* s_streamer = nullptr;

  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  static pthread_mutex_t mutex_thread[DataStoreStreamer::c_maxThreads];
//static char* evtbuf_thread[DataStoreStreamer::c_maxThreads];
  static int msg_compLevel[DataStoreStreamer::c_maxThreads];

  static std::queue<char*> my_evtbuf[DataStoreStreamer::c_maxThreads];

  static std::queue<int> my_nobjs;
  static std::queue<int> my_narrays;
  static std::queue<std::vector<TObject*>> my_objlist;
  static std::queue<std::vector<std::string>> my_namelist;

  static int my_decstat[DataStoreStreamer::c_maxThreads];
};

void* RunDecodeEvtMessage(void* targ)
{
  auto* id = (int*)targ;
  //  DataStoreStreamer::Instance().decodeEvtMessage(*id);
  s_streamer->decodeEvtMessage(*id);
  return nullptr;
}

DataStoreStreamer::DataStoreStreamer(int complevel, bool handleMergeable, int maxthread):
  m_compressionLevel(complevel),
  m_handleMergeable(handleMergeable),
  m_initStatus(0),
  m_maxthread(maxthread),
  m_threadin(0)
  //, m_threadout(0)
{
  if ((unsigned int)m_maxthread > c_maxThreads) {
    B2FATAL("DataStoreStreamer : Too many threads " << m_maxthread);
    m_maxthread = c_maxThreads;
  }
  m_msghandler = new MsgHandler(m_compressionLevel);

  if (m_maxthread > 0) {
    // Run decoder threads as sustainable detached threads
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    //    pthread_attr_setschedpolicy(&thread_attr , SCHED_FIFO);
    //    pthread_attr_setdetachstate(&thread_attr , PTHREAD_CREATE_DETACHED);
    for (int i = 0; i < m_maxthread; i++) {
      my_decstat[i] = 0;
      m_pt[i] = (pthread_t)0;
      m_id[i] = i;
      //      m_pmsghandler[i] = new MsgHandler ( m_compressionLevel );
      mutex_thread[i] = PTHREAD_MUTEX_INITIALIZER;
      msg_compLevel[i] = m_compressionLevel;
    }
    for (int i = 0; i < m_maxthread; i++) {
      //      args.evtbuf = m_evtbuf[i];
      pthread_create(&m_pt[i], nullptr, RunDecodeEvtMessage, (void*)&m_id[i]);
    }
    pthread_attr_destroy(&thread_attr);
  }
  s_streamer = this;
}

// Destructor
DataStoreStreamer::~DataStoreStreamer()
{
  delete m_msghandler;
}

void DataStoreStreamer::setStreamingObjects(const std::vector<std::string>& objlist)
{
  m_streamobjnames = objlist;
}

bool DataStoreStreamer::isMergeable(const TObject* object)
{
  return object->InheritsFrom(Mergeable::Class());
}

void DataStoreStreamer::clearMergeable(TObject* object)
{
  static_cast<Mergeable*>(object)->clear();
}
void DataStoreStreamer::mergeIntoExisting(TObject* existing, const TObject* received)
{
  auto* existingObject = static_cast<Mergeable*>(existing);
  existingObject->merge(static_cast<const Mergeable*>(received));
}
void DataStoreStreamer::removeSideEffects()
{
  DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(DataStore::c_Persistent);
  for (auto& entryPair : map) {
    DataStore::StoreEntry& entry = entryPair.second;
    if (isMergeable(entry.object)) {
      static_cast<Mergeable*>(entry.object)->removeSideEffects();
      static_cast<Mergeable*>(entry.object)->clear();
    }
  }
}


// Stream DataStore
EvtMessage* DataStoreStreamer::streamDataStore(bool addPersistentDurability, bool streamTransientObjects)
{
  // Clear Message Handler
  m_msghandler->clear();

  // Stream objects (for all included durabilities)
  int narrays = 0;
  int nobjs = 0;
  DataStore::EDurability durability = DataStore::c_Event;
  while (true) {
    auto& map = DataStore::Instance().getStoreEntryMap(durability);
    for (auto &[name, entry] : map) {
      //skip transient objects/arrays?
      if (!streamTransientObjects and entry.dontWriteOut)
        continue;

      //skip objects not in the list
      if (!m_streamobjnames.empty()) {
        auto pos = std::find(m_streamobjnames.begin(), m_streamobjnames.end(), name);
        if (pos == m_streamobjnames.end())  continue;
      }

      //verify that bits are unused
      if (entry.object->TestBit(c_IsTransient)) {
        B2FATAL("DataStoreStreamer::c_IsTransient bit is set for " << name << "!");
      }
      if (entry.object->TestBit(c_IsNull)) {
        B2FATAL("DataStoreStreamer::c_IsNull bit is set for " << name << "!");
      }
      if (entry.object->TestBit(c_PersistentDurability)) {
        B2FATAL("DataStoreStreamer::c_PersistentDurability bit is set for " << name << "!");
      }
      //verify TObject bits are serialised
      if (entry.object->IsA()->CanIgnoreTObjectStreamer()) {
        B2FATAL("TObject streamers disabled for " << name << "!");
      }
      //store some information in TObject bits to ensure consistent state even if entry.ptr is NULL
      entry.object->SetBit(c_IsTransient, entry.dontWriteOut);
      entry.object->SetBit(c_IsNull, (entry.ptr == nullptr));
      entry.object->SetBit(c_PersistentDurability, (durability == DataStore::c_Persistent));
      m_msghandler->add(entry.object, name);
      B2DEBUG(100, "adding item " << name);

      if (entry.isArray)
        narrays++;
      else
        nobjs++;

      //reset bits (are checked to be false when streaming the object)
      entry.object->SetBit(c_IsTransient, false);
      entry.object->SetBit(c_IsNull, false);
      entry.object->SetBit(c_PersistentDurability, false);

      bool merge = m_handleMergeable and entry.ptr != nullptr and isMergeable(entry.object);
      if (merge)
        clearMergeable(entry.object);
    }

    if (addPersistentDurability and durability == DataStore::c_Event)
      durability = DataStore::c_Persistent;
    else
      break;
  }

  // Encode EvtMessage
  EvtMessage* msg = m_msghandler->encode_msg(MSG_EVENT);
  (msg->header())->nObjects = nobjs;
  (msg->header())->nArrays = narrays;

  return msg;
}

// Restore DataStore
int DataStoreStreamer::restoreDataStore(EvtMessage* msg)
{
  if (msg->type() == MSG_TERMINATE) {
    B2INFO("Got termination message. Exitting...");
    //msg doesn't really contain data, set EventMetaData to something equivalent
    StoreObjPtr<EventMetaData> eventMetaData;
    if (m_initStatus == 0 && DataStore::Instance().getInitializeActive())
      eventMetaData.registerInDataStore();
    eventMetaData.create();
    eventMetaData->setEndOfData();
  } else {
    // Clear Message Handler
    m_msghandler->clear();

    // List of objects to be restored
    std::vector<TObject*> objlist;
    std::vector<std::string> namelist;

    // Decode EvtMessage
    m_msghandler->decode_msg(msg, objlist, namelist);
    int nobjs = (msg->header())->nObjects;
    int narrays = (msg->header())->nArrays;
    if (unsigned(nobjs + narrays) != objlist.size())
      B2WARNING("restoreDataStore(): inconsistent #objects/#arrays in header");

    // Restore objects in DataStore
    for (int i = 0; i < nobjs + narrays; i++) {
      TObject* obj = objlist.at(i);
      bool array = (dynamic_cast<TClonesArray*>(obj) != nullptr);
      if (obj != nullptr) {

        // Read and Build StreamerInfo
        if (msg->type() == MSG_STREAMERINFO) {
          restoreStreamerInfos(static_cast<TList*>(obj));
          return 0;
        }

        bool isPersistent = obj->TestBit(c_PersistentDurability);
        DataStore::EDurability durability = isPersistent ? (DataStore::c_Persistent) : (DataStore::c_Event);
        TClass* cl = obj->IsA();
        if (array)
          cl = static_cast<TClonesArray*>(obj)->GetClass();
        if (m_initStatus == 0 && DataStore::Instance().getInitializeActive()) { //are we called by the module's initialize() function?
          auto flags = obj->TestBit(c_IsTransient) ? DataStore::c_DontWriteOut : DataStore::c_WriteOut;
          DataStore::Instance().registerEntry(namelist.at(i), durability, cl, array, flags);
        }
        DataStore::StoreEntry* entry = DataStore::Instance().getEntry(StoreAccessorBase(namelist.at(i), durability, cl, array));
        B2ASSERT("Can not find a data store entry with the name " << namelist.at(i) << ". Did you maybe forget to register it?", entry);
        //only restore object if it is valid for current event
        bool ptrIsNULL = obj->TestBit(c_IsNull);
        if (!ptrIsNULL) {
          bool merge = m_handleMergeable and !array and entry->ptr != nullptr and isMergeable(obj);
          if (merge) {
            B2DEBUG(100, "Will now merge " << namelist.at(i));

            mergeIntoExisting(entry->ptr, obj);
            delete obj;
          } else {
            //note: replace=true
            DataStore::Instance().createObject(obj, true,
                                               StoreAccessorBase(namelist.at(i), durability, cl, array));

            //reset bits of object in DataStore (are checked to be false when streaming the object)
            obj->SetBit(c_IsTransient, false);
            obj->SetBit(c_IsNull, false);
            obj->SetBit(c_PersistentDurability, false);
          }
          //   B2DEBUG(100, "restoreDS: " << (array ? "Array" : "Object") << ": " << namelist.at(i) << " stored");
        } else {
          //usually entry should already be invalidated, but e.g. for CrashHandler, it might not be.
          if (entry->ptr)
            entry->invalidate();
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
  if (evtbuf == nullptr) {
    printf("queueEvtMessage : NULL evtbuf detected. \n");
    for (int i = 0; i < m_maxthread; i++) {
      while (my_evtbuf[i].size() >= c_maxQueueDepth) usleep(10);
      my_evtbuf[m_threadin].push(evtbuf);
    }
    return 0;
  }

  // Put the event buffer in the queue of current thread
  for (;;) {
    if (my_evtbuf[m_threadin].size() < c_maxQueueDepth) {
      pthread_mutex_lock(&mutex_thread[m_threadin]);
      my_evtbuf[m_threadin].push(evtbuf);
      pthread_mutex_unlock(&mutex_thread[m_threadin]);
      break;
    }
    usleep(20);
  }

  // Switch to next thread
  my_decstat[m_threadin] = 1; // Event queued for decoding
  m_threadin++;
  if (m_threadin >= m_maxthread) m_threadin = 0;
  return 1;
}

void* DataStoreStreamer::decodeEvtMessage(int id)
{
  printf("decodeEvtMessge : started. Thread ID = %d\n", id);
  // Clear Message Handler
  //  m_msghandler->clear();
  //  MsgHandler* msghandler = new MsgHandler(m_compressionLevel);
  //  MsgHandler* msghandler = m_pmsghandler[id];

  pthread_mutex_lock(&mutex);     // Lock thread
  MsgHandler msghandler(msg_compLevel[id]);
  pthread_mutex_unlock(&mutex);    // Unlock thread

  for (;;) {
    // Clear message handler event by event
    //    MsgHandler msghandler(m_compressionLevel);
    //    m_pmsghandler[id]->clear();
    // Wait for event in queue becomes ready
    msghandler.clear();

    while (my_evtbuf[id].size() <= 0) usleep(10);

    // Pick up event buffer
    pthread_mutex_lock(&mutex_thread[id]);
    int nqueue = my_evtbuf[id].size();
    if (nqueue <= 0) printf("!!!!! Nqueue = %d\n", nqueue);
    char* evtbuf = my_evtbuf[id].front(); my_evtbuf[id].pop();
    pthread_mutex_unlock(&mutex_thread[id]);

    // In case of EOF
    if (evtbuf == nullptr) {
      printf("decodeEvtMessage: NULL evtbuf detected, nq = %d\n", nqueue);
      my_nobjs.push(-1);
      return nullptr;
    }

    // Construct EvtMessage
    auto* msg = new EvtMessage(evtbuf);

    // Decode EvtMessage into Objects
    std::vector<TObject*> objlist;
    std::vector<std::string> namelist;

    //    pthread_mutex_lock(&mutex);     // Lock test
    msghandler.decode_msg(msg, objlist, namelist);
    //    pthread_mutex_unlock(&mutex);    // Unlock test


    // Queue them for the registration in DataStore
    while (my_nobjs.size() >= c_maxQueueDepth) usleep(10);
    pthread_mutex_lock(&mutex);     // Lock queueing
    my_objlist.push(objlist);
    my_namelist.push(namelist);
    my_nobjs.push((msg->header())->nObjects);
    my_narrays.push((msg->header())->nArrays);
    pthread_mutex_unlock(&mutex);    // Unlock queueing

    // Release EvtMessage
    delete msg;
    delete[] evtbuf;

    // Preparation for next event
    my_decstat[id]  = 0; // Ready to read next event

  }

  return nullptr;
}

int DataStoreStreamer::restoreDataStoreAsync()
{
// Wait for the queue to become ready
  while (my_nobjs.empty()) usleep(10);

  // Register decoded objects in DataStore

  // Pick up event on the top and remove it from the queue
  pthread_mutex_lock(&mutex);
  int nobjs = my_nobjs.front(); my_nobjs.pop();
  if (nobjs == -1) {
    printf("restoreDataStore: EOF detected. exitting with status 0\n");
    pthread_mutex_unlock(&mutex);
    return 0;
  }
  int narrays = my_narrays.front(); my_narrays.pop();
  std::vector<TObject*> objlist = my_objlist.front(); my_objlist.pop();
  std::vector<std::string> namelist = my_namelist.front(); my_namelist.pop();
  pthread_mutex_unlock(&mutex);

  // Restore objects in DataStore
  //TODO refactor, just copied & pasted right now
  for (int i = 0; i < nobjs + narrays; i++) {
    //    printf ( "restoring object %d = %s\n", i, namelist.at(i).c_str() );
    bool array = (dynamic_cast<TClonesArray*>(objlist.at(i)) != nullptr);
    if (objlist.at(i) != nullptr) {
      TObject* obj = objlist.at(i);
      bool isPersistent = obj->TestBit(c_PersistentDurability);
      DataStore::EDurability durability = isPersistent ? (DataStore::c_Persistent) : (DataStore::c_Event);
      TClass* cl = obj->IsA();
      if (array)
        cl = static_cast<TClonesArray*>(obj)->GetClass();
      if (m_initStatus == 0 && DataStore::Instance().getInitializeActive()) { //are we called by the module's initialize() function?
        auto flags = obj->TestBit(c_IsTransient) ? DataStore::c_DontWriteOut : DataStore::c_WriteOut;
        DataStore::Instance().registerEntry(namelist.at(i), durability, cl, array, flags);
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
        obj->SetBit(c_PersistentDurability, false);
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
  return (my_decstat[m_threadin]);
}

void DataStoreStreamer::setDecoderStatus(int val)
{
  //  printf ( "Decode thread %d = %d\n", m_threadin, m_done_decode[m_threadin] );
  m_decoderStatus[m_threadin] = val;
}


int DataStoreStreamer::restoreStreamerInfos(const TList* list)
{
  //
  // Copy from TSocket::RecvStreamerInfos()
  //

  //      TList *list = (TList*)mess->ReadObject(TList::Class());
  TStreamerInfo* info;
  TObjLink* lnk = list->FirstLink();

  std::vector<std::string> class_name;

  // First call BuildCheck for regular class
  while (lnk) {
    info = (TStreamerInfo*)lnk->GetObject();

    int ovlap = 0;
    for (auto& itr : class_name) {
      if (strcmp(itr.c_str(), info->GetName()) == 0) {
        ovlap = 1;
        B2DEBUG(100, "Regular Class Loop : The class " << info->GetName() << " has already appeared. Skipping...");
        break;
      }
    }

    // If the same class is in the object, ignore it. ( Otherwise it causes error. )
    if (ovlap == 0) {
      std::string temp_classname = info->GetName();
      class_name.push_back(temp_classname);

      TObject* element = info->GetElements()->UncheckedAt(0);
      Bool_t isstl = element && strcmp("This", element->GetName()) == 0;
      if (!isstl) {
        info->BuildCheck();
        //              if (gDebug > 0)
        B2INFO("importing TStreamerInfo: " << info->GetName() <<
               " version = " << info->GetClassVersion());
      }
    }
    lnk = lnk->Next();
  }


  class_name.clear();
  // Then call BuildCheck for stl class
  lnk = list->FirstLink();
  while (lnk) {
    info = (TStreamerInfo*)lnk->GetObject();

    int ovlap = 0;
    for (auto& itr : class_name) {
      if (strcmp(itr.c_str(), info->GetName()) == 0) {
        ovlap = 1;
        B2DEBUG(100, "STL Class Loop : The class " << info->GetName() << " has already appeared. Skipping...");
        break;
      }
    }

    // If the same class is in the object, ignore it. ( Otherwise it causes error. )
    if (ovlap == 0) {
      std::string temp_classname = info->GetName();
      class_name.push_back(temp_classname);

      TObject* element = info->GetElements()->UncheckedAt(0);
      Bool_t isstl = element && strcmp("This", element->GetName()) == 0;
      if (isstl) {
        info->BuildCheck();
        //              if (gDebug > 0)
        B2INFO("STL importing TStreamerInfo: " << info->GetName() <<
               " version = " << info->GetClassVersion());
      }
    }
    lnk = lnk->Next();
  }

  return 0;
}
