/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <top/reconstruction/TOPalign.h>
#include <string>

namespace Belle2 {

  /**
   * Alignment of TOP
   */
  class TOPAlignerModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPAlignerModule();

    /**
     * Destructor
     */
    virtual ~TOPAlignerModule();

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

    double m_minBkgPerBar = 0;    /**< minimal assumed background photons per module */
    double m_scaleN0 = 0;         /**< scale factor for figure-of-merit N0 */

    std::vector<TOP::TOPalign> m_align; /**< alignment objects (one per module) */

  };

} // Belle2 namespace

