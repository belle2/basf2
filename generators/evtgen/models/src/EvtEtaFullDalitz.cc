/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * Description: Routine to decay eta/eta' -> pi+ pi- pi0                  *
 *              with a Dalitz parametrization up to the                   *
 *              cubic terms in Y                                          *
 *                                                                        *
 * Modification history:                                                  *
 *   -  U.Tamponi    October 9th 2016   Module created                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/evtgen/EvtGenModelRegister.h>
#include "EvtGenBase/EvtPatches.hh"
#include <stdlib.h>
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtGenKine.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "generators/evtgen/models/EvtEtaFullDalitz.h"
#include "EvtGenBase/EvtReport.hh"
#include <string>

namespace Belle2 {

  B2_EVTGEN_REGISTER_MODEL(EvtEtaFullDalitz);


  EvtEtaFullDalitz::~EvtEtaFullDalitz() {}

  std::string EvtEtaFullDalitz::getName()
  {

    return "ETA_FULLDALITZ";

  }


  EvtDecayBase* EvtEtaFullDalitz::clone()
  {

    return new EvtEtaFullDalitz;

  }

  void EvtEtaFullDalitz::init()
  {

    // check that there is are arguments
    checkNArg(6);
    checkNDaug(3);


    checkSpinParent(EvtSpinType::SCALAR);

    checkSpinDaughter(0, EvtSpinType::SCALAR);
    checkSpinDaughter(1, EvtSpinType::SCALAR);
    checkSpinDaughter(2, EvtSpinType::SCALAR);
  }


  void EvtEtaFullDalitz::initProbMax()
  {

    setProbMax(2.5);

  }

  void EvtEtaFullDalitz::decay(EvtParticle* p)
  {

    const double a = getArg(0);
    const double b = getArg(1);
    const double c = getArg(2);
    const double d = getArg(3);
    const double e = getArg(4);
    const double f = getArg(5);

    p->initializePhaseSpace(getNDaug(), getDaugs());

    EvtVector4R mompip = p->getDaug(0)->getP4();
    EvtVector4R mompim = p->getDaug(1)->getP4();
    EvtVector4R mompi0 = p->getDaug(2)->getP4();

    double m_eta = p->mass();
    double m_pip = p->getDaug(0)->mass();
    double m_pi0 = p->getDaug(2)->mass();

    //Q value
    double deltaM = m_eta - 2 * m_pip - m_pi0;

    //The decay amplitude comes from BESIII collab, Phys.Rev. D92 (2015) 012014

    //Kinetic energies T
    double Tpip = (mompip.get(0) - m_pip);
    double Tpim = (mompim.get(0) - m_pip);
    double Tpi0 = (mompi0.get(0) - m_pip);

    //Dalitz variables
    double X = sqrt(3.) * (Tpip - Tpim) / deltaM;
    double Y = 3.*Tpi0 / deltaM - 1. ;

    double amp2 = 1. + a * Y + b * Y * Y + c * X + d * X * X + e * X * Y + f * Y * Y * Y;

    EvtComplex amp(sqrt(amp2), 0.0);

    vertex(amp);

    return ;

  }

} // Belle 2 Namespace
