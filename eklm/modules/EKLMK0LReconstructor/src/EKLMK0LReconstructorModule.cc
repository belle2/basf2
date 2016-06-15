/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* C++ headers. */
#include <map>

/* External headers. */
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Vector/LorentzVector.h>

/* Belle2 headers. */

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <eklm/dataobjects/EKLMHit2d.h>
#include <eklm/modules/EKLMK0LReconstructor/EKLMK0LReconstructorModule.h>
#include <mdst/dataobjects/KLMCluster.h>

using namespace Belle2;

REG_MODULE(EKLMK0LReconstructor)

/**
 * Hit status.
 */
enum HitStatus {
  c_Unknown,  /**< Hit is not processed. */
  c_Isolated, /**< Cannot find any associated hits. */
  c_Cluster,  /**< Part of a cluster. */
  c_Merged,   /**< Merged with another hit. */
};

/**
 * Hit information.
 */
struct HitData {
  EKLMHit2d* hit;                     /**< Hit. */
  enum HitStatus stat;                /**< Hit status. */
  std::vector<EKLMHit2d*> mergedHits; /**< Vector of merged hits. */
};

/**
 * Comparison by layer.
 * @param[in] hit1 First hit.
 * @param[in] hit2 Second hit.
 */
static bool layerLessThan(struct HitData hit1, struct HitData hit2)
{
  return hit1.hit->getLayer() < hit2.hit->getLayer();
}

/**
 * Merge 2d clusters (hits are in adjacent strips).
 * @param[in,out] hits   Hits.
 * @param[in.out] hits2d New 2d hits.
 */
static void merge2dClusters(std::vector<struct HitData>& hits,
                            std::vector<struct HitData>& newHits)
{
  std::vector<struct HitData>::iterator it, it2, itStart, itEnd;
  int j;
  bool add;
  int s[2];
  int clust[2][2];
  double e, de;
  struct HitData dat;
  TVector3 newPosition;
  dat.stat = c_Unknown;
  /* Clusterization. */
  itStart = hits.begin();
  while (itStart != hits.end()) {
    itEnd = itStart;
    while (itEnd != hits.end()) {
      if (itStart->hit->getLayer() != itEnd->hit->getLayer())
        break;
      itEnd++;
    }
    for (it = itStart; it != itEnd; it++) {
      if (it->stat != c_Unknown)
        continue;
      RelationVector<EKLMDigit> digits = it->hit->getRelationsTo<EKLMDigit>();
      if (digits.size() != 2)
        B2FATAL("Wrong number of related EKLMDigits.");
      for (j = 0; j < 2; j++) {
        clust[j][0] = digits[j]->getStrip();
        clust[j][1] = clust[j][0];
      }
      dat.hit = NULL;
      dat.mergedHits.clear();
      do {
        add = false;
        for (it2 = itStart; it2 != itEnd; it2++) {
          if (it2 == it)
            continue;
          if (it->stat != c_Unknown)
            continue;
          RelationVector<EKLMDigit> digits2 =
            it2->hit->getRelationsTo<EKLMDigit>();
          if (digits2.size() != 2)
            B2FATAL("Wrong number of related EKLMDigits.");
          for (j = 0; j < 2; j++)
            s[j] = digits2[j]->getStrip();
          if (s[0] > clust[0][1] + 1 || s[0] < clust[0][0] - 1 ||
              s[1] > clust[1][1] + 1 || s[1] < clust[1][0] - 1)
            continue;
          if ((s[0] > clust[0][1] || s[0] < clust[0][0]) &&
              (s[1] > clust[1][1] || s[1] < clust[1][0]))
            continue;
          for (j = 0; j < 2; j++) {
            if (s[j] > clust[j][1])
              clust[j][1] = s[j];
            if (s[j] < clust[j][0])
              clust[j][0] = s[j];
          }
          add = true;
          it2->stat = c_Merged;
          it->stat = c_Merged;
          if (dat.hit == NULL) {
            dat.hit = new EKLMHit2d(*(it->hit));
            dat.mergedHits.push_back(it->hit);
          }
          dat.mergedHits.push_back(it2->hit);
          e = dat.hit->getEDep();
          de = it2->hit->getEDep();
          dat.hit->setEDep(e + de);
          dat.hit->setTime(std::min(dat.hit->getTime(), it2->hit->getTime()));
          newPosition = (dat.hit->getPosition() * e +
                         it2->hit->getPosition() * de) * (1.0 / (e + de));
          dat.hit->setPosition(newPosition);
        }
      } while (add);
      if (dat.hit != NULL)
        newHits.push_back(dat);
    }
    itStart = itEnd;
  }
  /* Erase merged hits. */
  it = hits.begin();
  while (it != hits.end()) {
    if (it->stat == c_Merged)
      it = hits.erase(it);
    else
      it++;
  }
}

