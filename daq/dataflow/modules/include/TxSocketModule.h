/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TXMODULE_H
#define TXMODULE_H

#include <framework/core/Module.h>
#include <daq/dataflow/EvtSocket.h>

#include <string>

#include <framework/pcore/DataStoreStreamer.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class TxSocketModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    TxSocketModule();
    virtual ~TxSocketModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:

    //!Compression parameter
    int m_compressionLevel;

    // Parameters for EvtSocket

    //! Destination Host
    std::string m_dest;

    //! Destination port
    int m_port;

    //! EvtSocket
    EvtSocketSend* m_sock;

    //! DataStoreStreamer
    DataStoreStreamer* m_streamer;

    //! No. of sent events
    int m_nsent;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
