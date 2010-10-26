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

#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/pEventServer.h>
#include <framework/pcore/pOutputServer.h>

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
    void process_parallel(PathPtr spath);

    static int m_nproc;

  protected:

  private:

    ModulePtrList m_input_list;
    ModulePtrList m_output_list;
    ProcHandler* procHandler;

  };

}

#endif /* PEVENTPROCESSOR_H_ */
