/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <EvtGenBase/EvtParticle.hh>
#include <EvtGenBase/EvtPDL.hh>
#include <string>

#include <generators/evtgen/EvtGenModelRegister.h>
#include <generators/evtgen/models/EvtHNLBCL.h>
#include <generators/evtgen/models/EvtBCLFF.h>
#include <generators/evtgen/models/EvtHNLSemiLeptonicScalarAmp.h>
#include <generators/evtgen/models/EvtHNLSemiLeptonicVectorAmp.h>


namespace Belle2 {

  /** register the model in EvtGen */
  B2_EVTGEN_REGISTER_MODEL(EvtHNLBCL);

  EvtHNLBCL::EvtHNLBCL() : bclmodel(nullptr), calcamp(nullptr) {}

  EvtHNLBCL::~EvtHNLBCL()
  {
    delete bclmodel;
    bclmodel = nullptr;
    delete calcamp;
    calcamp = nullptr;
  }

  std::string EvtHNLBCL::getName()
  {
    return "HNLBCL";
  }

  EvtDecayBase* EvtHNLBCL::clone()
  {
    return new EvtHNLBCL;
  }

  void EvtHNLBCL::decay(EvtParticle* p)
  {
    p->initializePhaseSpace(getNDaug(), getDaugs());
    calcamp->CalcAmp(p, _amp2, bclmodel);
  }


  void EvtHNLBCL::initProbMax()
  {

    EvtId parnum, mesnum, lnum, nunum;

    parnum = getParentId();
    mesnum = getDaug(0);
    lnum = getDaug(1);
    nunum = getDaug(2);

    double mymaxprob = calcamp->CalcMaxProb(parnum, mesnum, lnum, nunum, bclmodel);

    setProbMax(mymaxprob);
  }


  void EvtHNLBCL::init()
  {

    checkNDaug(3);

    //We expect the parent to be a scalar
    //and the daughters to be X lepton neutrino

    checkSpinParent(EvtSpinType::SCALAR);
    checkSpinDaughter(1, EvtSpinType::DIRAC);
    checkSpinDaughter(2, EvtSpinType::DIRAC);

    EvtSpinType::spintype mesontype = EvtPDL::getSpinType(getDaug(0));

    bclmodel = new EvtBCLFF(getNArg(), getArgs());

    if (mesontype == EvtSpinType::SCALAR) {
      calcamp = new EvtHNLSemiLeptonicScalarAmp;
    }
    if (mesontype == EvtSpinType::VECTOR) {
      calcamp = new EvtHNLSemiLeptonicVectorAmp;
    }
    // Tensor Meson implementation is possible here.

  }

}

