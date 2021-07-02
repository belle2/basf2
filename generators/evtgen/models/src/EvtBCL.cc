/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtSemiLeptonicVectorAmp.hh"
#include "EvtGenBase/EvtSemiLeptonicScalarAmp.hh"
#include <string>

#include "generators/evtgen/EvtGenModelRegister.h"
#include "generators/evtgen/models/EvtBCL.h"
#include "generators/evtgen/models/EvtBCLFF.h"


namespace Belle2 {

  /** register the model in EvtGen */
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
    // Tensor Meson implementation is possible here.

  }

}

