//+
// File : DataStoreStreamer.h
// Description : Stream/Destream DataStore objects in EvtMessage
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 5 - Sep - 2012

#pragma once

#include <framework/pcore/EvtMessage.h>

#include <Rtypes.h> //for BIT()

#include <pthread.h>

#include <vector>
#include <string>

class TList;

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
    static const unsigned int c_maxThreads = 16;
    /** Ask Itoh-san. */
    static const unsigned int c_maxQueueDepth = 64;

    /** Constructor
     *  @param complevel  Compression level of streaming, 0 to disable
     *  @param handleMergeable perform special handling for Mergeable objects?
     *  @param maxthread  maximal number of threads, 0 to disable
     */
    explicit DataStoreStreamer(int complevel = 0, bool handleMergeable = true, int maxthread = 0);

    /** destructor */
    ~DataStoreStreamer();

    // DataStore->EvtMessage
    /** Store DataStore objects in EvtMessage
     *
     *  @param addPersistentDurability By default, only c_Event data is streamed. Setting this to true will add c_Persistent data to the EvtMessage.
     *  @param streamTransientObjects Should objects/arrays registered as transient be streamed?
     *  @return pointer to EvtMessage, caller is responsible for deletion
     */
    EvtMessage* streamDataStore(bool addPersistentDurability, bool streamTransientObjects = false);


    // EvtMessage->DataStore
    /** Restore DataStore objects from EvtMessage
     *  @param msg        EvtMessage to be restored.
     */
    int restoreDataStore(EvtMessage* msg);

    /** Set names of objects to be streamed/destreamed. */
    void setStreamingObjects(const std::vector<std::string>& list);

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

    /** Is the given object of a type that can be merged? */
    static bool isMergeable(const TObject* object);

    /** assuming object is mergeable, clear its contents.
     *
     * Use this after sending it to prevent sending the same data again in the next event.
     */
    static void clearMergeable(TObject* object);

    /** Assuming both objects are mergeable, merge 'received' into 'existing'. */
    static void mergeIntoExisting(TObject* existing, const TObject* received);

    /** call clear() and removeSideEffects() for all Mergeable objects in datastore (for c_Persistent durability). */
    static void removeSideEffects();

  private:

    /** restore StreamerInfo from data in a file */
    int restoreStreamerInfos(const TList* list);

    /** bits to store in TObject.
     *
     * Bits 14-23 are available for use in derived classes, and are reused here to transmit additional information. This is really quite ugly and should be replaced with some more sane way of transmitting object-level data.
     * All bits are checked before using them, so if they are used by other code we know what happens.
     */
    enum ETObjectBits {
      c_IsTransient = BIT(19), /**< The corresponding StoreEntry has flag c_DontWriteOut. */
      c_IsNull = BIT(20), /**< object is not valid for current event, set StoreEntry::ptr to NULL. */
      c_PersistentDurability = BIT(21) /**< Object is of persistent durability. */
    };

    /** MsgHandler
     *
     * MsgHandler object used to form/decode EvtMessage
     */
    MsgHandler* m_msghandler;

    /** Compression level in streaming */
    int m_compressionLevel;

    bool m_handleMergeable; /**< Whether to handle Mergeable objects. */

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
    /** thread index. */
    int m_id[c_maxThreads];
    /** current thread? */
    int m_threadin;
    //int m_threadout;
    /** thread decoder status. */
    int m_decoderStatus[c_maxThreads];
    //MsgHandler* m_pmsghandler[c_maxThreads];
    //    char* m_evtbuf[c_maxThreads];
    //std::queue<char*> m_evtbuf[c_maxThreads];

  };

  // Function to hook DataStoreStreamer::decodeEvtMessage to pthread

  //  void* RunDecodeEvtMessage ( void* );


} // namespace Belle2
