/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <EvtGenBase/EvtParticle.hh>
#include <EvtGenBase/EvtDiracSpinor.hh>
#include <EvtGenBase/EvtVector4C.hh>
#include <EvtGenBase/EvtVector4R.hh>
#include <string>

#include <generators/evtgen/EvtGenModelRegister.h>
#include <generators/evtgen/models/EvtHNLScalarL.h>


B2_EVTGEN_REGISTER_MODEL(EvtHNLScalarL);


EvtHNLScalarL::EvtHNLScalarL() {}

EvtHNLScalarL::~EvtHNLScalarL() {}

std::string EvtHNLScalarL::getName()
{
  return "HNLSCALARL";
}



EvtDecayBase* EvtHNLScalarL::clone()
{
  return new EvtHNLScalarL;
}


void EvtHNLScalarL::init()
{
  checkNArg(0);
  checkNDaug(2);

  checkSpinParent(EvtSpinType::DIRAC);

  checkSpinDaughter(0, EvtSpinType::SCALAR);
  checkSpinDaughter(1, EvtSpinType::DIRAC);
}


void EvtHNLScalarL::initProbMax()
{
  setProbMax(10000.0);
}


void EvtHNLScalarL::decay(EvtParticle* p)
{
  p->initializePhaseSpace(getNDaug(), getDaugs());

  const EvtParticle* s = p->getDaug(0);
  const EvtParticle* l = p->getDaug(1);

  const EvtVector4R p4_s{ s->getP4() };

  const int parId{ p->getPDGId() };

  for (int i{ 0 }; i < 2; ++i) {
    for (int j{ 0 }; j < 2; ++j) {
      const EvtVector4C current{
        EvtLeptonVACurrent(l->spParent(j), p->sp(i)) };

      // Apply CP conjugation for anti-particles
      if (parId < 0) {
        vertex(i, j, ::conj(p4_s * current));
      } else {
        vertex(i, j, p4_s * current);
      }
    }
  }
}

