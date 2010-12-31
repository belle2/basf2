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
  enum EHLTPort {
    c_DataInPort = 20000,
    c_DataOutPort = 25000,
    c_InfoInPort = 30000,
    c_InfoOutPort = 35000,
    c_ControlPort = 30000,
    c_MonitorPort = 35000
  };

  enum ENodeType {
    c_ManagerNode,
    c_ProcessNode,
    c_EventSeparator,
    c_WorkerNode,
    c_EventMerger,
    c_NotDefined
  };

  enum EMessage {
    c_Initialization,
    c_StartRun,
    c_EndRun,
    c_Termination
  };

  enum EStatus {
    c_FuncError,
    c_InitFailed,
    c_ProcFailed,
    c_TermCalled,
    c_Success
  };
}

#endif

