/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HLTOUTPUTMODULE_H
#define HLTOUTPUTMODULE_H

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

  //! HLTOutputModule class
  /*! This module read data from DataStore and send to other node
  */
  class HLTOutputModule : public Module {
  public:
    //! Constructor
    HLTOutputModule();
    //! Destructor
    virtual ~HLTOutputModule();

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

    //! Put data into ring buffer for outgoing communication
    void putData();
    //! Send terminate code to ring buffer
    void sendTerminate();

  private:
    RingBuffer* m_buffer;           /**< Buffer for outgoing data */

    SeqFile* m_file;                /**< Input file handler (only for ES) */
    std::string m_inputFileName;    /**< Input file name (only for ES) */

    std::string m_nodeType;         /**< Node type of this node */
    int m_eventsSent;               /**< Counter for sent events */

    DataStoreStreamer* m_streamer;  /**< DataStore streamer */

    //! Variables for performance check
    struct timeval m_t0;            /**< Time stamp at the beginning */
    struct timeval m_tEnd;          /**< Time stamp at the end */

    double m_timeSerialized;        /**< Elapsed time of serialization */
    double m_timeIO;                /**< Elapsed time of file I/O */
    double m_timeEvent;             /**< Total elapsed time */

    double m_size;                  /**< Total size of taken events */
    double m_size2;                 /**< Square of total size of taken events */
    int m_nEvents;                  /**< Total number of taken events */
  };
}

#endif
