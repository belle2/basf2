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
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationsObject.h>

//ECL
#include <ecl/dataobjects/ECLCalDigit.h>

//MDST
#include <mdst/dataobjects/ECLCluster.h>

//TRG
#include <trg/ecl/dataobjects/TRGECLUnpackerStore.h>
#include <trg/ecl/TrgEclMapping.h>

//Analysis
#include <analysis/dataobjects/ECLTRGInformation.h>
#include <analysis/dataobjects/ECLTC.h>

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
  m_eclTCs.registerInDataStore();

  /** relations */
  m_eclClusters.registerRelationTo(m_eclTCs);
  m_eclTCs.registerRelationTo(m_eclCalDigits);

  /** map to have direct access to storearrays based on id and not store array position */
  m_calDigitStoreArrPosition.resize(8736 + 1);
  m_TCStoreArrPosition.resize(ECLTRGInformation::c_nTCs + 1);

  /** ecl cell ids per TC, cleanup in terminate() */
  m_trgmap = new TrgEclMapping();

}

void ECLTRGInformationModule::event()
{
  // Fill a vector that can be used to map cellid -> store array position. Dont resize!
  memset(&m_calDigitStoreArrPosition[0], -1, m_calDigitStoreArrPosition.size() * sizeof m_calDigitStoreArrPosition[0]);
  for (int i = 0; i < m_eclCalDigits.getEntries(); i++) {
    m_calDigitStoreArrPosition[m_eclCalDigits[i]->getCellId()] = i;
  }

  // Store each TC and set relations to ECLClusters
  for (const auto& trg : m_trgUnpackerStore) {

    if (trg.getTCId()<1 or trg.getTCId()>ECLTRGInformation::c_nTCs) continue;

    const auto aTC = m_eclTCs.appendNew();

    const unsigned idx = trg.getTCId();

    aTC->setTCId(idx);
    aTC->setFADC(trg.getTCEnergy());
    aTC->setTiming(trg.getTCTime());
    aTC->setEvtTiming(trg.getEVTTime());
    aTC->setRevoGDL(trg.getRevoGDL());
    aTC->setRevoFAM(trg.getRevoFAM());
    aTC->setThetaId(m_trgmap->getTCThetaIdFromTCId(idx));
    aTC->setPhiId(m_trgmap->getTCPhiIdFromTCId(idx));

    // Fill ECLCalDigit energy sum
    float energySum = 0.;
    for (const auto& id : m_trgmap->getXtalIdFromTCId(idx)) {
      // m_trgmap returns fixed size vectors with '0' to indicate empty positions -> 1-based
      if (id > 0) {
        const int pos = m_calDigitStoreArrPosition[id]; // id is 1-based, m_calDigitStoreArrPosition is 1-based
        if (pos >= 0) { //not existing digits would return -1
          energySum += m_eclCalDigits[pos]->getEnergy();

          aTC->addRelationTo(m_eclCalDigits[pos]);
        }
      }
    }

    aTC->setECLCalDigitEnergy(energySum);
  }

  // Fill a vector that can be used to map tcid -> store array position. Dont resize!
  memset(&m_TCStoreArrPosition[0], -1, m_TCStoreArrPosition.size() * sizeof m_TCStoreArrPosition[0]);
  for (int i = 0; i < m_eclTCs.getEntries(); i++) {
    m_TCStoreArrPosition[m_eclTCs[i]->getTCId()] = i;
    if (m_eclTCs[i]->getTCId() == 0) {
      B2ERROR(m_eclTCs[i]->getTCId() << " not possible");
    }
  }

  // need relation for ECLClusters
  for (const auto& cluster : m_eclClusters) {

    // only photon clusters
    if (cluster.getHypothesisId() != ECLCluster::c_nPhotons) continue;

    // map TCId, energy
    tcmap TCMap;

    // loop over all digits of this cluster
    auto relationsCalDigits = cluster.getRelationsTo<ECLCalDigit>();
    for (unsigned int idx = 0; idx < relationsCalDigits.size(); ++idx) {
      const auto cal = relationsCalDigits.object(idx);
      const auto weight = relationsCalDigits.weight(idx);

      auto relationsTCs = cal->getRelationsWith<ECLTC>();
      for (unsigned int idxTC = 0; idxTC < relationsTCs.size(); ++idxTC) {
        const auto tc = relationsTCs.object(idxTC);

        const unsigned tcid = tc->getTCId();
        TCMap[tcid] += weight * cal->getEnergy();
      }
    }

    // set weighted relations between cluster and TC, where the weight is the energy of ECLCalDigits that belong to this shower in this TC
    for (const auto& mapentry : TCMap) {
      const int pos = m_TCStoreArrPosition[mapentry.first];
      cluster.addRelationTo(m_eclTCs[pos], mapentry.second);
    }
  }

  //energy sum
  float sumEnergyTCECLCalDigitInECLCluster = 0.0;
  float sumEnergyECLCalDigitInECLCluster = 0.0;

  // create the dataobject
  if (!m_eclTRGInformation) {
    m_eclTRGInformation.create();
  }

  // get the actual TC information (energy, timing, ...)
  float eventtiming = std::numeric_limits<float>::quiet_NaN();
  for (const auto& trg : m_trgUnpackerStore) {
    const int tcid = trg.getTCId();
    if (tcid<1 or tcid>ECLTRGInformation::c_nTCs) continue;

    m_eclTRGInformation->setEnergyTC(tcid, trg.getTCEnergy());
    m_eclTRGInformation->setTimingTC(tcid, trg.getTCTime());
    m_eclTRGInformation->setRevoGDLTC(tcid, trg.getRevoGDL());
    m_eclTRGInformation->setRevoFAMTC(tcid, trg.getRevoFAM());

    if (trg.getTCEnergy() > 0 and std::isnan(eventtiming)) {
      eventtiming = trg.getEVTTime();
    }

    B2DEBUG(29, "TC Id: (1.." << ECLTRGInformation::c_nTCs << ") " << trg.getTCId() << " FADC="  << trg.getTCEnergy() << ",  t=" <<
            trg.getTCTime() << ", tevent=" << eventtiming);
  }

  // loop over all possible TCs and fill the 'offline' ECLCalDigit information
  for (unsigned idx = 1; idx <= ECLTRGInformation::c_nTCs; idx++) {
    float energyInTC = 0.;
    float energyInTCInECLCluster = 0.;
    float highestEnergy = -1.;
    float time = std::numeric_limits<float>::quiet_NaN();

    for (const auto& id : m_trgmap->getXtalIdFromTCId(idx)) {
      // the mapping returns fixed size vectors with '0' to indicate empty positions
      if (id > 0) {
        const int pos = m_calDigitStoreArrPosition[id - 1];
        if (pos >= 0) {
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
  m_eclTRGInformation->setEvtTiming(eventtiming);

}

void ECLTRGInformationModule::terminate()
{
  if (m_trgmap) delete m_trgmap;
}
