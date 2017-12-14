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
#include <iostream>

namespace Belle2 {
  namespace SoftwareTrigger {
    void CalibSampleCalculator::requireStoreArrays()
    {
      for (unsigned int i = 0; i < m_particlelist.size(); i++) {
        m_particleList.isRequired(m_particlelist[i]);
      }
      m_particleList.isRequired("pi+:calib");
      m_particleList.isRequired("gamma:calib");

    };

    void CalibSampleCalculator::doCalculation(SoftwareTriggerObject& calculationResult) const
    {
      for (unsigned int i = 0; i < m_particlelist.size(); i++) {
        StoreObjPtr<ParticleList> plist(m_particlelist[i]);
        int npart = plist->getListSize();
        if (npart >= 1) {
          calculationResult[m_extrainfoname[i]] = plist->getParticle(0)->getExtraInfo(m_extrainfoname[i]);
        } else {
          calculationResult[m_extrainfoname[i]] = 999;
        }
      }


      StoreObjPtr<ParticleList> pionlist("pi+:calib");
      StoreObjPtr<ParticleList> gammalist("gamma:calib");

      calculationResult["ntracks"] = pionlist->getListSize();
      calculationResult["ngamma"] = gammalist->getListSize();

      // total Charge
      int totalCharge = 0;

      // Total pt
      double totpx = 99.;
      double totpy = 99.;

      // TODO: Why only in the case of more than one particle?
      if (pionlist->getListSize() >= 1) {
        for (const Particle& pion : *pionlist) {
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
