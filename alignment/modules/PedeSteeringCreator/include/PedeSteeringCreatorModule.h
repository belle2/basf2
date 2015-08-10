/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PEDESTEERINGCREATORMODULE_H
#define PEDESTEERINGCREATORMODULE_H

#include <framework/core/Module.h>


namespace Belle2 {
  /**
   * Create PedeSteering to configure Millepede
   *
   * Creates a PedeSteering persistent object in DataStore. Allows to fix sensors by mask,
   * include user constraints and set other commands for Pede. The minimal command
   * is selection of solution method. The CFiles are added to the object by calibration module
   * before the file is constructed and stored to disc to be used by Pede.
   *
   */
  class PedeSteeringCreatorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PedeSteeringCreatorModule();

    /** init */
    virtual void initialize();

    /** begin */
    virtual void beginRun();


  private:

    std::string m_name;  /**< Name of the persistent PedeSteering StoreObjPtr to be created */
    std::vector<std::string> m_commands;  /**< List of plain Pede commands */
    std::vector<std::string> m_parameters;  /**< List of layer.ladder.sensor.param (ints) to be fixed */
  };
}

#endif /* PEDESTEERINGCREATORMODULE_H */
