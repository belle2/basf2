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

#include <framework/pcore/ProcHelper.h>
#include <vector>
#include <string>
#include <map>

namespace Belle2 {

  /** A class to manage processes for parallel processing. */
  // TODO: Would be best to have a singleton here!
  class ProcHandler {
  public:
    /// Create a new process handler, which will handle nWorkerProc processes.
    static void initialize(unsigned int nWorkerProc);

    /// Returns true if multiple processes have been spawned, false in single-core mode.
    static bool parallelProcessingUsed();

    /// Return true if the process is of type procType.
    static bool isProcess(ProcType procType);
    /// Return true if the process is of type c_Output.
    static bool isOutputProcess();
    /// Return true if the process is of type c_Worker.
    static bool isWorkerProcess();
    /// Get a human readable name for this process. (input, event, output...).
    static std::string getProcessName();
    /// Get the ID of this process. Attention: this ID may not be a stable API feature.
    static int EvtProcID();
    /// Hard kill all processes.
    static void killAllProcesses();
    /// Return the PID list handled by the running ProcHandler. This PID list is updated using the signal handlers and the startup of processes.
    static const std::vector<int>& getPIDList();

    static unsigned int numEventProcesses();

    static bool startInputProcess();
    static bool startWorkerProcesses(unsigned int numProcesses);
    static bool startOutputProcess();
    static bool startProxyProcess();
    static bool startMonitoringProcess();

    static ProcType getProcType(int pid);

  private:
    static ProcType s_procType;
    static int s_processID;
    static int s_numEventProcesses;

    // global list of PIDs managed by ProcHandler.
    // (directly modifying STL structures in the signal handler is unsafe, so let's be overly
    // cautious and use only C-like functions there.)
    // PIDs are added using addPID() while forking, items are set to 0 when process stops
    static std::vector<int> s_pidVector;
    /// Which PIDs were started with which types
    static std::map<int, ProcType> s_startedPIDs;

    static void addPID(int pid);
    static bool findPID(int pid);
    static void removePID(int pid);
    static void clearPIDs();
    static bool pidListEmpty();
    static void childSignalHandler(int);

    /** Start a new process, sets the type and id and returns true if in this new process. Also adds the signal handling. */
    static bool startProc(ProcType procType, int id);
  };
}
