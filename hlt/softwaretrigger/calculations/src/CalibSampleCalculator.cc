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
    };

    void CalibSampleCalculator::doCalculation(SoftwareTriggerObject& calculationResult) const
    {

      if (m_rhoParticles->getListSize() >= 1) {
        calculationResult["rho0_dM"] = m_rhoParticles->getParticle(0)->getExtraInfo("rho0_dM");
      } else {
        calculationResult["rho0_dM"] = -999;
      }

      if (m_D0Particles->getListSize() >= 1) {
        calculationResult["D0_dM"] = m_D0Particles->getParticle(0)->getExtraInfo("D0_dM");
      } else {
        calculationResult["D0_dM"] = -999;
      }

      if (m_DstarParticles->getListSize() >= 1) {
        calculationResult["Dstar_dQ"] = m_DstarParticles->getParticle(0)->getExtraInfo("Dstar_dQ");
      } else {
        calculationResult["Dstar_dQ"] = -999;
      }

      if (m_XiParticles->getListSize() >= 1) {
        calculationResult["Xi_chiProb"] = m_XiParticles->getParticle(0)->getExtraInfo("Xi_chiProb");
      } else {
        calculationResult["Xi_chiProb"] = -999;
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
