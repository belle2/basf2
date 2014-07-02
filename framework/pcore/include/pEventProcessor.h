/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ryosuke Itoh                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PEVENTPROCESSOR_H_
#define PEVENTPROCESSOR_H_

#include <framework/core/Module.h>
#include <framework/core/EventProcessor.h>
#include <framework/core/Path.h>


namespace Belle2 {

  class ProcHandler;
  class RingBuffer;

  /*!
    This class provides the core event processing loop for parallel processing.
  */
  class pEventProcessor : public EventProcessor {

  public:

    /*! Constructor */
    pEventProcessor();

    /*! Destructor */
    virtual ~pEventProcessor();

    /*! Processes the full module chain, starting with the first module in the given path. */
    /*!
        Processes all events for the given run number and for events from 0 to maxEvent.
        \param spath The processing starts with the first module of this path.
    */
    void process(PathPtr spath, long maxEvent);

    /** signal handler for Ctrl+C (async-safe)
     *
     *  When called the first time, does nothing (input process handles SIGINT by itself).
     *  On subsequent calls, RingBuffers are cleared, discarding any events that have been partly
     *  produced (mostly equivalent to previous behaviour on Ctrl+C)
     * */
    void gotSigINT();

  private:
    /** Analyze given path. Fills m_*pathlist objects. */
    void analyzePath(const PathPtr& path);

    /** Adds internal modules to paths, prepare RingBuffers. */
    void preparePaths();

    /** Create RingBuffer with name from given environment variable, add Tx and Rx modules to a and b. */
    RingBuffer* connectViaRingBuffer(const char* name, PathPtr a, PathPtr& b);

    /*! Dump module names in the path */
    void dump_path(const std::string, PathPtr);

    /*! Dump module names in the ModulePtrList */
    void dump_modules(const std::string, const ModulePtrList);

    /** TFiles are stored in a global list and cleaned up by root
     * since this will happen in all forked processes, these will be corrupted if we don't clean the list!
     *
     * needs to be called at the end of every process.
     */
    void clearFileList();

    /** Insert a termination message at the end of the given RingBuffer. */
    void sendTerminationMessage(RingBuffer* rb);

    /** Return only modules which have the given Module flag set. */
    static ModulePtrList getModulesWithFlag(const ModulePtrList& modules, Module::EModulePropFlags flag);
    /** Return only modules which do not have the given Module flag set. */
    static ModulePtrList getModulesWithoutFlag(const ModulePtrList& modules, Module::EModulePropFlags flag);
    /** Prepend given 'prependModules' to 'modules', if they're not already present. */
    static void prependModulesIfNotPresent(ModulePtrList* modules, const ModulePtrList& prependModules);

  protected:

  private:

    /** input modules with multi-process support */
    ModulePtrList m_input_list;
    /** output modules with multi-process support */
    ModulePtrList m_output_list;
    /** handler to fork and manage processes. */
    ProcHandler* m_procHandler;

    /** list of input paths (contains one path. why a list?) */
    std::vector<PathPtr> m_inpathlist;
    /** list of main paths (contains one path. why a list?) */
    std::vector<PathPtr> m_mainpathlist;
    /** list of output paths (contains one path. why a list?) */
    std::vector<PathPtr> m_outpathlist;

    /** input RingBuffers */
    std::vector<RingBuffer*> m_rbinlist;
    /** output RingBuffers */
    std::vector<RingBuffer*> m_rboutlist;

    /** HistoManagerModule found? */
    bool m_histoManagerFound;
    /** Pointer to HistoManagerModule, if m_histoManagerFound is set */
    ModulePtr m_histoman;

    /** if false, SIGINT handler will not clear RingBuffers.
     *
     * Only set to true in sections using only safe functions (see signal(7)), e.g. waitForProcesses() calls
     */
    bool m_enableRBClearing;

  };

}

#endif /* PEVENTPROCESSOR_H_ */
