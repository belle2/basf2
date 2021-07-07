/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
#include <trg/ecl/dataobjects/TRGECLUnpackerEvtStore.h>

#include <trg/ecl/TrgEclMapping.h>

//Analysis
#include <analysis/dataobjects/ECLTRGInformation.h>
#include <analysis/dataobjects/ECLTriggerCell.h>

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
  m_trgUnpackerEvtStore.isRequired();

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

  int EventTiming = 0;
  int RevoGDL = 0; // L1 Revo clk  (Same value in an evnet)
  int RevoFAM = 0; // FAM Revo clk (Same value in an event)
  for (const auto& trgEvt : m_trgUnpackerEvtStore) {
    EventTiming = trgEvt.getEvtTime();
    RevoGDL = trgEvt.getL1Revo();
    RevoFAM = trgEvt.getEvtRevo();
  }

  // Store each TC and set relations to ECLClusters
  for (const auto& trg : m_trgUnpackerStore) {

    if (trg.getTCId()<1 or trg.getTCId()>ECLTRGInformation::c_nTCs) continue;

    const auto aTC = m_eclTCs.appendNew();

    const unsigned idx = trg.getTCId();

    aTC->setTCId(idx);
    aTC->setFADC(trg.getTCEnergy());
    aTC->setTiming(trg.getTCTime());
    aTC->setEvtTiming(EventTiming);
    aTC->setRevoGDL(RevoGDL);
    aTC->setRevoFAM(RevoFAM);
    aTC->setThetaId(m_trgmap->getTCThetaIdFromTCId(idx));
    aTC->setPhiId(m_trgmap->getTCPhiIdFromTCId(idx));
    aTC->setHitWin(trg.getHitWin());

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
    if (!cluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) continue;

    // map TCId, energy
    tcmap TCMap;

    // loop over all digits of this cluster
    auto relationsCalDigits = cluster.getRelationsTo<ECLCalDigit>();
    for (unsigned int idx = 0; idx < relationsCalDigits.size(); ++idx) {
      const auto cal = relationsCalDigits.object(idx);
      const auto weight = relationsCalDigits.weight(idx);

      auto relationsTCs = cal->getRelationsWith<ECLTriggerCell>();
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
  int maxtcid = -1;
  float maxtc = -1;

  for (const auto& trg : m_trgUnpackerStore) {
    const int tcid = trg.getTCId();
    if (tcid<1 or tcid>ECLTRGInformation::c_nTCs) continue;

    if (trg.getTCEnergy() > maxtc) {
      maxtc = trg.getTCEnergy();
      maxtcid = tcid;
    }

    m_eclTRGInformation->setEnergyTC(tcid, trg.getTCEnergy());
    m_eclTRGInformation->setTimingTC(tcid, trg.getTCTime());
    m_eclTRGInformation->setRevoGDLTC(tcid, RevoGDL);
    m_eclTRGInformation->setRevoFAMTC(tcid, RevoFAM);
    m_eclTRGInformation->setHitWinTC(tcid, trg.getHitWin());

    if (trg.getTCEnergy() > 0 and std::isnan(eventtiming)) {
      eventtiming = EventTiming;
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
        const int pos = m_calDigitStoreArrPosition[id];

        if (pos >= 0) {
          energyInTC += m_eclCalDigits[pos]->getEnergy();

          // check if that eclcaldigit is part of an ECLCluster above threshold
          auto rel = m_eclCalDigits[pos]->getRelationsFrom<ECLCluster>();
          for (unsigned int irel = 0; irel < rel.size(); ++irel) {

            const auto cluster = rel.object(irel);

            if (!cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) continue;

            const auto weight = rel.weight(irel);
            float clusterenergy = cluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);

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
  m_eclTRGInformation->setMaximumTCId(maxtcid);

  //flag highest energy TC
  if (maxtcid >= 0) {
    int pos = m_TCStoreArrPosition[maxtcid];
    m_eclTCs[pos]->setIsHighestFADC(true);
  }

}

void ECLTRGInformationModule::terminate()
{
  if (m_trgmap) delete m_trgmap;
}
