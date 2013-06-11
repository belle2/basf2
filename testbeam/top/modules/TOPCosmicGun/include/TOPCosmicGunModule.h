/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPCOSMICGUNMODULE_H
#define TOPCOSMICGUNMODULE_H

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /**
   * A simple cosmic ray generator for CRT.
   */
  class TOPCosmicGunModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPCosmicGunModule();

    /**
     * Destructor
     */
    virtual ~TOPCosmicGunModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:
    std::vector<double> m_upperPad; /**< upper rectangular surface (z,x,Dz,Dx,y) */
    std::vector<double> m_lowerPad; /**< lower rectangular surface (z,x,Dz,Dx,y)*/
    double m_startTime;             /**< start time (time at upperPad) */

  };

} // Belle2 namespace

#endif
