/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDCALIBRATIONSMONITORMODULE_H
#define SVDCALIBRATIONSMONITORMODULE_H

#include <framework/core/Module.h>


namespace Belle2 {
  /**
   * Module to check the updated calibration constants
   *
   *    *
   */
  class SVDCalibrationsMonitorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDCalibrationsMonitorModule();

    /**  */
    virtual void initialize();

    /**  */
    virtual void beginRun();

    /** y */
    virtual void event();

    /**  */
    virtual void endRun();

    /**  */
    virtual void terminate();


  private:

  };
}

#endif /* SVDCALIBRATIONSMONITORMODULE_H */
