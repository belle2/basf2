/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef STORAGE_OUTPUT_H
#define STORAGE_OUTPUT_H

#include <framework/core/Module.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/DataStoreStreamer.h>

#include <daq/storage/SharedEventBuffer.h>

#include <string>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class StorageSerializerModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    StorageSerializerModule();
    virtual ~StorageSerializerModule();

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

    //! DataStoreStreamer
    DataStoreStreamer* m_streamer;

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! Ring Buffer to dump sampled output stream
    std::string m_obuf_name;
    int m_obuf_size;
    SharedEventBuffer m_obuf;

    //! Exp number, Run number
    unsigned int m_count;
    unsigned int m_count_0;
    int m_expno;
    int m_runno;
    unsigned long long m_nbyte;
    double m_ctime;
    int m_nodeid;

  private:
    int writeStreamerInfos();

  };
} // end namespace Belle2

#endif
