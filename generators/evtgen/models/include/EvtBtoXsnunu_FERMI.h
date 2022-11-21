/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include "EvtGenBase/EvtDecayIncoherent.hh"
#include "EvtGenBase/EvtParticle.hh"
#include <string>

namespace Belle2 {

  /** The evtgen model to produce non-resonant B-> Xs nu nubar decay sample.
    *
    * In this model, kinematics of two neutrinos is not carefully considered. Di-neutrino just decay by EvtGenKine::PhaseSpace
    *
    * This model adopts the Fermi motion model which is used for EvtBtoXsll model.
    *
    * The momentums of b-quark and spectator quark are determined by the Fermi motion model.
    * The decay of b-quark is determined by dGdsb probability density function from [arXiv:1509.06248v2].
    *
    * This model requires 6 parameters: m_ms, m_mq, m_pf, m_mxmin, m_mb_prob, m_ms_prob.
    * m_mb_prob and m_ms_prob are used to calculate dGdsb. m_mb_prob=4.68 and m_ms_prob=0.1 are used in [arXiv:0902.0160].
    * In [arXiv:hep-ph/9603237], m_mb_prob=4.8 and m_ms_prob=0.2 are used for B->Xs l l decay.
    * m_ms and m_mq are masses of s-quark and the spectator quark for the Fermi motion model. They are related with the kinematics of Xs.
    * m_pf is the Fermi motion momentum used in the Fermi motion model.
    * m_mxmin is the minimum mass of Xs.
    *
    * You can use the model as follows:
    *
    * Decay MyB-
    * 1.0 anti-Xsu anti-nu_e nu_e           BTOXSNUNU_FERMI 0.2 0.0 0.461 1.1 4.8 0.2;
    * Enddecay
    */
  class EvtBtoXsnunu_FERMI : public  EvtDecayIncoherent {


  public:

    /**
     * Constructor.
     */
    EvtBtoXsnunu_FERMI() {}

    /**
     * Destructor.
     */
    virtual ~EvtBtoXsnunu_FERMI();

    /**
     * The function which returns the name of the model.
     */
    std::string getName();

    /**
     * The function which makes a copy of the model.
     */
    EvtDecayBase* clone();

    /**
     * The function to sets a maximum probability.
     * In this model, noProbMax() is called because maximum probability is determined by m_dGdsbProbMax
     */
    void initProbMax();

    /**
     * The function for an initialization.
     */
    void init();

    /**
     * The function to determine kinematics of daughter particles based on dGdsb distribution.
     */
    void decay(EvtParticle* p);

    /**
     * The function returns the probability density value depending on sb.
     */
    double dGdsbProb(double _sb);

    /**
     * The function returns a momentum of b quark.
     * The distribution of the momentum is based on the Fermi motion model.
     */
    double FermiMomentum(double pf);

    /**
     * The function returns a probability based on the Fermi motion model.
     * @pf Fermi motion momentum for the Fermi motion model
     */
    double FermiMomentumProb(double pb, double pf);

  private:
    /** The maximum value of dGdsb.   */
    double m_dGdsbProbMax{0.0};

    /** mass of s-quark for the Fermi motion model and EvtGenKine::PhaseSpace.   */
    double m_ms{0.2};

    /** mass of spectator quark for the Fermi motion model.   */
    double m_mq{0.0};

    /** Parameter for the Fermi motioin model.   */
    double m_pf{0.461};

    /** Minimum mass of Xs.   */
    double m_mxmin{1.1};

    /** b-quark mass to calculate dGdsb.   */
    double m_mb_prob{4.8};

    /** s-quark mass to calculate dGdsb.   */
    double m_ms_prob{0.2};

  };

} // Belle 2 Namespace
