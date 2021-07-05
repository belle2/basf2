/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef RBUF2DSMODULE_H
#define RBUF2DSMODULE_H

#include <framework/core/Module.h>
#include <framework/pcore/RingBuffer.h>

#include <string>

#include <framework/pcore/DataStoreStreamer.h>

#define RBUFSIZE 100000000
#define MAXEVTSIZE  400000000


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class Rbuf2DsModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    Rbuf2DsModule();
    virtual ~Rbuf2DsModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:

    //! RingBuffer ID
    std::string m_rbufname;
    RingBuffer* m_rbuf;

    //! DataStore streamer
    DataStoreStreamer* m_streamer;

    //! Compression Level
    int m_compressionLevel;

    //! No. of sent events
    int m_nrecv;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
