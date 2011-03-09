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
    c_InfoInPort = 30000,     /**< Port for incoming information */
    c_InfoOutPort = 35000,    /**< Port for outgoing information */
    c_ControlPort = 30000,    /**< Port for control */
    c_MonitorPort = 35000     /**< Port for monitoring information */
  };

  /** Types of nodes in HLT
   *  At this moment (r1076), only c_ManagerNode and c_ProcessNode are used
   */
  enum ENodeType {
    c_ManagerNode,      /**< Manager node */
    c_ProcessNode,      /**< Process nodes including event separator, worker node, and event merger */
    c_EventSeparator,   /**< Event separator node */
    c_WorkerNode,       /**< Worker node */
    c_EventMerger,      /**< Event merger node */
    c_NotDefined        /**< Not specified node */
  };

  /** Types of message passed in HLT
   *  Not used right now
   */
  enum EMessage {
    c_Initialization,   /**< To initialize */
    c_StartRun,         /**< To start runs */
    c_EndRun,           /**< To end runs */
    c_Termination       /**< To terminate all */
  };

  /** Status of functions in hbasf2
   *  ProcFailed is not used at this moment
   */
  enum EStatus {
    c_FuncError,        /**< Function error somehow (for normal functions) */
    c_InitFailed,       /**< Initialization failure (for initialization functions) */
    c_ProcFailed,       /**< Process failure */
    c_TermCalled,       /**< Termination requested */
    c_Success           /**< Everything's ok :D */
  };

  const int gBufferSize = 10000000 * 4;
}

#endif

