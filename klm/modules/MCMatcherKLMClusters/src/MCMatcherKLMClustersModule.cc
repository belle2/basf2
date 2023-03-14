/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/modules/MCMatcherKLMClusters/MCMatcherKLMClustersModule.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMHit1d.h>
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/KLMHit2d.h>
#include <klm/dataobjects/KLMSimHit.h>

/* Basf2 headers. */
#include <mdst/dataobjects/MCParticle.h>

/* C++ headers. */
#include <map>

using namespace Belle2;

REG_MODULE(MCMatcherKLMClusters);

MCMatcherKLMClustersModule::MCMatcherKLMClustersModule() : Module()
{
  setDescription("Module for MC matching for KLM clusters.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("Hit2dRelations", m_Hit2dRelations,
           "Add also relations for KLMHit2d and KLMHit2d.", false);
}

MCMatcherKLMClustersModule::~MCMatcherKLMClustersModule()
{
}

void MCMatcherKLMClustersModule::initialize()
{
  m_KLMClusters.isRequired();
  m_MCParticles.isOptional();
  if (m_MCParticles.isValid()) {
    m_KLMClusters.registerRelationTo(m_MCParticles);
  }
  if (m_Hit2dRelations) {
    StoreArray<KLMHit2d> bklmHit2ds;
    StoreArray<KLMHit2d> eklmHit2ds;
    if (m_MCParticles.isValid()) {
      bklmHit2ds.registerRelationTo(m_MCParticles);
      eklmHit2ds.registerRelationTo(m_MCParticles);
    }
  }
}

void MCMatcherKLMClustersModule::event()
{
  // Don't do anything if MCParticles aren't present
  if (not m_MCParticles.isValid()) {
    return;
  }

  double weightSum;
  /* cppcheck-suppress variableScope */
  int i1, i2, i3, i4, i5, i6, n1, n2, n3, n4, n5, n6;
  std::map<MCParticle*, double> mcParticles, mcParticlesHit;
  std::map<MCParticle*, double>::iterator it;
  n1 = m_KLMClusters.getEntries();
  for (i1 = 0; i1 < n1; i1++) {
    mcParticles.clear();
    RelationVector<KLMHit2d> bklmHit2ds =
      m_KLMClusters[i1]->getRelationsTo<KLMHit2d>();
    n2 = bklmHit2ds.size();
    for (i2 = 0; i2 < n2; i2++) {
      if (m_Hit2dRelations)
        mcParticlesHit.clear();
      RelationVector<BKLMHit1d> bklmHit1ds =
        bklmHit2ds[i2]->getRelationsTo<BKLMHit1d>();
      n3 = bklmHit1ds.size();
      for (i3 = 0; i3 < n3; i3++) {
        RelationVector<KLMDigit> bklmDigits =
          bklmHit1ds[i3]->getRelationsTo<KLMDigit>();
        n4 = bklmDigits.size();
        for (i4 = 0; i4 < n4; i4++) {
          RelationVector<KLMSimHit> bklmSimHits =
            bklmDigits[i4]->getRelationsTo<KLMSimHit>();
          n5 = bklmSimHits.size();
          for (i5 = 0; i5 < n5; i5++) {
            RelationVector<MCParticle> bklmMCParticles =
              bklmSimHits[i5]->getRelationsFrom<MCParticle>();
            n6 = bklmMCParticles.size();
            for (i6 = 0; i6 < n6; i6++) {
              it = mcParticles.find(bklmMCParticles[i6]);
              if (it == mcParticles.end()) {
                mcParticles.insert(std::pair<MCParticle*, double>(
                                     bklmMCParticles[i6],
                                     bklmSimHits[i5]->getEnergyDeposit()));
              } else {
                it->second = it->second + bklmSimHits[i5]->getEnergyDeposit();
              }
              if (m_Hit2dRelations) {
                it = mcParticlesHit.find(bklmMCParticles[i6]);
                if (it == mcParticlesHit.end()) {
                  mcParticlesHit.insert(std::pair<MCParticle*, double>(
                                          bklmMCParticles[i6],
                                          bklmSimHits[i5]->getEnergyDeposit()));
                } else {
                  it->second = it->second + bklmSimHits[i5]->getEnergyDeposit();
                }
              }
            }
          }
        }
      }
      if (m_Hit2dRelations) {
        weightSum = 0;
        for (it = mcParticlesHit.begin(); it != mcParticlesHit.end(); ++it)
          weightSum = weightSum + it->second;
        for (it = mcParticlesHit.begin(); it != mcParticlesHit.end(); ++it)
          bklmHit2ds[i2]->addRelationTo(it->first, it->second / weightSum);
      }
    }
    RelationVector<KLMHit2d> eklmHit2ds =
      m_KLMClusters[i1]->getRelationsTo<KLMHit2d>();
    n2 = eklmHit2ds.size();
    for (i2 = 0; i2 < n2; i2++) {
      if (m_Hit2dRelations)
        mcParticlesHit.clear();
      RelationVector<KLMDigit> eklmDigits =
        eklmHit2ds[i2]->getRelationsTo<KLMDigit>();
      n3 = eklmDigits.size();
      for (i3 = 0; i3 < n3; i3++) {
        RelationVector<KLMSimHit> eklmSimHits =
          eklmDigits[i3]->getRelationsTo<KLMSimHit>();
        n4 = eklmSimHits.size();
        for (i4 = 0; i4 < n4; i4++) {
          RelationVector<MCParticle> eklmMCParticles =
            eklmSimHits[i4]->getRelationsFrom<MCParticle>();
          n5 = eklmMCParticles.size();
          for (i5 = 0; i5 < n5; i5++) {
            it = mcParticles.find(eklmMCParticles[i5]);
            if (it == mcParticles.end()) {
              mcParticles.insert(std::pair<MCParticle*, double>(
                                   eklmMCParticles[i5],
                                   eklmSimHits[i4]->getEnergyDeposit()));
            } else {
              it->second = it->second + eklmSimHits[i4]->getEnergyDeposit();
            }
            if (m_Hit2dRelations) {
              it = mcParticlesHit.find(eklmMCParticles[i5]);
              if (it == mcParticlesHit.end()) {
                mcParticlesHit.insert(std::pair<MCParticle*, double>(
                                        eklmMCParticles[i5],
                                        eklmSimHits[i4]->getEnergyDeposit()));
              } else {
                it->second = it->second + eklmSimHits[i4]->getEnergyDeposit();
              }
            }
          }
        }
      }
      if (m_Hit2dRelations) {
        weightSum = 0;
        for (it = mcParticlesHit.begin(); it != mcParticlesHit.end(); ++it)
          weightSum = weightSum + it->second;
        for (it = mcParticlesHit.begin(); it != mcParticlesHit.end(); ++it)
          eklmHit2ds[i2]->addRelationTo(it->first, it->second / weightSum);
      }
    }
    weightSum = 0;
    for (it = mcParticles.begin(); it != mcParticles.end(); ++it)
      weightSum = weightSum + it->second;
    for (it = mcParticles.begin(); it != mcParticles.end(); ++it)
      m_KLMClusters[i1]->addRelationTo(it->first, it->second / weightSum);
  }
}
