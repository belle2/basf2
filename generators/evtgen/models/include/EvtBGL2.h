/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include "EvtGenBase/EvtDecayAmp.hh"
#include "EvtGenBase/EvtSemiLeptonicFF.hh"
#include "EvtGenBase/EvtSemiLeptonicAmp.hh"

class EvtParticle;

/** The class provides the form factors for orbitally excited semileptonic decays
 */
class EvtBGL2: public  EvtDecayAmp  {

public:

  /** Default constructor */
  EvtBGL2();

  /** virtual destructor */
  virtual ~EvtBGL2();

  /** Returns name of module */
  std::string getName();

  /** Clones module */
  EvtDecayBase* clone();

  /** Creates a decay */
  void decay(EvtParticle* p);

  /** Sets maximal probab. */
  void initProbMax();

  /** Initializes module */
  void init();

private:

  /** Pointers needed for form factors */
  EvtSemiLeptonicFF* m_bgl2ffmodel = nullptr;

  /** Pointers needed to calculate amplitude */
  EvtSemiLeptonicAmp* m_calcamp = nullptr;

};
