/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtPDL.hh"
#include <string>

#include "generators/evtgen/EvtGenModelRegister.h"
#include "generators/evtgen/models/EvtHNLLLSW.h"
#include "generators/evtgen/models/EvtLLSWFF.h"
#include "generators/evtgen/models/EvtHNLSemiLeptonicScalarAmp.h"
#include "generators/evtgen/models/EvtHNLSemiLeptonicVectorAmp.h"
#include "generators/evtgen/models/EvtHNLSemiLeptonicTensorAmp.h"

B2_EVTGEN_REGISTER_MODEL(EvtHNLLLSW);

EvtHNLLLSW::EvtHNLLLSW():
  llswffmodel(0)
  , calcamp(0)
{}


EvtHNLLLSW::~EvtHNLLLSW()
{
  delete llswffmodel;
  llswffmodel = 0;
  delete calcamp;
  calcamp = 0;
}

std::string EvtHNLLLSW::getName()
{

  return "HNLLLSW";

}



EvtDecayBase* EvtHNLLLSW::clone()
{

  return new EvtHNLLLSW;

}

void EvtHNLLLSW::decay(EvtParticle* p)
{

  p->initializePhaseSpace(getNDaug(), getDaugs());

  calcamp->CalcAmp(p, _amp2, llswffmodel);

}

void EvtHNLLLSW::initProbMax()
{

  EvtId lnum; //parnum, mesnum;

  //parnum = getParentId();
  //mesnum = getDaug(0);
  lnum = getDaug(1);

  // Leptons
  static EvtId EM = EvtPDL::getId("e-");
  static EvtId EP = EvtPDL::getId("e+");
  static EvtId MUM = EvtPDL::getId("mu-");
  static EvtId MUP = EvtPDL::getId("mu+");
  static EvtId TAUM = EvtPDL::getId("tau-");
  static EvtId TAUP = EvtPDL::getId("tau+");


  if (lnum == EP || lnum == EM || lnum == MUP || lnum == MUM) {
    setProbMax(5000.0);
    return;
  }
  if (lnum == TAUP || lnum == TAUM) {
    setProbMax(4000.0);
    return;
  }


}

void EvtHNLLLSW::init()
{

  if (getNArg() < 2) checkNArg(2);
  checkNDaug(3);

  static EvtId D1P1P = EvtPDL::getId("D_1+");
  static EvtId D1P1N = EvtPDL::getId("D_1-");
  static EvtId D1P10 = EvtPDL::getId("D_10");
  static EvtId D1P1B = EvtPDL::getId("anti-D_10");
  static EvtId D3P2P = EvtPDL::getId("D_2*+");
  static EvtId D3P2N = EvtPDL::getId("D_2*-");
  static EvtId D3P20 = EvtPDL::getId("D_2*0");
  static EvtId D3P2B = EvtPDL::getId("anti-D_2*0");

  static EvtId DS1P = EvtPDL::getId("D_s1+");
  static EvtId DS1M = EvtPDL::getId("D_s1-");
  static EvtId DS2STP = EvtPDL::getId("D_s2*+");
  static EvtId DS2STM = EvtPDL::getId("D_s2*-");

  EvtId daughter = getDaug(0);

  bool isNarrow = false;

  if (daughter == D1P1P || daughter == D1P1N || daughter == D1P10 || daughter == D1P1B ||
      daughter == D3P2P || daughter == D3P2N || daughter == D3P20 || daughter == D3P2B ||
      daughter == DS1P || daughter == DS1M || daughter == DS2STP || daughter == DS2STM)
    isNarrow = true;

  //We expect the parent to be a scalar
  //and the daughters to be X lepton neutrino

  checkSpinParent(EvtSpinType::SCALAR);
  checkSpinDaughter(1, EvtSpinType::DIRAC);
  checkSpinDaughter(2, EvtSpinType::DIRAC);

  EvtSpinType::spintype mesontype = EvtPDL::getSpinType(getDaug(0));

  if (isNarrow)
    llswffmodel = new EvtLLSWFF(getArg(0), getArg(1), getNArg() > 2 ? getArg(2) : 0., getNArg() > 3 ? getArg(3) : 0.);
  else
    llswffmodel = new EvtLLSWFF(getArg(0), getArg(1), getNArg() > 2 ? getArg(2) : 0.);

  if (mesontype == EvtSpinType::SCALAR) {
    calcamp = new EvtHNLSemiLeptonicScalarAmp;
  }
  if (mesontype == EvtSpinType::VECTOR) {
    calcamp = new EvtHNLSemiLeptonicVectorAmp;
  }
  if (mesontype == EvtSpinType::TENSOR) {
    calcamp = new EvtHNLSemiLeptonicTensorAmp;
  }

}







