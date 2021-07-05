/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef Belle2_Ds2RawFileModule_h
#define Belle2_Ds2RawFileModule_h

#include <framework/core/Module.h>

#include <framework/pcore/DataStoreStreamer.h>

#define RBUFSIZE 100000000
#define HLT_SUBSYS_ID 300

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class Ds2RawFileModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    Ds2RawFileModule();
    virtual ~Ds2RawFileModule();

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
    //std::string m_rbufname;
    //RingBuffer* m_rbuf;

    //! DataStore streamer
    DataStoreStreamer* m_streamer;

    //! No. of sent events
    int m_nsent;
    int m_file;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
