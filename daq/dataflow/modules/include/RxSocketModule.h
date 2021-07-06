/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef RXMODULE_H
#define RXMODULE_H

#include <framework/core/Module.h>
#include <daq/dataflow/EvtSocket.h>

#include <framework/pcore/DataStoreStreamer.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class RxSocketModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    RxSocketModule();
    virtual ~RxSocketModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:

    //! Receiver Port
    int m_port;

    //! Reciever Socket
    EvtSocketRecv* m_recv;

    //! DataStoreStreamer
    DataStoreStreamer* m_streamer;

    //! Compression Level
    int m_compressionLevel;

    //! No. of sent events
    int m_nrecv;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
