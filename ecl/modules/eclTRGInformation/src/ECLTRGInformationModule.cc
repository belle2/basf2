/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (torben.ferber@desy.de)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
//This module
#include <ecl/modules/eclTRGInformation/ECLTRGInformationModule.h>

//Framework
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

//ECL
#include <ecl/dataobjects/ECLCalDigit.h>

//MDST
#include <mdst/dataobjects/ECLCluster.h>

//TRG
#include <trg/ecl/dataobjects/TRGECLUnpackerStore.h>
#include <trg/ecl/TrgEclMapping.h>

//Analysis
#include <analysis/dataobjects/ECLTRGInformation.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLTRGInformation)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLTRGInformationModule::ECLTRGInformationModule() : Module()
{
  // Set module properties
  setDescription("Get ECL TRG energy information");
  addParam("clusterEnergyThreshold", m_clusterEnergyThreshold, "Include ECLClusters above this energy in the energy sum.",
           100.0 * Belle2::Unit::MeV);

  setPropertyFlags(c_ParallelProcessingCertified);
}

void ECLTRGInformationModule::initialize()
{

  /** Required dataobjects */
  m_eclCalDigits.isRequired();
  m_eclClusters.isRequired();
  m_trgUnpackerStore.isRequired();

  /** output dataobjects */
  m_eclTRGInformation.registerInDataStore();

  /** map to have direct access to ecl caldigits based on cellid */
  m_calDigitStoreArrPosition.resize(8737);

  /** ecl cell ids per TC */
  m_trgmap = new TrgEclMapping();

}

void ECLTRGInformationModule::event()
{
  // Fill a vector that can be used to map cellid -> store array position
  memset(&m_calDigitStoreArrPosition[0], -1, m_calDigitStoreArrPosition.size() * sizeof m_calDigitStoreArrPosition[0]);
  for (int i = 0; i < m_eclCalDigits.getEntries(); i++) {
    m_calDigitStoreArrPosition[m_eclCalDigits[i]->getCellId()] = i;
  }

  //energy sum
  float sumEnergyTCECLCalDigitInECLCluster = 0.0;
  float sumEnergyECLCalDigitInECLCluster = 0.0;

  // create the dataobject
  if (!m_eclTRGInformation) {
    m_eclTRGInformation.create();
  }

  // get the actual TC information (energy, timing, ...)
  for (const auto& trg : m_trgUnpackerStore) {
    m_eclTRGInformation->setEnergyTC(trg.getTCId(), trg.getTCEnergy());
    m_eclTRGInformation->setTimingTC(trg.getTCId(), trg.getTCTime());
    m_eclTRGInformation->setRevoGDLTC(trg.getTCId(), trg.getRevoGDL());
    m_eclTRGInformation->setRevoFAMTC(trg.getTCId(), trg.getRevoFAM());
    B2DEBUG(29, "TC Id: (1.." << ECLTRGInformation::c_nTCs << ") " << trg.getTCId() << " FADC="  << trg.getTCEnergy() << ",  t=" <<
            trg.getTCTime());
  }

  // loop over all possible TCs and fill the 'offline' ECLCalDigit information
  for (unsigned idx = 1; idx <= ECLTRGInformation::c_nTCs; idx++) {
    float energyInTC = 0.;
    float energyInTCInECLCluster = 0.;
    float energyECLCluster = 0.;
    float highestEnergy = -1.;
    float time = std::numeric_limits<float>::quiet_NaN();

    for (const auto& id : m_trgmap->getXtalIdFromTCId(idx)) {
      // the mapping returns fixed size vectors with '0' to indicate empty positions
      if (id > 0) {
        const int pos = m_calDigitStoreArrPosition[id - 1];
        if (pos > 0) {
          energyInTC += m_eclCalDigits[pos]->getEnergy();

          // check if that eclcaldigit is part of an ECLCluster above threshold
          auto rel = m_eclCalDigits[pos]->getRelationsFrom<ECLCluster>();
          for (unsigned int irel = 0; irel < rel.size(); ++irel) {

            const auto cluster = rel.object(irel);

            if (cluster->getHypothesisId() != ECLCluster::c_nPhotons) continue;

            const auto weight = rel.weight(irel);
            float clusterenergy = cluster->getEnergy();

            B2DEBUG(28, irel << " " << clusterenergy << " " << m_clusterEnergyThreshold);

            if (clusterenergy >= m_clusterEnergyThreshold) {
              energyInTCInECLCluster += weight * m_eclCalDigits[pos]->getEnergy();
            }
          }

          // get the highest energy caldigit for the approximate timing
          if (m_eclCalDigits[pos]->getEnergy() > highestEnergy) {
            highestEnergy = m_eclCalDigits[pos]->getEnergy();
            time = m_eclCalDigits[pos]->getTime();
          }
        }
      }
    }

    // only add this to the total sum if the TC is read out and within trigger acceptance
    if (m_eclTRGInformation->getEnergyTC(idx)) {
      B2DEBUG(28, energyInTCInECLCluster << " " << m_trgmap->getTCThetaIdFromTCId(idx));
    }

    if (m_trgmap->getTCThetaIdFromTCId(idx) >= 2 and
        m_trgmap->getTCThetaIdFromTCId(idx) <= 15) {
      if (m_eclTRGInformation->getEnergyTC(idx)) sumEnergyTCECLCalDigitInECLCluster += energyInTCInECLCluster;
      sumEnergyECLCalDigitInECLCluster += energyInTCInECLCluster;
    }


    m_eclTRGInformation->setEnergyTCECLCalDigit(idx, energyInTC);
    m_eclTRGInformation->setTimingTCECLCalDigit(idx, time);

    m_eclTRGInformation->setThetaIdTC(idx, m_trgmap->getTCThetaIdFromTCId(idx));
    m_eclTRGInformation->setPhiIdTC(idx, m_trgmap->getTCPhiIdFromTCId(idx));

    if (energyInTC > 0) B2DEBUG(29, "ECLCalDigits TC Id:  (1.." << ECLTRGInformation::c_nTCs << ") " << idx << " E="  << energyInTC <<
                                  ", t=" << time);

  }

  m_eclTRGInformation->setClusterEnergyThreshold(m_clusterEnergyThreshold);
  m_eclTRGInformation->setSumEnergyTCECLCalDigitInECLCluster(sumEnergyTCECLCalDigitInECLCluster);
  m_eclTRGInformation->setSumEnergyECLCalDigitInECLCluster(sumEnergyECLCalDigitInECLCluster);

}

void ECLTRGInformationModule::terminate()
{
  if (m_trgmap) delete m_trgmap;
}