/**
 * Find cluster and fill EKLMK0L.
 * @param[in]     hit     This hit.
 * @param[in,out] hits    All hits (including this).
 */
static void findAssociatedHits(struct HitData* hit,
                               std::vector<struct HitData>& hits)
{
  int i, n;
  int layerHits[14], nLayers;
  float e, de;
  std::vector<struct HitData*> cluster;
  std::vector<struct HitData*>::iterator itClust;
  std::vector<EKLMHit2d*>::iterator itHit;
  HepGeom::Point3D<double> hitPos, hitPos2;
  CLHEP::Hep3Vector p;
  float mt, t, m;
  double v;
  KLMCluster* klmCluster;
  /* Initially fill the cluster with the hit in question. */
  cluster.push_back(hit);
  hitPos.setX(hit->hit->getPositionX());
  hitPos.setY(hit->hit->getPositionY());
  hitPos.setZ(hit->hit->getPositionZ());
  /* Collect other hits. */
  n = hits.size();
  for (i = 0; i < n; i++) {
    if (&hits[i] == hit)
      continue;
    if (hits[i].stat != c_Unknown)
      continue;
    hitPos2.setX(hits[i].hit->getPositionX());
    hitPos2.setY(hits[i].hit->getPositionY());
    hitPos2.setZ(hits[i].hit->getPositionZ());
    if (hitPos2.angle(hitPos) < 0.1) {
      hits[i].stat = c_Cluster;
      cluster.push_back(&hits[i]);
    }
  }
  /* Get minimal time. */
  mt = (*cluster.begin())->hit->getTime();
  for (itClust = cluster.begin() + 1; itClust != cluster.end(); ++itClust) {
    t = (*itClust)->hit->getTime();
    if (t < mt) {
      mt = t;
    }
  }
  /* Get number of layers and number of the innermost layer with hits */
  for (i = 0; i < 14; i++)
    layerHits[i] = 0;
  for (itClust = cluster.begin(); itClust != cluster.end(); ++itClust)
    layerHits[(*itClust)->hit->getLayer() - 1]++;
  nLayers = 0;
  int nInnermostLayer = 0;
  for (i = 13; i >= 0; i--)
    if (layerHits[i] > 0) {
      nInnermostLayer = i;
      nLayers++;
    }
  /* Get hit position as weighted average of cluster hit positions. */
  e = 0;
  hitPos = HepGeom::Point3D<double>(0., 0., 0.);
  for (itClust = cluster.begin(); itClust != cluster.end(); ++itClust) {
    if ((*itClust)->hit->getLayer() != hit->hit->getLayer())
      continue;
    de = (*itClust)->hit->getEDep();
    e = e + de;
    hitPos.setX(hitPos.x() + de * (*itClust)->hit->getPositionX());
    hitPos.setY(hitPos.y() + de * (*itClust)->hit->getPositionY());
    hitPos.setZ(hitPos.z() + de * (*itClust)->hit->getPositionZ());
  }
  hitPos = hitPos / e;
  /* Calculate momentum. */
  m = 0.497614;
  p.setX(hitPos.x());
  p.setY(hitPos.y());
  p.setZ(hitPos.z());
  p = p / mt;
  v = p.mag() / Const::speedOfLight;
  if (v < 0.999999)
    p = p.unit() * m * v / sqrt(1.0 - v * v);
  else
    p = CLHEP::Hep3Vector(0, 0, 0);
  /* Set the status of hit. */
  if (cluster.size() == 1) {
    hit->stat = c_Isolated;
    return;
  }
  hit->stat = c_Cluster;
  /* Fill KLMClusters. */
  StoreArray<KLMCluster> klmClusters;
  klmCluster = klmClusters.appendNew(
                 hitPos.x(), hitPos.y(), hitPos.z(), mt, nLayers,
                 nInnermostLayer, p.x(), p.y(), p.z());
  /* Fill cluster-hit relation array */
  StoreArray<EKLMHit2d> hits2d;
  for (itClust = cluster.begin(); itClust != cluster.end(); ++itClust) {
    if ((*itClust)->mergedHits.size() == 0) {
      klmCluster->addRelationTo((*itClust)->hit);
    } else {
      for (itHit = (*itClust)->mergedHits.begin();
           itHit != (*itClust)->mergedHits.end(); ++itHit)
        klmCluster->addRelationTo(*itHit);
    }
  }
}

