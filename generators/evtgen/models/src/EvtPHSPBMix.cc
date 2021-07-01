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
//   R. Louvot, EPFL, 2010/03/09  Module created
//   C. MacQueen, 2016/10/03    Adapted to Belle II
//
//------------------------------------------------------------------------
//
#include <cmath>
#include <stdlib.h>
#include "EvtGenBase/EvtConst.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtVector4C.hh"
#include "generators/evtgen/models/EvtPHSPBMix.h"
#include <generators/evtgen/EvtGenModelRegister.h>
#include "EvtGenBase/EvtId.hh"
#include <string>
#include <sstream>
#include "EvtGenBase/EvtRandom.hh"

using std::endl;

B2_EVTGEN_REGISTER_MODEL(EvtPHSPBBMix);


EvtPHSPBBMix::~EvtPHSPBBMix() {}

std::string EvtPHSPBBMix::getName()
{
  return "PHSP_BB_MIX";
}


EvtDecayBase* EvtPHSPBBMix::clone()
{
  return new EvtPHSPBBMix;
}

void EvtPHSPBBMix::init()
{
  // check that there we are provided exactly one parameter
  //One arg: Delta m
  const unsigned short int narg(2);
  if (getNArg() != narg) {
    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "EvtPHSPBBMix generator expected "
                                         << " " << narg
                                         << "argument(s) (deltam, C=-1, +1 or 0 (incoherent)) but found:"
                                         << getNArg() << endl;
    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Will terminate execution!" << endl;
    ::abort();
  }


  //report(DEBUG,"EvtGen") << "EvtPHSPBBMIX::init point (2)"<<std::endl;
  //usage: mixing P0-P0b       : 2 arg. -> P0  P0b                PHSP_BB_MIX dm -1;
  //     : mixing P0*-P0b + cc : 4 arg. -> P0* P0b P0*b P0        PHSP_BB_MIX dm +1;
  //     : mixing P0-P0b   pi  : 3 arg. -> P0 P0b pi0             PHSP_BB_MIX dm -1;
  //     : mixing P0*-P0b  pi  : 5 arg. -> P0* P0b pi0 P0*b P0    PHSP_BB_MIX dm +1;
  //     : mixing P0-P0b pi pi : 4 arg. -> P0 P0b pi pi           PHSP_BB_MIX dm -1;

  if (getNDaug() < 2 || getNDaug() > 5) {
    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "PHSP_BB_MIX n daughters not ok :"
                                         << getNDaug() << endl;
    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Will terminate execution!" << endl;
    ::abort();
  }
  //  checkNDaug(2,3,4,5,6);
  // report(DEBUG,"EvtGen") << "EvtPHSPBBMIX::init point (3)"<<std::endl;



  _BBpipi = false;

  if (getNDaug() == 4 && (EvtPDL::chargeConj(getDaug(0)) == getDaug(1)))
    _BBpipi = true;


  if (!_BBpipi && getNDaug() > 3) {
    if (!(EvtPDL::chargeConj(getDaug(0)) == getDaug(getNDaug() - 2) && EvtPDL::chargeConj(getDaug(1)) == getDaug(getNDaug() - 1))) {
      EvtGenReport(EVTGEN_ERROR, "EvtGen") << "EvtPHSPBBMix generator expected daughters "
                                           << "to be charge conjugate." << endl
                                           << "  Found " << EvtPDL::name(getDaug(0)).c_str()
                                           << "," << EvtPDL::name(getDaug(1)).c_str()
                                           << "," << EvtPDL::name(getDaug(2)).c_str()
                                           << " and "
                                           << EvtPDL::name(getDaug(3)).c_str() << endl;
      EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Will terminate execution!" << endl;
      ::abort();
    }
  } else {

    if (!(EvtPDL::chargeConj(getDaug(0)) == getDaug(1))) {
      EvtGenReport(EVTGEN_ERROR, "EvtGen") << "EvtPHSPBBMix generator expected daughters "
                                           << "to be charge conjugate." << endl
                                           << "  Found " << EvtPDL::name(getDaug(0)).c_str()
                                           << " and "
                                           << EvtPDL::name(getDaug(1)).c_str() << endl;
      EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Will terminate execution!" << endl;
      ::abort();
    }

  }
  //report(DEBUG,"EvtGen") << "EvtPHSPBBMIX::init point (4)"<<std::endl;
  // check that we are asked to decay a vector particle into a pair
  // of scalar particles

  // checkSpinParent(EvtSpinType::VECTOR);

  // checkSpinDaughter(0,EvtSpinType::SCALAR);
  // checkSpinDaughter(1,EvtSpinType::SCALAR);

  // check that our daughter particles are charge conjugates of each other


  // check that both daughter particles have the same lifetime
  if (EvtPDL::getctau(getDaug(0)) != EvtPDL::getctau(getDaug(1))) {
    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "EvtPHSPBBMix generator expected daughters "
                                         << "to have the same lifetime." << endl
                                         << "  Found ctau = "
                                         << EvtPDL::getctau(getDaug(0)) << " mm and "
                                         << EvtPDL::getctau(getDaug(1)) << " mm" << endl;
    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Will terminate execution!" << endl;
    ::abort();
  }
  // precompute quantities that will be used to generate events
  // and print out a summary of parameters for this decay
  //report(DEBUG,"EvtGen") << "EvtPHSPBBMIX::init point (5)"<<std::endl;

  // mixing frequency in hbar/mm
  _freq = getArg(0) / EvtConst::c;

  _C = getArg(1);

  // deltaG
  //double gamma= 1/EvtPDL::getctau(getDaug(0));  // gamma/c (1/mm)
  // q/p
  // decay amplitudes
  // CPT violation in mixing
  // some printout
  double tau = 1e12 * EvtPDL::getctau(getDaug(0)) / EvtConst::c; // in ps
  double dm = 1e-12 * getArg(0); // B0/anti-B0 mass difference in hbar/ps
  double x = dm * tau;

  //report(DEBUG,"EvtGen") << "EvtPHSPBBMIX::init point (6)"<<std::endl;


  std::ostringstream sss;
  sss << "    " << EvtPDL::name(getParentId()).c_str() << " --> "
      << EvtPDL::name(getDaug(0)).c_str() << " + "
      << EvtPDL::name(getDaug(1)).c_str();
  switch (getNDaug()) {
    case 3:
    case 5:
      sss << " + " << EvtPDL::name(getDaug(2)).c_str();
      break;
    case 4:
      if (_BBpipi)
        sss << " + " << EvtPDL::name(getDaug(2)).c_str() << " + " << EvtPDL::name(getDaug(3)).c_str();
      break;
    default: ;
  }
  EvtGenReport(EVTGEN_INFO, "EvtGen") << "PHSP_BB_MIX will generate mixing :"
                                      << endl << endl
                                      << sss.str() << endl << endl
                                      << "using parameters:" << endl << endl
                                      << "  delta(m)  = " << dm << " hbar/ps" << endl
                                      << " C (B0-B0b) = " << _C << endl
                                      << "  _freq     = " << _freq << " hbar/mm" << endl
                                      << "  dgog      = "  << 0 << endl
                                      << "  dGamma    = "  << 0 << " hbar/mm" << endl
                                      << "  q/p       = " << 1 << endl
                                      << "  tau       = " << tau << " ps" << endl
                                      << "  x         = " << x << endl
                                      << endl;





}

