/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DS2SAMPLEMODULE_H
#define DS2SAMPLEMODULE_H

#include <framework/core/Module.h>
#include <framework/pcore/RingBuffer.h>

#include <framework/pcore/DataStoreStreamer.h>

#include <string>

#define RBUFSIZE 100000000

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class Ds2SampleModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    Ds2SampleModule();
    virtual ~Ds2SampleModule();

    //! Module functions to be called from main process
    void initialize() override;

    //! Module functions to be called from event process
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

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

    /** List of objects to be saved. */
    std::vector<std::string> m_saveObjs;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
