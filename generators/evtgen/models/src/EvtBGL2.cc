/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <stdlib.h>
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtGenKine.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtSemiLeptonicScalarAmp.hh"
#include "EvtGenBase/EvtSemiLeptonicVectorAmp.hh"
#include <string>

#include "generators/evtgen/EvtGenModelRegister.h"
#include "generators/evtgen/models/EvtBGL2.h"
#include "generators/evtgen/models/EvtBGL2FF.h"

B2_EVTGEN_REGISTER_MODEL(EvtBGL2);

EvtBGL2::EvtBGL2():
  m_bgl2ffmodel(nullptr)
  , m_calcamp(nullptr)
{}

EvtBGL2::~EvtBGL2()
{
  delete m_bgl2ffmodel;
  m_bgl2ffmodel = nullptr;
  delete m_calcamp;
  m_calcamp = nullptr;
}

std::string EvtBGL2::getName()
{

  return "BGL2";

}



EvtDecayBase* EvtBGL2::clone()
{

  return new EvtBGL2;

}


void EvtBGL2::decay(EvtParticle* p)
{

  p->initializePhaseSpace(getNDaug(), getDaugs());
  m_calcamp->CalcAmp(p, _amp2, m_bgl2ffmodel);

}

void EvtBGL2::initProbMax()
{

  EvtId parnum, mesnum, lnum, nunum;

  parnum = getParentId();
  mesnum = getDaug(0);
  lnum = getDaug(1);
  nunum = getDaug(2);

  double mymaxprob = m_calcamp->CalcMaxProb(parnum, mesnum,
                                            lnum, nunum, m_bgl2ffmodel);

  // Leptons
  static EvtId EM = EvtPDL::getId("e-");
  static EvtId EP = EvtPDL::getId("e+");
  static EvtId MUM = EvtPDL::getId("mu-");
  static EvtId MUP = EvtPDL::getId("mu+");
  static EvtId TAUM = EvtPDL::getId("tau-");
  static EvtId TAUP = EvtPDL::getId("tau+");

  if (lnum == EP || lnum == EM || lnum == MUP || lnum == MUM) {
    setProbMax(mymaxprob);
    return;
  }
  if (lnum == TAUP || lnum == TAUM) {
    setProbMax(6500);
    return;
  }



}


void EvtBGL2::init()
{

  checkNDaug(3);

  //We expect the parent to be a scalar
  //and the daughters to be X lepton neutrino
  checkSpinParent(EvtSpinType::SCALAR);

  checkSpinDaughter(1, EvtSpinType::DIRAC);
  checkSpinDaughter(2, EvtSpinType::NEUTRINO);

  EvtSpinType::spintype d1type = EvtPDL::getSpinType(getDaug(0));
  if (d1type == EvtSpinType::SCALAR) {
    if (getNArg() == 8) {
      m_bgl2ffmodel = new EvtBGL2FF(getArg(0), getArg(1), getArg(2), getArg(3), getArg(4), getArg(5), getArg(6), getArg(7));
      m_calcamp = new EvtSemiLeptonicScalarAmp;
    } else {
      EvtGenReport(EVTGEN_ERROR, "EvtGen") << "BGL2 (N=3) model for scalar meson daughters needs 8 arguments. Sorry." << std::endl;

      ::abort();
    }
  }  else {
    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "BGL2 model handles only scalar meson daughters. Use the EvtBGL class for vector mesons." <<
                                         std::endl;
    ::abort();
  }

}