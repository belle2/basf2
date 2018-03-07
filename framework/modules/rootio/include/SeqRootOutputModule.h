//+
// Description : Sequential ROOT I/O output module
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#pragma once

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/SeqFile.h>
#include <framework/pcore/DataStoreStreamer.h>

#include <string>

#include <sys/time.h>

namespace Belle2 {
  /** Output module for sequential ROOT I/O.
   *
   * As an alternative to the TTree format, this format stores data as a sequential stream of objects plus a small meta-data header, which doesn't impose the overhead of the TTree and may result in higher read rates from hard disks. It is also the storage format used by DAQ and HLT. SeqRoot files however tend be a factor 2-3 larger than their .root equivalents.
   */
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

    ///! Write StreamerInfos to a file
    void getStreamerInfos();

    //! File name
    std::string m_outputFileName;

    //! If true the output filename will be interpreted as a boost::format pattern
    bool m_fileNameIsPattern;

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

    //! StreamerInfo to be written
    char* m_streamerinfo;

    //! The size of the StreamerInfo
    int m_streamerinfo_size;

    //! Time
    struct timeval m_t0; /**< time at begin of current run. */
    struct timeval m_tend; /**< time at end of current run. */

    // Data flow
    double m_size; /**< total transferred data, in kB. */
    double m_size2; /**< sum of squares of data transferred in each event, in kB^2. */

    /** List of objects to be saved. */
    std::vector<std::string> m_saveObjs;

  };
} // end namespace Belle2
