/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Florian Bernlochner                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EvtHQET3_HH
#define EvtHQET3_HH

#include "EvtGenBase/EvtDecayAmp.hh"
#include "EvtGenBase/EvtSemiLeptonicFF.hh"
#include "EvtGenBase/EvtSemiLeptonicAmp.hh"

class EvtParticle;

/** The class provides the form factors for orbitally excited semileptonic decays
 */
class EvtHQET3: public  EvtDecayAmp  {

public:

  /** Default constructor */
  EvtHQET3();

  /** virtual destructor */
  virtual ~EvtHQET3();

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
  EvtSemiLeptonicFF* hqetffmodel;

  /** Pointers needed to calculate amplitude */
  EvtSemiLeptonicAmp* calcamp;

};
#endif



