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

#include "generators/evtgen/EvtGenModelRegister.h"
#include "generators/evtgen/models/EvtBSemiTauonic2HDMType2.h"
#include "generators/evtgen/models/EvtBSemiTauonicHelicityAmplitudeCalculator.h"
#include "generators/evtgen/models/EvtBSemiTauonicVectorMesonAmplitude.h"
#include "generators/evtgen/models/EvtBSemiTauonicScalarMesonAmplitude.h"

namespace Belle2 {

  B2_EVTGEN_REGISTER_MODEL(EvtBSemiTauonic2HDMType2);

  EvtBSemiTauonic2HDMType2::EvtBSemiTauonic2HDMType2(): m_CalcHelAmp(0), m_CalcAmp(0) {}

  EvtBSemiTauonic2HDMType2::~EvtBSemiTauonic2HDMType2()
  {
    if (m_CalcHelAmp)delete m_CalcHelAmp;
    if (m_CalcAmp)delete m_CalcAmp;
  }

  std::string EvtBSemiTauonic2HDMType2::getName()
  {
    return "BSTD_2HDMTYPE2";
  }



  EvtDecayBase* EvtBSemiTauonic2HDMType2::clone()
  {
    return new EvtBSemiTauonic2HDMType2;
  }


  void EvtBSemiTauonic2HDMType2::decay(EvtParticle* p)
  {
    p->initializePhaseSpace(getNDaug(), getDaugs());
    m_CalcAmp->CalcAmp(p, _amp2, m_CalcHelAmp);
  }

  void EvtBSemiTauonic2HDMType2::initProbMax()
  {
    EvtId parnum, mesnum, lnum, nunum;

    parnum = getParentId();
    mesnum = getDaug(0);
    lnum = getDaug(1);
    nunum = getDaug(2);

    double maxprob = m_CalcAmp->CalcMaxProb(parnum, mesnum,
                                            lnum, nunum,
                                            m_CalcHelAmp);

    B2INFO("EvtBSemiTauonic2HDMType2::initProbMax()>> maxprob: " << maxprob);

    setProbMax(maxprob);
  }


  void EvtBSemiTauonic2HDMType2::init()
  {
    checkNDaug(3);

    //We expect the parent to be a scalar
    //and the daughters to be X lepton neutrino
    checkSpinParent(EvtSpinType::SCALAR);

    checkSpinDaughter(1, EvtSpinType::DIRAC);
    checkSpinDaughter(2, EvtSpinType::NEUTRINO);

    const double m_tau = EvtPDL::getMeanMass(getDaug(1));

    EvtSpinType::spintype d1type = EvtPDL::getSpinType(getDaug(0));

    if (d1type == EvtSpinType::VECTOR) {
      B2INFO("EvtBSemiTauonic2HDMType2::init()>> Initializing for decays to a vector type meson ");
      checkNArg(7, 8);

      const double rhoa12 = getArg(0);
      const double R11 = getArg(1);
      const double R21 = getArg(2);
      const double aR3 = getArg(3);

      const double rho12 = 0; // not used for D*taunu decays
      const double aS1 = 0;   // not used for D*taunu decays

      EvtComplex Coeffs[5];

      const double m_b = getArg(4);
      const double m_c = getArg(5);
      const double tanBetaOverMH = getArg(6);
      B2INFO("tan(beta)/m_H+ = " << tanBetaOverMH);

      Coeffs[0] = 0; // CV1
      Coeffs[1] = 0; // CV2
      Coeffs[2] = -m_b * m_tau * tanBetaOverMH * tanBetaOverMH; // CS1
      Coeffs[3] = 0; // CS2, neglected by default
      Coeffs[4] = 0; // CT

      if (getNArg() == 8) {
        // if m_H is explicitly given
        const double m_H = getArg(7);
        Coeffs[3] = -m_c * m_tau / m_H / m_H; // CS2
      }

      m_CalcHelAmp = new EvtBSemiTauonicHelicityAmplitudeCalculator(rho12, rhoa12, R11, R21, aS1, aR3, m_b, m_c,
          Coeffs[0], Coeffs[1], Coeffs[2], Coeffs[3], Coeffs[4],
          EvtPDL::getMeanMass(getParentId()),
          -1, /*dummy for Dmass*/
          EvtPDL::getMeanMass(getDaug(0)));
      m_CalcAmp = new EvtBSemiTauonicVectorMesonAmplitude();
    } else if (d1type == EvtSpinType::SCALAR) {
      B2INFO("EvtBSemiTauonic2HDMType2::init()>> Initializing for decays to a scalar type meson ");
      checkNArg(5, 6);

      const double rho12 = getArg(0);
      const double aS1 = getArg(1);

      const double rhoa12 = 0; // not used for Dtaunu decays
      const double R11 = 0; // not used for Dtaunu decays
      const double R21 = 0; // not used for Dtaunu decays
      const double aR3 = 0; // not used for Dtaunu decays

      EvtComplex Coeffs[5];

      const double m_b = getArg(2);
      const double m_c = getArg(3);
      const double tanBetaOverMH = getArg(4);
      B2INFO("tan(beta)/m_H+ = " << tanBetaOverMH);

      Coeffs[0] = 0; // CV1
      Coeffs[1] = 0; // CV2
      Coeffs[2] = -m_b * m_tau * tanBetaOverMH * tanBetaOverMH; // CS1
      Coeffs[3] = 0; // CS2, neglected by default
      Coeffs[4] = 0; // CT

      if (getNArg() == 6) {
        // if m_H is explicitly given
        const double m_H = getArg(5);
        Coeffs[3] = -m_c * m_tau / m_H / m_H; // CS2
      }

      m_CalcHelAmp = new EvtBSemiTauonicHelicityAmplitudeCalculator(rho12, rhoa12, R11, R21, aS1, aR3, m_b, m_c,
          Coeffs[0], Coeffs[1], Coeffs[2], Coeffs[3], Coeffs[4],
          EvtPDL::getMeanMass(getParentId()),
          EvtPDL::getMeanMass(getDaug(0)),
          -1 /*dummy for D*mass*/);
      m_CalcAmp = new EvtBSemiTauonicScalarMesonAmplitude();
    } else {
      B2ERROR("BSemiTauonic2HDMType2 model handles only scalar and vector meson daughters.");
//    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "BSemiTauonic2HDMType2 model handles only scalar and vector meson daughters."<<std::endl;
      ::abort();
    }
  }
} // Belle 2 Namespace
