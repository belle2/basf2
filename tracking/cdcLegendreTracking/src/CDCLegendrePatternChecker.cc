#include <tracking/cdcLegendreTracking/CDCLegendrePatternChecker.h>


using namespace std;
using namespace Belle2;
using namespace TrackFinderCDCLegendre;

#define SQR(x) ((x)*(x)) //we will use it in least squares fit


void PatternChecker::checkCurler(
  std::pair<std::vector<TrackHit*>, std::pair<double, double> >* track)
{
  HitPatternCDC pattern_P, pattern_N;           //patterns for first check
  HitPatternCDC pattern_lead, pattern_sublead;  //patterns for actual checks
  int charge;
  int nhits_P = 0, nhits_N = 0;
  double radius = 1. / track->second.second;
  double theta = 1. / track->second.first;
  std::pair<double, double> center = std::make_pair(cos(theta) / radius, sin(theta) / radius);


  // Make two patterns: for "positive" and "negative" parts of track (curler)
  for (TrackHit * hit : track->first) {
    if (hit->getCurvatureSignWrt(center.first, center.second) == TrackCandidate::charge_positive) {
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
      [ &center, &charge, &ii](TrackHit * hit) {
      if ((hit->getSuperlayerId() == ii) && (hit->getCurvatureSignWrt(center.first, center.second) == TrackCandidate::charge_positive)) {
        hit->setHitUsage(TrackHit::bad);
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
      [ &center, &charge, &ii](TrackHit * hit) {
      if ((hit->getSuperlayerId() == ii) && (hit->getCurvatureSignWrt(center.first, center.second) == TrackCandidate::charge_negative)) {
        hit->setHitUsage(TrackHit::bad);
        return true;
      } else {
        return false;
      }
    }), track->first.end());
  }

  // After cleanup leaving only those pattern which has most of hits

  if (pattern_P.getNHits() > pattern_N.getNHits()) {
    track->first.erase(std::remove_if(track->first.begin(), track->first.end(),
    [&center, &charge](TrackHit * hit) {
      if (hit->getCurvatureSignWrt(center.first, center.second) != TrackCandidate::charge_positive) {
        hit->setHitUsage(TrackHit::bad);
        return true;
      } else {
        return false;
      }
    }), track->first.end());
  } else {
    track->first.erase(std::remove_if(track->first.begin(), track->first.end(),
    [&center, &charge](TrackHit * hit) {
      if (hit->getCurvatureSignWrt(center.first, center.second) != TrackCandidate::charge_negative) {
        hit->setHitUsage(TrackHit::bad);
        return true;
      } else {
        return false;
      }
    }), track->first.end());
  }

  //TODO: check other conditions

}


bool PatternChecker::checkCandidate(TrackCandidate* track, int minNHitsSLayer)
{
  HitPatternCDC trackHitPattern = track->getHitPattern();
  B2DEBUG(100, "Initial pattern: " << trackHitPattern.getBitSet());

  // The variable is true if we are following part of pattern with number of hits >= 3
  // Set it to false when nhits in sLayer < 3
  // Allows to find tracklets and longest path of hits
  bool trackHitsFollowing = false;

  //here we store all patterns of hits created during procedure
  std::vector<HitPatternCDC> patterns;
  std::bitset<64> temp_pattern;
  temp_pattern.reset();

  std::bitset<64> sLayerZer(static_cast<std::string>("0000000000000000000000000000000000000000000000000000000011111111"));
  std::bitset<64> sLayerOne(static_cast<std::string>("0000000100000000000000000000000000000000000000000011111100000000"));
  std::bitset<64> sLayerTwo(static_cast<std::string>("0000001000000000000000000000000000000000000011111100000000000000"));
  std::bitset<64> sLayerThr(static_cast<std::string>("0000010000000000000000000000000000000011111100000000000000000000"));
  std::bitset<64> sLayerFou(static_cast<std::string>("0000100000000000000000000000000011111100000000000000000000000000"));
  std::bitset<64> sLayerFiv(static_cast<std::string>("0001000000000000000000000011111100000000000000000000000000000000"));
  std::bitset<64> sLayerSix(static_cast<std::string>("0010000000000000000011111100000000000000000000000000000000000000"));
  std::bitset<64> sLayerSev(static_cast<std::string>("0100000000000011111100000000000000000000000000000000000000000000"));
  std::bitset<64> sLayerEig(static_cast<std::string>("1000000011111100000000000000000000000000000000000000000000000000"));
  const std::bitset<64> s_sLayerMasks[9] = {sLayerZer, sLayerOne, sLayerTwo, sLayerThr, sLayerFou,
                                            sLayerFiv, sLayerSix, sLayerSev, sLayerEig
                                           };
  for (int sLayer = 8; sLayer >= 0; sLayer -= 2) {
    unsigned short nHits = trackHitPattern.getSLayerNHits(sLayer);
    if (nHits < minNHitsSLayer) {
      trackHitPattern.resetSLayer(sLayer);
      if (trackHitsFollowing) {
        patterns.emplace_back(temp_pattern.to_ulong());
        temp_pattern.reset();
      }
      trackHitsFollowing = false;
    } else {
      trackHitsFollowing = true;
      temp_pattern = temp_pattern | (trackHitPattern.getBitSet() & s_sLayerMasks[sLayer]);
    }

  }

  if (trackHitsFollowing)
    patterns.emplace_back(temp_pattern.to_ulong());

  if (patterns.size() == 0) return false;

  B2DEBUG(100, "Cleaned pattern: " << trackHitPattern.getBitSet());
  B2DEBUG(100, "Number of found tracklets: " << patterns.size());

  if (patterns.size() > 1) B2DEBUG(100, "Track has been splitted!");

  int nHits_longest = 0;
  unsigned int index_longest = 0;
  int ii = 0;

  for (HitPatternCDC & pattern : patterns) {
    B2DEBUG(100, "Tracklet pattern: " << pattern.getBitSet());
    int nHits_current = 0;
    for (int sLayer = 8; sLayer >= 0; sLayer -= 2) {
      nHits_current += pattern.getSLayerNHits(sLayer);
    }
    if (nHits_current > nHits_longest) {
      nHits_longest = nHits_current;
      index_longest = ii;
    }
    ii++;
  }

  std::vector<TrackHit*> trackletHits;
  trackletHits.clear();
  for (unsigned int ii = 0; ii < patterns.size(); ii++) {
    if (ii == index_longest) continue;
    for (TrackHit * hit : track->getTrackHits()) {
      if (patterns[ii].hasLayer(hit->getLayerId())) trackletHits.push_back(hit);
    }
    if (trackletHits.size() > 0) m_cdcLegendreTrackCreator->createLegendreTracklet(trackletHits)->setCandidateType(TrackCandidate::tracklet);
    trackletHits.clear();
  }

  track->getTrackHits().erase(std::remove_if(track->getTrackHits().begin(), track->getTrackHits().end(), [&patterns, &index_longest](TrackHit * hit) {return not patterns[index_longest].hasLayer(hit->getLayerId());}), track->getTrackHits().end());

  track->setCandidateType(TrackCandidate::tracklet);

  return true;

}

/*
int CDCLegendreTrackCandidate::getCandidateType()
{
  int type = 0;
  int innermostAxialLayer = getInnermostSLayer();
  int outermostAxialLayer = getOutermostSLayer();

  B2DEBUG(100, "innermost: " <<  innermostAxialLayer << "; outermost:" << outermostAxialLayer);
  //check whether track is "fullTrack"
  if ((innermostAxialLayer == 0) && (outermostAxialLayer == 8)) { //hardcoded since CDC will not change
    type = fullTrack;
  } else
    //check whether track is "curler"
    if ((innermostAxialLayer == 0) && (m_hitPattern.getSLayerNHits(0) > 2)) {
//    for(int layer = 2; layer < 8; layer+=2)
//      if(hitPatternAxial.getNSLayer(layer) > 3)
      type = curlerTrack;
    } else
      //check whether track is "tracklet"
      if ((innermostAxialLayer != 0) || (m_hitPattern.getSLayerNHits(0) < 2)) {
        type = tracklet;
      }

  return type;
}
*/
int PatternChecker::getMaxSLayer(HitPatternCDC* pattern)
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


int PatternChecker::getMinSLayer(HitPatternCDC* pattern)
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


void PatternChecker::clearBadHits(std::pair<std::vector<TrackHit*>, std::pair<double, double> >* track,
                                  std::pair<double, double>& ref_point)
{
  track->first.erase(std::remove_if(track->first.begin(), track->first.end(),
  [&track, &ref_point](TrackHit * hit) {
    double R = fabs(1. / track->second.second);
    double x0_track = cos(track->second.first) / track->second.second + ref_point.first;
    double y0_track = sin(track->second.first) / track->second.second + ref_point.second;
    double x0_hit = hit->getOriginalWirePosition().X();
    double y0_hit = hit->getOriginalWirePosition().Y();
    double dist = fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftLength();
    if (dist > hit->getDriftLength() / 2.) {
      hit->setHitUsage(TrackHit::bad);
      return true;
    } else {
      return false;
    }
  }), track->first.end());



}


void PatternChecker::checkPattern(HitPatternCDC* /*pattern*/)
{
//  double minSLayer, maxSLayer;
//  minSLayer = getMinSLayer(pattern);
//  maxSLayer = getMaxSLayer(pattern);
}

