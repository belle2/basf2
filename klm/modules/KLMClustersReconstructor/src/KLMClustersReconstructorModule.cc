/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMClustersReconstructor/KLMClustersReconstructorModule.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>
#include <klm/modules/KLMClustersReconstructor/KLMHit2d.h>

/* C++ headers. */
#include <algorithm>

using namespace Belle2;

REG_MODULE(KLMClustersReconstructor)

KLMClustersReconstructorModule::KLMClustersReconstructorModule() : Module(),
  m_PositionMode(c_FirstLayer),
  m_ClusterMode(c_AnyHit)
{
  setDescription("Unified BKLM/EKLM module for the reconstruction of KLMClusters.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("ClusteringAngle", m_ClusteringAngle, "Clustering angle (rad).",
           0.26);
  addParam("PositionMode", m_PositionModeString,
           "Vertex position calculation mode ('FullAverage' or 'FirstLayer').",
           std::string("FirstLayer"));
  addParam("ClusterMode", m_ClusterModeString,
           "Clusterization mode ('AnyHit' or 'FirstHit').",
           std::string("AnyHit"));
}

KLMClustersReconstructorModule::~KLMClustersReconstructorModule()
{
}

void KLMClustersReconstructorModule::initialize()
{
  m_KLMClusters.registerInDataStore();
  m_BKLMHit2ds.isRequired();
  m_EKLMHit2ds.isRequired();
  m_KLMClusters.registerRelationTo(m_BKLMHit2ds);
  m_KLMClusters.registerRelationTo(m_EKLMHit2ds);
  if (m_PositionModeString == "FullAverage")
    m_PositionMode = c_FullAverage;
  else if (m_PositionModeString == "FirstLayer")
    m_PositionMode = c_FirstLayer;
  else
    B2FATAL("Incorrect PositionMode argument.");
  if (m_ClusterModeString == "AnyHit")
    m_ClusterMode = c_AnyHit;
  else if (m_ClusterModeString == "FirstHit")
    m_ClusterMode = c_FirstHit;
  else
    B2FATAL("Incorrect ClusterMode argument.");
}

void KLMClustersReconstructorModule::beginRun()
{
}

static bool compareDistance(const KLMHit2d& hit1, const KLMHit2d& hit2)
{
  return hit1.getPosition().Mag() < hit2.getPosition().Mag();
}

void KLMClustersReconstructorModule::event()
{
  //static double mass = Const::Klong.getMass();
  int i, nLayers, innermostLayer, nHits;
  int nLayersBKLM = BKLMElementNumbers::getMaximalLayerNumber();
  int nLayersEKLM = EKLMElementNumbers::getMaximalLayerNumber();
  int* layerHitsBKLM, *layerHitsEKLM;
  float minTime = -1;
  double p;//, v;
  std::vector<KLMHit2d> klmHit2ds, klmClusterHits;
  std::vector<KLMHit2d>::iterator it, it0, it2;
  KLMCluster* klmCluster;
  TVector3 hitPos;
  layerHitsBKLM = new int[nLayersBKLM];
  layerHitsEKLM = new int[nLayersEKLM];
  /* Fill vector of 2d hits. */
  int nHitsBKLM = m_BKLMHit2ds.getEntries();
  for (i = 0; i < nHitsBKLM; i++) {
    if (m_BKLMHit2ds[i]->isOutOfTime())
      continue;
    klmHit2ds.push_back(KLMHit2d(m_BKLMHit2ds[i]));
  }
  int nHitsEKLM = m_EKLMHit2ds.getEntries();
  for (i = 0; i < nHitsEKLM; i++) {
    klmHit2ds.push_back(KLMHit2d(m_EKLMHit2ds[i]));
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
      switch (m_ClusterMode) {
        case c_AnyHit:
          while (it2 != klmClusterHits.end()) {
            if (it->getPosition().Angle(it2->getPosition()) <
                m_ClusteringAngle) {
              klmClusterHits.push_back(*it);
              it = klmHit2ds.erase(it);
              goto clusterFound;
            } else
              ++it2;
          }
          break;
        case c_FirstHit:
          if (it->getPosition().Angle(it2->getPosition()) <
              m_ClusteringAngle) {
            klmClusterHits.push_back(*it);
            it = klmHit2ds.erase(it);
            goto clusterFound;
          }
          break;
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
    /* Find minimal time, fill layer array, find hit position. */
    it0 = klmClusterHits.begin();
    nHits = 0;
    for (it = klmClusterHits.begin(); it != klmClusterHits.end(); ++it) {
      if ((it->getLayer() == it0->getLayer() &&
           it->inBKLM() == it0->inBKLM()) || m_PositionMode == c_FullAverage) {
        hitPos = hitPos + it->getPosition();
        nHits++;
      }
      if (minTime < 0 || it->getTime() < minTime)
        minTime = it->getTime();
      if (it->inBKLM())
        layerHitsBKLM[it->getLayer() - 1]++;
      else
        layerHitsEKLM[it->getLayer() - 1]++;
    }
    hitPos = hitPos * (1.0 / nHits);
    /* Find innermost layer. */
    nLayers = 0;
    innermostLayer = -1;
    for (i = 0; i < nLayersBKLM; i++) {
      if (layerHitsBKLM[i] > 0) {
        nLayers++;
        if (innermostLayer < 0)
          innermostLayer = i + 1;
      }
    }
    for (i = 0; i < nLayersEKLM; i++) {
      if (layerHitsEKLM[i] > 0) {
        nLayers++;
        if (innermostLayer < 0)
          innermostLayer = i + 1;
      }
    }
    /* Calculate energy. */
    //if (it0->inBKLM()) {
    /*
     * TODO: The constant is from BKLM K0L reconstructor,
     * it must be recalculated.
     */
    p = klmClusterHits.size() * 0.215;
    /* FIXME: Reimplement time calculation after completion of time calibration.
    } else {
      v = hitPos.Mag() / minTime / Const::speedOfLight;
      if (v < 0.999999)
        p = mass * v / sqrt(1.0 - v * v);
      else
        p = 0;
    }*/
    klmCluster = m_KLMClusters.appendNew(
                   hitPos.x(), hitPos.y(), hitPos.z(), minTime, nLayers,
                   innermostLayer, p);
    for (it = klmClusterHits.begin(); it != klmClusterHits.end(); ++it) {
      if (it->inBKLM())
        klmCluster->addRelationTo(it->getBKLMHit2d());
      else
        klmCluster->addRelationTo(it->getEKLMHit2d());
    }
  }
  delete[] layerHitsBKLM;
  delete[] layerHitsEKLM;
}

void KLMClustersReconstructorModule::endRun()
{
}

void KLMClustersReconstructorModule::terminate()
{
}

