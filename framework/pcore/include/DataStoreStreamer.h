#ifndef DATASTORESTREAMER_H
#define DATASTORESTREAMER_H
//+
// File : DataStoreStreamer.h
// Description : Stream/Destream DataStore objects in EvtMessage
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 5 - Sep - 2012
//-

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

#include <vector>
#include <queue>
#include <string>

#include <framework/datastore/DataStore.h>
#include <framework/pcore/EvtMessage.h>

#define MAXTHREADS 16
#define MAXQUEUEDEPTH 64

namespace Belle2 {
  class MsgHandler;

  /** Stream/restore DataStore objects to/from EvtMessage. */
  class DataStoreStreamer {
  public:
    /** Constructor
     *  @param complevel  Compression level of streaming
     */
    DataStoreStreamer(int complevel = 0, int maxthread = 4);

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
    EvtMessage* streamDataStore(DataStore::EDurability durability, bool streamTransientObjects = false);

    // EvtMessage->DataStore
    /** Restore DataStore objects from EvtMessage
     *  @param msg        EvtMessage to be restored.
     */
    int restoreDataStore(EvtMessage* msg);

    // Pipelined destreaming of EvtMessage using thread

    /** Queue EvtMessage for destreaming
     *  @param msg        Event buffer to be restored.
     */
    int queueEvtMessage(char* msg);

    /** Decode EvtMessage and store objects in temporary buffer
     *  @param id         Thread id
     *  @param msg        EvtMessage to be restored
     */
    void* decodeEvtMessage(int id);

    /** Restore objects in DataStore from temporary buffer
     */
    int restoreDataStoreAsync();

    /** Functions to retrieve info
     */
    void setMaxThreads(int);
    int  getMaxThreads();
    void setDecoderStatus(int);
    int  getDecoderStatus();

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

    /** Max. number of threads for asynchronous processing
     *
     * 4 for default
     */
    int m_maxthread;

    /** thread pointer
     */
    pthread_t m_pt[MAXTHREADS];
    int m_id[MAXTHREADS];
    int m_threadin;
    int m_threadout;
    int m_decstat[MAXTHREADS];
    //    char* m_evtbuf[MAXTHREADS];
    std::queue<char*> m_evtbuf[MAXTHREADS];

    /** Object arrays in temporary buffer
     */

    //    int m_nobjs[MAXTHREADS];
    //    int m_narrays[MAXTHREADS];
    //    DataStore::EDurability m_durability[MAXTHREADS];
    //    std::vector<TObject*> m_objlist[MAXTHREADS];
    //    std::vector<std::string> m_namelist[MAXTHREADS];

    std::queue<int> m_nobjs;
    std::queue<int> m_narrays;
    std::queue<DataStore::EDurability> m_durability;
    std::queue<std::vector<TObject*>> m_objlist;
    std::queue<std::vector<std::string>> m_namelist;
  };

  // Function to hook DataStoreStreamer::decodeEvtMessage to pthread

  //  void* RunDecodeEvtMessage ( void* );


} // namespace Belle2

#endif
