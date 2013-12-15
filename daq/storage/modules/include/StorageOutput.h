//+
// File : storageoutput.h
// Description : Sequential ROOT I/O output module for DAQ
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//        9  - Dec - 2013 ; update for DAQ
//-

#ifndef STORAGE_OUTPUT_H
#define STORAGE_OUTPUT_H

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/SeqFile.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/pcore/RingBuffer.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <string>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cmath>
#include <fcntl.h>


namespace Belle2 {
  /**
   * \addtogroup modules
   * @{
   * \addtogroup framework_modules
   * \ingroup modules
   * @{ SeqRootOutputModule @} @}
   */

  /*! Class definition for the output module of Sequential ROOT I/O */

  class StorageOutputModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    StorageOutputModule();
    virtual ~StorageOutputModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    //! open data file by looking at exp/run number
    SeqFile* openDataFile();
    //! Function to close file from outside
    //    void closeDataFile();


    // Data members
  private:
    //! Exp number, Run number
    int m_exp;
    int m_run;

    //! Compression level
    int m_compressionLevel;

    //! Storage directory
    std::string m_stordir;

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

    //! Data flow
    double m_size; /**< total transferred data, in kB. */
    double m_size2; /**< sum of squares of data transferred in each event, in kB^2. */

    //! Ring Buffer to dump sampled output stream
    std::string m_obufname;
    RingBuffer* m_obuf;

    //! Event interval to dump output in RingBuffer
    int m_interval;

  };
} // end namespace Belle2

#endif
