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

// Instance
DataStoreStreamer& DataStoreStreamer::Instance()
{
  static DataStoreStreamer instance;
  return instance;
}

// Constructor
DataStoreStreamer::DataStoreStreamer(int complevel) : m_compressionLevel(complevel), m_initStatus(0)
{
  m_msghandler = new MsgHandler(m_compressionLevel);
}

// Destructor
DataStoreStreamer::~DataStoreStreamer()
{
  delete m_msghandler;
}

// Stream DataStore
EvtMessage* DataStoreStreamer::streamDataStore(DataStore::EDurability durability)
{
  // Clear Message Handler
  m_msghandler->clear();

  // Stream objects
  const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(durability);
  int narrays = 0;
  int nobjs = 0;
  for (DataStore::StoreObjConstIter it = map.begin(); it != map.end(); ++it) {
    DataStore::StoreEntry* entry = it->second;
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
        } else {
          if (m_initStatus == 1) {
            delete obj;
          }
        }
        //reset bits (are checked to be false when streaming the object)
        obj->SetBit(c_IsTransient, false);
        obj->SetBit(c_IsNull, false);

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
