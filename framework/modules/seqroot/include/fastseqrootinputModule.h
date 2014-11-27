//+
// File : fastseqrootoutput.h
// Description : Sequential ROOT I/O input module with pipeline
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 28 - Nov - 2013
//-

#ifndef FASTSEQROOT_INPUT_H
#define FASTSEQROOT_INPUT_H

#include <framework/core/Module.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/pcore/SeqFile.h>

#include <string>

#include <sys/time.h>
#include <pthread.h>

namespace Belle2 {
  /** A class definition of an input module for Sequential ROOT I/O. */
  class FastSeqRootInputModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    FastSeqRootInputModule();
    virtual ~FastSeqRootInputModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    //! Function to read events
    void ReadFileInThread();

    // Data members
  private:
    //! File name
    std::string m_inputFileName;

    //! Compression Level
    int m_compressionLevel;

    //! Number of decoder threads
    int m_numThread;

    //! Blocked file handler
    SeqFile* m_file;
    /** file-reading thread. */
    pthread_t m_thr_input;

    //! Total nr. of events in the file
    int m_nevt;

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
