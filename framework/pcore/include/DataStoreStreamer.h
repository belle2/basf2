#ifndef DATASTORESTREAMER_H
#define DATASTORESTREAMER_H
//+
// File : DataStoreStreamer.h
// Description : Stream/Destream DataStore objects in EvtMessage
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 5 - Sep - 2012

#include <framework/pcore/EvtMessage.h>
#include <framework/datastore/DataStore.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

#include <vector>
#include <queue>
#include <string>

namespace Belle2 {
  class MsgHandler;

  /** Stream/restore DataStore objects to/from EvtMessage.
   *
   * Main interface is provided by streamDataStore() and restoreDataStore().
   * Other functions provide more obscure features.
   */
  class DataStoreStreamer {
  public:
    /** global maximum number of threads (cannot set higher number). */
    static const int c_maxThreads = 16;
    /** Ask Itoh-san. */
    static const int c_maxQueueDepth = 64;

    /** Constructor
     *  @param complevel  Compression level of streaming
     */
    DataStoreStreamer(int complevel = 0, int maxthread = 0);

    /** destructor */
    ~DataStoreStreamer();

    /** Return singleton instance. */
    static DataStoreStreamer& Instance();

    // DataStore->EvtMessage
    /** Store DataStore objects in EvtMessage
     *
     *  @param durability Durability of DataStore objects to be streamed
     *  @param streamTransientObjects Should objects/arrays registered as transient be streamed?
     *  @return pointer to EvtMessage, caller is responsible for deletion
     *
     */
    EvtMessage* streamDataStore(DataStore::EDurability durability, bool streamTransientObjects = false,
                                bool removeEmptyArrays = false);

    // EvtMessage->DataStore
    /** Restore DataStore objects from EvtMessage
     *  @param msg        EvtMessage to be restored.
     */
    int restoreDataStore(EvtMessage* msg);

    /** Set names of objects to be streamed/destreamed. */
    void registerStreamObjs(const std::vector<std::string>& list);

    // Pipelined destreaming of EvtMessage using thread

    /** Queue EvtMessage for destreaming
     *  @param msg        Event buffer to be restored.
     */
    int queueEvtMessage(char* msg);

    /** Decode EvtMessage and store objects in temporary buffer
     *  @param id         Thread id
     */
    void* decodeEvtMessage(int id);

    /** Restore objects in DataStore from temporary buffer
     */
    int restoreDataStoreAsync();

    /** maximum number of threads. */
    void setMaxThreads(int);
    /** maximum number of threads. */
    int  getMaxThreads();
    /** Ask Itoh-san about this. */
    void setDecoderStatus(int);
    /** Ask Itoh-san about this. */
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

    /** names of object to be streamed
     *
     * If size=0, all objects to be streamed
     */
    std::vector<std::string> m_streamobjnames;

    /** Max. number of threads for asynchronous processing
     *
     * 4 for default
     */
    int m_maxthread;

    /** thread pointer
     */
    pthread_t m_pt[c_maxThreads];
    int m_id[c_maxThreads];
    int m_threadin;
    //int m_threadout;
    int m_decstat[c_maxThreads];
    //MsgHandler* m_pmsghandler[c_maxThreads];
    //    char* m_evtbuf[c_maxThreads];
    std::queue<char*> m_evtbuf[c_maxThreads];

    //@{
    /** Object arrays in temporary buffer
     *
     */

    //    int m_nobjs[c_maxThreads];
    //    int m_narrays[c_maxThreads];
    //    DataStore::EDurability m_durability[c_maxThreads];
    //    std::vector<TObject*> m_objlist[c_maxThreads];
    //    std::vector<std::string> m_namelist[c_maxThreads];

    std::queue<int> m_nobjs;
    std::queue<int> m_narrays;
    std::queue<DataStore::EDurability> m_durability;
    std::queue<std::vector<TObject*>> m_objlist;
    std::queue<std::vector<std::string>> m_namelist;
    //@}
  };

  // Function to hook DataStoreStreamer::decodeEvtMessage to pthread

  //  void* RunDecodeEvtMessage ( void* );


} // namespace Belle2

#endif
