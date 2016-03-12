/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* C++ headers. */
#include <set>

/* Belle2 headers. */
#include <bklm/dataobjects/BKLMDigit.h>
#include <bklm/dataobjects/BKLMHit1d.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <bklm/dataobjects/BKLMSimHit.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dataobjects/EKLMHit2d.h>
#include <eklm/dataobjects/EKLMSimHit.h>
#include <eklm/modules/MCMatcherKLMClusters/MCMatcherKLMClustersModule.h>
#include <framework/core/ModuleManager.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/MCParticle.h>

using namespace Belle2;

REG_MODULE(MCMatcherKLMClusters)

MCMatcherKLMClustersModule::MCMatcherKLMClustersModule() : Module()
{
  setDescription("Module for MC matching for KLM clusters.");
  setPropertyFlags(c_ParallelProcessingCertified);
}

MCMatcherKLMClustersModule::~MCMatcherKLMClustersModule()
{
}

void MCMatcherKLMClustersModule::initialize()
{
  StoreArray<KLMCluster> klmClusters;
  StoreArray<MCParticle> mcParticles;
  klmClusters.registerRelationTo(mcParticles);
}

void MCMatcherKLMClustersModule::beginRun()
{
}

void MCMatcherKLMClustersModule::event()
{
  StoreArray<KLMCluster> klmClusters;
  int i1, i2, i3, i4, i5, i6, n1, n2, n3, n4, n5, n6;
  std::set<MCParticle*> mcParticles;
  std::set<MCParticle*>::iterator it;
  n1 = klmClusters.getEntries();
  for (i1 = 0; i1 < n1; i1++) {
    mcParticles.clear();
    RelationVector<BKLMHit2d> bklmHit2ds =
      klmClusters[i1]->getRelationsTo<BKLMHit2d>();
    n2 = bklmHit2ds.size();
    for (i2 = 0; i2 < n2; i2++) {
      RelationVector<BKLMHit1d> bklmHit1ds =
        bklmHit2ds[i2]->getRelationsTo<BKLMHit1d>();
      n3 = bklmHit1ds.size();
      for (i3 = 0; i3 < n3; i3++) {
        RelationVector<BKLMDigit> bklmDigits =
          bklmHit1ds[i3]->getRelationsTo<BKLMDigit>();
        n4 = bklmDigits.size();
        for (i4 = 0; i4 < n4; i4++) {
          RelationVector<BKLMSimHit> bklmSimHits =
            bklmDigits[i4]->getRelationsFrom<BKLMSimHit>();
          n5 = bklmSimHits.size();
          for (i5 = 0; i5 < n5; i5++) {
            RelationVector<MCParticle> bklmMCParticles =
              bklmSimHits[i5]->getRelationsFrom<MCParticle>();
            n6 = bklmMCParticles.size();
            for (i6 = 0; i6 < n6; i6++)
              mcParticles.insert(bklmMCParticles[i6]);
          }
        }
      }
    }
    for (it = mcParticles.begin(); it != mcParticles.end(); ++it)
      klmClusters[i1]->addRelationTo(*it);
    mcParticles.clear();
    RelationVector<EKLMHit2d> eklmHit2ds =
      klmClusters[i1]->getRelationsTo<EKLMHit2d>();
    n2 = eklmHit2ds.size();
    for (i2 = 0; i2 < n2; i2++) {
      RelationVector<EKLMDigit> eklmDigits =
        eklmHit2ds[i2]->getRelationsTo<EKLMDigit>();
      n3 = eklmDigits.size();
      for (i3 = 0; i3 < n3; i3++) {
        RelationVector<EKLMSimHit> eklmSimHits =
          eklmDigits[i3]->getRelationsTo<EKLMSimHit>();
        n4 = eklmSimHits.size();
        for (i4 = 0; i4 < n4; i4++) {
          RelationVector<MCParticle> eklmMCParticles =
            eklmSimHits[i4]->getRelationsFrom<MCParticle>();
          n5 = eklmMCParticles.size();
          for (i5 = 0; i5 < n5; i5++)
            mcParticles.insert(eklmMCParticles[i5]);
        }
      }
    }
    for (it = mcParticles.begin(); it != mcParticles.end(); ++it)
      klmClusters[i1]->addRelationTo(*it);
  }
}

void MCMatcherKLMClustersModule::endRun()
{
}

void MCMatcherKLMClustersModule::terminate()
{
}

