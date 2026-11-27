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
    virtual ~StorageSerializerModule() override;

    //! Module functions to be called from main process
    void initialize() override;

    //! Module functions to be called from event process
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

    // Data members
  private:
    //! Compression level
    int m_compressionLevel;

    //! DataStoreStreamer
    DataStoreStreamer* m_streamer = nullptr;

    //! Messaage handler
    MsgHandler* m_msghandler = nullptr;

    //! Ring Buffer to dump sampled output stream
    std::string m_obuf_name;
    int m_obuf_size;
    SharedEventBuffer m_obuf;

    //! Exp number, Run number
    unsigned int m_count = 0;
    unsigned int m_count_0 = 0;
    int m_expno = 0;
    int m_runno = 0;
    unsigned long long m_nbyte = 0;
    int m_nodeid;

  private:
    int writeStreamerInfos();

  };
} // end namespace Belle2

#endif
