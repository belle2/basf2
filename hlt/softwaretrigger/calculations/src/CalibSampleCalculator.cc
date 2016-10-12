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

      double rhodM = 999.;
      if (m_rhoParticles->getListSize() >= 1)
        rhodM = m_rhoParticles->getParticle(0)->getExtraInfo("rho0_dM");
      calculationResult["rho0_dM"] = rhodM;

      double D0dM = 999.;
      if (m_D0Particles->getListSize() >= 1)
        D0dM = m_D0Particles->getParticle(0)->getExtraInfo("D0_dM");

      calculationResult["D0_dM"] = D0dM;


      double DstardQ = 999.;
      if (m_DstarParticles->getListSize() >= 1)
        DstardQ = m_DstarParticles->getParticle(0)->getExtraInfo("Dstar_dQ");
      calculationResult["Dstar_dQ"] = DstardQ;

      double Xichi = -999.;
      if (m_XiParticles->getListSize() >= 1)
        Xichi = m_XiParticles->getParticle(0)->getExtraInfo("Xi_chiProb");
      calculationResult["Xi_chiProb"] = Xichi;


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

      double totpx = 99.;
      double totpy = 99.;
      if (m_pionParticles->getListSize() >= 1) {

        for (unsigned int i = 0; i < m_pionParticles->getListSize(); i++) {
          totpx += m_pionParticles->getParticle(i)->getPx();
          totpy += m_pionParticles->getParticle(i)->getPy();
        }

        calculationResult["Pt_event"] = sqrt(totpx * totpx + totpy * totpy);

      }
    }
  }

}
