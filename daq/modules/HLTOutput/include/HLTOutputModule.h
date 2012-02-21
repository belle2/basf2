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
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

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
    void putData(const DataStore::EDurability& durability);
    //! Test data (Development purpose only)
    EHLTStatus testData(char* buffer);
    //! Send terminate code to ring buffer
    void sendTerminate();

    //! Compare two data set (Development purpose only)
    bool checkData(std::string data1, char* data2);
    //! Write a data into a file (Development purpose only)
    void writeFile(char* data, int size);

  private:
    RingBuffer* m_buffer;           /**< Buffer for outgoing data */
    MsgHandler* m_msgHandler;       /**< MsgHandler to encode data */

    std::string m_nodeType;         /**< Node type of this node */
    int m_eventsSent;               /**< Counter for sent events */

    std::vector<std::string> m_branchNames[DataStore::c_NDurabilityTypes];  /**< Branch names */
    bool m_done[DataStore::c_NDurabilityTypes];                             /**< Flag for data */
    StoreIter* m_objectIterator[DataStore::c_NDurabilityTypes];             /**< Iterator for objects in DataStore */
    StoreIter* m_arrayIterator[DataStore::c_NDurabilityTypes];              /**< Iterator for arrays in DataStore */
  };
}

#endif
