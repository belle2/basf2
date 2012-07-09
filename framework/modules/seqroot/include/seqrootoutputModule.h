//+
// File : seqrootoutput.h
// Description : Sequential ROOT I/O output module
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#ifndef PSEQROOT_OUTPUT_H
#define PSEQROOT_OUTPUT_H

#include <framework/core/Module.h>
#include <framework/core/Framework.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/SeqFile.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <framework/datastore/DataStore.h>

namespace Belle2 {
  /**
   * \addtogroup modules
   * @{
   */

  /*! Class definition for the output module of Sequential ROOT I/O */

  class SeqRootOutputModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    SeqRootOutputModule();
    virtual ~SeqRootOutputModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  private:
    //! Build EvtMessage from DataStore
    EvtMessage* buildMessage(RECORD_TYPE);

    // Data members
  private:
    //! File name
    std::string m_outputFileName;

    //! Compression level
    int m_compressionLevel;


    //! Blocked file handler
    SeqFile* m_file;
    int m_fd;

    // Parallel processing parameters

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
