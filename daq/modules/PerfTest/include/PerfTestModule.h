/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PERFTESTMODULE_H
#define PERFTESTMODULE_H

#include <vector>

#include <framework/core/Module.h>

namespace Belle2 {

  //! PerfTestModule class
  /*! This class records event processing time (ms) in various ways
  */
  class PerfTestModule : public Module {

  public:
    //! Constructor
    PerfTestModule();
    //! Destructor
    virtual ~PerfTestModule();

    //! Initialize the module
    virtual void initialize();
    //! Begin a run
    virtual void beginRun();
    //! Event processing
    virtual void event();
    //! End a run
    virtual void endRun();
    //! Terminate the module
    virtual void terminate();

  protected:
    //! Calculate elapsed time between two points
    double timeDifference(clock_t, clock_t);


  private:
    clock_t m_start;                        /**< Starting time */
    clock_t m_temp;                         /**< Time that an event processed */

    int m_nEvents;                          /**< Number of processed events */

    std::string m_eventsOutputFileName;     /**< File name to record processing time per event */
    std::string m_overallOutputFileName;    /**< File name to record overall elapsed time */
  };

}

#endif
