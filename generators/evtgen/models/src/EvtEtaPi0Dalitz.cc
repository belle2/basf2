//--------------------------------------------------------------------------
//
// Environment:
//      This software is part of the EvtGen package for the BelleII
//      collaboration. If you use all or part of it, please give an
//      appropriate acknowledgement.
//
// Copyright Information: See EvtGen/COPYRIGHT
//      Copyright (C) 1998      Caltech, UCSB
//
// Module: EvtEtaPi0Dalitz.cc
//
// Description: Routine to decay eta/eta' -> pi0 pi0 pi0
//
// Developer: Umberto Tamponi (tamponi@to.infn.it)
//
// Modification history:
//
//    U.Tamponi    October 6th 2016   Module created
//
//------------------------------------------------------------------------
//
#include <generators/evtgen/EvtGenModelRegister.h>
#include "EvtGenBase/EvtPatches.hh"
#include <stdlib.h>
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtGenKine.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "generators/evtgen/models/EvtEtaPi0Dalitz.h"
#include "EvtGenBase/EvtReport.hh"
#include <string>

namespace Belle2 {

  B2_EVTGEN_REGISTER_MODEL(EvtEtaPi0Dalitz);


  EvtEtaPi0Dalitz::~EvtEtaPi0Dalitz() {}

  std::string EvtEtaPi0Dalitz::getName()
  {

    return "ETA_PI0DALITZ";

  }


  EvtDecayBase* EvtEtaPi0Dalitz::clone()
  {

    return new EvtEtaPi0Dalitz;

  }

  void EvtEtaPi0Dalitz::init()
  {

    // check that there is 1 argument
    checkNArg(1);
    checkNDaug(3);


    checkSpinParent(EvtSpinType::SCALAR);

    checkSpinDaughter(0, EvtSpinType::SCALAR);
    checkSpinDaughter(1, EvtSpinType::SCALAR);
    checkSpinDaughter(2, EvtSpinType::SCALAR);
  }


  void EvtEtaPi0Dalitz::initProbMax()
  {

    setProbMax(2.5);

  }

  void EvtEtaPi0Dalitz::decay(EvtParticle* p)
  {

    const double alpha = getArg(0);

    p->initializePhaseSpace(getNDaug(), getDaugs());

    EvtVector4R mompi0_0 = p->getDaug(0)->getP4();
    EvtVector4R mompi0_1 = p->getDaug(1)->getP4();
    EvtVector4R mompi0_2 = p->getDaug(2)->getP4();

    double m_eta = p->mass();
    double m_pi0 = p->getDaug(0)->mass();
    double deltaM = m_eta - 3 * m_pi0;

    //The decay amplitude comes from KLOE collab., Phys.Lett. B694 (2011) 16-21
    double z0 = (3 * mompi0_0.get(0) - m_eta) / deltaM;
    double z1 = (3 * mompi0_1.get(0) - m_eta) / deltaM;
    double z2 = (3 * mompi0_2.get(0) - m_eta) / deltaM;

    double z = (2. / 3.) * (z0 * z0 + z1 * z1 + z2 * z2) ;

    double Amp2 = 1.0 + alpha * 2.0 * z;
    if (Amp2 < 0)
      Amp2 = 0;

    EvtComplex amp(sqrt(Amp2), 0.);

    vertex(amp);

    return ;

  }

} // Belle 2 Namespace
