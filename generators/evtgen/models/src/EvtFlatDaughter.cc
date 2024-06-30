/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/evtgen/EvtGenModelRegister.h>

#include <stdlib.h>
#include <cmath>
#include "EvtGenBase/EvtRandom.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtGenKine.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtConst.hh"
#include "EvtGenBase/EvtId.hh"

#include "generators/evtgen/models/EvtFlatDaughter.h"

using std::endl;

namespace Belle2 {

  /** register the model in EvtGen */
  B2_EVTGEN_REGISTER_MODEL(EvtFlatDaughter);

  EvtFlatDaughter::~EvtFlatDaughter() {}

  std::string EvtFlatDaughter::getName()
  {
    return "FLAT_DAUGHTER";
  }

  EvtDecayBase* EvtFlatDaughter::clone()
  {
    return new EvtFlatDaughter;
  }

  void EvtFlatDaughter::decay(EvtParticle* p)
  {
    // variables to find.
    EvtVector4R p4KorKstar;
    EvtVector4R p4norantin;
    EvtVector4R p4antinorn;

    // start!
    p->makeDaughters(getNDaug(), getDaugs());

    EvtParticle* KorKstar = p->getDaug(0);
    EvtParticle* norantin = p->getDaug(1);
    EvtParticle* antinorn = p->getDaug(2);

    double mass[3];

    findMasses(p, getNDaug(), getDaugs(), mass);

    double m_B = p->mass();
    double m_K = mass[0];
    double m_norantin = mass[1];
    double m_antinorn = mass[2];

    double M_max = m_B - m_K;
    double M_min = m_norantin + m_antinorn;

    // determined M_nn
    double Mnn = EvtRandom::Flat(M_min, M_max);

    // B -> K(*) (nnbar) decay. Momentums are determined in B meson's rest frame
    EvtVector4R p4Kdin[2];

    double mKdin[2];
    mKdin[0] = m_K;
    mKdin[1] = Mnn;

    EvtGenKine::PhaseSpace(2, mKdin, p4Kdin, m_B);

    // (nnbar) -> n nbar decay. Momentums are determined in (nnbar) rest frame
    EvtVector4R p4nn[2];

    double mnn[2];
    mnn[0] = m_norantin;
    mnn[1] = m_antinorn;

    EvtGenKine::PhaseSpace(2, mnn, p4nn, Mnn);

    // boost to B meson rest frame
    p4norantin = boostTo(p4nn[0], p4Kdin[1]);
    p4antinorn = boostTo(p4nn[1], p4Kdin[1]);

    // initialize the decay products
    KorKstar->init(getDaug(0), p4Kdin[0]);
    norantin->init(getDaug(1), p4norantin);
    antinorn->init(getDaug(2), p4antinorn);


    return;

  }


  void EvtFlatDaughter::initProbMax()
  {
    noProbMax();
  }


  void EvtFlatDaughter::init()
  {

    // check that there are two arguments - Mmin Mmax
    checkNArg(0);

    // check the number of daughters. It should be 3
    checkNDaug(3);

  }


}


