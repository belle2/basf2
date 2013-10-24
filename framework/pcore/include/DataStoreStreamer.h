#ifndef DATASTORESTREAMER_H
#define DATASTORESTREAMER_H
//+
// File : DataStoreStreamer.h
// Description : Stream/Destream DataStore objects in EvtMessage
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 5 - Sep - 2012
//-

#include <framework/datastore/DataStore.h>
#include <framework/pcore/EvtMessage.h>


namespace Belle2 {
  class MsgHandler;

  /** Stream/restore DataStore objects to/from EvtMessage. */
  class DataStoreStreamer {
  public:
    /** Constructor
     *  @param complevel  Compression level of streaming
     */
    DataStoreStreamer(int complevel = 0);

    // Destructor
    /** destructor */
    ~DataStoreStreamer();


    // Instance
    /** Instance of singleton pDataStore
     *
     * This method is to access pDataStore methods directly
     */
    static DataStoreStreamer& Instance();

    // DataStore->EvtMessage
    /** Store DataStore objects in EvtMessage
     *
     *  @param durability Durability of DataStore objects to be streamed
     *  @param streamTransientObjects Should objects/arrays registered as transient be streamed?
     *  @return pointer to EvtMessage, caller is responsible for deletion
     *
     */
    EvtMessage* streamDataStore(DataStore::EDurability durability, bool streamTransientObjects = true);

    // EvtMessage->DataStore
    /** Restore DataStore objects from EvtMessage
     *  @param msg        EvtMessage to be restored.
     */
    int restoreDataStore(EvtMessage* msg);


  private:
    /** bits to store in TObject. */
    enum ETObjectBits {
      c_IsTransient = BIT(19), /**< The corresponding StoreEntry is transient. */
      c_IsNull = BIT(20) /**< object is not valid for current event, set StoreEntry::ptr to NULL. */
    };

    /** MsgHandler
     *
     * MsgHandler object used to form/decode EvtMessage
     */
    MsgHandler* m_msghandler;

    /** Compression level in streaming */
    int m_compressionLevel;

    /** first event flag.
     *
     *  0 during first event, 1 otherwise.
     */
    int m_initStatus;

  };
} // namespace Belle2

#endif
