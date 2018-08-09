/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * Description: Routine to decay eta/eta' -> pi+ pi- pi0                  *
 *              with a Dalitz parametrization up to the                   *
 *              cubic terms in Y                                          *
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
  /**Class for the simulation of the eta -> pi+pi-pi0 decay with an improved dalitz description.
   *
   * Evolution of the former EvtEtaDalitz, that implements a simpler Dalitz description with
   * hard-coded parameters. The general structure of the class, as well as several methods, are inherited
   * from it.
   *
   * The chosen pametrization is based on a power expansion of the decay amplitude squared with real
   * cefficients (see PDG 2015):
   *
   * |M|^2 = 1 + a*Y + b*Y^2 + c*X + d*X^2 + e*X*Y + f*Y^3,
   *
   * where the kinematic variable X and Y are both linear combinations of the pions' kinetic energies in
   * the eta frame.
   *
   * The model takes 6 parameters corresponing to the six coeafficients a,b,c,d,e,f;
   * c!=0 or  e=0 are indication of CP violation in eta decay, and are therefore usually set to 0.
   * An implementation of the model with the parameters report in  Phys.Rev. D92 (2015) 012014 is:
   *
   * ETA_FULLDALITZ  -1.128  0.153  0.0  0.085  0.0  0.173;
   *
   * In principle the class could be used also to model  eta' -> pi+pi-pi0
   */
  class EvtEtaFullDalitz: public  EvtDecayAmp  {

  public:

    /** Default Constructor */
    EvtEtaFullDalitz() {}

    /** Default Destructor */
    virtual ~EvtEtaFullDalitz();

    /** Returns the model name: ETA_FULLDALITZ */
    std::string getName();

    /** Makes a copy of the pointer to the class */
    EvtDecayBase* clone();

    /** Checks that the number of input parameters are correct:
     *  - 3 scalar particles (check on spin but not on charge)
     *  - 6 real parameters (no check on the actual values)
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
