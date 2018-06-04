/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Markus Prim                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "EvtGenBase/EvtPatches.hh"
#include <stdlib.h>
#include <assert.h>
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtGenKine.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtSemiLeptonicVectorAmp.hh"
#include "EvtGenBase/EvtSemiLeptonicScalarAmp.hh"
#include <string>

#include "generators/evtgen/EvtGenModelRegister.h"
#include "generators/evtgen/models/EvtBCL.h"
#include "generators/evtgen/models/EvtBCLFF.h"


namespace Belle2 {

  B2_EVTGEN_REGISTER_MODEL(EvtBCL);

  EvtBCL::EvtBCL() : bclmodel(nullptr), calcamp(nullptr) {}

  EvtBCL::~EvtBCL()
  {
    delete bclmodel;
    bclmodel = nullptr;
    delete calcamp;
    calcamp = nullptr;
  }

  std::string EvtBCL::getName()
  {
    return "BCL";
  }

  EvtDecayBase* EvtBCL::clone()
  {
    return new EvtBCL;
  }

  void EvtBCL::decay(EvtParticle* p)
  {
    p->initializePhaseSpace(getNDaug(), getDaugs());
    calcamp->CalcAmp(p, _amp2, bclmodel);
  }


  void EvtBCL::initProbMax()
  {

    EvtId parnum, mesnum, lnum, nunum;

    parnum = getParentId();
    mesnum = getDaug(0);
    lnum = getDaug(1);
    nunum = getDaug(2);

    double mymaxprob = calcamp->CalcMaxProb(parnum, mesnum, lnum, nunum, bclmodel);

    setProbMax(mymaxprob);
  }


  void EvtBCL::init()
  {

    checkNDaug(3);

    //We expect the parent to be a scalar
    //and the daughters to be X lepton neutrino

    checkSpinParent(EvtSpinType::SCALAR);
    checkSpinDaughter(1, EvtSpinType::DIRAC);
    checkSpinDaughter(2, EvtSpinType::NEUTRINO);

    EvtSpinType::spintype mesontype = EvtPDL::getSpinType(getDaug(0));

    bclmodel = new EvtBCLFF(getNArg(), getArgs());

    if (mesontype == EvtSpinType::SCALAR) {
      calcamp = new EvtSemiLeptonicScalarAmp;
    }
    if (mesontype == EvtSpinType::VECTOR) {
      calcamp = new EvtSemiLeptonicVectorAmp;
    }
//    if (mesontype == EvtSpinType::TENSOR) {
//      calcamp = new EvtSemiLeptonicTensorAmp;
//    }

  }

}

