/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPBUNCHFINDERMODULE_H
#define TOPBUNCHFINDERMODULE_H

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>
#include <string>

namespace Belle2 {

  class DedxLikelihood;

  /**
   *
   */
  class TOPBunchFinderModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPBunchFinderModule();

    /**
     * Destructor
     */
    virtual ~TOPBunchFinderModule();

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

    int m_bunchHalfRange; /**< half range of relative bunch numbers */
    double m_maxTime;   /**< time limit for photons */

    double m_bunchTimeSep; /**< time between two bunches */

    std::vector<int> m_xxx;
    std::vector<int> m_yyy[20];


    /**
     * Return mass of the most probable charged stable particle using dEdx likelihoods
     * @return mass
     */
    double getMostProbableMass(const DedxLikelihood* dedx) const;

  };

} // Belle2 namespace

#endif
