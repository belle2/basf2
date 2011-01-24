//+
// File : pseqrootoutput.h
// Description : Sequential ROOT I/O output server for pbasf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#ifndef PSEQROOT_INPUT_H
#define PSEQROOT_INPUT_H

#include <framework/core/Module.h>
#include <framework/core/Framework.h>
#include <framework/pcore/pEventServer.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/pcore/RbCtlMgr.h>
#include <framework/pcore/SeqFile.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <framework/datastore/DataStore.h>

/*! RINBGBUF_SIZE and MAXEVTSIZE defines maximum size of IPC shared memory for RingBuffer */
/*!
  Defaulted to 4MBytes which is the default maximum of standard Linux setting.
  The size should be increased for better performance. To increase the size, system parameter
  has to be changed by adding
      kernel.shmmax = MAX_SHAREDMEM_SIZE
  in /etc/sysctl.conf and issue
   % sysctl -p
  in root account. The default Linux setting(4MB) comes from ancient limitation on the IPC memory
  size of old UNIX which is now completely obsolete. You can set any values to it.
  100MBytes could be a recommended value for it.
*/
#define RINGBUF_SIZE 1000000*4     // 1Mwords=4MB for ring buffer
#define MAXEVTSIZE   1000000*4     // 1Mwords=4MB for maximum event size

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class pSeqRootInputModule : public pEventServer {

    // Public functions
  public:

    //! Constructor / Destructor
    pSeqRootInputModule();
    virtual ~pSeqRootInputModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    //! Module function to be called from event server process
    virtual void event_server();

    // Data members
  private:
    //! File name
    std::string m_inputFileName;

    //! Compression level
    int m_compressionLevel;

    //! DataStore iterators
    StoreIter* m_obj_iter[DataStore::c_NDurabilityTypes];
    StoreIter* m_array_iter[DataStore::c_NDurabilityTypes];


    //! Blocked file handler
    SeqFile* m_file;

    // Parallel processing parameters

    //! Nr. of processes
    int m_nproc;

    //! RingBuffer ID
    RingBuffer* m_rbuf;

    //! Total nr. of events in the file
    int m_nevt;

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! Process contrl manager
    RbCtlMgr* m_rbctl;

    //! No. of sent events
    int m_nsent;

    //! No. of received events
    int m_nrecv;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
