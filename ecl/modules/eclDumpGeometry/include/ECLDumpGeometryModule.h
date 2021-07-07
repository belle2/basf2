/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

namespace Belle2 {

  /** Dump location and direction of all ECL crystals */
  class ECLDumpGeometryModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ECLDumpGeometryModule();

    /** Register input and output data */
    virtual void initialize() override;

    /** Event */
    virtual void event() override;

  private:
    bool firstEvent = true; /**< print out geometry in the first event */

  };
}

