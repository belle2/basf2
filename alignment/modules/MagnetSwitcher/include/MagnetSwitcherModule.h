/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MAGNETSWITCHERMODULE_H
#define MAGNETSWITCHERMODULE_H

#include <framework/core/Module.h>


namespace Belle2 {
  /**
   * Module to switch magnetic field ON/OFF for reconstruction
   *
   * Currently can be only used to control field for GBLfit module. One can specify
   * list of runs, where the magnet is off. Magnetic field must be loaded always by
   * geometry module. In future this should come from ConfigurationDB
   *
   */
  class MagnetSwitcherModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    MagnetSwitcherModule();

    /** Initialize */
    virtual void initialize();

    /** Switch magnet on/off based on input run list */
    virtual void beginRun();


  private:

    std::vector<int> m_expList;  /**< List of experiments to switch magnet off */
    std::vector<int> m_runList;  /**< List of runs to switch magnet off */
  };
}

#endif /* MAGNETSWITCHERMODULE_H */
