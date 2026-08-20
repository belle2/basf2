/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <EvtGenBase/EvtDecayAmp.hh>

class EvtParticle;
class EvtId;

/** The class provides the decay amplitude for orbitally excited semileptonic decays
 */
class EvtHNLGoityRoberts : public EvtDecayAmp {
public:

  /** Returns name of module */
  std::string getName() override;
  /** Clones module */
  EvtDecayBase* clone() override;

  /** Initializes module */
  void init() override;
  /** Creates a decay */
  void decay(EvtParticle* p) override;
  /** Sets maximal probab. */
  void initProbMax() override;

private:
  /** Some comments */
  void DecayBDstarpilnuGR(EvtParticle* pb, EvtId ndstar, EvtId nlep,
                          EvtId nnu);
  /** Some comments */
  void DecayBDpilnuGR(EvtParticle* pb, EvtId nd, EvtId nlep, EvtId nnu);
};

