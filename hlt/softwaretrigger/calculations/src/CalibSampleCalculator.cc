/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua Li                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <hlt/softwaretrigger/calculations/CalibSampleCalculator.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    void CalibSampleCalculator::requireStoreArrays()
    {
      m_pionParticles.isRequired();
      m_gammaParticles.isRequired();
      m_rhoParticles.isRequired();
      m_D0Particles.isRequired();
      m_DstarParticles.isRequired();
      m_XiParticles.isRequired();
      m_dqmD0.isRequired();
      m_dqmDplus.isRequired();
      m_dqmDstar.isRequired();
      m_dqmJpsiee.isRequired();
      m_dqmJpsimumu.isRequired();
    };

    void CalibSampleCalculator::doCalculation(SoftwareTriggerObject& calculationResult) const
    {

      if (m_rhoParticles->getListSize() >= 1) {
        calculationResult["rho0_dM"] = m_rhoParticles->getParticle(0)->getExtraInfo("rho0_dM");
      } else {
        calculationResult["rho0_dM"] = 999;
      }

      if (m_D0Particles->getListSize() >= 1) {
        calculationResult["D0_dM"] = m_D0Particles->getParticle(0)->getExtraInfo("D0_dM");
      } else {
        calculationResult["D0_dM"] = 999;
      }

      if (m_DstarParticles->getListSize() >= 1) {
        calculationResult["Dstar_dQ"] = m_DstarParticles->getParticle(0)->getExtraInfo("Dstar_dQ");
      } else {
        calculationResult["Dstar_dQ"] = 999;
      }

      if (m_XiParticles->getListSize() >= 1) {
        calculationResult["Xi_chiProb"] = m_XiParticles->getParticle(0)->getExtraInfo("Xi_chiProb");
      } else {
        calculationResult["Xi_chiProb"] = -999;
      }

      if (m_dqmD0->getListSize() >= 1) {
        calculationResult["dqm_D0_M"] = m_dqmD0->getParticle(0)->getExtraInfo("D0_dqm_M");
      } else {
        calculationResult["dqm_D0_M"] = -999;
      }

      if (m_dqmDplus->getListSize() >= 1) {
        calculationResult["dqm_Dplus_M"] = m_dqmDplus->getParticle(0)->getExtraInfo("Dplus_dqm_M");
      } else {
        calculationResult["dqm_Dplus_M"] = -999;
      }

      if (m_dqmDstar->getListSize() >= 1) {
        calculationResult["dqm_Dstar_M"] = m_dqmDstar->getParticle(0)->getExtraInfo("Dstar_dqm_M");
      } else {
        calculationResult["dqm_Dstar_M"] = -999;
      }


      if (m_dqmJpsiee->getListSize() >= 1) {
        calculationResult["dqm_Jpsiee_M"] = m_dqmJpsiee->getParticle(0)->getExtraInfo("Jpsi_dqm_ee_M");
      } else {
        calculationResult["dqm_Jpsiee_M"] = -999;
      }

      if (m_dqmJpsimumu->getListSize() >= 1) {
        calculationResult["dqm_Jpsimumu_M"] = m_dqmJpsimumu->getParticle(0)->getExtraInfo("Jpsi_dqm_mumu_M");
      } else {
        calculationResult["dqm_Jpsimumu_M"] = -999;
      }

      calculationResult["ntracks"] = m_pionParticles->getListSize();
      calculationResult["ngamma"] = m_gammaParticles->getListSize();

      // total Charge
      int totalCharge = 0;

      // Total pt
      double totpx = 99.;
      double totpy = 99.;

      // TODO: Why only in the case of more than one particle?
      if (m_pionParticles->getListSize() >= 1) {
        for (const Particle& pion : *m_pionParticles) {
          totalCharge += pion.getCharge();

          totpx += pion.getPx();
          totpy += pion.getPy();
        }
      }
      calculationResult["netCharge"] = totalCharge;
      calculationResult["Pt_event"] = sqrt(totpx * totpx + totpy * totpy);
    }
  }

}
