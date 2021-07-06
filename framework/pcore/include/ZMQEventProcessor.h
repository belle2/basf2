/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/core/EventProcessor.h>
#include <framework/core/Path.h>
#include <framework/pcore/ProcessMonitor.h>

namespace Belle2 {
  /**
    This class provides the core event processing loop for parallel processing with ZMQ.
  */
  class ZMQEventProcessor : public EventProcessor {
  public:

    /// Init the socket cleaning at exit
    ZMQEventProcessor();

    /// Make sure we remove all sockets cleanly
    virtual ~ZMQEventProcessor();

    /// Processes the full module chain using parallel processing, starting with the first module in the given path.
    void process(const PathPtr& spath, long maxEvent);

    /// clean up IPC resources (should only be called in one process).
    void cleanup();

  private:
    /// First step in the process: init the module in the list
    void initialize(const ModulePtrList& moduleList, const ModulePtr& histogramManager);

    /// Second step in the process: fork out the processes we need to have and call the event loop
    void forkAndRun(long maxEvent, const PathPtr& inputPath, const PathPtr& mainPath, const PathPtr& outputPath,
                    const ModulePtrList& terminateGlobally);

    /// Last step in the process: run the termination and cleanup (kill all remaining processes)
    void terminateAndCleanup(const ModulePtr& histogramManager);

    /// Start the monitoring (without forking)
    void runMonitoring(const PathPtr& inputPath, const PathPtr& mainPath, const ModulePtrList& terminateGlobally, long maxEvent);

    /// Fork out the input process
    void runInput(const PathPtr& inputPath, const ModulePtrList& terminateGlobally, long maxEvent);

    /// Fork out the output process
    void runOutput(const PathPtr& outputPath, const ModulePtrList& terminateGlobally, long maxEvent);

    /// Fork out the N worker process
    void runWorker(unsigned int numProcesses, const PathPtr& inputPath, const PathPtr& mainPath, const ModulePtrList& terminateGlobally,
                   long maxEvent);

    /// Basic function run in every process: process the event loop of the given path
    void processPath(const PathPtr& localPath, const ModulePtrList& terminateGlobally, long maxEvent);

    /// Instance of the process monitor
    ProcessMonitor m_processMonitor;
  };
}