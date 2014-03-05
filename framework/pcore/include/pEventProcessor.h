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

  class PathManager;
  class ProcHandler;
  class RingBuffer;

  /*!
    This class provides the core event processing loop for parallel processing.
  */
  class pEventProcessor : public EventProcessor {

  public:

    /*! Constructor */
    pEventProcessor(PathManager& pathManager);

    /*! Destructor */
    virtual ~pEventProcessor();

    /*! Processes the full module chain, starting with the first module in the given path. */
    /*!
        Processes all events for the given run number and for events from 0 to maxEvent.
        \param spath The processing starts with the first module of this path.
    */
    void process(PathPtr spath, long maxEvent);


  private:
    /*! Analyze given path */
    void analyze_path(const PathPtr& path, Module* mod = NULL, int cstate = 0);

    /*! Dump module names in the path */
    void dump_path(const std::string, PathPtr);

    /*! Dump module names in the ModulePtrList */
    void dump_modules(const std::string, const ModulePtrList);

    /** Should this module be initialized in the global process?
     *
     *  Returns false if initialize() should be called inside forked process.
     */
    bool initializeGlobally(ModulePtr module);
    /* TFiles are stored in a global list and cleaned up by root
     * since this will happen in all forked processes, these will be corrupted if we don't clean the list!
     *
     * needs to be called at the end of every process.
     */
    void clearFileList();

    /*! Extract modules to be initialized in main process */
    ModulePtrList init_modules_in_main(const ModulePtrList& modlist);

    /*! Extract modules to be initialized in forked process */
    ModulePtrList init_modules_in_process(const ModulePtrList& modlist);

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
    std::vector<PathPtr> m_bodypathlist;
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

  };

}

#endif /* PEVENTPROCESSOR_H_ */
