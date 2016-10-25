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
#include <sstream>

class EvtPHSPBBMix: public EvtDecayAmp {

public:

  EvtPHSPBBMix() {}//std::cout<<"AREARE BB"<<std::endl;}
  virtual ~EvtPHSPBBMix();

  std::string getName(); /**< Get function Name  */
  EvtDecayBase* clone();  /**< Clone the decay  */

  void decay(EvtParticle* p);
  void init();
  void initProbMax();

  int nRealDaughters();

private:

  void prlp(int)const;
  EvtComplex Amplitude(const double& t1, const double& t2, bool B1_is_B0, bool B2_is_B0) const;
  double _freq;   // mixing frequency in hbar/mm
  double _C; //C eigenvalue, 0= incoherent
  bool _BBpipi;
  //  double _dGamma;
  //EvtComplex _qoverp;
  //EvtComplex _poverq;
  //EvtComplex _z;
  //double _chib0_b0bar;
  //double _chib0bar_b0;

  //EvtComplex _A_f;
  //EvtComplex _Abar_f;

  //EvtComplex _A_fbar;
  //EvtComplex _Abar_fbar;

};

class EvtPHSPBMix : public EvtDecayAmp  {

public:

  EvtPHSPBMix() {}//std::cout<<"AREARE B"<<std::endl;}
  virtual ~EvtPHSPBMix();

  std::string getName(); /**< Get function Name  */
  EvtDecayBase* clone();  /**< Clone the decay  */

  void decay(EvtParticle* p);
  void init();
  void initProbMax();

  int nRealDaughters();

private:

  double _freq;   // mixing frequency in hbar/mm

};

#endif
