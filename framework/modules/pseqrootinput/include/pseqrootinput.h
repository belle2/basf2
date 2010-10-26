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
#include <framework/core/ModuleManager.h>
#include <framework/core/Framework.h>
#include <framework/pcore/pEventServer.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/pcore/pFramework.h>
#include <framework/pcore/RbCtlMgr.h>
#include <framework/pcore/SeqFile.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreDefs.h>

#define RINGBUF_SIZE 4000000*4     // 4Mwords for ring buffer
#define MAXEVTSIZE   4000000*4     // 4Mwords for maximum event size


namespace Belle2 {
  class pSeqRootInput : public pEventServer {

    // Public functions
  public:

    //! Constructor / Destructor
    pSeqRootInput();
    virtual ~pSeqRootInput();

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

    //! Process contrl manager
    RbCtlMgr* m_rbctl;

    //! No. of sent events
    int m_nsent;

    //! No. of received events
    int m_nrecv;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
