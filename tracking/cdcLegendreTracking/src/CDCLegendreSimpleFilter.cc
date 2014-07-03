/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <tracking/cdcLegendreTracking/CDCLegendreSimpleFilter.h>
//#include <tracking/cdcLegendreTracking/filter/CDCLegendreFilterCandidate.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>

#include <cstdlib>
#include <list>
#include <map>

using namespace std;

using namespace Belle2;



double CDCLegendreSimpleFilter::getAssigmentProbability(CDCLegendreTrackHit* hit, CDCLegendreTrackCandidate* track)
{

  double prob = 0;

  double x0_track = cos(track->getTheta()) / track->getR() + track->getReferencePoint().X();
  double y0_track = sin(track->getTheta()) / track->getR() + track->getReferencePoint().Y();
  double R = fabs(1. / track->getR());


  double x0_hit = hit->getOriginalWirePosition().X();
  double y0_hit = hit->getOriginalWirePosition().Y();
  double dist = fabs(fabs(R - sqrt((x0_track - x0_hit) * (x0_track - x0_hit) + (y0_track - y0_hit) * (y0_track - y0_hit))) - hit->getDriftTime());



  prob = exp(-1. * dist / m_distFactor);

  return prob;
}



void CDCLegendreSimpleFilter::processTracks(std::list<CDCLegendreTrackCandidate*>& m_trackList)
{

  /*
  double total_prob = 1.;
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

  int ii = 0;

  B2INFO("NCands = " << m_trackList.size());

  for (CDCLegendreTrackCandidate * cand : m_trackList) {
    ii++;
    B2INFO("ii = " << ii);
    B2INFO("Processing: Cand hits vector size = " << cand->getTrackHits().size());
    B2INFO("Processing: Cand R = " << cand->getR());

    if (cand->getTrackHits().size() == 0)continue;

    for (CDCLegendreTrackHit * hit : cand->getTrackHits()) {
      double prob = getAssigmentProbability(hit, cand);;

//      if(prob<m_minProb){

      double bestHitProb = prob;
      CDCLegendreTrackCandidate* BestCandidate = NULL;

      for (CDCLegendreTrackCandidate * candInner : m_trackList) {
        if (candInner == cand) continue;
        double probTemp = getAssigmentProbability(hit, candInner);

//        int curve_sign = hit->getCurvatureSignWrt(cos(candInner->getTheta()) / candInner->getR(), sin(candInner->getTheta()) / candInner->getR());

        if (probTemp > bestHitProb /*&& curve_sign == candInner->getCharge()*/) {
          BestCandidate = candInner;
          bestHitProb = probTemp;
        }
      }

      if (bestHitProb > prob) {
//          filterCandidate->getLegendreCandidate()->removeHit(hit);
        BestCandidate->addHit(hit);
        cand->removeHit(hit);
//        B2INFO("Hit has been reassigned.");
      }


//      }

    }

  }


}

/*
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

          if (probTemp > otherProbs && curve_sign == candInner->getCharge()) {

            otherProbs = probTemp;
          }
        }

        if (otherProbs > prob || otherProbs > 0.6) {
          removeHit = true;
        }
      }

      if (removeHit) {
        cand->removeHit(hit);
//        B2INFO("Hit has been removed!");
      }


    }

  }



}

*/


void CDCLegendreSimpleFilter::appenUnusedHits(std::list<CDCLegendreTrackCandidate*>& m_trackList, std::vector<CDCLegendreTrackHit*> AxialHitList)
{
  for (CDCLegendreTrackHit * hit : AxialHitList) {
    if (hit->isUsed() != CDCLegendreTrackHit::not_used) continue;
    double bestHitProb = 0;
    CDCLegendreTrackCandidate* BestCandidate;

    for (CDCLegendreTrackCandidate * cand : m_trackList) {
      double probTemp = getAssigmentProbability(hit, cand);

//      int curve_sign = hit->getCurvatureSignWrt(cos(cand->getTheta()) / cand->getR(), sin(cand->getTheta()) / cand->getR());

      if (probTemp > bestHitProb /*&& curve_sign == cand->getCharge()*/) {
        BestCandidate = cand;
        bestHitProb = probTemp;
      }
    }

    if (bestHitProb > 0.8) {
      BestCandidate->addHit(hit);
      hit->setUsed(CDCLegendreTrackHit::used_in_track);
//      B2INFO("Unused hit has been assigned.");
    }

  }
}

