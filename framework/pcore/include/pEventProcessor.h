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

#include <framework/core/EventProcessor.h>
#include <framework/core/ModuleStatistics.h>

#include <framework/pcore/pEventServer.h>
#include <framework/pcore/pOutputServer.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/core/Module.h>
#include <framework/pcore/RxModule.h>
#include <framework/pcore/TxModule.h>

#define RBUFSIZE 4000000

namespace Belle2 {

  /*! The pEventProcessor Class */
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
    void process(PathPtr spath);

    /*!
      Keep this function just for backword compatibility
    */
    void process_old(PathPtr spath);

    /*! Set number of processes for parallel processing */
    /*!
        Number of processes. 0 means normal single event processing
    \param nproc Number of processes
    */
    void nprocess(int nproc);

    /*! Get number of processes from outside */
    int nprocess(void);

  private:
    /*! Analyze given path */
    void analyze_path(PathPtr& path, Module* mod = NULL, int cstate = 0);

    /*! Dump module names in the path */
    void dump_path(const std::string, PathPtr);

    /*! Dump module names in the ModulePtrList */
    void dump_modules(const std::string, const ModulePtrList);

    /*! Get hexadecimal expression of PathPtr address */
    std::string to_hex(PathPtr& path);

    /*! Extract modules to be initialized in main process */
    ModulePtrList init_modules_in_main(const ModulePtrList& modlist);

    /*! Extract modules to be initialized in forked process */
    ModulePtrList init_modules_in_process(const ModulePtrList& modlist);

    /*! Initialize modules (a ka processInitialize() in EventProcessor.cc) */
    void pProcessInitialize(const ModulePtrList&);



  protected:

  private:

    int m_nproc;
    ModulePtrList m_input_list;
    ModulePtrList m_output_list;
    ProcHandler* procHandler;

    std::vector<PathPtr> m_inpathlist;
    std::vector<PathPtr> m_bodypathlist;
    std::vector<PathPtr> m_outpathlist;

    std::vector<RingBuffer*> m_rbinlist;
    std::vector<RingBuffer*> m_rboutlist;

    bool m_histoflag;
    ModulePtr m_histoman;

  };

}

#endif /* PEVENTPROCESSOR_H_ */
