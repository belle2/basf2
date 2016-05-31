/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Matthew Barrett                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPCOSMICGUNMODULE_H
#define TOPCOSMICGUNMODULE_H

#include <framework/core/Module.h>
#include <string>
#include "TH1.h"
#include "TFile.h"


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

    /**
     * Function to generate a random variable according to a polyline.
     * (borrowed from generators/particlegun/src/ParticleGun.cc)
     * @param n number of points in the polyline
     * @param x pointer to the x coordinates
     * @param y pointer to the y coordinates
     * @return value within x0 and xn, distributed according to the line
     * connecting (xi,yi)
     */
    double randomPolyline(size_t n, const double* x, const double* y);

    std::vector<double> m_upperPad; /**< Upper rectangular surface (z,x,Dz,Dx,y). */
    std::vector<double> m_lowerPad; /**< Lower rectangular surface (z,x,Dz,Dx,y). */
    double m_alpha; /**< rotation angle of trigger paddles [deg] */
    double m_swimBackDistance; /**< swim back a muon by this distance */
    double m_momentum; /**< Momentum for generation of mono-energetic cosmic ray muons */
    double m_momentumCutOff; /**< Minimum momentum for generated muons (GeV/c). */
    double m_startTime;      /**< start time (time at upperPad). */

    std::string m_momentumDistributionType; /**< Type of momentum distribution to use */
    std::string m_momentumHistogramFileName; /**< Name of ROOT file containing momentum histogram. */
    std::string m_momentumHistogramName; /**< Name of momentum histogram in ROOT file. */
    std::vector<double> m_momentumPolyline; /**< momentum distribution as polyline */

    std::string m_angularDistributionType; /**< Type of angular distribution to use */
    std::string m_angularHistogramFileName; /**< Name of ROOT file containing angular histogram. */
    std::string m_angularHistogramName; /**< Name of histogram in ROOT file. */

    TFile* m_momentumHistogramFile = 0; /**< ROOT file containing momentum histogram. */
    TH1F* m_momentumDistribution = 0; /**< Momentum distribution (root histogram). */
    TFile* m_angularHistogramFile = 0; /**< ROOT file containing angular histogram. */
    TH1F* m_angularDistribution = 0;  /**< Angular distribution (root histogram). */

  };

} // Belle2 namespace

#endif
