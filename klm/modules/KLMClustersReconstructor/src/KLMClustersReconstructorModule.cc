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
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>

/* ROOT headers. */
#include <Math/VectorUtil.h>

/* C++ headers. */
#include <algorithm>
#include <cmath>
#include <unordered_set>

using namespace Belle2;

REG_MODULE(KLMClustersReconstructor);

KLMClustersReconstructorModule::KLMClustersReconstructorModule() : Module(),
  m_PositionMode(c_FirstLayer),
  m_ClusterMode(c_AnyHit),
  m_RemoveOutlierHits(false),
  m_OutlierTrimAngle(0.18)
{
  setDescription("Unified BKLM/EKLM module for the reconstruction of KLMClusters.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("ClusteringAngle", m_ClusteringAngle, "Clustering angle (rad).",
           0.26);
  addParam("PositionMode", m_PositionModeString,
           "Vertex position mode: 'FullAverage', 'FirstLayer', 'FirstTwoLayers', "
           "or 'SuccessiveTwoLayers'.",
           std::string("FirstTwoLayers"));
  addParam("ClusterMode", m_ClusterModeString,
           "Clusterization mode ('AnyHit' or 'FirstHit').",
           std::string("AnyHit"));
  addParam("RemoveOutlierHits", m_RemoveOutlierHits,
           "If true, iteratively remove angular outliers after clustering "
           "(dropped hits re-enter the pool). If false, behavior matches the "
           "original algorithm.",
           false);
  addParam("OutlierTrimAngle", m_OutlierTrimAngle,
           "Used only if removeOutlierHits: floor (minimum) angular threshold (rad). "
           "Effective cut is max(OutlierTrimAngle, k * MAD(residuals)) with fixed k=3.",
           0.18);
}

KLMClustersReconstructorModule::~KLMClustersReconstructorModule()
{
}

