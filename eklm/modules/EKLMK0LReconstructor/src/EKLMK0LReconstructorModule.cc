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

/* Belle2 headers. */
#include <framework/core/ModuleManager.h>
#include <framework/datastore/StoreArray.h>
#include <eklm/dataobjects/EKLMHit2d.h>
#include <eklm/dataobjects/EKLMK0L.h>
#include <eklm/geometry/EKLMObjectNumbers.h>
#include <eklm/modules/EKLMK0LReconstructor/EKLMK0LReconstructorModule.h>

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
  EKLMHit2d* hit;       /**< Hit. */
  enum HitStatus stat;  /**< Hit status. */
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
static void merge2dClusters(std::vector<struct HitData> &hits,
                            std::vector<EKLMHit2d*> &hits2d)
{
  std::vector<struct HitData>::iterator it, it2, itStart, itEnd;
  int layerRange[15];
  int layer, currLayer;
  int i, j;
  bool add;
  int s[2];
  int clust[2][2];
  double e, de;
  EKLMHit2d* hit2d;
  /* Nothing to do. */
  if (hits.size() <= 1)
    return;
  /* Fill layer ranges. */
  currLayer = 1;
  layerRange[0] = 0;
  for (it = hits.begin(); it != hits.end(); it++) {
    layer = it->hit->getLayer();
    if (layer > currLayer) {
      for (i = currLayer; i < layer; i++)
        layerRange[i] = it - hits.begin();
      currLayer = layer;
    }
  }
  for (i = currLayer; i < 15; i++)
    layerRange[i] = hits.size();
  /* Clusterization. */
  for (i = 0; i < 14; i++) {
    itStart = hits.begin() + layerRange[i];
    itEnd = hits.begin() + layerRange[i + 1];
    /* Nothing to do. */
    if (itEnd - itStart <= 1)
      continue;
    for (it = itStart; it != itEnd; it++) {
      if (it->stat != c_Unknown)
        continue;
      for (j = 0; j < 2; j++) {
        clust[j][0] = it->hit->getStrip(j + 1);
        clust[j][1] = clust[j][0];
      }
      hit2d = NULL;
      do {
        add = false;
        for (it2 = itStart; it2 != itEnd; it2++) {
          if (it2 == it)
            continue;
          if (it->stat != c_Unknown)
            continue;
          for (j = 0; j < 2; j++)
            s[j] = it2->hit->getStrip(j + 1);
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
          if (hit2d == NULL) {
            hit2d = new EKLMHit2d(*(it->hit));
            hits2d.push_back(hit2d);
          }
          e = hit2d->getEDep();
          de = it2->hit->getEDep();
          hit2d->setEDep(e + de);
          hit2d->setTime(std::min(hit2d->getTime(), it2->hit->getTime()));
          hit2d->setCrossPoint((hit2d->getCrossPoint() * e +
                                it2->hit->getCrossPoint() * de) / (e + de));
        }
      } while (add);
    }
  }
  /* Erase merged hits. */
  for (it = hits.begin(); it != hits.end(); it++) {
    if (it->stat == c_Merged) {
      hits.erase(it);
      it--;
    }
  }
}

/**
 * Associated hit finder.
 * @param[in] hit  This hit.
 * @param[in,out] hits All hits (including this).
 * @param[out] hitPos Estimated hit position.
 */
static std::vector<EKLMHit2d*>
findAssociatedHits(std::vector<struct HitData>::iterator hit,
                   std::vector<struct HitData> &hits,
                   HepGeom::Point3D<double> &hitPos)
{
  std::vector<EKLMHit2d*> res;
  std::vector<struct HitData>::iterator it;
  /**
   * Initially fill the result with the hit in question.
   */
  res.push_back(hit->hit);
  hitPos = hit->hit->getCrossPoint();
  /**
   * Collect other hits.
   */
  for (it = hits.begin(); it != hits.end(); it++) {
    if (it == hit)
      continue;
    if (it->stat != c_Unknown)
      continue;
    if (it->hit->getCrossPoint().angle(hitPos) < 0.05) {
      it->stat = c_Cluster;
      res.push_back(it->hit);
    }
  }
  /**
   * If no other hits found, clear the result.
   * Set the status of hit.
   */
  if (res.size() == 1) {
    res.clear();
    hit->stat = c_Isolated;
  } else
    hit->stat = c_Cluster;
  return res;
}

EKLMK0LReconstructorModule::EKLMK0LReconstructorModule() : Module()
{
  setDescription("EKLM K0L reconstruction module.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
}

EKLMK0LReconstructorModule::~EKLMK0LReconstructorModule()
{
}

void EKLMK0LReconstructorModule::initialize()
{
  StoreArray<EKLMK0L>::registerPersistent();
}

void EKLMK0LReconstructorModule::beginRun()
{
}

void EKLMK0LReconstructorModule::event()
{
  int i;
  int n;
  StoreArray<EKLMHit2d> hits2d;
  StoreArray<EKLMK0L> k0lArray;
  struct HitData dat;
  std::vector<EKLMHit2d*> new2dHits;
  std::vector<EKLMHit2d*> cluster;
  std::vector<EKLMHit2d*>::iterator itClust;
  HepGeom::Point3D<double> hitPos;
  float mt, t;
  EKLMK0L* k0l;
  /* Hits in endcap, index is (number of endcap - 1). */
  std::vector<struct HitData> endcap[2];
  std::vector<struct HitData>::iterator it;
  dat.stat = c_Unknown;
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
    merge2dClusters(endcap[i], new2dHits);
    for (itClust = new2dHits.begin(); itClust != new2dHits.end(); itClust++) {
      dat.hit = *itClust;
      endcap[i].push_back(dat);
    }
    /* Sort by layer again. */
    sort(endcap[i].begin(), endcap[i].end(), layerLessThan);
    /* Search for clusters. */
    for (it = endcap[i].begin(); it != endcap[i].end(); it++) {
      if ((*it).stat != c_Unknown)
        continue;
      cluster = findAssociatedHits(it, endcap[i], hitPos);
      /* A cluster is found, write new K0L. */
      if (!cluster.empty()) {
        k0l = new(k0lArray.nextFreeAddress()) EKLMK0L();
        k0l->setHitPosition(hitPos);
        mt = (*cluster.begin())->getTime();
        for (itClust = cluster.begin() + 1; itClust != cluster.end();
             itClust++) {
          t = (*itClust)->getTime();
          if (t < mt)
            mt = t;
        }
        k0l->setTime(mt);
        cluster.clear();
      }
    }
    /* Free memory. */
    for (itClust = new2dHits.begin(); itClust != new2dHits.end(); itClust++)
      delete *itClust;
    new2dHits.clear();
  }
}

void EKLMK0LReconstructorModule::endRun()
{
}

void EKLMK0LReconstructorModule::terminate()
{
}

