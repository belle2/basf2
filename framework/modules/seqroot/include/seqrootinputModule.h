//+
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#ifndef SEQROOT_INPUT_H
#define SEQROOT_INPUT_H

#include <framework/core/Module.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/pcore/SeqFile.h>

#include <string>

#include <sys/time.h>

namespace Belle2 {
  /** Module to read files produced by SeqRootOutputModule. */
  class SeqRootInputModule : public Module {
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

  private:
    //! File name
    std::string m_inputFileName;

    //! Blocked file handler
    SeqFile* m_file;

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
