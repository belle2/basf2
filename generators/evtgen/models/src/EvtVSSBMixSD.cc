/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/evtgen/EvtGenModelRegister.h>
#include "generators/evtgen/models/EvtVSSBMixSD.h"

#include "EvtGenBase/EvtConst.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtVector4C.hh"
#include "EvtGenBase/EvtId.hh"
#include "EvtGenBase/EvtRandom.hh"

#include <stdlib.h>
#include <string>

/** register the model in EvtGen */
B2_EVTGEN_REGISTER_MODEL(EvtVSSBMixSD);

using std::endl;

EvtVSSBMixSD::~EvtVSSBMixSD() {}

std::string EvtVSSBMixSD::getName()
{
  return "VSS_BMIX_SD";
}

EvtDecayBase* EvtVSSBMixSD::clone()
{
  return new EvtVSSBMixSD;
}

void EvtVSSBMixSD::init()
{

  if (getNArg() > 4)
    checkNArg(14, 12, 8);

  if (getNArg() < 1) {
    EvtGenReport(EVTGEN_ERROR, "EvtGen")
        << "EvtVSSBMix generator expected "
        << " at least 1 argument (deltam) but found:" << getNArg() << endl;
    EvtGenReport(EVTGEN_ERROR, "EvtGen")
        << "Will terminate execution!" << endl;
    ::abort();
  }
  // check that we are asked to produced exactly 2 daughters
  //4 are allowed if they are aliased..
  checkNDaug(2, 4);

  if (getNDaug() == 4) {
    if (getDaug(0) != getDaug(2) || getDaug(1) != getDaug(3)) {
      EvtGenReport(EVTGEN_ERROR, "EvtGen")
          << "EvtVSSBMixSD generator allows "
          << " 4 daughters only if 1=3 and 2=4"
          << " (but 3 and 4 are aliased " << endl;
      EvtGenReport(EVTGEN_ERROR, "EvtGen")
          << "Will terminate execution!" << endl;
      ::abort();
    }
  }
  // check that we are asked to decay a vector particle into a pair
  // of scalar particles

  checkSpinParent(EvtSpinType::VECTOR);

  checkSpinDaughter(0, EvtSpinType::SCALAR);
  checkSpinDaughter(1, EvtSpinType::SCALAR);

  // check that our daughter particles are charge conjugates of each other
  if (!(EvtPDL::chargeConj(getDaug(0)) == getDaug(1))) {
    EvtGenReport(EVTGEN_ERROR, "EvtGen")
        << "EvtVSSBMixSD generator expected daughters "
        << "to be charge conjugate." << endl
        << "  Found " << EvtPDL::name(getDaug(0)).c_str() << " and "
        << EvtPDL::name(getDaug(1)).c_str() << endl;
    EvtGenReport(EVTGEN_ERROR, "EvtGen")
        << "Will terminate execution!" << endl;
    ::abort();
  }
  // check that both daughter particles have the same lifetime
  if (EvtPDL::getctau(getDaug(0)) != EvtPDL::getctau(getDaug(1))) {
    EvtGenReport(EVTGEN_ERROR, "EvtGen")
        << "EvtVSSBMixSD generator expected daughters "
        << "to have the same lifetime." << endl
        << "  Found ctau = " << EvtPDL::getctau(getDaug(0))
        << " mm and " << EvtPDL::getctau(getDaug(1)) << " mm" << endl;
    EvtGenReport(EVTGEN_ERROR, "EvtGen")
        << "Will terminate execution!" << endl;
    ::abort();
  }
  // precompute quantities that will be used to generate events
  // and print out a summary of parameters for this decay

  // mixing frequency in hbar/mm
  _freq = getArg(0) / EvtConst::c;

  _sdprob = 0.0;
  if (getNArg() > 1) {
    _sdprob = getArg(1);
  }

  // some printout
  double tau = 1e12 * EvtPDL::getctau(getDaug(0)) / EvtConst::c;        // in ps
  double dm = 1e-12 * getArg(0);      // B0/anti-B0 mass difference in hbar/ps
  double x = dm * tau;

  if (verbose()) {
    EvtGenReport(EVTGEN_INFO, "EvtGen")
        << "VSS_BMix_SD will generate mixing with Spontanous Disentanglement:"
        << endl
        << endl
        << "    " << EvtPDL::name(getParentId()).c_str() << " --> "
        << EvtPDL::name(getDaug(0)).c_str() << " + "
        << EvtPDL::name(getDaug(1)).c_str() << endl
        << endl
        << "using parameters:" << endl
        << endl
        << "  delta(m)  = " << dm << " hbar/ps" << endl
        << "  _freq     = " << _freq << " hbar/mm" << endl
        << "  probSD    = " << _sdprob << endl
        << "  tau       = " << tau << " ps" << endl
        << "  x         = " << x << endl
        << endl;
  }
}

void EvtVSSBMixSD::initProbMax()
{
  // this value is ok for reasonable values of all the parameters
  setProbMax(4.0);
}

