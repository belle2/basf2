/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "EvtGenBase/EvtPatches.hh"
#include <stdlib.h>
#include <assert.h>
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtGenKine.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtReport.hh"
#include <string>

#include "generators/evtgen/EvtGenModelRegister.h"
#include "generators/evtgen/models/EvtHNLHQET3.h"
#include "generators/evtgen/models/EvtHQET3FF.h"
#include "generators/evtgen/models/EvtHNLSemiLeptonicScalarAmp.h"
#include "generators/evtgen/models/EvtHNLSemiLeptonicVectorAmp.h"

B2_EVTGEN_REGISTER_MODEL(EvtHNLHQET3);

using std::endl;

EvtHNLHQET3::EvtHNLHQET3():
  hqetffmodel(0)
  , calcamp(0)
{}

EvtHNLHQET3::~EvtHNLHQET3()
{
  delete hqetffmodel;
  hqetffmodel = 0;
  delete calcamp;
  calcamp = 0;
}

std::string EvtHNLHQET3::getName()
{

  return "HNLHQET3";

}



EvtDecayBase* EvtHNLHQET3::clone()
{

  return new EvtHNLHQET3;

}


void EvtHNLHQET3::decay(EvtParticle* p)
{

  p->initializePhaseSpace(getNDaug(), getDaugs());
  calcamp->CalcAmp(p, _amp2, hqetffmodel);

}

void EvtHNLHQET3::initProbMax()
{

  EvtId parnum, mesnum, lnum, nunum;

  parnum = getParentId();
  mesnum = getDaug(0);
  lnum = getDaug(1);
  nunum = getDaug(2);

  double mymaxprob = calcamp->CalcMaxProb(parnum, mesnum,
                                          lnum, nunum, hqetffmodel);

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


void EvtHNLHQET3::init()
{

  checkNDaug(3);

  //We expect the parent to be a scalar
  //and the daughters to be X lepton neutrino
  checkSpinParent(EvtSpinType::SCALAR);

  checkSpinDaughter(1, EvtSpinType::DIRAC);
  checkSpinDaughter(2, EvtSpinType::DIRAC);

  EvtSpinType::spintype d1type = EvtPDL::getSpinType(getDaug(0));
  if (d1type == EvtSpinType::SCALAR) {
    if (getNArg() == 3) {
      hqetffmodel = new EvtHQET3FF(getArg(0), getArg(1), getArg(2));
      calcamp = new EvtHNLSemiLeptonicScalarAmp;
    } else {
      EvtGenReport(EVTGEN_ERROR, "EvtGen") << "HNLHQET3 model for scalar meson daughters needs 2 arguments. Sorry." << endl;
      ::abort();
    }
  } else if (d1type == EvtSpinType::VECTOR) {
    if (getNArg() == 5) {
      hqetffmodel = new EvtHQET3FF(getArg(0), getArg(1), getArg(2), getArg(3), getArg(4));
      calcamp = new EvtHNLSemiLeptonicVectorAmp;
    } else  {
      EvtGenReport(EVTGEN_ERROR, "EvtGen") << "HNLHQET3 model for vector meson daughtersneeds 4 arguments. Sorry." << endl;
      ::abort();
    }
  } else {
    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "HNLHQET3 model handles only scalar and vector meson daughters. Sorry." << endl;
    ::abort();
  }


}

