/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
// Junewoo Park      December 02, 2021

#pragma once

#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtDecayAmp.hh"

class EvtParticle;

namespace Belle2 {
  class EvtKnunu : public  EvtDecayAmp {

  public:

    EvtKnunu() {}
    virtual ~EvtKnunu();

    std::string getName();
    EvtDecayBase* clone();

    void init();

    void decay(EvtParticle* p);

    void initProbMax();

  };

} // Belle 2 Namespace
