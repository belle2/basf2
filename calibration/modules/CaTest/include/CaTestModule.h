/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CATESTMODULEMODULE_H
#define CATESTMODULEMODULE_H

#include <calibration/CalibrationCollectorModule.h>

namespace Belle2 {
  /// Testing module for collection of calibration data
  class CaTestModule : public CalibrationCollectorModule {

  public:

    /// Constructor: Sets the description, the properties and the parameters of the module.
    CaTestModule();

    /// Prepare collecction
    virtual void prepare();
    /// Collect data event-wise
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

    /** Spread of gaussian (mean=42) filling test histogram (range=<0,100>) - probability of algo iterations depend on it */
    int m_spread;
    /** Time (us) to wait during prepare method - useful for slowing down the collector for checking*/
    int m_wait;
  };
}

#endif /* CATESTMODULEMODULE_H */

