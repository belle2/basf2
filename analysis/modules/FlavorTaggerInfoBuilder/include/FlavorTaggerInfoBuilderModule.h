/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Roca                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <string>

namespace Belle2 {

  /**
   * Creates for each Particle and RestOfEvent object in the DataStore a FlavorTaggerInfo dataobject and makes BASF2 relation between them.
   * NOTE: this module DOES NOT fill the FlavorTaggerInfo dataobject, but only initializes it so can be filled during the run
   * of FlavorTagger.py
   */
  class FlavorTaggerInfoBuilderModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    FlavorTaggerInfoBuilderModule();

    /**  */
    virtual void initialize() override;

    /**  */
    virtual void event() override;


  private:

  };
}

