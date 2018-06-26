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
  class ProcHandler {
  public:
    /// Create a new process handler, which will handle nWorkerProc processes.
    static void initialize(unsigned int nWorkerProc);

    /** Fork and initialize an input process */
    static bool startInputProcess();
    /** Fork and initialize as many worker processes as requested. */
    static bool startWorkerProcesses(unsigned int numProcesses);
    /** Fork and initialize an output process. If local is true, do not fork. */
    static bool startOutputProcess(bool local = false);
    /** Fork and initialize a proxy process. */
    static bool startProxyProcess();
    /** Fork and initialize a monitoring process. */
    static bool startMonitoringProcess();

    /** Wait until all forked processes handled by this ProcHandler terminate. */
    static void waitForAllProcesses();

    /** Returns true if multiple processes have been spawned, false in single-core mode. */
    static bool parallelProcessingUsed();

    /// Return true if the process is of type c_Input.
    static bool isInputProcess();
    /// Return true if the process is of type c_Worker.
    static bool isWorkerProcess();
    /// Return true if the process is of type c_Output.
    static bool isOutputProcess();

    /** Return number of worker processes (configured value, not current) */
    static int numEventProcesses();
    /// Get the ID of this process. Attention: this ID may not be a stable API feature.
    static int EvtProcID();

    /// Get a human readable name for this process. (input, event, output...).
    static std::string getProcessName();
    /// Hard kill all processes.
    static void killAllProcesses();
    /// Return the PID list handled by the running ProcHandler. This PID list is updated using the signal handlers and the startup of processes.
    static const std::vector<int>& getPIDList();

    /// Return true if the process is of type procType.
    static bool isProcess(ProcType procType);

    /// Return the proc type of this process
    static ProcType getProcType(int pid);
  private:
    /** Start a new process, sets the type and id and returns true if in this new process. Also adds the signal handling. */
    static bool startProc(ProcType procType, int id);

    /// Our current proc type
    static ProcType s_procType;
    /// Our current process id
    static int s_processID;
    /// How many processes are handled in this ProcHandler
    static int s_numEventProcesses;

    // global list of PIDs managed by ProcHandler.
    static std::vector<int> s_pidVector;
    /// Which PIDs were started with which types
    static std::map<int, ProcType> s_startedPIDs;

    /// Add a new PID. Is called when forking
    static void addPID(int pid);
    /// Find a PID in the list and return true, if found
    static bool findPID(int pid);
    /// Remove a PID from the list by setting it to 0
    static void removePID(int pid);
    /// Remove all PIDs
    static void clearPIDs();
    /// Check if the PID list is empty (only 0).
    static bool pidListEmpty();
    /// This function is called on SIG_CLD
    static void childSignalHandler(int);
  };
}
