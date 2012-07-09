//+
// File : seqrootoutput.h
// Description : Sequential ROOT I/O input module
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#ifndef SEQROOT_INPUT_H
#define SEQROOT_INPUT_H

#include <framework/core/Module.h>
#include <framework/core/Framework.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/SeqFile.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <sys/time.h>

#include <framework/datastore/DataStore.h>

#define MAXEVTSIZE 400000000*4

namespace Belle2 {
  /**
   * \addtogroup modules
   * @{
   */

  /*! A class definition of an input module for Sequential ROOT I/O */

  class SeqRootInputModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    SeqRootInputModule();
    virtual ~SeqRootInputModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

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
    int m_fd;

    //! Total nr. of events in the file
    int m_nevt;

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! Time
    struct timeval m_t0;
    struct timeval m_tend;

    //! Data flow
    double m_size;
    double m_size2;

  };

  /*! @} */
} // end namespace Belle2

#endif // MODULEHELLO_H
