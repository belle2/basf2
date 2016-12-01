/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Oberhof                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/MCMatcherECLClusters/MCMatcherECLClustersModule.h>
//#include <iostream>

using namespace Belle2;
using namespace ECL;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MCMatcherECLClusters)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MCMatcherECLClustersModule::MCMatcherECLClustersModule() : Module(),
  m_eclHitToSimHitRelationArray(m_eclHits, m_eclSimHits),
  m_mcParticleToECLHitRelationArray(m_mcParticles, m_eclHits),
  m_mcParticleToECLSimHitRelationArray(m_mcParticles, m_eclSimHits),
  m_eclCalDigitToMCParticleRelationArray(m_eclCalDigits, m_mcParticles),
  m_eclDigitToMCParticleRelationArray(m_eclDigits, m_mcParticles),
  m_eclShowerToMCPart(m_eclShowers, m_mcParticles)
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
  m_mcParticles.registerInDataStore();
  m_eclHits.registerInDataStore();
  m_eclCalDigits.registerInDataStore();
  m_eclDigits.registerInDataStore();
  m_eclClusters.registerInDataStore();

  m_mcParticles.registerRelationTo(m_eclHits);
  m_eclCalDigits.registerRelationTo(m_mcParticles);
  m_eclDigits.registerRelationTo(m_mcParticles);
  m_eclShowers.registerRelationTo(m_mcParticles);
  m_eclClusters.registerRelationTo(m_mcParticles);
}

void MCMatcherECLClustersModule::beginRun()
{
}

void MCMatcherECLClustersModule::event()
{
  short int Index[8736];
  std::fill_n(Index, 8736, -1);
  const TClonesArray* cd = m_eclCalDigits.getPtr();
  TObject** ocd = cd->GetObjectRef();
  for (int i = 0, imax = cd->GetEntries(); i < imax; i++) { // avoiding call of StoreArray::getArrayIndex() member function
    const ECLCalDigit& t = static_cast<const ECLCalDigit&>(*ocd[i]);
    double calEnergy = t.getEnergy(); // Calibrated Energy in GeV
    if (calEnergy < 0) continue;
    Index[t.getCellId() - 1] = i;
  }

  RelationArray& p2sh = m_mcParticleToECLSimHitRelationArray;
  RelationArray& cd2p = m_eclCalDigitToMCParticleRelationArray;
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
    for (const std::pair<int, double>& t : e) {
      int ind = Index[t.first];
      double w = t.second;
      if (ind >= 0 && w > 0) cd2p.add(ind, re.getFromIndex(), w);
    }

    //------------------------------------------
    // relation from ECLShower to MC particle
    //------------------------------------------
    for (const ECLShower& shower : m_eclShowers) {

      double shower_mcParWeight = 0; //Weight between shower and MCParticle

      //Loop on ECLCalDigits related to this MCParticle
      const auto shower_CalDigitRelations = shower.getRelationsTo<ECLCalDigit>();
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
  const TClonesArray* ed = m_eclDigits.getPtr();
  TObject** oed = ed->GetObjectRef();
  for (int i = 0, imax = ed->GetEntries(); i < imax; i++) { // avoiding call of StoreArray::getArrayIndex() member function
    const ECLDigit& t = static_cast<const ECLDigit&>(*oed[i]);
    if (t.getAmp() <= 0) continue;
    Index[t.getCellId() - 1] = i;
  }

  RelationArray& p2eh = m_mcParticleToECLHitRelationArray;
  RelationArray& ed2p = m_eclDigitToMCParticleRelationArray;
  ECLHit** eclhits = (ECLHit**)(m_eclHits.getPtr()->GetObjectRef());
  for (int i = 0, imax = p2eh.getEntries(); i < imax; i++) {
    const RelationElement& re = p2eh[i];
    const std::vector<unsigned int>& eclhitindx = re.getToIndices();
    for (unsigned int j : eclhitindx) {
      const ECLHit* t = eclhits[j];
      int id = t->getCellId() - 1;
      if (t->getBackgroundTag() == 0 && Index[id] >= 0)
        ed2p.add(Index[id], re.getFromIndex()); // relation ECLDigit to MC particle
    }
  }

  // to create the relation between ECLCluster->MCParticle with the same weight as
  // the relation between ECLShower->MCParticle.  StoreArray<ECLCluster> eclClusters;
  for (const auto& eclShower : m_eclShowers) {
    const ECLCluster* eclCluster = eclShower.getRelatedFrom<ECLCluster>();
    const RelationVector<MCParticle> mcParticles = eclShower.getRelationsTo<MCParticle>();
    for (unsigned int i = 0; i < mcParticles.size(); ++i) {
      const auto mcParticle = mcParticles.object(i);
      const auto weight = mcParticles.weight(i);
      eclCluster->addRelationTo(mcParticle, weight);
    }
  }
  // for (const auto& eclCluster : m_eclClusters) {
  //   const auto mcParticleWeightPair = eclCluster.getRelatedToWithWeight<MCParticle>();
  //   std::cout<< "ClusterID: " << eclCluster.getArrayIndex()
  //       << " Energy: " <<  eclCluster.getEnergy()
  //       << " MCParticle: " << mcParticleWeightPair.first->getArrayIndex()
  //       << " Weight: " << mcParticleWeightPair.second<<std::endl;
  // }
}

void MCMatcherECLClustersModule::endRun()
{
}

void MCMatcherECLClustersModule::terminate()
{
}
