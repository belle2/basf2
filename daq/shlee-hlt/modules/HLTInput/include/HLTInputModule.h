/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HLTINPUTMODULE_H
#define HLTINPUTMODULE_H

#include <string>
#include <fstream>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/SeqFile.h>
#include <framework/pcore/DataStoreStreamer.h>

#include <TSystem.h>

#include <framework/logging/Logger.h>
#include <daq/hlt/HLTDefs.h>
#include <framework/pcore/RingBuffer.h>

namespace Belle2 {

  //! HLTInputModule class
  /*! This module accepts data from network and store them into DataStore
  */
  class HLTInputModule : public Module {
  public:
    //! Constructor
    HLTInputModule();
    //! Destructor
    virtual ~HLTInputModule();

    //! Initialize the module
    virtual void initialize();
    //! Begin a run
    virtual void beginRun();
    //! Event processing
    virtual void event();
    //! End a run
    virtual void endRun();
    //! Terminate the module
    virtual void terminate();

    //! Get data from network (incoming ring buffer, actually)
    EHLTStatus getData();

  private:
    RingBuffer* m_buffer;         /**< Buffer for incoming data */

    SeqFile* m_file;                /**< Output file handler (only for EM) */
    std::string m_outputFileName;   /**< Output file name (only for EM) */

    std::string m_nodeType;         /**< Node type of this node */

    DataStoreStreamer* m_streamer;  /**< DataStore streamer */

    int m_nDataSources;             /**< Number of data sources */

    //! Variables for performance check
    struct timeval m_t0;            /**< Time stamp at the beginning */
    struct timeval m_tEnd;          /**< Time stamp at the end */

    double m_timeDeserialized;      /**< Elapsed time of deserialization */
    double m_timeIO;                /**< Elapsed time of file I/O */
    double m_timeEvent;             /**< Total elapsed time for event processing */

    double m_size;                  /**< Total size of taken events */
    double m_size2;                 /**< Square of total size of taken events */
    int m_nEvents;                  /**< Total number of taken events */
  };
}

#endif
