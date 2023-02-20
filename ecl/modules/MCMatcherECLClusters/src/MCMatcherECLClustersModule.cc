/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//This module
#include <ecl/modules/MCMatcherECLClusters/MCMatcherECLClustersModule.h>

//MDST
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/MCParticle.h>

//ECL
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLSimHit.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MCMatcherECLClusters);
REG_MODULE(MCMatcherECLClustersPureCsI);
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MCMatcherECLClustersModule::MCMatcherECLClustersModule() : Module(),
  m_eclDigitArrayName(getECLDigitArrayName()),
  m_eclCalDigitArrayName(getECLCalDigitArrayName()),
  m_eclClusterArrayName(getECLClusterArrayName()),
  m_eclShowerArrayName(getECLShowerArrayName()),
  m_mcParticleToECLHitRelationArray(m_mcParticles, m_eclHits),
  m_mcParticleToECLSimHitRelationArray(m_mcParticles, m_eclSimHits)
{
  // Set description
  setDescription("MCMatcherECLClustersModule");
  setPropertyFlags(c_ParallelProcessingCertified);
}

MCMatcherECLClustersModule::~MCMatcherECLClustersModule()
{
}

void MCMatcherECLClustersModule::initialize()
{
  m_mcParticles.isOptional();

  m_eclHits.isRequired();
  m_eclCalDigits.isRequired(m_eclCalDigitArrayName);
  m_eclDigits.isRequired(m_eclDigitArrayName);
  m_eclClusters.isRequired(m_eclClusterArrayName);
  m_eclShowers.isRequired(m_eclShowerArrayName);

  if (m_mcParticles.isValid()) {
    m_mcParticles.registerRelationTo(m_eclHits);
    m_eclCalDigits.registerRelationTo(m_mcParticles);
    m_eclDigits.registerRelationTo(m_mcParticles);
    m_eclShowers.registerRelationTo(m_mcParticles);
    m_eclClusters.registerRelationTo(m_mcParticles);
  }
}

void MCMatcherECLClustersModule::event()
{
  // Don't do anything if MCParticles aren't present
  if (not m_mcParticles.isValid()) {
    return;
  }

  //CalDigits
  short int Index[8736] = {-1};
  int k = 0;
  for (const ECLCalDigit& eclCalDigit : m_eclCalDigits) {
    double calEnergy = eclCalDigit.getEnergy(); // Calibrated Energy in GeV
    if (calEnergy < 0) continue;
    Index[eclCalDigit.getCellId() - 1] = k;
    ++k;
  }

  const RelationArray& p2sh = m_mcParticleToECLSimHitRelationArray;
  //RelationArray& cd2p = m_eclCalDigitToMCParticleRelationArray;
  ECLSimHit** simhits = (ECLSimHit**)(m_eclSimHits.getPtr()->GetObjectRef());
  MCParticle** mcs = (MCParticle**)(m_mcParticles.getPtr()->GetObjectRef());

  for (int i = 0, imax = p2sh.getEntries(); i < imax; i++) {
    const RelationElement& re = p2sh[i];
    const std::vector<unsigned int>& simhitindx = re.getToIndices();
    std::map<int, double> e;
    for (unsigned int j : simhitindx) {
      const ECLSimHit* h = simhits[j];
      int id = h->getCellId() - 1;
      e[id] += h->getEnergyDep();
    }

    // relation from CalibratedDigit to MC particle
    for (std::pair<int, double> t : e) {
      int ind = Index[t.first];
      double w = t.second;
      if (ind >= 0
          && w > 0) {//cd2p.add(ind, re.getFromIndex(), w); //old relation setter from ECLCalDigit to MC particle, not working when pure CsI digits are introduced
        const ECLCalDigit* cal_digit = m_eclCalDigits[ind];
        cal_digit->addRelationTo(mcs[re.getFromIndex()], w);
      }
    }

    //------------------------------------------
    // relation from ECLShower to MC particle
    //------------------------------------------
    for (const ECLShower& shower : m_eclShowers) {

      double shower_mcParWeight = 0; //Weight between shower and MCParticle

      //Loop on ECLCalDigits related to this MCParticle
      const auto shower_CalDigitRelations = shower.getRelationsTo<ECLCalDigit>(m_eclCalDigitArrayName);
      for (unsigned int iRelation = 0; iRelation < shower_CalDigitRelations.size(); ++iRelation) {

        //Retrieve calDigit
        const ECLCalDigit* calDigit = shower_CalDigitRelations.object(iRelation);

        //Retrieve weight between calDigit and shower
        //Important if the calDigit is related to multiple showers with different weights (e.g. with high energy pi0)
        const double calDigit_showerWeight = shower_CalDigitRelations.weight(iRelation);

        int id = calDigit->getCellId() - 1;
        auto it = e.find(id);
        if (it != e.end()) {
          const double calDigit_mcParWeight = (*it).second;
          shower_mcParWeight += (calDigit_mcParWeight * calDigit_showerWeight);
        }
      }
      if (shower_mcParWeight > 0)
        shower.addRelationTo(mcs[re.getFromIndex()], shower_mcParWeight);
    }
  }

  // reuse Index
  std::fill_n(Index, 8736, -1);
  k = 0;
  for (const ECLDigit& eclDigit : m_eclDigits) {
    if (eclDigit.getAmp() <= 0) continue;
    Index[eclDigit.getCellId() - 1] = k;
    ++k;
  }

  const RelationArray& p2eh = m_mcParticleToECLHitRelationArray;
  //RelationArray& ed2p = m_eclDigitToMCParticleRelationArray;
  ECLHit** eclhits = (ECLHit**)(m_eclHits.getPtr()->GetObjectRef());
  for (const auto& relationElement : p2eh) {
    const std::vector<unsigned int>& eclhitindx = relationElement.getToIndices();
    for (unsigned int j : eclhitindx) {
      const ECLHit* t = eclhits[j];
      int id = t->getCellId() - 1;
      if (t->getBackgroundTag() == 0
          && Index[id] >=
          0) { //ed2p.add(Index[id], re.getFromIndex()); // old relation setter from ECLDigit to MC particle, not working when pure CsI digits are introduced
        const ECLDigit* mdigit = m_eclDigits[Index[id]];
        mdigit->addRelationTo(mcs[relationElement.getFromIndex()]);
      }
    }
  }

  // to create the relation between ECLCluster->MCParticle with the same weight as
  // the relation between ECLShower->MCParticle.  StoreArray<ECLCluster> eclClusters;
  for (const auto& eclShower : m_eclShowers) {
    const ECLCluster* eclCluster = eclShower.getRelatedFrom<ECLCluster>(m_eclClusterArrayName);
    if (!eclCluster) continue;

    const RelationVector<MCParticle> mcParticles = eclShower.getRelationsTo<MCParticle>();
    for (unsigned int i = 0; i < mcParticles.size(); ++i) {
      const auto mcParticle = mcParticles.object(i);
      const auto weight = mcParticles.weight(i);
      eclCluster->addRelationTo(mcParticle, weight);
    }
  }
}
