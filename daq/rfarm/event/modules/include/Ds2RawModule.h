/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DS2RBUFMODULE_H
#define DS2RBUFMODULE_H

#include <framework/core/Module.h>
#include <framework/pcore/RingBuffer.h>

#include <framework/pcore/DataStoreStreamer.h>

#include <string>

#define RBUFSIZE 100000000
#define HLT_SUBSYS_ID 300

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class Ds2RawModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    Ds2RawModule();
    virtual ~Ds2RawModule();

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

    //! RingBuffer ID
    std::string m_rbufname;
    RingBuffer* m_rbuf;

    //! DataStore streamer
    DataStoreStreamer* m_streamer;

    //! No. of sent events
    int m_nsent;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
