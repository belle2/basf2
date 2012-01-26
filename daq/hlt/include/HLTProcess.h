/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HLTPROCESS_H
#define HLTPROCESS_H

#include <sstream>
#include <wait.h>

#include <boost/lexical_cast.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <framework/logging/Logger.h>
#include <daq/hlt/HLTDefs.h>

#include <daq/hlt/HLTSender.h>
#include <daq/hlt/HLTReceiver.h>
#include <daq/hlt/NodeInfo.h>
#include <framework/pcore/RingBuffer.h>

namespace Belle2 {

  //! HLTProcess class
  /*! This class is for process nodes
  */
  class HLTProcess {
  public:
    //! Constructor
    HLTProcess();
    //! Destructor
    ~HLTProcess();

    //! Initialize the control line
    EHLTStatus initControl();
    //! Initialize HLTSenders for data transfer
    EHLTStatus initSenders();
    //! Initialize HLTReceivers for data transfer
    EHLTStatus initReceivers();

    //! Process data (This forks out basf2)
    EHLTStatus process();
    //! Check the status of child processes
    EHLTStatus checkChildren();

    //! Check if the process is child
    bool isChild();
    //! Display process mode
    void displayMode(const std::string mode);

  private:
    std::vector<pid_t> m_HLTSenders;        /**< Container for process identifiers of HLTSenders */
    std::vector<pid_t> m_HLTReceivers;      /**< Container for process identifiers of HLTReceivers */
    pid_t m_Process;                        /**< Container for process identifier of basf2 */
    pid_t m_Control;                        /**< Container for process identifier of HLTReceiver for management */
    pid_t m_Monitor;                        /**< Container for process identifier of HLTSender for monitoring */

    bool m_isChild;                         /**< Flag that denotes if the process is child process or not */

    RingBuffer* m_controlInBuffer;          /**< Pointer to ring buffer for management */
    RingBuffer* m_controlOutBuffer;         /**< Pointer to ring buffer for monitoring */
    RingBuffer* m_dataInBuffer;             /**< Pointer to ring buffer for outgoing data */
    RingBuffer* m_dataOutBuffer;            /**< Pointer to ring buffer for incoming data */

    NodeInfo m_nodeInfo;                    /**< Node information of this node */
  };
}

#endif
