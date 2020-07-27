/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2020 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christopher Hearty hearty@physics.ubc.ca                 *
*                                                                        *
* Print out the location and direction of every ECL crystal              *
* Sample script: ecl/examples/EclDumpGeometry.py                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
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