int EvtPHSPBBMix::nRealDaughters()
{
  if (getNDaug() > 3) {
    if (_BBpipi)
      return 4;
    else
      return getNDaug() - 2;
  }
  //else
  return getNDaug();


}


void EvtPHSPBBMix::initProbMax()
{
  // this value is ok for reasonable values of all the parameters
  setProbMax(4.0);
  //noProbMax();
}

void EvtPHSPBBMix::prlp(int i) const
{
  EvtGenReport(EVTGEN_INFO, "EvtGen") << "decay p" << i << endl;
}

void EvtPHSPBBMix::decay(EvtParticle* p)
{
  //p->initializePhaseSpace(getNDaug(),getDaugs());
  //return;

  if (_print_info) prlp(1);

  static const EvtId B0(EvtPDL::getId("B0"));
  static const EvtId B0B(EvtPDL::getId("anti-B0"));

  // generate a final state according to phase space

  //  const bool ADaug(getNDaug()==4);//true if aliased daughters
  if (_print_info) prlp(102);

  const bool NCC(getNDaug() >= 4 && !_BBpipi); //true if daughters not charged-conjugated
  //    const bool BBpi(getNDaug()==
  if (_print_info) prlp(103);
  if (NCC) {
    if (_print_info) prlp(2);
    std::vector<EvtId> tempDaug(getNDaug() - 2);
    tempDaug[0] = getDaug(0);
    tempDaug[1] = getDaug(1);
    if (getNDaug() == 5)
      tempDaug[2] = getDaug(2);

    p->initializePhaseSpace(getNDaug() - 2, tempDaug.data());

    if (_print_info) prlp(222);
  } else { //nominal case.
    p->initializePhaseSpace(getNDaug(), getDaugs());

  }
  if (_print_info) prlp(3);
  EvtParticle* s1, *s2;

  s1 = p->getDaug(0);
  s2 = p->getDaug(1);
  //delete any daughters - if there are daughters, they
  //are from the initialization and will be redone later
  if (s1->getNDaug() > 0) { s1->deleteDaughters();}
  if (s2->getNDaug() > 0) { s2->deleteDaughters();}

  EvtVector4R p1 = s1->getP4();
  EvtVector4R p2 = s2->getP4();

  // throw 2 random numbers to decide whether B1 and B2 are B0 or B0B
  const EvtId B1(EvtRandom::random() > 0.5 ? B0 : B0B);
  const EvtId B2(EvtRandom::random() > 0.5 ? B0 : B0B);
  if (_print_info) prlp(5);

  if (NCC)

  {
    if (getNDaug() == 4) {
      s1->init(B1 == B0 ? getDaug(0) : getDaug(2), p1);
      s2->init(B2 == B0 ? getDaug(3) : getDaug(1), p2);
    }
    if (getNDaug() == 5) {
      s1->init(B1 == B0 ? getDaug(0) : getDaug(3), p1);
      s2->init(B2 == B0 ? getDaug(4) : getDaug(1), p2);
    }
  } else {
    s1->init(B1 == B0 ? getDaug(0) : getDaug(1), p1);
    s2->init(B2 == B0 ? getDaug(0) : getDaug(1), p2);
  }

  if (_print_info) prlp(6);

  // choose a decay time for each final state particle using the
  // lifetime (which must be the same for both particles) in pdt.table
  // and calculate the lifetime difference for this event
  s1->setLifetime();
  s2->setLifetime();

  const double t1(s1->getLifetime());
  const double t2(s2->getLifetime());

  // calculate the oscillation amplitude, based on whether this event is mixed or not
  EvtComplex osc_amp(Amplitude(t1, t2, B1 == B0, B2 == B0)); //Amplitude return <B0(B)B0(B)|B1B2>
  if (_print_info) prlp(8);


  // store the amplitudes for each parent spin basis state
  double norm = 1.0 / p1.d3mag();
  if (_print_info)  prlp(9);
  vertex(0, norm * osc_amp * p1 * (p->eps(0)));
  vertex(1, norm * osc_amp * p1 * (p->eps(1)));
  vertex(2, norm * osc_amp * p1 * (p->eps(2)));
  if (_print_info)   prlp(10);
  return ;
}

