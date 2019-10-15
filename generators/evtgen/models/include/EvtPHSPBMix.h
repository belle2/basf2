//--------------------------------------------------------------------------
//
// Environment:
//      This software is part of the EvtGen package developed jointly
//      for the BaBar and CLEO collaborations.  If you use all or part
//      of it, please give an appropriate acknowledgement.
//
// Copyright Information: See EvtGen/COPYRIGHT
//      Copyright (C) 2002      INFN-Pisa
//
// Module: EvtPHSPBMix.cc
//
// Description:
//    Routine to decay vector-> particle particle with B0 mixing, coherent B0B0-like mixing if any.
//    EvtPHSPBBMix: handle  states with two neutral B
//    EvtPHSPBMix : handles states with only one neutral B
//    Phase-space kinematics, CP conservation, deltaGamma=0, p/q=1
//
//    Based on EvtVSSBMixCPT
//
// Modification history:
//
//   R. Louvot, EPFL, 2010/03/09
//   C. MacQueen, 2016/10/03            Adapted to Belle II
//
//------------------------------------------------------------------------

#ifndef EVTPHSPBMIX_HH
#define EVTPHSPBMIX_HH

#include "EvtGenBase/EvtDecayAmp.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtComplex.hh"

/** The class provides routine to decay vector-> particle particle with B0 mixing, coherent B0B0-like mixing if any.
 */
class EvtPHSPBBMix: public EvtDecayAmp {

public:

  /** Default constructor */
  EvtPHSPBBMix() {}

  /** Default destructor */
  virtual ~EvtPHSPBBMix();

  /** Get function Name  */
  std::string getName();

  /** Clone the decay  */
  EvtDecayBase* clone();

  /** Decay function */
  void decay(EvtParticle* p);

  /** Init function */
  void init();

  /** Init maximal prob.  function */
  void initProbMax();

  /** Number of real daughters */
  int nRealDaughters();

private:

  /** Number of real daughters */
  void prlp(int)const;

  /** Calculate amplitude */
  EvtComplex Amplitude(const double& t1, const double& t2, bool B1_is_B0, bool B2_is_B0) const;

  /** mixing frequency in hbar/mm */
  double _freq{0};   //

  /** C eigenvalue, 0= incoherent */
  double _C{0}; //

  /** Is BBpipi? */
  bool _BBpipi{0};

};

/** The class provides routine to decay vector-> particle particle with B0 mixing, handles states with only one neutral B
 */
class EvtPHSPBMix : public EvtDecayAmp  {

public:

  /** Default constructor */
  EvtPHSPBMix() {}

  /** Default destructor */
  virtual ~EvtPHSPBMix();

  /** Get function Name  */
  std::string getName();

  /** Clone the decay  */
  EvtDecayBase* clone();

  /** Decay function */
  void decay(EvtParticle* p);

  /** Init function */
  void init();

  /** Init maximal prob.  function */
  void initProbMax();

  /** Number of real daughters */
  int nRealDaughters();

private:

  /** mixing frequency in hbar/mm */
  double _freq{0};

};

#endif
