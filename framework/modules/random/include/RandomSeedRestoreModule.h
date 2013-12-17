/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RANDOMSEEDRESTOREMODULE_H
#define RANDOMSEEDRESTOREMODULE_H

#include <framework/core/Module.h>


namespace Belle2 {
  /**
   * A module that restores the random seed from an output file.
   *
   * This module sets the random number generator seed to the
   * one that is stored in the given output file.
   */
  class RandomSeedRestoreModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description, the properties and the parameters of the module.
     */
    RandomSeedRestoreModule();

    /** Initializes the Module.
     *
     * Opens the file and restores the random number seed.
     */
    virtual void initialize();

  private:

    std::string m_fileName;    /**< The name of the file that contains the random number seed. */
  };
}

#endif /* RANDOMSEEDRESTOREMODULE_H_ */
