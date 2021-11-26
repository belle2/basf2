/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtDecayAmp.hh"

class EvtParticle;

namespace Belle2 {
  class EvtKstarnunu_REV : public  EvtDecayAmp {

  public:

    EvtKstarnunu_REV() {}
    virtual ~EvtKstarnunu_REV();

    std::string getName();
    EvtDecayBase* clone();

    void init();

    void decay(EvtParticle* p);

    void initProbMax();

  };

} // Belle 2 Namespace
