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

#include <daq/storage/SharedEventBuffer.h>

#include <daq/slc/system/Time.h>

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

    // Data members
  private:
    //! Compression level
    int m_compressionLevel;

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! DataStoreStreamer
    DataStoreStreamer* m_streamer;

    //! Ring Buffer to dump sampled output stream
    std::string m_obufname;
    SharedEventBuffer m_obuf;

    //! Exp number, Run number
    unsigned int m_count;
    unsigned int m_count_0;
    unsigned int m_expno;
    unsigned int m_runno;
    unsigned long long m_nbyte;
    double m_ctime;

  };
} // end namespace Belle2

#endif
