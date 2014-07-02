/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <tracking/cdcLegendreTracking/filter/CDCLegendreSimpleFilter.h>
#include <tracking/cdcLegendreTracking/filter/CDCLegendreFilterCandidate.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>

#include <cstdlib>
#include <list>
#include <map>

using namespace std;

using namespace Belle2;

CDCLegendreSimpleFilter::CDCLegendreSimpleFilter()
{

}

CDCLegendreSimpleFilter::~CDCLegendreSimpleFilter()
{
  for (CDCLegendreFilterCandidate * filterTrack : m_cdcLegendreFilterCandidateList) {
    delete filterTrack;
  }
  m_cdcLegendreFilterCandidateList.clear();

}

double CDCLegendreSimpleFilter::getAssigmentProbability(CDCLegendreTrackHit* hit, CDCLegendreTrackCandidate* track)
{

  double prob = 0;

  double x0_track = cos(track->getTheta()) / track->getR() + track->getReferencePoint().X();
  double y0_track = sin(track->getTheta()) / track->getR() + track->getReferencePoint().Y();
  double R = fabs(1. / track->getR());
  double dist_min = 999;
  double x_pos, y_pos;


  double x0_hit = hit->getOriginalWirePosition().X();
  double y0_hit = hit->getOriginalWirePosition().Y();
  double dist = fabs(fabs(R - sqrt((x0_track - x0_hit) * (x0_track - x0_hit) + (y0_track - y0_hit) * (y0_track - y0_hit))) - hit->getDriftTime());



  prob = exp(-1. * dist / m_distFactor);

  return prob;
}


void CDCLegendreSimpleFilter::createFilterCandidateList(std::list<CDCLegendreTrackCandidate*> trackList)
{
  m_trackList = trackList;

  for (CDCLegendreTrackCandidate * cand : m_trackList) {
    CDCLegendreFilterCandidate* filterCandidate = new CDCLegendreFilterCandidate(cand);
    for (CDCLegendreTrackHit * hit : cand->getTrackHits()) {
      double prob = getAssigmentProbability(hit, cand);
      filterCandidate->addHit(hit, prob);
    }
    m_cdcLegendreFilterCandidateList.push_back(filterCandidate);
  }

}


void CDCLegendreSimpleFilter::processTracks()
{
  double total_prob = 1.;

  /*
  for (CDCLegendreFilterCandidate* filterCandidate : m_cdcLegendreFilterCandidateList) {

    for(auto& hitMap: filterCandidate->getHitsMap()){
      CDCLegendreTrackHit* hit = hitMap.first;
      double prob = hitMap.second;

  //     if(prob<m_minProb){

        double bestHitProb = prob;
        CDCLegendreTrackCandidate* BestCandidate;

        for (CDCLegendreTrackCandidate * cand : m_trackList) {
          double probTemp = getAssigmentProbability(hit, cand);

          int curve_sign = hit->getCurvatureSignWrt(cos(cand->getTheta()) / cand->getR(), sin(cand->getTheta()) / cand->getR());

          if(probTemp > bestHitProb && curve_sign == cand->getCharge()) {
            BestCandidate = cand;
            bestHitProb = probTemp;
          }
        }

        if(bestHitProb > prob) {
          filterCandidate->getLegendreCandidate()->removeHit(hit);
          BestCandidate->addHit(hit);
  //         filterCandidate->removeHit(hit);
          B2INFO("Hit has been reassigned.");
        }


  //     }

    }

  }

  */



  for (CDCLegendreTrackCandidate * cand : m_trackList) {

    for (CDCLegendreTrackHit * hit : cand->getTrackHits()) {
      double prob = getAssigmentProbability(hit, cand);;

//      if(prob<m_minProb){

      double bestHitProb = prob;
      CDCLegendreTrackCandidate* BestCandidate;

      for (CDCLegendreTrackCandidate * candInner : m_trackList) {
        if (candInner == cand) continue;
        double probTemp = getAssigmentProbability(hit, candInner);

        int curve_sign = hit->getCurvatureSignWrt(cos(candInner->getTheta()) / candInner->getR(), sin(candInner->getTheta()) / candInner->getR());

        if (probTemp > bestHitProb /*&& curve_sign == candInner->getCharge()*/) {
          BestCandidate = candInner;
          bestHitProb = probTemp;
        }
      }

      if (bestHitProb > prob) {
//          filterCandidate->getLegendreCandidate()->removeHit(hit);
        BestCandidate->addHit(hit);
        cand->removeHit(hit);
        B2INFO("Hit has been reassigned.");
      }


//      }

    }

  }


}


void CDCLegendreSimpleFilter::trackCore()
{

  for (CDCLegendreTrackCandidate * cand : m_trackList) {

//    if(cand->getTrackHits().size() < 10)

    for (CDCLegendreTrackHit * hit : cand->getTrackHits()) {
      bool removeHit = false;
      double prob = getAssigmentProbability(hit, cand);;

      if (prob < 0.9) {
        removeHit = true;
      } else {

        double otherProbs = 0;

        for (CDCLegendreTrackCandidate * candInner : m_trackList) {
          if (candInner == cand) continue;
          double probTemp = getAssigmentProbability(hit, candInner);

          if (probTemp > otherProbs /*&& curve_sign == candInner->getCharge()*/) {
            otherProbs = probTemp;
          }
        }

        if (otherProbs > prob || otherProbs > 0.6) {
          removeHit = true;
        }
      }

      if (removeHit) {
        cand->removeHit(hit);
        B2INFO("Hit has been removed!");
      }


    }

  }



}




void CDCLegendreSimpleFilter::appenUnusedHits(std::vector<CDCLegendreTrackHit*> AxialHitList)
{
  for (CDCLegendreTrackHit * hit : AxialHitList) {
    if (hit->isUsed() != CDCLegendreTrackHit::not_used) continue;
    double bestHitProb = 0;
    CDCLegendreTrackCandidate* BestCandidate;

    for (CDCLegendreTrackCandidate * cand : m_trackList) {
      double probTemp = getAssigmentProbability(hit, cand);

      int curve_sign = hit->getCurvatureSignWrt(cos(cand->getTheta()) / cand->getR(), sin(cand->getTheta()) / cand->getR());

      if (probTemp > bestHitProb /*&& curve_sign == cand->getCharge()*/) {
        BestCandidate = cand;
        bestHitProb = probTemp;
      }
    }

    if (bestHitProb > 0.8) {
      BestCandidate->addHit(hit);
      hit->setUsed(CDCLegendreTrackHit::used_in_track);
      B2INFO("Unused hit has been assigned.");
    }

  }
}

