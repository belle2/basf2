/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anselm Baur, Nils Braun                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/core/EventProcessor.h>
#include <framework/core/Path.h>
#include <framework/pcore/ProcessMonitor.h>

namespace Belle2 {

  class ProcHandler;

  /**
    This class provides the core event processing loop for parallel processing.
  */
  class ZMQEventProcessor : public EventProcessor {
  public:

    /** Constructor */
    ZMQEventProcessor();

    /** Destructor */
    virtual ~ZMQEventProcessor();

    /** Processes the full module chain using parallel processing, starting with the first module in the given path. */
    /**
        Processes all events for the given run number and for events from 0 to maxEvent.
        \param spath The processing starts with the first module of this path.
        \param maxEvent The maximum number of events that will be processed.
            If the number is smaller or equal 0, all events will be processed.
    */
    void process(PathPtr spath, long maxEvent);

    /** clean up IPC resources (should only be called in one process). */
    void cleanup();

  private:
    void initialize(const ModulePtrList& moduleList, const ModulePtr& histogramManager);

    void forkAndRun(long maxEvent, const PathPtr& inputPath, const PathPtr& mainPath, const PathPtr& outputPath,
                    const ModulePtrList& terminateGlobally);

    void terminateAndCleanup(const ModulePtr& histogramManager);

    void runMonitoring(const PathPtr& inputPath, const PathPtr& mainPath, const ModulePtrList& terminateGlobally, long maxEvent);
    void runInput(const PathPtr& inputPath, const ModulePtrList& terminateGlobally, long maxEvent);
    void runOutput(const PathPtr& outputPath, const ModulePtrList& terminateGlobally, long maxEvent);
    void runWorker(unsigned int numProcesses, const PathPtr& inputPath, const PathPtr& mainPath, const ModulePtrList& terminateGlobally,
                   long maxEvent);
    void processPath(const PathPtr& localPath, const ModulePtrList& terminateGlobally, long maxEvent);

    ProcessMonitor m_processMonitor;
  };
}