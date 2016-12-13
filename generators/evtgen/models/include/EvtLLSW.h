/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Florian Bernlochner                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVTLLSW_HH
#define EVTLLSW_HH

#include "EvtGenBase/EvtDecayAmp.hh"
#include "EvtGenBase/EvtSemiLeptonicFF.hh"
#include "EvtGenBase/EvtSemiLeptonicAmp.hh"

class EvtParticle;

/** The class provides the decay amplitude for orbitally excited semileptonic decays
 */
class EvtLLSW: public  EvtDecayAmp  {

public:

  /** Default constructor */
  EvtLLSW();

  /** virtual destructor */
  virtual ~EvtLLSW();

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

  /** Pointers needed for FFs */
  EvtSemiLeptonicFF* llswffmodel;

  /** Pointers needed to calculate amplitude */
  EvtSemiLeptonicAmp* calcamp;

};

#endif

