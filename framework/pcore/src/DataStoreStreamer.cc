//+
// File : DataStoreStreamer.cc
// Description : Stream/Restore DataStore to/from EvtMessage
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 5 - Sep - 2012
//-

#include <framework/pcore/DataStoreStreamer.h>

using namespace std;
using namespace Belle2;

// Instance
DataStoreStreamer& DataStoreStreamer::Instance()
{
  static DataStoreStreamer instance;
  return instance;
}

// Default Constructor
DataStoreStreamer::DataStoreStreamer(void) : m_compressionLevel(0), m_initStatus(0)
{
  m_msghandler = new MsgHandler(m_compressionLevel);
}

// Constructor with compression level
DataStoreStreamer::DataStoreStreamer(int complevel) : m_compressionLevel(complevel), m_initStatus(0)
{
  m_msghandler = new MsgHandler(m_compressionLevel);
}

// Destructor
DataStoreStreamer::~DataStoreStreamer(void)
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
    if (m_msghandler->add(it->second->ptr, it->first)) {
      B2DEBUG(100, "adding item " << it->first);

      if (it->second->isArray)
        narrays++;
      else
        nobjs++;
    }
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
      const TClass* cl = objlist.at(i)->IsA();
      if (array)
        cl = static_cast<TClonesArray*>(objlist.at(i))->GetClass();
      if (m_initStatus == 0) {
        DataStore::Instance().createEntry(namelist.at(i), durability, cl, array, false, false);
      }
      DataStore::Instance().createObject(objlist.at(i), true,
                                         namelist.at(i), durability,
                                         cl, array);
      B2DEBUG(100, "restoreDS: " << (array ? "Array" : "Object") << ": " << namelist.at(i) << " stored");
    } else {
      B2INFO("restoreDS: " << (array ? "Array" : "Object") << ": " << namelist.at(i) << " is NULL!");
    }
  }
  // Return with normal exit status
  if (m_initStatus == 0) m_initStatus = 1;
  return 0;
}



