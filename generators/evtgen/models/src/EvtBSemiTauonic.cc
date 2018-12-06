/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Koji Hara                                                *
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


#include <string>

#include "framework/logging/Logger.h"
#include <generators/evtgen/EvtGenModelRegister.h>
#include "generators/evtgen/models/EvtBSemiTauonic.h"
#include "generators/evtgen/models/EvtBSemiTauonicHelicityAmplitudeCalculator.h"
#include "generators/evtgen/models/EvtBSemiTauonicVectorMesonAmplitude.h"
#include "generators/evtgen/models/EvtBSemiTauonicScalarMesonAmplitude.h"

namespace Belle2 {


  B2_EVTGEN_REGISTER_MODEL(EvtBSemiTauonic);

  EvtBSemiTauonic::EvtBSemiTauonic(): m_CalcHelAmp(0), m_CalcAmp(0) {}

  EvtBSemiTauonic::~EvtBSemiTauonic()
  {
    if (m_CalcHelAmp)delete m_CalcHelAmp;
    if (m_CalcAmp)delete m_CalcAmp;
  }

  std::string EvtBSemiTauonic::getName()
  {
    return "BSTD";
  }



  EvtDecayBase* EvtBSemiTauonic::clone()
  {
    return new EvtBSemiTauonic;
  }


  void EvtBSemiTauonic::decay(EvtParticle* p)
  {
    p->initializePhaseSpace(getNDaug(), getDaugs());
    m_CalcAmp->CalcAmp(p, _amp2, m_CalcHelAmp);
  }

  void EvtBSemiTauonic::initProbMax()
  {
    EvtId parnum, mesnum, lnum, nunum;

    parnum = getParentId();
    mesnum = getDaug(0);
    lnum = getDaug(1);
    nunum = getDaug(2);

    double maxprob = m_CalcAmp->CalcMaxProb(parnum, mesnum,
                                            lnum, nunum,
                                            m_CalcHelAmp);

    B2INFO("EvtBSemiTauonic::initProbMax()>> maxprob: " << maxprob);

    setProbMax(maxprob);
  }


  void EvtBSemiTauonic::init()
  {
    checkNDaug(3);

    //We expect the parent to be a scalar
    //and the daughters to be X lepton neutrino
    checkSpinParent(EvtSpinType::SCALAR);

    checkSpinDaughter(1, EvtSpinType::DIRAC);
    checkSpinDaughter(2, EvtSpinType::NEUTRINO);

    EvtSpinType::spintype d1type = EvtPDL::getSpinType(getDaug(0));

    if (d1type == EvtSpinType::VECTOR) {
      B2INFO("EvtBSemiTauonic::init()>> Initializing for decays to a vector type meson ");
      checkNArg(16);
      const double rhoa12 = getArg(0);
      const double R11 = getArg(1);
      const double R21 = getArg(2);
      const double aR3 = getArg(3);

      const double rho12 = 0; // not used for D*taunu decays
      const double aS1 = 0; // not used for D*taunu decays

      const double m_b = getArg(4);
      const double m_c = getArg(5);

      EvtComplex Coeffs[5];
      Coeffs[0] = EvtComplex(getArg(6) * cos(getArg(7)),   getArg(6) * sin(getArg(7)));
      Coeffs[1] = EvtComplex(getArg(8) * cos(getArg(9)),   getArg(8) * sin(getArg(9)));
      Coeffs[2] = EvtComplex(getArg(10) * cos(getArg(11)),   getArg(10) * sin(getArg(11)));
      Coeffs[3] = EvtComplex(getArg(12) * cos(getArg(13)), getArg(12) * sin(getArg(13)));
      Coeffs[4] = EvtComplex(getArg(14) * cos(getArg(15)), getArg(14) * sin(getArg(15)));

      m_CalcHelAmp = new EvtBSemiTauonicHelicityAmplitudeCalculator(rho12, rhoa12, R11, R21, aS1, aR3, m_b, m_c,
          Coeffs[0], Coeffs[1], Coeffs[2], Coeffs[3], Coeffs[4],
          EvtPDL::getMeanMass(getParentId()),
          -1, /*dummy for Dmass*/
          EvtPDL::getMeanMass(getDaug(0)));
      m_CalcAmp = new EvtBSemiTauonicVectorMesonAmplitude();
    } else if (d1type == EvtSpinType::SCALAR) {
      B2INFO("EvtBSemiTauonic::init()>> Initializing for decays to a scalar type meson ");
      checkNArg(14);
      const double rho12 = getArg(0);
      const double aS1 = getArg(1);

      const double rhoa12 = 0; // not used for Dtaunu decays
      const double R11 = 0; // not used for Dtaunu decays
      const double R21 = 0; // not used for Dtaunu decays
      const double aR3 = 0; // not used for Dtaunu decays

      const double m_b = getArg(2);
      const double m_c = getArg(3);

      EvtComplex Coeffs[5];
      Coeffs[0] = EvtComplex(getArg(4) * cos(getArg(5)),   getArg(4) * sin(getArg(5)));
      Coeffs[1] = EvtComplex(getArg(6) * cos(getArg(7)),   getArg(6) * sin(getArg(7)));
      Coeffs[2] = EvtComplex(getArg(8) * cos(getArg(9)),   getArg(8) * sin(getArg(9)));
      Coeffs[3] = EvtComplex(getArg(10) * cos(getArg(11)), getArg(10) * sin(getArg(11)));
      Coeffs[4] = EvtComplex(getArg(12) * cos(getArg(13)), getArg(12) * sin(getArg(13)));

      m_CalcHelAmp = new EvtBSemiTauonicHelicityAmplitudeCalculator(rho12, rhoa12, R11, R21, aS1, aR3, m_b, m_c,
          Coeffs[0], Coeffs[1], Coeffs[2], Coeffs[3], Coeffs[4],
          EvtPDL::getMeanMass(getParentId()),
          EvtPDL::getMeanMass(getDaug(0)),
          -1 /*dummy for D*mass*/);
      m_CalcAmp = new EvtBSemiTauonicScalarMesonAmplitude();
    } else {
      B2ERROR("BSemiTauonic model handles only scalar and vector meson daughters.");
//    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "BSemiTauonic model handles only scalar and vector meson daughters." << std::endl;
      ::abort();
    }
  }
} // Belle 2 Namespace
