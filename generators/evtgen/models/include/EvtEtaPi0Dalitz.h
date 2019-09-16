/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * Description: Routine to decay eta/eta' -> pi0 pi0 pi0                  *
 *              with a linear Dalitz parametrization                      *
 *                                                                        *
 * Modification history:                                                  *
 *   -  U.Tamponi    October 9th 2016   Module created                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include "EvtGenBase/EvtDecayAmp.hh"

class EvtParticle;


namespace Belle2 {
  /**Class for the simulation of the eta -> 3pi0 and eta'->3pi0 decays
   *
   * The chosen pametrization is based on a power expansion of the decay amplitude squared with real
   * cefficients (see PDG 2015):
   *
   * |M|^2 = 1 + 2*alpha*Z,
   *
   * where the kinematic variable Z is funciton of the kinetic energies of the 3 pi0s in the
   * mother reference frame.
   *
   * The model takes 1 parameter corresponding to the slope coeafficient alpha. alpha should be greater
   * than -0.5, however the class is able to handle non physical values by shrinking the dalitz phase space.
   * This feature is required since the PDG quotes alpha = -0.61 +- 0.07 for the eta' -> 3pi0 decay.
   *
   * An implementation of the model with the parameters report in the PDG 2015 for
   * the eta -> 3pi0 decay is :
   *
   * ETA_PI0DALITZ  -0.0135;
   *
   * The class could be used also to model  eta' -> 3pi0
   */
  class EvtEtaPi0Dalitz: public  EvtDecayAmp  {

  public:

    /** Default constructor */
    EvtEtaPi0Dalitz() {}

    /** Default destructor */
    virtual ~EvtEtaPi0Dalitz();

    /** Returns the name of the model: ETA_PI0DALITZ */
    std::string getName();

    /** Makes a copy of the class object */
    EvtDecayBase* clone();

    /** Checks that the number of input parameters are correct:
     *  - 3 scalar particles (check on spin but not on charge)
     *  - 1 real parameters (no check on the actual values)
     */
    void init();

    /** Sets the Maximum probability for the PHSP reweight.
     *  Maximum value is hardcoded and inherited from the EvtEtaDalitz class.
     */
    void initProbMax();

    /** Function that implements the energy-dependent Dalitz */
    void decay(EvtParticle* p);

  };

} // Belle 2 Namespace
