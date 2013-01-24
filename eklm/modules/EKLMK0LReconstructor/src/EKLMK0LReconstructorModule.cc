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
 * @param[in] hits All hits (including this).
 */
static std::vector<EKLMHit2d*>
findAssociatedHits(std::vector<struct HitData>::iterator hit,
                   std::vector<struct HitData> &hits)
{
  EKLMHit2d* h, *h0;
  std::vector<EKLMHit2d*> res;
  std::vector<struct HitData>::iterator it;
  h0 = hit->hit;
  for (it = hits.begin(); it != hits.end(); it++) {
    if (it == hit)
      continue;
    if (it->stat != c_Unknown)
      continue;
    h = it->hit;
    if (h->getCrossPoint().angle(h0->getCrossPoint()) < 0.05) {
      if (it->stat == c_Unknown) {
        res.push_back(h0);
        it->stat = c_Cluster;
      }
      res.push_back(h);
      hit->stat = c_Cluster;
    }
  }
  if (hit->stat == c_Unknown)
    hit->stat = c_Isolated;
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
        cluster = findAssociatedHits(it, endcap[i]);
      /* A cluster is found, write new K0L. */
      if (!cluster.empty()) {
        k0l = new(k0lArray.nextFreeAddress()) EKLMK0L();
        k0l->setHitPosition((*it).hit->getCrossPoint());
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

