/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "generators/evtgen/EvtGenModelRegister.h"
#include "generators/evtgen/models/EvtVSSBMixNP.h"

#include "EvtGenBase/EvtConst.hh"
#include "EvtGenBase/EvtGenKine.hh"
#include "EvtGenBase/EvtId.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtPatches.hh"
#include "EvtGenBase/EvtRandom.hh"
#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtVector4C.hh"

#include <stdlib.h>
#include <string>

/** register the model in EvtGen */
B2_EVTGEN_REGISTER_MODEL(EvtVSSBMixNP);

using std::endl;

EvtVSSBMixNP::~EvtVSSBMixNP() {}

std::string EvtVSSBMixNP::getName()
{
  return "VSSBMixNP";
}

EvtDecayBase* EvtVSSBMixNP::clone()
{
  return new EvtVSSBMixNP;
}

void EvtVSSBMixNP::init()
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
          << "EvtVSSBMixNP generator allows "
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
        << "EvtVSSBMixNP generator expected daughters "
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
        << "EvtVSSBMixNP generator expected daughters "
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

  // deltaG
  double gamma = 1 / EvtPDL::getctau(getDaug(0));        // gamma/c (1/mm)
  _dGamma = 0.0;
  if (getNArg() > 1) _dGamma =  getArg(1) * gamma;

  // lambda
  _lambda = 0.0;
  if (getNArg() > 2) _lambda = getArg(2) * (1 / EvtConst::c);
  printf("freq %e gamma %e dgamma %e lambda %e\n", _freq, gamma, _dGamma, _lambda);
  // some printout
  double dm = 1e-12 * getArg(0);      // B0/anti-B0 mass difference in hbar/ps
  if (verbose()) {
    EvtGenReport(EVTGEN_INFO, "EvtGen")
        << "VSS_NP will generate mixing with possible decoherence:"
        << endl
        << endl
        << "    " << EvtPDL::name(getParentId()).c_str() << " --> "
        << EvtPDL::name(getDaug(0)).c_str() << " + "
        << EvtPDL::name(getDaug(1)).c_str() << endl
        << endl
        << "using parameters:" << endl
        << endl
        << "  delta(m)  = " << dm << " hbar/ps" << endl
        << "  dGamma    = " << _dGamma << " hbar/mm" << endl
        << "  lambda    = " << _lambda << endl
        << endl;
  }
}

void EvtVSSBMixNP::initProbMax()
{
  // this value is ok for reasonable values of all the parameters
  setProbMax(1);
}

void EvtVSSBMixNP::decay(EvtParticle* p)
{
  // generate a final state according to phase space

  if (getNDaug() == 4) {
    double rndm = EvtRandom::random();
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

  EvtParticle* s1 = p->getDaug(0), *s2 = p->getDaug(1);

  //delete any daughters - if there are daughters, they
  //are from the initialization and will be redone later
  if (s1->getNDaug() > 0) s1->deleteDaughters();
  if (s2->getNDaug() > 0) s2->deleteDaughters();

  EvtVector4R p1 = s1->getP4();

  // choose a decay time for each final state particle using the
  // lifetime (which must be the same for both particles) in pdt.table
  // and calculate the lifetime difference for this event
  double t0, t1, tl, tr, dt; //, prob;
  do {
    s1->setLifetime();
    s2->setLifetime();
    tl = s1->getLifetime();
    tr = s2->getLifetime(); // in mm
    dt = tl - tr;
    t0 = cosh(-0.5 * _dGamma * dt);
  } while (EvtRandom::random() * 1.0001 > t0);
  t1 = exp(-_lambda * std::min(tl, tr)) * cos(_freq * dt);


  double fl = EvtRandom::random();
  int flv = 2 * t0 * EvtRandom::random() < t0 - t1;
  if (getNDaug() == 4) {
    if (flv) {
      // same flavor
      if (fl > 0.5) {
        s1->setId(getDaug(0));
        s2->setId(getDaug(2));
      } else {
        s1->setId(getDaug(3));
        s2->setId(getDaug(1));
      }
    } else {
      // opposite flavors
      if (fl > 0.5) {
        s1->setId(getDaug(0));
        s2->setId(getDaug(3));
      } else {
        s1->setId(getDaug(2));
        s2->setId(getDaug(1));
      }
    }
  } else {
    if (flv) {
      // same flavor
      if (fl > 0.5) {
        s1->setId(getDaug(0));
        s2->setId(getDaug(0));
      } else {
        s1->setId(getDaug(1));
        s2->setId(getDaug(1));
      }
    } else {
      // opposite flavors
      if (fl > 0.5) {
        s1->setId(getDaug(0));
        s2->setId(getDaug(1));
      } else {
        s1->setId(getDaug(1));
        s2->setId(getDaug(0));
      }
    }
  }

  // store the amplitudes for each parent spin basis state
  double norm = 1.0 / p1.d3mag();
  vertex(0, norm * p1 * (p->eps(0)));
  vertex(1, norm * p1 * (p->eps(1)));
  vertex(2, norm * p1 * (p->eps(2)));

  return;
}

