//+
// File : pseqrootoutput.h
// Description : Sequential ROOT I/O output server for pbasf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#ifndef PSEQROOT_OUTPUT_H
#define PSEQROOT_OUTPUT_H

#include <framework/core/Module.h>
#include <framework/core/Framework.h>
#include <framework/pcore/pEventServer.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/pcore/SeqFile.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreDefs.h>

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

  /*! Class definition for the output module of Sequential ROOT I/O */

  class pSeqRootOutput : public pOutputServer {

    // Public functions
  public:

    //! Constructor / Destructor
    pSeqRootOutput();
    virtual ~pSeqRootOutput();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    //! Module function to be called from event server process
    virtual void output_server();

  private:
    //! Build EvtMessage from DataStore
    EvtMessage* buildMessage(RECORD_TYPE);

    // Data members
  private:
    //! File name
    std::string m_outputFileName;

    //! Compression level
    int m_compressionLevel;

    //! DataStore iterators
    StoreIter* m_obj_iter[c_NDurabilityTypes];
    StoreIter* m_array_iter[c_NDurabilityTypes];


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

    //! No. of sent events
    int m_nsent;

    //! No. of received events
    int m_nrecv;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
