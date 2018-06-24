/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee, Anseln Baur, Nils Braun                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <zmq.hpp>
#include <set>
#include <string>
#include <framework/pcore/ProcHelper.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <memory>

namespace Belle2 {

  /** A class to manage processes for parallel processing. */
  class ProcHandler {
  public:
    /** Constructor */
    ProcHandler(unsigned int nWorkerProc);

    bool startInputProcess();
    bool startWorkerProcesses();
    bool startOutputProcess();
    bool startProxyProcess();
    bool startMonitoringProcess();

    /** Returns true if multiple processes have been spawned, false in single-core mode. */
    static bool parallelProcessingUsed();

    /** Return true if the process is the same ProcType then the parameter*/
    static bool isProcess(ProcType procType);
    static bool isOutputProcess();
    static bool isWorkerProcess();

    /** Get a name for this process. (input, event, output...). */
    static std::string getProcessName();

    static int EvtProcID();

    static void killAllProcesses();

    const std::vector<int>& getPIDList() const;

  private:
    /** Start a new process, sets the type and id and returns true if in this new process. Also adds the signal handling. */
    bool startProc(ProcType procType, int id);

    /**< Number of worker processes controlled by this ProcHandler. */
    unsigned int m_numWorkerProcesses;
  };
}
