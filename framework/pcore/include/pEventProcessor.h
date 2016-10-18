/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ryosuke Itoh                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

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

    /** Processes the full module chain, starting with the first module in the given path. */
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
     *  On subsequent calls, RingBuffers are cleared, discarding any events that have been partly
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

    /** Adds internal modules to paths, prepare RingBuffers. */
    void preparePaths();

    void appendModule(PathPtr& modules, ModulePtr module);
    void prependModule(PathPtr& modules, ModulePtr module);

    /** TFiles are stored in a global list and cleaned up by root
     * since this will happen in all forked processes, these will be corrupted if we don't clean the list!
     *
     * needs to be called at the end of every process.
     */
    void clearFileList();

    /** Return only modules which have the given Module flag set. */
    static ModulePtrList getModulesWithFlag(const ModulePtrList& modules, Module::EModulePropFlags flag);
    /** Return only modules which do not have the given Module flag set. */
    static ModulePtrList getModulesWithoutFlag(const ModulePtrList& modules, Module::EModulePropFlags flag);
    /** Prepend given 'prependModules' to 'modules', if they're not already present. */
    static void prependModulesIfNotPresent(ModulePtrList* modules, const ModulePtrList& prependModules);

  private:
    /** handler to fork and manage processes. */
    std::unique_ptr<ProcHandler> m_procHandler;

    /** Input path. */
    PathPtr m_inputPath;
    /** Main (parallel section) path. */
    PathPtr m_mainPath;
    /** Output path. */
    PathPtr m_outputPath;

    /** Pointer to HistoManagerModule, or nullptr if not found. */
    ModulePtr m_histoman;

    /** Name of the input socket */
    std::string m_inputSocketName;

    /** Name of the output socket */
    std::string m_outputSocketName;

    const std::string m_socketProtocol = "ipc";
  };

}
