/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * Description: Routine to decay eta' -> eta pi pi                        *
 *              with a Dalitz parametrization up to the                   *
 *              quadratic terms in X and Y                                *
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
  /**Class for the simulation of the eta' -> pi+ pi- eta and pi0 pi0 eta decays
   *
   * Please not that the order of the daugther particles matters! The eta must be the last one.
   *
   * Evolution of the former EvtEtaDalitz, that implements a simpler Dalitz description with
   * hard-coded parameters. The general structure of the class, as well as several methods, are inherited
   * from it.
   *
   * The chosen pametrization is based on a power expansion of the decay amplitude squared with real
   * cefficients (see PDG 2015):
   *
   * |M|^2 = 1 + a*Y + b*Y^2 + c*X + d*X^2,
   *
   * where the kinematic variable X and Y are both linear combinations of the pions' and eta kinetic energies in
   * the eta' frame.
   *
   * The model takes 4 parameters corresponing to the six coeafficients a,b,c,d;
   * c!=0 is indication of CP violation in eta decay, and it is therefore usually set to 0.
   * An implementation of the model with the parameters measure by BESIII collab, Phys.Rev. D92 (2015) 012014 is:
   *
   * ETAPRIME_DALITZ -0.047 -0.069 0.0 0.073;
   *
   * The same coeafficients are used for eta' -> eta pi0 pi0
   */
  class EvtEtaPrimeDalitz: public  EvtDecayAmp  {

  public:

    /** Default constructor */
    EvtEtaPrimeDalitz() {}

    /** Default destructor */
    virtual ~EvtEtaPrimeDalitz();

    /** Returns the model name: ETAPRIME_DALITZ */
    std::string getName();

    /** Returns a copy of the class object */
    EvtDecayBase* clone();

    /** Checks that the number of input parameters are correct:
     *  - 3 scalar particles (check on spin but not on charge)
     *  - 4 real parameters (no check on the actual values)
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
