#ifndef DATASTORESTREAMER_H
#define DATASTORESTREAMER_H
//+
// File : DataStoreStreamer.h
// Description : Stream/Destream DataStore objects in EvtMessage
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 5 - Sep - 2012
//-

#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/datastore/DataStore.h>

namespace Belle2 {

  /** Stream/restore DataStore objects to/from EvtMessage. */
  class DataStoreStreamer {
  public:

    // Constructor
    /** default constructor */
    DataStoreStreamer();

    /** Constructor
     *  @param complevel  Compression level of streaming
     */
    DataStoreStreamer(int complevel);

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
     *  @return pointer to EvtMessage, caller is responsible for deletion
     *
     */
    EvtMessage* streamDataStore(DataStore::EDurability durability);

    // EvtMessage->DataStore
    /** Restore DataStore objects from EvtMessage
     *  @param msg        EvtMessage to be restored.
     */
    int restoreDataStore(EvtMessage* msg);


  private:

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
