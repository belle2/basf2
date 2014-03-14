#include <tracking/cdcLegendreTracking/CDCLegendrePatternChecker.h>

#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>

#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>

using namespace std;
using namespace Belle2;

#define SQR(x) ((x)*(x)) //we will use it in least squares fit


void CDCLegendrePatternChecker::checkCurler(
  std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* track)
{
  HitPatternCDC pattern_P, pattern_N;           //patterns for first check
  HitPatternCDC pattern_lead, pattern_sublead;  //patterns for actual checks
  int charge;
  int nhits_P = 0, nhits_N = 0;
  double radius = 1. / track->second.second;
  double theta = 1. / track->second.first;
  std::pair<double, double> center = std::make_pair(cos(theta) / radius, sin(theta) / radius);


  // Make two patterns: for "positive" and "negative" parts of track (curler)
  for (CDCLegendreTrackHit * hit : track->first) {
    if (hit->getCurvatureSignWrt(center.first, center.second) == CDCLegendreTrackCandidate::charge_positive) {
      pattern_P.setLayer(hit->getLayerId());
      nhits_P++;
    } else {
      pattern_N.setLayer(hit->getLayerId());
      nhits_N++;
    }
  }

  // Clear hits: if superlayer has 1 or 2 hits, those hits should be deleted

  for (int ii = 0; ii < getMaxSLayer(&pattern_P); ii++) {
    if (pattern_P.getSLayerNHits(ii) <= 2)
      track->first.erase(std::remove_if(track->first.begin(), track->first.end(),
      [&center, &charge, &ii](CDCLegendreTrackHit * hit) {
      if ((hit->getSuperlayerId() == ii) && (hit->getCurvatureSignWrt(center.first, center.second) == CDCLegendreTrackCandidate::charge_positive)) {
        hit->setUsed(CDCLegendreTrackHit::used_bad);
        return true;
      } else {
        return false;
      }
    }), track->first.end());
  }

  // Clear hits: if superlayer has 1 or 2 hits, those hits should be deleted

  for (int ii = 0; ii < getMaxSLayer(&pattern_N); ii++) {
    if (pattern_P.getSLayerNHits(ii) <= 2)
      track->first.erase(std::remove_if(track->first.begin(), track->first.end(),
      [&center, &charge, &ii](CDCLegendreTrackHit * hit) {
      if ((hit->getSuperlayerId() == ii) && (hit->getCurvatureSignWrt(center.first, center.second) == CDCLegendreTrackCandidate::charge_negative)) {
        hit->setUsed(CDCLegendreTrackHit::used_bad);
        return true;
      } else {
        return false;
      }
    }), track->first.end());
  }

  // After cleanup leaving only those pattern which has most of hits

  if (pattern_P.getNHits() > pattern_N.getNHits()) {
    track->first.erase(std::remove_if(track->first.begin(), track->first.end(),
    [&center, &charge](CDCLegendreTrackHit * hit) {
      if (hit->getCurvatureSignWrt(center.first, center.second) != CDCLegendreTrackCandidate::charge_positive) {
        hit->setUsed(CDCLegendreTrackHit::used_bad);
        return true;
      } else {
        return false;
      }
    }), track->first.end());
  } else {
    track->first.erase(std::remove_if(track->first.begin(), track->first.end(),
    [&center, &charge](CDCLegendreTrackHit * hit) {
      if (hit->getCurvatureSignWrt(center.first, center.second) != CDCLegendreTrackCandidate::charge_negative) {
        hit->setUsed(CDCLegendreTrackHit::used_bad);
        return true;
      } else {
        return false;
      }
    }), track->first.end());
  }

  //TODO: check other conditions

}


void CDCLegendrePatternChecker::checkCandidate(std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* track)
{
  if (track->second.second < m_rc) checkCurler(track);

}

int CDCLegendrePatternChecker::getMaxSLayer(HitPatternCDC* pattern)
{
  double outermostAxialSLayer;
  int maxSLayer = 8;
  do {
    if (pattern->hasSLayer(maxSLayer)) {
      outermostAxialSLayer = maxSLayer;
      break;
    }
    --maxSLayer;
  } while (maxSLayer >= 0);
  outermostAxialSLayer = maxSLayer + 1;

  return outermostAxialSLayer;

}


int CDCLegendrePatternChecker::getMinSLayer(HitPatternCDC* pattern)
{

  double innermostAxialSLayer;
  int minSLayer = 0;
  do {
    if (pattern->hasSLayer(minSLayer)) {
      innermostAxialSLayer = minSLayer;
      break;
    }
    ++minSLayer;
  } while (minSLayer <= 8);
  innermostAxialSLayer = minSLayer - 1;

  return innermostAxialSLayer;

}


void CDCLegendrePatternChecker::clearBadHits(std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* track,
                                             std::pair<double, double>& ref_point)
{
  track->first.erase(std::remove_if(track->first.begin(), track->first.end(),
  [&track, &ref_point](CDCLegendreTrackHit * hit) {
    double R = fabs(1. / track->second.second);
    double x0_track = cos(track->second.first) / track->second.second + ref_point.first;
    double y0_track = sin(track->second.first) / track->second.second + ref_point.second;
    double x0_hit = hit->getOriginalWirePosition().X();
    double y0_hit = hit->getOriginalWirePosition().Y();
    double dist = fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftTime();
    if (dist > hit->getDriftTime() / 2.) {
      hit->setUsed(CDCLegendreTrackHit::used_bad);
      return true;
    } else {
      return false;
    }
  }), track->first.end());



}


void CDCLegendrePatternChecker::checkPattern(HitPatternCDC* pattern, double radius)
{
  double minSLayer, maxSLayer;
  minSLayer = getMinSLayer(pattern);
  maxSLayer = getMaxSLayer(pattern);
}

