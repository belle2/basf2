//
// Created by abaur on 02.05.18.
//

#pragma once

#include <framework/core/Module.h>
#include <framework/core/EventProcessor.h>
#include <framework/core/Path.h>


namespace Belle2 {

  class ProcHandler;

  /**
    This class provides the core event processing loop for parallel processing.
  */
  class pEventProcessor : public EventProcessor {
  public:

    /** Constructor */
    pEventProcessor();

    /** Destructor */
    virtual ~pEventProcessor();

    /** Processes the full module chain using parallel processing, starting with the first module in the given path. */
    /**
        Processes all events for the given run number and for events from 0 to maxEvent.
        \param spath The processing starts with the first module of this path.
        \param maxEvent The maximum number of events that will be processed.
            If the number is smaller or equal 0, all events will be processed.
    */
    void process(PathPtr spath, long maxEvent);

    /** signal handler for Ctrl+C (async-safe)
     *
     *  When called the first time, does nothing (input process handles SIGINT by itself).
     *  produced (mostly equivalent to previous behaviour on Ctrl+C)
     * */
    void gotSigINT();

    /** signal handler (async-safe)
     *
     * Fairly abrupt termination after the current event.
     */
    void killRingBuffers();

    /** clean up IPC resources (should only be called in one process). */
    void cleanup();


  private:
    /** Analyze given path. Fills m_*path objects. */
    void analyzePath(const PathPtr& path);

    /** Adds internal modules to pathsrepare RingBuffers (setups ZeroMQ, defines communication grid). */
    void preparePaths(int numProcesses);

    /** TFiles are stored in a global list and cleaned up by root
     * since this will happen in all forked processes, these will be corrupted if we don't clean the list!
     *
     * needs to be called at the end of every process.
     */
    void clearFileList();

    /** Tries a soft shutdown when this fails -> hard kill */
    void  terminateProcesses(ModulePtrList* localModules, const ModulePtrList& prependModules);

    /** handler to fork and manage processes. */
    std::unique_ptr<ProcHandler> m_procHandler;

    /** are there forked processes? */
    bool m_multicastOnline = false;

    /** Input path. */
    PathPtr m_inputPath;
    /** Main (parallel section) path. */
    PathPtr m_mainPath;
    /** Output path. */
    PathPtr m_outputPath;

    /** Pointer to HistoManagerModule, or nullptr if not found. */
    ModulePtr m_histoman;


    // TODO: here comes the PCB stuff
    const std::string m_socketProtocol = "ipc";

    /** Name of the input socket */
    std::string m_inputSocketName;
    /** Name of the output socket */
    std::string m_outputSocketName;
    /** Name of the xpub proxy socket */
    std::string m_xpubProxySocketName;
    /** Name of the xsub proxy socket */
    std::string m_xsubProxySocketName;

  };
}