void EvtVSSBMixSD::decay(EvtParticle* p)
{
  static EvtId B0 = EvtPDL::getId("B0");
  static EvtId B0B = EvtPDL::getId("anti-B0");

  // generate a final state according to phase space

  double rndm = EvtRandom::random();

  if (getNDaug() == 4) {
    EvtId tempDaug[2];

    if (rndm < 0.5) {
      tempDaug[0] = getDaug(0);
      tempDaug[1] = getDaug(3);
    } else {
      tempDaug[0] = getDaug(2);
      tempDaug[1] = getDaug(1);
    }

    p->initializePhaseSpace(2, tempDaug);
  } else {    //nominal case.
    p->initializePhaseSpace(2, getDaugs());
  }

  EvtParticle* s1, *s2;

  s1 = p->getDaug(0);
  s2 = p->getDaug(1);
  //delete any daughters - if there are daughters, they
  //are from the initialization and will be redone later
  if (s1->getNDaug() > 0) {
    s1->deleteDaughters();
  }
  if (s2->getNDaug() > 0) {
    s2->deleteDaughters();
  }

  EvtVector4R p1 = s1->getP4();
  EvtVector4R p2 = s2->getP4();

  // throw a random number to decide if this final state should be mixed
  rndm = EvtRandom::random();
  int mixed = (rndm < 0.5) ? 1 : 0;

  // if this decay is mixed, choose one of the 2 possible final states
  // with equal probability (re-using the same random number)
  if (mixed == 1) {
    EvtId mixedId = (rndm < 0.25) ? getDaug(0) : getDaug(1);
    EvtId mixedId2 = mixedId;
    if (getNDaug() == 4 && rndm < 0.25)
      mixedId2 = getDaug(2);
    if (getNDaug() == 4 && rndm > 0.25)
      mixedId2 = getDaug(3);
    s1->init(mixedId, p1);
    s2->init(mixedId2, p2);
  }

  // if this decay is unmixed, choose one of the 2 possible final states
  // with equal probability (re-using the same random number)
  if (mixed == 0) {
    EvtId unmixedId = (rndm < 0.75) ? getDaug(0) : getDaug(1);
    EvtId unmixedId2 = (rndm < 0.75) ? getDaug(1) : getDaug(0);
    if (getNDaug() == 4 && rndm < 0.75)
      unmixedId2 = getDaug(3);
    if (getNDaug() == 4 && rndm > 0.75)
      unmixedId2 = getDaug(2);
    s1->init(unmixedId, p1);
    s2->init(unmixedId2, p2);
  }

  // choose a decay time for each final state particle using the
  // lifetime (which must be the same for both particles) in pdt.table
  // and calculate the lifetime difference for this event
  s1->setLifetime();
  s2->setLifetime();

  // calculate the oscillation amplitude, based on wether this event is mixed or not
  EvtComplex osc_amp;

  if (EvtRandom::random() < _sdprob) {
    double t1 = s1->getLifetime();
    double t2 = s2->getLifetime();

    EvtComplex exp1(0, 0.5 * _freq * t1);
    EvtComplex exp2(0, 0.5 * _freq * t2);

    EvtId finalBsig = (EvtRandom::random() > 0.5) ? B0B : B0;

    EvtComplex g1p = 0.5 * (exp(-exp1) + exp(exp1));
    EvtComplex g1m = 0.5 * (exp(-exp1) - exp(exp1));
    EvtComplex g2p = 0.5 * (exp(-exp2) + exp(exp2));
    EvtComplex g2m = 0.5 * (exp(-exp2) - exp(exp2));

    EvtComplex BB       =  g1p * g2p;  // <B0|B0(t)>
    EvtComplex barBB    =  g1m * g2p;  // <B0bar|B0(t)>
    EvtComplex BbarB    =  g1p * g2m;  // <B0|B0bar(t)>
    EvtComplex barBbarB =  g1m * g2m;  // <B0bar|B0bar(t)>

    if (!mixed && finalBsig == B0) {
      osc_amp = BB;
    }
    if (!mixed && finalBsig == B0B) {
      osc_amp = barBbarB;
    }
    if (mixed && finalBsig == B0) {
      osc_amp = barBB;
    }
    if (mixed && finalBsig == B0B) {
      osc_amp = BbarB;
    }
  } else {
    double dct = s1->getLifetime() - s2->getLifetime();    // in mm

    EvtComplex exp1(0, 0.5 * _freq * dct);

    EvtId stateAtDeltaTeq0 = (s2->getId() == B0) ? B0B : B0;

    //define some useful functions: (see BAD #188 eq. 39 for ref.)
    EvtComplex gp = 0.5 * (exp(-1.0 * exp1) + exp(exp1));
    EvtComplex gm = 0.5 * (exp(-1.0 * exp1) - exp(exp1));
    EvtComplex sqz = exp(EvtComplex(0, 0.5));

    EvtComplex BB = gp;              // <B0|B0(t)>
    EvtComplex barBB = -sqz * gm;    // <B0bar|B0(t)>
    EvtComplex BbarB = -sqz * gm;    // <B0|B0bar(t)>
    EvtComplex barBbarB = gp;        // <B0bar|B0bar(t)>

    if (!mixed && stateAtDeltaTeq0 == B0) {
      osc_amp = BB;
    }
    if (!mixed && stateAtDeltaTeq0 == B0B) {
      osc_amp = barBbarB;
    }

    if (mixed && stateAtDeltaTeq0 == B0) {
      osc_amp = barBB;
    }
    if (mixed && stateAtDeltaTeq0 == B0B) {
      osc_amp = BbarB;
    }

  }
  // store the amplitudes for each parent spin basis state
  double norm = 1.0 / p1.d3mag();
  vertex(0, norm * osc_amp * p1 * (p->eps(0)));
  vertex(1, norm * osc_amp * p1 * (p->eps(1)));
  vertex(2, norm * osc_amp * p1 * (p->eps(2)));

  return;
}
