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

  class CDCDedxLikelihood;
  class VXDDedxLikelihood;

  /**
   * Bunch finder: searches for the bunch crossing where the interaction happened
   * using track-based TOP likelihood
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
    double m_minTime;     /**< lower time limit for photons [ns] */
    double m_maxTime;     /**< upper time limit for photons [ns] */
    double m_sigmaSmear;  /**< additional smearing of PDF in [ns] */
    double m_minSignal;   /**< minimal number of signal photons */
    double m_minSBRatio;  /**< minimal signal-to-background ratio */
    double m_maxDERatio;  /**< maximal ratio of detected over expected photons */
    bool m_useMCTruth;    /**< use MC truth for mass instead of dEdx most probable */

    double m_bunchTimeSep; /**< time between two bunches */

    /**
     * Return mass of the most probable charged stable particle according to dEdx
     * @return mass
     */
    double getMostProbableMass(const CDCDedxLikelihood* cdcdedx, const VXDDedxLikelihood* vxddedx) const;

  };

} // Belle2 namespace

#endif
