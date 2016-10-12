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
#include <hlt/softwaretrigger/core/utilities.h>
// TODO: Also cache it
#include <analysis/utility/PCmsLabTransform.h>

#include <numeric>

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
      // Prefetch some later needed objects/values
      // Simple to calculate variables
      // EC1CMSLE
      if (m_rhoParticles)
        calculationResult["rho0_dM"] = m_rhoParticles->getParticle(0)->getExtraInfo("rho0_dM");

      if (m_D0Particles)
        calculationResult["D0_dM"] = m_D0Particles->getParticle(0)->getExtraInfo("D0_dM");

      if (m_DstarParticles)
        calculationResult["Dstar_dQ"] = m_DstarParticles->getParticle(0)->getExtraInfo("Dstar_dQ");

      if (m_XiParticles)
        calculationResult["Xi_chiProb"] = m_XiParticles->getParticle(0)->getExtraInfo("Xi_chiProb");


      calculationResult["ntracks"] = m_pionParticles->getListSize();
      calculationResult["ngamma"] = m_gammaParticles->getListSize();

      // total Charge
      int totalCharge = 0;
      if (m_pionParticles->getListSize() >= 1) {
        for (unsigned int i = 0; i < m_pionParticles->getListSize(); i++) {
          totalCharge += m_pionParticles->getParticle(i)->getCharge();
        }
      }

      calculationResult["netCharge"] = totalCharge;

      double totpx = 0;
      double totpy = 0;
      if (m_pionParticles->getListSize() >= 1) {

        for (unsigned int i = 0; i < m_pionParticles->getListSize() - 1; i++) {
          totpx += m_pionParticles->getParticle(i)->getPx();
          totpy += m_pionParticles->getParticle(i)->getPy();
        }

        calculationResult["Pt_event"] = sqrt(totpx * totpx + totpy * totpy);

      }
    }
  }

}
