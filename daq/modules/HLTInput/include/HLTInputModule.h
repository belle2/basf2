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
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

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
    //! Write a data into a file (Development purpose only)
    void writeFile(char* data, int size);

  private:
    std::string m_nodeType;       /**< Node type of this node */

    RingBuffer* m_inBuffer;       /**< Buffer for incoming data */
    RingBuffer* m_outBuffer;      /**< Buffer for outgoing data */

    MsgHandler* m_msgHandler;     /**< MsgHandler to decode data */

    int m_eventsTaken;            /**< Counter for received data */
    int m_nDataSources;           /**< Number of data sources */
  };
}

#endif
