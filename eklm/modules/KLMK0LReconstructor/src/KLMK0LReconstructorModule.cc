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
#include <algorithm>

/* Belle2 headers. */
#include <bklm/dbobjects/BKLMGeometryPar.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <eklm/modules/KLMK0LReconstructor/KLMHit2d.h>
#include <eklm/modules/KLMK0LReconstructor/KLMK0LReconstructorModule.h>
#include <mdst/dataobjects/KLMCluster.h>

using namespace Belle2;

REG_MODULE(KLMK0LReconstructor)

KLMK0LReconstructorModule::KLMK0LReconstructorModule() : Module()
{
  setDescription("EKLM K0L reconstruction module.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("ClusteringAngle", m_ClusteringAngle, "Clustering angle (rad).",
           0.1);
}

KLMK0LReconstructorModule::~KLMK0LReconstructorModule()
{
}

void KLMK0LReconstructorModule::initialize()
{
  StoreArray<KLMCluster> klmClusters;
  StoreArray<BKLMHit2d> bklmHit2ds;
  StoreArray<EKLMHit2d> eklmHit2ds;
  klmClusters.registerInDataStore();
  bklmHit2ds.isRequired();
  eklmHit2ds.isRequired();
  klmClusters.registerRelationTo(bklmHit2ds);
  klmClusters.registerRelationTo(eklmHit2ds);
  m_GeoDat = &(EKLM::GeometryData::Instance());
}

void KLMK0LReconstructorModule::beginRun()
{
}

static bool compareDistance(KLMHit2d* hit1, KLMHit2d* hit2)
{
  return hit1->getPosition().Mag() < hit2->getPosition().Mag();
}

void KLMK0LReconstructorModule::event()
{
  int i, n, nLayers;
  int nLayersBKLM = NLAYER, nLayersEKLM;
  int* layerHitsBKLM, *layerHitsEKLM;
  float minTime = -1;
  StoreArray<KLMCluster> klmClusters;
  StoreArray<BKLMHit2d> bklmHit2ds;
  StoreArray<EKLMHit2d> eklmHit2ds;
  std::vector<KLMHit2d*> klmHit2ds, klmClusterHits;
  std::vector<KLMHit2d*>::iterator it, it2;
  KLMHit2d* hit2d;
  KLMCluster* klmCluster;
  TVector3 hitPos;
  nLayersEKLM = m_GeoDat->getNLayers();
  layerHitsBKLM = new int[nLayersBKLM];
  layerHitsEKLM = new int[nLayersEKLM];
  /* Fill vector of 2d hits. */
  n = bklmHit2ds.getEntries();
  for (i = 0; i < n; i++) {
    if (bklmHit2ds[i]->isOutOfTime())
      continue;
    hit2d = new KLMHit2d(bklmHit2ds[i]);
    klmHit2ds.push_back(hit2d);
  }
  n = eklmHit2ds.getEntries();
  for (i = 0; i < n; i++) {
    hit2d = new KLMHit2d(eklmHit2ds[i]);
    klmHit2ds.push_back(hit2d);
  }
  /* Sort by the distance from center. */
  sort(klmHit2ds.begin(), klmHit2ds.end(), compareDistance);
  /* Clustering. */
  while (klmHit2ds.size() > 0) {
    klmClusterHits.clear();
    it = klmHit2ds.begin();
    klmClusterHits.push_back(*it);
    it = klmHit2ds.erase(it);
    while (it != klmHit2ds.end()) {
      it2 = klmClusterHits.begin();
      while (it2 != klmClusterHits.end()) {
        if ((*it)->getPosition().Angle((*it2)->getPosition()) <
            m_ClusteringAngle) {
          klmClusterHits.push_back(*it);
          it = klmHit2ds.erase(it);
          goto clusterFound;
        } else
          ++it2;
      }
      ++it;
clusterFound:;
    }
    hitPos.SetX(0);
    hitPos.SetY(0);
    hitPos.SetZ(0);
    for (i = 0; i < nLayersBKLM; i++)
      layerHitsBKLM[i] = 0;
    for (i = 0; i < nLayersEKLM; i++)
      layerHitsEKLM[i] = 0;
    nLayers = 0;
    for (it = klmClusterHits.begin(); it != klmClusterHits.end(); ++it) {
      hitPos = hitPos + (*it)->getPosition();
      if (minTime < 0 || (*it)->getTime() < minTime)
        minTime = (*it)->getTime();
      if ((*it)->inBKLM())
        layerHitsBKLM[(*it)->getLayer() - 1]++;
      else
        layerHitsEKLM[(*it)->getLayer() - 1]++;
    }
    hitPos = hitPos * (1.0 / klmClusterHits.size());
    for (i = 0; i < nLayersBKLM; i++) {
      if (layerHitsBKLM[i] > 0)
        nLayers++;
    }
    for (i = 0; i < nLayersEKLM; i++) {
      if (layerHitsEKLM[i] > 0)
        nLayers++;
    }
    klmCluster = klmClusters.appendNew(
                   hitPos.x(), hitPos.y(), hitPos.z(), minTime, nLayers, 0, 0,
                   0, 0);
    for (it = klmClusterHits.begin(); it != klmClusterHits.end(); ++it) {
      if ((*it)->inBKLM())
        klmCluster->addRelationTo((*it)->getBKLMHit2d());
      else
        klmCluster->addRelationTo((*it)->getEKLMHit2d());
    }
  }
  delete layerHitsBKLM;
  delete layerHitsEKLM;
}

void KLMK0LReconstructorModule::endRun()
{
}

void KLMK0LReconstructorModule::terminate()
{
}

