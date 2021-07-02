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
#include "generators/evtgen/models/EvtBGL.h"
#include "generators/evtgen/models/EvtBGLFF.h"

B2_EVTGEN_REGISTER_MODEL(EvtBGL);

using std::endl;

EvtBGL::EvtBGL():
  bglffmodel(0)
  , calcamp(0)
{}

EvtBGL::~EvtBGL()
{
  delete bglffmodel;
  bglffmodel = 0;
  delete calcamp;
  calcamp = 0;
}

std::string EvtBGL::getName()
{

  return "BGL";

}



EvtDecayBase* EvtBGL::clone()
{

  return new EvtBGL;

}


void EvtBGL::decay(EvtParticle* p)
{

  p->initializePhaseSpace(getNDaug(), getDaugs());
  calcamp->CalcAmp(p, _amp2, bglffmodel);

}

void EvtBGL::initProbMax()
{

  EvtId parnum, mesnum, lnum, nunum;

  parnum = getParentId();
  mesnum = getDaug(0);
  lnum = getDaug(1);
  nunum = getDaug(2);

  double mymaxprob = calcamp->CalcMaxProb(parnum, mesnum,
                                          lnum, nunum, bglffmodel);

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


void EvtBGL::init()
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
      bglffmodel = new EvtBGLFF(getArg(0), getArg(1), getArg(2), getArg(3), getArg(4), getArg(5), getArg(6), getArg(7));
      calcamp = new EvtSemiLeptonicScalarAmp;
    } else {
      EvtGenReport(EVTGEN_ERROR, "EvtGen") << "BGL (N=3) model for scalar meson daughters needs 8 arguments. Sorry." << endl;

      ::abort();
    }
  }  else if (d1type == EvtSpinType::VECTOR) {
    if (getNArg() == 6) {
      bglffmodel = new EvtBGLFF(getArg(0), getArg(1), getArg(2), getArg(3), getArg(4), getArg(5));
      calcamp = new EvtSemiLeptonicVectorAmp;
    } else {
      EvtGenReport(EVTGEN_ERROR, "EvtGen") << "BGL model for vector meson daughters needs 6 arguments. Sorry." << endl;
      ::abort();
    }
  } else {
    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "BGL model handles only scalar and vector meson daughters. Sorry." << endl;
    ::abort();
  }


}

