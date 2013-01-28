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
  int i;
  int s[2];
  int layer;
  int clust[2][2];
  bool add;
  double e, de;
  std::vector<EKLMHit2d*> res;
  std::vector<EKLMHit2d*>::iterator itClust;
  std::vector<struct HitData>::iterator it, it2, it3;
  /**
   * Initially fill the result with the hit in question.
   */
  res.push_back(hit->hit);
  /**
   * Find all the hits from the same layer.
   * Hit vector is already sorted by layer.
   */
  layer = hit->hit->getLayer();
  it2 = hit;
  it3 = hit;
  it = hit;
  while (it != hits.begin()) {
    it--;
    if (it->hit->getLayer() != layer)
      break;
    it2 = it;
  }
  it = hit;
  while (1) {
    it++;
    if (it == hits.end())
      break;
    if (it->hit->getLayer() != layer)
      break;
    it3 = it;
  }
  /**
   * Search for a 2d cluster (hits in adjacent strips).
   */
  for (i = 0; i < 2; i++) {
    clust[i][0] = hit->hit->getStrip(i + 1);
    clust[i][1] = clust[i][0];
  }
  do {
    add = false;
    for (it = it2; it != it3; it++) {
      if (it == hit)
        continue;
      if (it->stat != c_Unknown)
        continue;
      for (i = 0; i < 2; i++)
        s[i] = it->hit->getStrip(i + 1);
      if (s[0] > clust[0][1] + 1 || s[0] < clust[0][0] - 1 ||
          s[1] > clust[1][1] + 1 || s[1] < clust[1][0] - 1)
        continue;
      if ((s[0] > clust[0][1] || s[0] < clust[0][0]) &&
          (s[1] > clust[1][1] || s[1] < clust[1][0]))
        continue;
      for (i = 0; i < 2; i++) {
        if (s[i] > clust[i][1])
          clust[i][1] = s[i];
        if (s[i] < clust[i][0])
          clust[i][0] = s[i];
      }
      add = true;
      it->stat = c_Cluster;
      res.push_back(it->hit);
    }
  } while (add);
  /**
   * Get hit position as weighed average of cluster hit positions.
   */
  e = 0;
  hitPos = HepGeom::Point3D<double>(0., 0., 0.);
  for (itClust = res.begin(); itClust != res.end(); itClust++) {
    de = (*itClust)->getEDep();
    e = e + de;
    hitPos = hitPos + de * (*itClust)->getCrossPoint();
  }
  hitPos = hitPos / e;
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
  std::vector<EKLMHit2d*> cluster;
  HepGeom::Point3D<double> hitPos;
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
    /* Analyse hits. */
    for (it = endcap[i].begin(); it != endcap[i].end(); it++) {
      if ((*it).stat == c_Unknown)
        cluster = findAssociatedHits(it, endcap[i], hitPos);
      /* A cluster is found, write new K0L. */
      if (!cluster.empty()) {
        k0l = new(k0lArray.nextFreeAddress()) EKLMK0L();
        k0l->setHitPosition(hitPos);
        cluster.clear();
      }
    }
  }
}

void EKLMK0LReconstructorModule::endRun()
{
}

void EKLMK0LReconstructorModule::terminate()
{
}