void KLMClustersReconstructorModule::initialize()
{
  m_KLMClusters.registerInDataStore();
  m_Hit2ds.isRequired();
  m_KLMClusters.registerRelationTo(m_Hit2ds);
  if (m_PositionModeString == "FullAverage")
    m_PositionMode = c_FullAverage;
  else if (m_PositionModeString == "FirstLayer")
    m_PositionMode = c_FirstLayer;
  else if (m_PositionModeString == "FirstTwoLayers")
    m_PositionMode = c_FirstTwoLayers;
  else if (m_PositionModeString == "SuccessiveTwoLayers")
    m_PositionMode = c_SuccessiveTwoLayers;
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

static bool compareDistance(const KLMHit2d* hit1, const KLMHit2d* hit2)
{
  return hit1->getPosition().R() < hit2->getPosition().R();
}

static KLMHit2d* hitWithMinR(const std::vector<KLMHit2d*>& hits)
{
  KLMHit2d* best = hits[0];
  double bestR = best->getPosition().R();
  for (size_t k = 1; k < hits.size(); k++) {
    double r = hits[k]->getPosition().R();
    if (r < bestR) {
      bestR = r;
      best = hits[k];
    }
  }
  return best;
}

/* Unit vector from origin to the hit position (zero if hit is at origin). */
static ROOT::Math::XYZVector unitDirection(const ROOT::Math::XYZVector& v)
{
  double r = v.R();
  if (r <= 0)
    return ROOT::Math::XYZVector{0, 0, 0};
  return v * (1.0 / r);
}

/* Reference direction: innermost-R hit direction on first iteration,
 * otherwise the normalized sum of unit directions of the current inliers. */
static ROOT::Math::XYZVector referenceDirection(const std::vector<KLMHit2d*>& inliers,
                                                bool seedByInnermost)
{
  if (seedByInnermost)
    return unitDirection(hitWithMinR(inliers)->getPosition());
  ROOT::Math::XYZVector sum{0, 0, 0};
  for (KLMHit2d* h : inliers)
    sum = sum + unitDirection(h->getPosition());
  if (sum.R() <= 0)
    return unitDirection(hitWithMinR(inliers)->getPosition());
  return sum * (1.0 / sum.R());
}

/* Adaptive angular threshold: max(floor, k * MAD of angular residuals to ref). */
static double adaptiveThreshold(const ROOT::Math::XYZVector& ref,
                                const std::vector<KLMHit2d*>& hits,
                                double floorAngle, double madFactor)
{
  std::vector<double> residuals;
  residuals.reserve(hits.size());
  for (KLMHit2d* h : hits)
    residuals.push_back(ROOT::Math::VectorUtil::Angle(h->getPosition(), ref));
  std::vector<double> sorted = residuals;
  std::sort(sorted.begin(), sorted.end());
  double median = sorted[sorted.size() / 2];
  std::vector<double> absDev;
  absDev.reserve(residuals.size());
  for (double r : residuals)
    absDev.push_back(std::fabs(r - median));
  std::sort(absDev.begin(), absDev.end());
  double mad = absDev[absDev.size() / 2];
  return std::max(floorAngle, madFactor * mad);
}

void KLMClustersReconstructorModule::applyOutlierRemoval(std::vector<KLMHit2d*>& clusterHits,
                                                         std::vector<KLMHit2d*>& poolHits)
{
  /* Fixed tuning: standard k=3 for MAD scale; iteration cap (fixed-point exits early);
   * min inlier fraction 0.3 avoids over-trimming a cluster to fewer than 30% the hits. */
  constexpr double kMADFactor = 3.0;
  constexpr int kMaxOutlierIterations = 5;
  constexpr double kMinInlierFraction = 0.3;

  if (!m_RemoveOutlierHits || clusterHits.size() <= 1)
    return;

  const std::vector<KLMHit2d*> before = clusterHits;
  const std::size_t minInliers = std::max<std::size_t>(
                                   2, static_cast<std::size_t>(std::ceil(kMinInlierFraction * before.size())));

  std::vector<KLMHit2d*> inliers = before;
  for (int iter = 0; iter < kMaxOutlierIterations; iter++) {
    const ROOT::Math::XYZVector ref =
      referenceDirection(inliers, /*seedByInnermost=*/iter == 0);
    const double threshold =
      adaptiveThreshold(ref, before, m_OutlierTrimAngle, kMADFactor);
    std::vector<KLMHit2d*> next;
    next.reserve(before.size());
    for (KLMHit2d* h : before) {
      if (ROOT::Math::VectorUtil::Angle(h->getPosition(), ref) <= threshold)
        next.push_back(h);
    }
    if (next.size() < minInliers)
      break;
    if (next.size() == inliers.size()) {
      std::vector<KLMHit2d*> a = next, b = inliers;
      std::sort(a.begin(), a.end());
      std::sort(b.begin(), b.end());
      if (a == b) {
        inliers = std::move(next);
        break;
      }
    }
    inliers = std::move(next);
  }

  if (inliers.size() < minInliers)
    return;
  if (inliers.size() == before.size())
    return;
  if (inliers.empty())
    return;

  std::unordered_set<KLMHit2d*> inlierSet(inliers.begin(), inliers.end());
  std::vector<KLMHit2d*> outliers;
  outliers.reserve(before.size() - inliers.size());
  for (KLMHit2d* h : before) {
    if (inlierSet.find(h) == inlierSet.end())
      outliers.push_back(h);
  }
  clusterHits.swap(inliers);
  poolHits.insert(poolHits.end(), outliers.begin(), outliers.end());
  sort(poolHits.begin(), poolHits.end(), compareDistance);
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
  std::vector<KLMHit2d*> klmHit2ds, klmClusterHits;
  std::vector<KLMHit2d*>::iterator it, it0, it2;
  KLMCluster* klmCluster;
  layerHitsBKLM = new int[nLayersBKLM];
  layerHitsEKLM = new int[nLayersEKLM];
  /* Fill vector of 2d hits. */
  nHits = m_Hit2ds.getEntries();
  for (i = 0; i < nHits; i++) {
    if (m_Hit2ds[i]->isOutOfTime())
      continue;
    klmHit2ds.push_back(m_Hit2ds[i]);
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
            if (ROOT::Math::VectorUtil::Angle(
                  (*it)->getPosition(), (*it2)->getPosition()) <
                m_ClusteringAngle) {
              klmClusterHits.push_back(*it);
              it = klmHit2ds.erase(it);
              goto clusterFound;
            } else
              ++it2;
          }
          break;
        case c_FirstHit:
          if (ROOT::Math::VectorUtil::Angle(
                (*it)->getPosition(), (*it2)->getPosition()) <
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
    if (m_RemoveOutlierHits)
      applyOutlierRemoval(klmClusterHits, klmHit2ds);
    ROOT::Math::XYZVector clusterPosition{0, 0, 0};
    for (i = 0; i < nLayersBKLM; i++)
      layerHitsBKLM[i] = 0;
    for (i = 0; i < nLayersEKLM; i++)
      layerHitsEKLM[i] = 0;
    minTime = -1;
    /* Minimal time and per-layer hit counts. */
    for (it = klmClusterHits.begin(); it != klmClusterHits.end(); ++it) {
      if (minTime < 0 || (*it)->getTime() < minTime)
        minTime = (*it)->getTime();
      if ((*it)->getSubdetector() == KLMElementNumbers::c_BKLM)
        layerHitsBKLM[(*it)->getLayer() - 1]++;
      else
        layerHitsEKLM[(*it)->getLayer() - 1]++;
    }
    it0 = klmClusterHits.begin();
    nHits = 0;
    if (m_PositionMode == c_FullAverage) {
      for (it = klmClusterHits.begin(); it != klmClusterHits.end(); ++it) {
        clusterPosition = clusterPosition + (*it)->getPosition();
        nHits++;
      }
    } else {
      int selSubdet[2];
      int selLayer[2];
      int selCount = 0;
      if (m_PositionMode == c_FirstLayer) {
        selSubdet[0] = (*it0)->getSubdetector();
        selLayer[0] = (*it0)->getLayer();
        selCount = 1;
      } else if (m_PositionMode == c_FirstTwoLayers) {
        for (i = 0; i < nLayersBKLM && selCount < 2; i++) {
          if (layerHitsBKLM[i] > 0) {
            selSubdet[selCount] = KLMElementNumbers::c_BKLM;
            selLayer[selCount] = i + 1;
            selCount++;
          }
        }
        for (i = 0; i < nLayersEKLM && selCount < 2; i++) {
          if (layerHitsEKLM[i] > 0) {
            selSubdet[selCount] = KLMElementNumbers::c_EKLM;
            selLayer[selCount] = i + 1;
            selCount++;
          }
        }
        if (selCount == 0) {
          selSubdet[0] = (*it0)->getSubdetector();
          selLayer[0] = (*it0)->getLayer();
          selCount = 1;
        }
      } else if (m_PositionMode == c_SuccessiveTwoLayers) {
        bool foundPair = false;
        for (i = 0; i < nLayersBKLM - 1; i++) {
          if (layerHitsBKLM[i] > 0 && layerHitsBKLM[i + 1] > 0) {
            selSubdet[0] = KLMElementNumbers::c_BKLM;
            selLayer[0] = i + 1;
            selSubdet[1] = KLMElementNumbers::c_BKLM;
            selLayer[1] = i + 2;
            selCount = 2;
            foundPair = true;
            break;
          }
        }
        if (!foundPair) {
          for (i = 0; i < nLayersEKLM - 1; i++) {
            if (layerHitsEKLM[i] > 0 && layerHitsEKLM[i + 1] > 0) {
              selSubdet[0] = KLMElementNumbers::c_EKLM;
              selLayer[0] = i + 1;
              selSubdet[1] = KLMElementNumbers::c_EKLM;
              selLayer[1] = i + 2;
              selCount = 2;
              foundPair = true;
              break;
            }
          }
        }
        if (!foundPair) {
          selSubdet[0] = (*it0)->getSubdetector();
          selLayer[0] = (*it0)->getLayer();
          selCount = 1;
        }
      }
      for (it = klmClusterHits.begin(); it != klmClusterHits.end(); ++it) {
        int sd = (*it)->getSubdetector();
        int ly = (*it)->getLayer();
        bool use = false;
        for (i = 0; i < selCount; i++) {
          if (sd == selSubdet[i] && ly == selLayer[i]) {
            use = true;
            break;
          }
        }
        if (use) {
          clusterPosition = clusterPosition + (*it)->getPosition();
          nHits++;
        }
      }
      if (nHits == 0)
        B2FATAL("KLMClustersReconstructor: no hits for vertex position.");
    }
    clusterPosition = clusterPosition * (1.0 / nHits);
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
      v = clusterPosition.R() / minTime / Const::speedOfLight;
      if (v < 0.999999)
        p = mass * v / sqrt(1.0 - v * v);
      else
        p = 0;
    }*/
    klmCluster = m_KLMClusters.appendNew(
                   clusterPosition.X(), clusterPosition.Y(), clusterPosition.Z(), minTime, nLayers,
                   innermostLayer, p);
    for (it = klmClusterHits.begin(); it != klmClusterHits.end(); ++it)
      klmCluster->addRelationTo(*it);

    /* number of KLM digits in the cluster (BKLM via BKLMHit1d and EKLM directly). */
    int nDigits = 0;
    for (it = klmClusterHits.begin(); it != klmClusterHits.end(); ++it) {
      auto bklmhit1ds = (*it)->getRelationsTo<BKLMHit1d>();
      for (const auto& bklmhit1d : bklmhit1ds) {
        auto klmdigits = bklmhit1d.getRelationsTo<KLMDigit>();
        nDigits += klmdigits.size();
      }
      auto klmdigits = (*it)->getRelationsTo<KLMDigit>();
      nDigits += klmdigits.size();
    }
    klmCluster->setKLMnDigits(nDigits);
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