EKLMK0LReconstructorModule::EKLMK0LReconstructorModule() : Module()
{
  setDescription("EKLM K0L reconstruction module.");
  setPropertyFlags(c_ParallelProcessingCertified);
}

EKLMK0LReconstructorModule::~EKLMK0LReconstructorModule()
{
}

void EKLMK0LReconstructorModule::initialize()
{
  StoreArray<KLMCluster> clusters;
  StoreArray<EKLMHit2d> hit2ds;
  clusters.registerInDataStore();
  hit2ds.isRequired();
  clusters.registerRelationTo(hit2ds);
}

void EKLMK0LReconstructorModule::beginRun()
{
}

void EKLMK0LReconstructorModule::event()
{
  int i, j, n;
  StoreArray<EKLMHit2d> hits2d;
  struct HitData dat;
  std::vector<struct HitData> newHits;
  std::vector<struct HitData>::iterator itClust;
  /* Hits in endcap, index is (number of endcap - 1). */
  std::vector<struct HitData> endcap[2];
  std::vector<struct HitData>::iterator it;
  dat.stat = c_Unknown;
  dat.mergedHits.clear();
  /* Sort hits by endcap. */
  n = hits2d.getEntries();
  for (i = 0; i < n; i++) {
    dat.hit = hits2d[i];
    endcap[hits2d[i]->getEndcap() - 1].push_back(dat);
  }
  for (i = 0; i <= 1; i++) {
    /* Sort hits by layer. */
    sort(endcap[i].begin(), endcap[i].end(), layerLessThan);
    /* Merge 2d clusters. */
    merge2dClusters(endcap[i], newHits);
    for (itClust = newHits.begin(); itClust != newHits.end(); itClust++)
      endcap[i].push_back(*itClust);
    /* Sort by layer again. */
    sort(endcap[i].begin(), endcap[i].end(), layerLessThan);
    /* Search for clusters. */
    n = endcap[i].size();
    for (j = 0; j < n; j++) {
      if (endcap[i][j].stat != c_Unknown)
        continue;
      findAssociatedHits(&(endcap[i][j]), endcap[i]);
    }
    /* Free memory. */
    for (itClust = newHits.begin(); itClust != newHits.end(); itClust++)
      delete itClust->hit;
    newHits.clear();
  }
}

void EKLMK0LReconstructorModule::endRun()
{
}

void EKLMK0LReconstructorModule::terminate()
{
}