EvtComplex EvtPHSPBBMix::Amplitude(const double& t1, const double& t2, bool B1_is_B0, bool B2_is_B0) const
{
  if (_C != 0 && _C != -1 && _C != 1)
    return EvtComplex(0., 0.);

  const double f(_freq);
  if (B1_is_B0 && !B2_is_B0) {
    if (_C == 0)
      return EvtComplex(cos(f * t1 / 2.) * cos(f * t2 / 2.), 0.);
    else
      return EvtComplex(cos(f * (t2 + _C * t1) / 2.) / sqrt(2.), 0.);
  }
  if (!B1_is_B0 && B2_is_B0) {
    if (_C == 0)
      return EvtComplex(-sin(f * t1 / 2.) * sin(f * t2 / 2.), 0.);
    else
      return EvtComplex(cos(f * (t2 + _C * t1) / 2.) * _C / sqrt(2.), 0.);
  }
  if (B1_is_B0 && B2_is_B0) {
    if (_C == 0)
      return EvtComplex(0., -cos(f * t1 / 2.) * sin(f * t2 / 2.));
    else
      return EvtComplex(0., -sin(f * (t2 + _C * t1) / 2.) / sqrt(2.));
  }
  if (!B1_is_B0 && !B2_is_B0) {
    if (_C == 0)
      return EvtComplex(0., -sin(f * t1 / 2.) * cos(f * t2 / 2.));
    else
      return EvtComplex(0., -sin(f * (t2 + _C * t1) / 2.) * _C / sqrt(2.));
  }
  // no way to reach this but compiler complains without a return statement
  return EvtComplex(0., 0.);
}


