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
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>

#include <eklm/dataobjects/EKLMHit2d.h>
#include <eklm/dataobjects/EKLMK0L.h>
#include <eklm/geometry/EKLMObjectNumbers.h>
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
static void merge2dClusters(std::vector<struct HitData>& hits,
                            std::vector<EKLMHit2d*>& hits2d)
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
          hit2d->setGlobalPosition((hit2d->getGlobalPosition() * e +
                                    it2->hit->getGlobalPosition() * de) / (e + de));
        }
      } while (add);
    }
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
static void findAssociatedHits(std::vector<struct HitData>::iterator hit,
                               std::vector<struct HitData>& hits)
{
  int i;
  int layerHits[14], nLayers;
  float e, de;
  std::vector<EKLMHit2d*> cluster;
  std::vector<EKLMHit2d*>::iterator itClust;
  std::vector<struct HitData>::iterator it;
  HepGeom::Point3D<double> hitPos;
  CLHEP::Hep3Vector p;
  float mt, t, m;
  double v;
  EKLMK0L* k0l;
  /* Initially fill the cluster with the hit in question. */
  cluster.push_back(hit->hit);
  hitPos = hit->hit->getGlobalPosition();
  /* Collect other hits. */
  for (it = hits.begin(); it != hits.end(); it++) {
    if (it == hit)
      continue;
    if (it->stat != c_Unknown)
      continue;
    if (it->hit->getGlobalPosition().angle(hitPos) < 0.1) {
      it->stat = c_Cluster;
      cluster.push_back(it->hit);
    }
  }
  /* Get minimal time. */
  mt = (*cluster.begin())->getTime();
  for (itClust = cluster.begin() + 1; itClust != cluster.end(); itClust++) {
    t = (*itClust)->getTime();
    if (t < mt) {
      mt = t;
    }
  }
  /* Get number of layers  and number of the innermost layer with hits */
  for (i = 0; i < 14; i++)
    layerHits[i] = 0;
  for (itClust = cluster.begin(); itClust != cluster.end(); itClust++)
    layerHits[(*itClust)->getLayer() - 1]++;
  nLayers = 0;
  int   nInnermostLayer = 0;
  for (i = 13; i >= 0; i--)
    if (layerHits[i] > 0) {
      nInnermostLayer = i;
      nLayers++;
    }
  /* Get hit position as weighed average of cluster hit positions. */
  e = 0;
  hitPos = HepGeom::Point3D<double>(0., 0., 0.);
  for (itClust = cluster.begin(); itClust != cluster.end(); itClust++) {
    if ((*itClust)->getLayer() != hit->hit->getLayer())
      continue;
    de = (*itClust)->getEDep();
    e = e + de;
    hitPos = hitPos + de * (*itClust)->getGlobalPosition();
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

  /* Fill EKLMK0L. */
  StoreArray<EKLMK0L> k0lArray;
  k0l = k0lArray.appendNew();
  k0l->setGlobalPosition(hitPos);
  k0l->setTime(mt);
  k0l->setLayers(nLayers);
  CLHEP::HepLorentzVector momentum4 = CLHEP::HepLorentzVector(p, sqrt(p.mag2() + m * m));
  k0l->setMomentum(momentum4);
  k0l->setInnermostLayer(nInnermostLayer);

  /* Fill cluster-hit relation array */
  StoreArray<EKLMHit2d> hits2d;
  RelationArray EKLMClustersToHits2d(k0lArray, hits2d);
  for (const EKLMHit2d * eklmHit : cluster) {
    k0l->addRelationTo(eklmHit);
  }

}








void EKLMK0LReconstructorModule::fillMdstDataobjects()
{


  /* Fill MDST dataobject */
  StoreArray<EKLMK0L> k0lArray;
  StoreArray<KLMCluster> klmClusterMDST;

  for (int i = 0; i < k0lArray.getEntries(); i++) {
    HepGeom::Point3D<double> pos = k0lArray[i]->getGlobalPosition();
    CLHEP::HepLorentzVector momentum = k0lArray[i]->getMomentum();
    new(klmClusterMDST.nextFreeAddress()) KLMCluster(
      pos.x(), pos.y(), pos.z(),
      k0lArray[i]->getTime(),
      k0lArray[i]->getLayers(),
      k0lArray[i]->getInnermostLayer(),
      momentum.x(), momentum.y(), momentum.z()
    );
  }

}
/*
  bool EKLMK0LReconstructorModule::hasAssociatedTrack(genfit::Track &gfTrack)
  {

  genfit::AbsTrackRep* gfTrackRep = gfTrack->getCardinalRep();
  const genfit::TrackPoint* firstPoint = gfTrack->getPointWithMeasurementAndFitterInfo(0, gfTrackRep);
  const genfit::AbsFitterInfo* firstFitterInfo = firstPoint->getFitterInfo(gfTrackRep);
  const genfit::MeasuredStateOnPlane& firstState = firstFitterInfo->getFittedState(true);
  TVector3 firstPosition, firstMomentum;
  TMatrixDSym firstCov(6);
  gfTrackRep->getPosMomCov(firstState, firstPosition, firstMomentum, firstCov);
  int charge = gfTrackRep->getCharge(firstState);
  TVector3 firstDirection(firstMomentum.Unit());

}
*/

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
  RelationArray::registerPersistent<EKLMK0L, EKLMHit2d>();
  StoreArray<EKLMK0L>::registerPersistent();
  StoreArray<KLMCluster>::registerPersistent();
}

void EKLMK0LReconstructorModule::beginRun()
{
}

void EKLMK0LReconstructorModule::event()
{
  int i;
  int n;
  StoreArray<EKLMHit2d> hits2d;
  struct HitData dat;
  std::vector<EKLMHit2d*> new2dHits;
  std::vector<EKLMHit2d*>::iterator itClust;
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
      findAssociatedHits(it, endcap[i]);
    }

    /* Free memory. */
    for (itClust = new2dHits.begin(); itClust != new2dHits.end(); itClust++)
      delete *itClust;
    new2dHits.clear();
  }
  /* create MDST objects */
  fillMdstDataobjects();

}

void EKLMK0LReconstructorModule::endRun()
{
}

void EKLMK0LReconstructorModule::terminate()
{
}

