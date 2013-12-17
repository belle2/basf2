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
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/SeqFile.h>
#include <framework/pcore/DataStoreStreamer.h>

#include <string>

#include <sys/time.h>

namespace Belle2 {
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

    //! Total nr. of events in the file
    int m_nevt;

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! DataStoreStreamer
    DataStoreStreamer* m_streamer;

    //! Time
    struct timeval m_t0; /**< time at begin of current run. */
    struct timeval m_tend; /**< time at end of current run. */

    // Data flow
    double m_size; /**< total transferred data, in kB. */
    double m_size2; /**< sum of squares of data transferred in each event, in kB^2. */

  };
} // end namespace Belle2

#endif