/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////


B2_EVTGEN_REGISTER_MODEL(EvtPHSPBMix);


EvtPHSPBMix::~EvtPHSPBMix() {}

std::string EvtPHSPBMix::getName()
{
  return "PHSP_B_MIX";
}


EvtDecayBase* EvtPHSPBMix::clone()
{
  return new EvtPHSPBMix;
}

void EvtPHSPBMix::init()
{
  // check that there we are provided exactly one parameter
  //One arg: Delta m
  const unsigned short int narg(1);
  if (getNArg() != narg) {
    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "EvtPHSPBMix generator expected "
                                         << " " << narg
                                         << " argument(s) (delta m) but found:"
                                         << getNArg() << endl;
    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Will terminate execution!" << endl;
    ::abort();
  }


  //report(DEBUG,"EvtGen") << "EvtPHSPBMIX::init point (2)"<<std::endl;
  //usage: mixing P0 P- pi+    : 4 arg. -> P0  P- pi+       P0b    PHSP_B_MIX dm;
  //     : mixing P0 P- pi+ pi : 5 arg. -> P0  P- pi+  pi0  P0b    PHSP_B_MIX dm;


  checkNDaug(4, 5);
  // report(DEBUG,"EvtGen") << "EvtPHSPBMIX::init point (3)"<<std::endl;

  bool BBpipi(getNDaug() == 5);

  if (!(EvtPDL::chargeConj(getDaug(0)) == getDaug(getNDaug() - 1))) {
    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "EvtPHSPBMix generator expected two first particles "
                                         << "to be charge conjugate." << endl
                                         << "  Found " << EvtPDL::name(getDaug(0)).c_str()
                                         << "," << EvtPDL::name(getDaug(getNDaug() - 1)).c_str() << endl;
    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Will terminate execution!" << endl;
    ::abort();
  }
  //parameters for mixing of particle1

  _freq = getArg(0) / EvtConst::c;

  //double gamma= 1/EvtPDL::getctau(getDaug(0));  // gamma/c (1/mm)

  double tau = 1e12 * EvtPDL::getctau(getDaug(0)) / EvtConst::c; // in ps
  double dm = 1e-12 * getArg(0); // B0/anti-B0 mass difference in hbar/ps
  double x = dm * tau;

  std::ostringstream sss;
  sss << "    " << EvtPDL::name(getParentId()).c_str() << " --> "
      << EvtPDL::name(getDaug(0)).c_str() << " + "
      << EvtPDL::name(getDaug(1)).c_str();
  sss << " + " << EvtPDL::name(getDaug(2)).c_str();
  if (BBpipi)
    sss << " + " << EvtPDL::name(getDaug(3)).c_str();

  EvtGenReport(EVTGEN_INFO, "EvtGen") << "PHSP_B_MIX will generate mixing :"
                                      << endl << endl
                                      << sss.str() << endl << endl
                                      << "using parameters:" << endl << endl
                                      << "  delta(m)  = " << dm << " hbar/ps" << endl
                                      << "  _freq     = " << _freq << " hbar/mm" << endl
                                      << "  dgog      = "  << 0 << endl
                                      << "  dGamma    = "  << 0 << " hbar/mm" << endl
                                      << "  q/p       = " << 1 << endl
                                      << "  tau       = " << tau << " ps" << endl
                                      << "  x         = " << x << endl
                                      << endl;

}

