/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HLTDEFS_H
#define HLTDEFS_H

namespace Belle2 {
  /** Port numbers to be used in HLT
   *  Info*Port and ControlPort, MonitorPort are redundant so need to be cleaned up
   */
  enum EHLTPort {
    c_DataInPort = 20000,     /**< Port for incoming data */
    c_DataOutPort = 25000,    /**< Port for outgoing data */
    c_ControlPort = 30000,    /**< Port for control */
    c_MonitorPort = 35000     /**< Port for monitoring information */
  };

  /** Types of nodes in HLT
   *  At this moment (r1076), only c_ManagerNode and c_ProcessNode are used
   */
  enum EHLTNodeType {
    c_ManagerNode,      /**< Manager node */
    c_ProcessNode,      /**< Process nodes including event separator, worker node, and event merger */
    c_EventSeparator,   /**< Event separator node */
    c_WorkerNode,       /**< Worker node */
    c_EventMerger,      /**< Event merger node */
    c_Undefined         /**< Not specified node */
  };

  /** Types of message passed in HLT
   *  Not used right now
   */
  enum EHLTMessage {
    c_Data,
    c_Termination       /**< To terminate all */
  };

  /** Status of functions in hbasf2
   *  ProcFailed is not used at this moment
   */
  enum EHLTStatus {
    c_FuncError,        /**< Function error somehow (for normal functions) */
    c_InitFailed,       /**< Initialization failure (for initialization functions) */
    c_ProcFailed,       /**< Process failure */
    c_TermCalled,       /**< Termination requested */
    c_Success,          /**< Everything's ok :D */

    c_ChildFuncError,   /**< Functional error from children */
    c_ChildInitFailed,  /**< Initialization error from children */
    c_ChildProcFailed,  /**< Process failure from children */
    c_ChildTermCalled,  /**< Termination requested from children */
    c_ChildSuccess      /**< Everything's ok from children */
  };

  const int gBufferSize = 100000000 * 4;   /**< Buffer size for HLT */
  const int gMaxHosts = 100;              /**< Maximum number of hosts to connect */
  const int gMaxConnections = 100;        /**< Maximum number of connection allowed */
  const int gMaxReceives = 10000000;       /**< Maximum size of incoming data allowed */
  const int gBufferArray = 10;

  const std::string gEOSTag = "EOSS";      /**< End-of-stream tag for data capsulation */
  const std::string gTerminate = "Terminat";
  const std::string gTerminationTag = gTerminate + gEOSTag;
  const std::string gEndOfEvent = "EndOfEvent";

  const std::string gDataInBufferKey = "B2DataInBuffer";
  const std::string gDataOutBufferKey = "B2DataOutBuffer";
}

#endif

