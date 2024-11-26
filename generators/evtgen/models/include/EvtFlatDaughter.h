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

  /** The evtgen model to produce flat invariant mass distribution for 2nd and 3rd daughters.
    *
    * In this model, M -> D1 D2 D3 decay is produced. Invariant mass(D2, D3) follows flat distribution.
    *
    * You can use the model as follows:
    *
    * Decay MyB+
    * 1.0 K+  n0  anti-n0           FLAT_DAUGHTER;
    * Enddecay
    */
  class EvtFlatDaughter : public  EvtDecayIncoherent {


  public:

    /**
     * Constructor.
     */
    EvtFlatDaughter() {}

    /**
     * Destructor.
     */
    virtual ~EvtFlatDaughter();

    /**
     * The function which returns the name of the model.
     */
    std::string getName();

    /**
     * The function which makes a copy of the model.
     */
    EvtDecayBase* clone();

    /**
     * The function to sets a maximum probability. At this model, noProbMax() is used.
     */
    void initProbMax();

    /**
     * The function for an initialization.
     */
    void init();

    /**
     * The function to determine kinematics of daughter particles.
     */
    void decay(EvtParticle* p);

  private:

  };

} // Belle 2 Namespace


