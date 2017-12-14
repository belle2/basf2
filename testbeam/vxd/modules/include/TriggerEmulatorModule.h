/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TRIGGEREMULATORMODULE_H
#define TRIGGEREMULATORMODULE_H

#include <framework/core/Module.h>


namespace Belle2 {
  /**
   * The modules return value will be set to the number of sensors with a cluster on them.
   * Only sensor given in the sensor list parameter are considered.
   *
   *    *
   */
  class TriggerEmulatorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TriggerEmulatorModule();

    /**  */
    virtual ~TriggerEmulatorModule();

    /**  */
    virtual void initialize();

    /**  */
    virtual void beginRun();

    /**  */
    virtual void event();

    /**  */
    virtual void endRun();

    /**  */
    virtual void terminate();


  private:

    std::string m_sensors; // sensor ids which should be used seperated by white space (" "). All other sesnors will be ignored!
    std::string m_inputArrayName; // name of array with telescope (and scintilator cluster)
  };
}

#endif /* TRIGGEREMULATORMODULE_H */
