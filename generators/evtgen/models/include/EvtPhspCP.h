/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include "EvtGenBase/EvtDecayAmp.hh"
#include "EvtGenBase/EvtParticle.hh"

#include <fstream>

namespace Belle2 {
//! Register Decay model EvtPhspCP
  class EvtPhspCP : public  EvtDecayAmp {

  public:

    EvtPhspCP() {};
    virtual ~EvtPhspCP();

    std::string getName();  /**< Get function Name  */

    EvtDecayBase* clone();  /**< Clone the decay */

    void init();  /**< Initialize standard stream objects  */

    void initProbMax();  /**< Initialize standard stream objects for probability function  */

    void decay(EvtParticle* p); /**< Member of particle in EvtGen */

  }; //! end of EvtDecayAmp

} // Belle 2 Namespace
