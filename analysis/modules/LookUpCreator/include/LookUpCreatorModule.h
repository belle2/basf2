/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef LOOKUPCREATORMODULE_H
#define LOOKUPCREATORMODULE_H

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /**
   * prints particle list to screen
   */
  class LookUpCreatorModule : public Module {

  public:

    /**
     * Constructor
     */
    LookUpCreatorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

  };

} // Belle2 namespace

#endif
