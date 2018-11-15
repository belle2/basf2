/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Lu Cao                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EvtBGL_HH
#define EvtBGL_HH

#include "EvtGenBase/EvtDecayAmp.hh"
#include "EvtGenBase/EvtSemiLeptonicFF.hh"
#include "EvtGenBase/EvtSemiLeptonicAmp.hh"

class EvtParticle;

/** The class provides the form factors for orbitally excited semileptonic decays
 */
class EvtBGL: public  EvtDecayAmp  {

public:

  /** Default constructor */
  EvtBGL();

  /** virtual destructor */
  virtual ~EvtBGL();

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
  EvtSemiLeptonicFF* bglffmodel;

  /** Pointers needed to calculate amplitude */
  EvtSemiLeptonicAmp* calcamp;

};
#endif