int EvtPHSPBMix::nRealDaughters()
{
  return getNDaug() - 1;
}

void EvtPHSPBMix::initProbMax()
{
  // this value is ok for reasonable values of all the parameters
  setProbMax(4.0);
  //noProbMax();
}

void EvtPHSPBMix::decay(EvtParticle* p)
{

  // generate a final state according to phase space

  //  const bool BBpipi(getNDaug()!=4);//true if BBpipi, if not BBpi
  if (_print_info)    std::cout << "decay B_MIX (0)" << std::endl;
  std::vector<EvtId> tempDaug(getNDaug() - 1);
  tempDaug[0] = getDaug(0);
  tempDaug[1] = getDaug(1);
  tempDaug[2] = getDaug(2);
  if (getNDaug() == 5)
    tempDaug[3] = getDaug(3);

  if (_print_info)    std::cout << "decay B_MIX (1)" << std::endl;


  p->initializePhaseSpace(getNDaug() - 1, tempDaug.data());

  EvtParticle* s1;

  s1 = p->getDaug(0);

  //delete any daughters - if there are daughters, they
  //are from the initialization and will be redone later
  if (s1->getNDaug() > 0) { s1->deleteDaughters();}

  EvtVector4R p1 = s1->getP4();

  // throw 1 random numbers to decide whether B1 is B0 or B0B
  const bool changed_flavor(EvtRandom::random() > 0.5); //Daug(0) becomes Daug(1)
  if (_print_info)    std::cout << "decay B_MIX (3)" << std::endl;
  s1->init(changed_flavor ? getDaug(getNDaug() - 1) : getDaug(0), p1);

  // choose a decay time for each final state particle using the
  // lifetime (which must be the same for both particles) in pdt.table
  // and calculate the lifetime difference for this event
  s1->setLifetime();
  if (_print_info)      std::cout << "decay B_MIX (4)" << std::endl;
  const double t1(s1->getLifetime());

  // calculate the oscillation amplitude, based on whether this event is mixed or not
  EvtComplex osc_amp(changed_flavor ? EvtComplex(0,
                                                 -sin(_freq * t1 / 2.)) : EvtComplex(cos(_freq * t1 / 2.))); //Amplitude return <B0(B)|B1>
  if (_print_info)    std::cout << "decay B_MIX (5)" << std::endl;
  // store the amplitudes for each parent spin basis state
  double norm = 1.0 / p1.d3mag();
  if (_print_info)    std::cout << "decay B_MIX (6)" << std::endl;
  vertex(0, norm * osc_amp * p1 * (p->eps(0)));
  vertex(1, norm * osc_amp * p1 * (p->eps(1)));
  vertex(2, norm * osc_amp * p1 * (p->eps(2)));
  if (_print_info)  std::cout << "decay B_MIX (7)" << std::endl;
  return ;
}



