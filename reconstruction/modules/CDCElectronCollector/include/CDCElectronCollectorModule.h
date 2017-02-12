/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCELECTRONCOLLECTORMODULE_H
#define CDCELECTRONCOLLECTORMODULE_H

#include <calibration/CalibrationCollectorModule.h>


namespace Belle2 {
  /**
   * A collector module for CDC dE/dx electron calibration
   *
   */
  class CDCElectronCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    CDCElectronCollectorModule();

    /**
     * Create and book ROOT objects
     */
    virtual void prepare();

    /**
     * Fill ROOT objects
     */
    virtual void collect();


  private:
    /** Current event id */
    int m_evt = -1;
    /** Current run id */
    int m_run = -1;
    /** Current experiment id */
    int m_exp = -1;
    /** Current process id */
    int m_procId = -1;

    /** dE/dx truncated mean */
    double m_dedx = -1;
  };
}

#endif /* CDCELECTRONCOLLECTORMODULE_H */
