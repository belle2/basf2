/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLegendreTracking/CDCLegendreTrackMerger.h>
#include <tracking/cdcLegendreTracking/CDCLegendreFastHough.h>

#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackFitter.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCreator.h>


#include <cdc/geometry/CDCGeometryPar.h>

#include <cmath>

#define SQR(x) ((x)*(x))

using namespace std;
using namespace Belle2;
using namespace CDC;

CDCLegendreTrackMerger::CDCLegendreTrackMerger(
  std::list<CDCLegendreTrackCandidate*>& trackList, std::list<CDCLegendreTrackCandidate*>& trackletList, std::list<CDCLegendreTrackCandidate*>& stereoTrackletList, CDCLegendreTrackFitter* cdcLegendreTrackFitter, CDCLegendreFastHough* cdcLegendreFastHough, CDCLegendreTrackCreator* cdcLegendreTrackCreator):
  m_trackList(trackList), m_trackletList(trackletList), m_stereoTrackletList(stereoTrackletList), m_cdcLegendreTrackFitter(cdcLegendreTrackFitter), m_cdcLegendreFastHough(cdcLegendreFastHough), m_cdcLegendreTrackCreator(cdcLegendreTrackCreator)
{

}

void CDCLegendreTrackMerger::mergeTracks(CDCLegendreTrackCandidate* cand1, CDCLegendreTrackCandidate* cand2)
{

  B2INFO("Track merging: chi2_1 = " << cand1->getChi2() << "; chi2_2 = " << cand2->getChi2() << "; new chi2 = " << tryToMergeAndFit(cand1, cand2));

  if (cand1 == cand2)return;

  cand1->setR(
    (cand1->getR() * cand1->getNHits() + cand2->getR() * cand2->getNHits())
    / (cand1->getNHits() + cand2->getNHits()));
  cand1->setTheta(
    (cand1->getTheta() * cand1->getNHits()
     + cand2->getTheta() * cand2->getNHits())
    / (cand1->getNHits() + cand2->getNHits()));

  for (CDCLegendreTrackHit * hit : cand2->getTrackHits()) {
    cand1->addHit(hit);
    hit->setUsed(CDCLegendreTrackHit::used_in_track);
  }


  m_trackList.remove(cand2);
  delete cand2;
  cand2 = NULL;

  /*  std::list<CDCLegendreTrackCandidate*>::iterator from = m_trackList.end();
  std::list<CDCLegendreTrackCandidate*>::iterator to = std::find(m_trackList.begin(), m_trackList.end(), cand2);

    CDCLegendreTrackCandidate* tmp1;
    CDCLegendreTrackCandidate* tmp2;
  if ( (*from) != cand2 )
  {
    tmp1 = *from;
    tmp2 = cand2;
    cand2 = tmp1;
    *from = tmp2;
  }


    node *tmp;

    tmp = a;
    a = b;
    b = tmp;

  std::list<CDCLegendreTrackCandidate*>::iterator cand_end = m_trackList.end();

  CDCLegendreTrackCandidate* temp = cand2;
  *i = *j;
  *j = temp;
  cout << (*i) << endl;
  cout << (*j) << endl;
  */
//  m_trackList.remove(cand2);
//  delete cand2;
// cand2 = NULL;
}

void CDCLegendreTrackMerger::mergeTracks(CDCLegendreTrackCandidate* cand1,
                                         const std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >& track,
                                         std::set<CDCLegendreTrackHit*>& /*hits_set*/)
{
  /*
    cand1->setR(
      (cand1->getR() * cand1->getNHits() + track.second.second * track.first.size())
      / (cand1->getNHits() + track.second.second * track.first.size()));
    cand1->setTheta(
      (cand1->getTheta() * cand1->getNHits()
       + track.second.first * track.second.second * track.first.size())
      / (cand1->getNHits() + track.second.second * track.first.size()));
  */
  for (CDCLegendreTrackHit * hit : track.first) {
    /*    cand1->addHit(hit);
        hit->setUsed(CDCLegendreTrackHit::used_in_track);
    */
    double R = fabs(1. / cand1->getR());
    double x0_track = cos(cand1->getTheta()) / cand1->getR() + cand1->getReferencePoint().X() ;
    double y0_track = sin(cand1->getTheta()) / cand1->getR() + cand1->getReferencePoint().Y();
    double x0_hit = hit->getOriginalWirePosition().X();
    double y0_hit = hit->getOriginalWirePosition().Y();
    double dist = SQR(fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftTime());
    if (hit->getDriftTime() / 2. < dist) {
      cand1->addHit(hit);
      hit->setUsed(CDCLegendreTrackHit::used_in_track);
//        hits_set.erase(hit);
    }
  }

//  m_trackList.remove(cand2);
//  delete cand2;
//  cand2 = NULL;
}



void CDCLegendreTrackMerger::MergeCurler()
{

  int ii = 0;

  B2DEBUG(100, "Merger: NCands = " << m_trackList.size());

  //loop over all candidates
  for (std::list<CDCLegendreTrackCandidate*>::iterator it1 =
         m_trackList.begin(); it1 != m_trackList.end(); ++it1) {
    CDCLegendreTrackCandidate* cand1 = *it1;

    m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand1);

    double chi2_best = 999;
    CDCLegendreTrackCandidate* candidateToMergeBest;

    //loop over remaining candidates
    std::list<CDCLegendreTrackCandidate*>::iterator it2 = std::next(it1);
    while (it2 != m_trackList.end()) {
      CDCLegendreTrackCandidate* cand2 = *it2;
      ++it2;

      m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand2);
      double chi2_temp = tryToMergeAndFit(cand1, cand2);

      if (chi2_best > chi2_temp) {
        candidateToMergeBest = cand2;
        chi2_best = chi2_temp;
      }

    }

    if (chi2_best < 1.) {
//      B2INFO("chi2_best: " << chi2_best << "; chi2_track1: " << chi2_track1);

      double chi2_best_reverse = 999.;
      CDCLegendreTrackCandidate* candidateToMergeBestReverse;
      for (CDCLegendreTrackCandidate * cand_temp : m_trackList) {
        if (cand_temp == cand1) continue;
        if (cand_temp == candidateToMergeBest) continue;

        m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand_temp);
        double chi2_temp = tryToMergeAndFit(candidateToMergeBest, cand_temp);

        if (chi2_best_reverse > chi2_temp) {
          candidateToMergeBestReverse = cand_temp;
          chi2_best_reverse = chi2_temp;
        }
      }

      if (chi2_best < chi2_best_reverse) {
        mergeTracks(cand1, candidateToMergeBest);
        m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand1);
        it1 = m_trackList.begin();
      } else {
        mergeTracks(candidateToMergeBest, candidateToMergeBestReverse);
        m_cdcLegendreTrackFitter->fitTrackCandidateFast(candidateToMergeBest);
        it1 = m_trackList.begin();
      }

//      mergeTracks(cand1, candidateToMergeBest);
//      m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand1);

      ii++;
      B2DEBUG(100, "ii = " << ii);

    }

    B2DEBUG(100, "Cand hits vector size = " << cand1->getTrackHits().size());
    B2DEBUG(100, "Cand R = " << cand1->getR());

  }

}


double CDCLegendreTrackMerger::tryToMergeAndFit(CDCLegendreTrackCandidate* cand1, CDCLegendreTrackCandidate* cand2)
{
  /*
    double chi2_track1 = cand1->getChi2() / cand1->getNHits();
    double x0_track1 = cos(cand1->getTheta()) / cand1->getR() + cand1->getReferencePoint().X();
    double y0_track1 = sin(cand1->getTheta()) / cand1->getR() + cand1->getReferencePoint().Y();

    double  chi2_track2 = cand2->getChi2() / cand2->getNHits();
    double x0_track2 = cos(cand2->getTheta()) / cand2->getR() + cand2->getReferencePoint().X();
    double y0_track2 = sin(cand2->getTheta()) / cand2->getR() + cand2->getReferencePoint().Y();
  */

  std::vector<CDCLegendreTrackHit*> c_list_temp_2;
  for (CDCLegendreTrackHit * hit : cand1->getTrackHits()) {
    c_list_temp_2.push_back(hit);
  }
  for (CDCLegendreTrackHit * hit : cand2->getTrackHits()) {
    c_list_temp_2.push_back(hit);
  }




  std::pair<double, double> ref_point = std::make_pair(0., 0.);
  std::pair<double, double> track_par = std::make_pair(-999, -999);
  double chi2_temp;
  chi2_temp = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp_2, track_par, ref_point);


  /*
   //можно сделать следование по треклетам - берем первый с той стороны, где больше хитов, потом добавляем второй, если норм - добавляем третий и т.д. если не норм - удаляем первый, но последний добавленый оставляем; если хорошо зашло - продолжаем без первого

   struct TrackletToMerge {
     std::vector<CDCLegendreTrackHit*> hits;
     int charge;
     int sLayer;
     CDCLegendreTrackCandidate* cand;
     int nhits;
   };

   std::vector<TrackletToMerge> listOfTracklets;

   std::vector<CDCLegendreTrackHit*> h_list_1[2][5];
   std::vector<CDCLegendreTrackHit*> h_list_2[2][5];

  double theta1 = cand1->getTheta();
  double r1 = cand1->getR();
  double yc1 = sin(theta1) / r1;
  double xc1 = cos(theta1) / r1;
  double rc1 = fabs(1 / r1);

  double theta2 = cand2->getTheta();
  double r2 = cand2->getR();
  double yc2 = sin(theta2) / r2;
  double xc2 = cos(theta2) / r2;
  double rc2 = fabs(1 / r2);


  for(int jj=0; jj<5; jj++){
   for (CDCLegendreTrackHit * hit : cand1->getTrackHits()) {
     if((hit->getCurvatureSignWrt(xc1, yc1)==CDCLegendreTrackCandidate::charge_positive) && (hit->getSuperlayerId() == jj*2))h_list_1[0][jj].push_back(hit);
     if((hit->getCurvatureSignWrt(xc1, yc1)==CDCLegendreTrackCandidate::charge_negative) && (hit->getSuperlayerId() == jj*2))h_list_1[1][jj].push_back(hit);
   }
   for (CDCLegendreTrackHit * hit : cand2->getTrackHits()) {
     if((hit->getCurvatureSignWrt(xc2, yc2)==CDCLegendreTrackCandidate::charge_positive) && (hit->getSuperlayerId() == jj*2))h_list_2[0][jj].push_back(hit);
     if((hit->getCurvatureSignWrt(xc2, yc2)==CDCLegendreTrackCandidate::charge_negative) && (hit->getSuperlayerId() == jj*2))h_list_2[1][jj].push_back(hit);
   }
  }


  for(int jj=0; jj<5; jj++){
   TrackletToMerge newTracklet;
   newTracklet.sLayer = jj*2;
   newTracklet.charge = CDCLegendreTrackCandidate::charge_positive;
   newTracklet.cand = cand1;
   listOfTracklets.push_back(newTracklet);
  }
  for(int jj=0; jj<5; jj++){
   TrackletToMerge newTracklet;
   newTracklet.sLayer = jj*2;
   newTracklet.charge = CDCLegendreTrackCandidate::charge_negative;
   newTracklet.cand = cand1;
   listOfTracklets.push_back(newTracklet);
  }
  for(int jj=0; jj<5; jj++){
   TrackletToMerge newTracklet;
   newTracklet.sLayer = jj*2;
   newTracklet.charge = CDCLegendreTrackCandidate::charge_positive;
   newTracklet.cand = cand2;
   listOfTracklets.push_back(newTracklet);
  }
  for(int jj=0; jj<5; jj++){
   TrackletToMerge newTracklet;
   newTracklet.sLayer = jj*2;
   newTracklet.charge = CDCLegendreTrackCandidate::charge_negative;
   newTracklet.cand = cand2;
   listOfTracklets.push_back(newTracklet);
  }

  for (CDCLegendreTrackHit * hit : cand1->getTrackHits()) {
   int charge_temp = hit->getCurvatureSignWrt(xc1, yc1);
   for(TrackletToMerge& trackletToFill : listOfTracklets){
     if(charge_temp == trackletToFill.charge && hit->getSuperlayerId() == trackletToFill.sLayer && trackletToFill.cand == cand1) trackletToFill.hits.push_back(hit);
   }
  }
  for (CDCLegendreTrackHit * hit : cand2->getTrackHits()) {
   int charge_temp = hit->getCurvatureSignWrt(xc2, yc2);
   for(TrackletToMerge& trackletToFill : listOfTracklets){
     if(charge_temp == trackletToFill.charge && hit->getSuperlayerId() == trackletToFill.sLayer && trackletToFill.cand == cand2) trackletToFill.hits.push_back(hit);
   }
  }


  for(TrackletToMerge& trackletToFill : listOfTracklets){
   trackletToFill.nhits = trackletToFill.hits.size();
  }


   std::vector<CDCLegendreTrackHit*> c_list_temp_1;
  for(int ii=0; ii<2; ii++){
   for(int jj=0; jj<5; jj++){
     if((h_list_1[ii][jj].size() == 0) && (h_list_2[ii][jj].size() != 0)) {
       for (CDCLegendreTrackHit * hit : h_list_2[ii][jj]) {
           c_list_temp_1.push_back(hit);
       }
     }
     if((h_list_1[ii][jj].size() != 0) && (h_list_2[ii][jj].size() == 0)) {
       for (CDCLegendreTrackHit * hit : h_list_1[ii][jj]) {
           c_list_temp_1.push_back(hit);
       }
     }


   }
  }

  // делаем фит, находим центр нового трека;
  // далее ищем его знак (к-во хитов слева\справа) потом сортируем треклеты по кругу (ф-ла где-то в стереотреклетах) далее, можно найти самый длинный участок (необходимо использовать итератор наверное), где треклеты не перекрываються, и к нему добавлять новые треклеты
  // если где-то хи2 начнет расти, откатиться назад; также можно сначала промерджить все неперекрывающиеся треклеты
  // так же, этот метод хорошо зайдет для мерджинга "кюрлеров", потому что будут чистые треклеты; еще, хорошо было бы сравнивать не два трека, а больше, что бы составлять корректные треки, так же это позволит сделать reassignment хитов

  //  for()


   std::vector<CDCLegendreTrackHit*> c_list_temp_2;
   for (CDCLegendreTrackHit * hit : cand1->getTrackHits()) {
     c_list_temp_2.push_back(hit);
   }
   for (CDCLegendreTrackHit * hit : cand2->getTrackHits()) {
     c_list_temp_2.push_back(hit);
   }




   std::pair<double, double> ref_point = std::make_pair(0., 0.);
   std::pair<double, double> track_par = std::make_pair(-999, -999);
   double chi2_temp;
   if((c_list_temp_1.size() > cand1->getTrackHits().size()) && (c_list_temp_1.size() > cand2->getTrackHits().size())) chi2_temp = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp_1, track_par, ref_point) ;
   else chi2_temp = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp_2, track_par, ref_point);
  */
  /*
  if (chi2_temp < 2.) {
    std::vector<CDCLegendreTrackHit*> c_list_temp_new;
    for (CDCLegendreTrackHit * hit : c_list_temp) {
      c_list_temp_new.push_back(hit);
    }
    for (CDCLegendreTrackHit * hitToRemove : c_list_temp) {
      c_list_temp_new.erase(std::remove(c_list_temp_new.begin(), c_list_temp_new.end(), hitToRemove), c_list_temp_new.end());
      double chi2_temp_new = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp_new, track_par, ref_point) / c_list_temp_new.size();
      if (chi2_temp_new > chi2_temp) c_list_temp_new.push_back(hitToRemove);
    }
    double chi2_temp_new = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp_new, track_par, ref_point) / c_list_temp_new.size();
    if (chi2_temp > chi2_temp_new) {

      /*      c_list_temp.clear();
            for (CDCLegendreTrackHit * hit : c_list_temp_new) {
              c_list_temp.push_back(hit);
            }
            chi2_temp = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp, track_par, ref_point) / c_list_temp.size();
            for(CDCLegendreTrackHit* hitToRemove: c_list_temp){
              c_list_temp_new.erase(std::remove(c_list_temp_new.begin(), c_list_temp_new.end(), hitToRemove), c_list_temp_new.end());
              double chi2_temp_new = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp_new, track_par, ref_point) / c_list_temp_new.size();
              if(chi2_temp_new > chi2_temp) c_list_temp_new.push_back(hitToRemove);
            }
            double chi2_temp_new = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp_new, track_par, ref_point) / c_list_temp_new.size();
            if (chi2_temp > chi2_temp_new)
            {
              c_list_temp.clear();
              for (CDCLegendreTrackHit * hit : c_list_temp_new) {
                c_list_temp.push_back(hit);
              }
              chi2_temp = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp, track_par, ref_point) / c_list_temp.size();
            }*/
  /*
       chi2_temp = chi2_temp_new;
     }
   }
  */
  return chi2_temp;

}


double CDCLegendreTrackMerger::selectCoreMergeFit(CDCLegendreTrackCandidate* cand1, CDCLegendreTrackCandidate* cand2)
{
  std::vector<CDCLegendreTrackHit*> hitList1;
  for (CDCLegendreTrackHit * hit : cand1->getTrackHits()) {
    if (checkDist(hit, cand1) <  4.*hit->getDeltaDriftTime()) hitList1.push_back(hit);
  }

  std::vector<CDCLegendreTrackHit*> hitList2;
  for (CDCLegendreTrackHit * hit : cand2->getTrackHits()) {
    if (checkDist(hit, cand2) <  4.*hit->getDeltaDriftTime()) hitList2.push_back(hit);
  }

  std::vector<CDCLegendreTrackHit*> c_list_temp;
  for (CDCLegendreTrackHit * hit : hitList1) {
    c_list_temp.push_back(hit);
  }
  for (CDCLegendreTrackHit * hit : hitList2) {
    c_list_temp.push_back(hit);
  }


  std::pair<double, double> ref_point = std::make_pair(0., 0.);
  std::pair<double, double> track_par = std::make_pair(-999, -999);
  double chi2_temp = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp, track_par, ref_point) / c_list_temp.size();

  return chi2_temp;

}


double CDCLegendreTrackMerger::checkDist(CDCLegendreTrackHit* hit, CDCLegendreTrackCandidate* track)
{
  double x0_track = cos(track->getTheta()) / track->getR() + track->getReferencePoint().X();
  double y0_track = sin(track->getTheta()) / track->getR() + track->getReferencePoint().Y();
  double R = fabs(1. / track->getR());

  double x0_hit = hit->getOriginalWirePosition().X();
  double y0_hit = hit->getOriginalWirePosition().Y();
  double dist = fabs(fabs(R - sqrt((x0_track - x0_hit) * (x0_track - x0_hit) + (y0_track - y0_hit) * (y0_track - y0_hit))) - hit->getDriftTime());

  return dist;
}

void CDCLegendreTrackMerger::splitTracks()
{
  for (CDCLegendreTrackCandidate * cand : m_trackList) {
//    if(cand->getCandidateType() == CDCLegendreTrackCandidate::tracklet) continue;
    double theta = cand->getTheta();
    double r = cand->getR();
    double yc = sin(theta) / r;
    double xc = cos(theta) / r;

    int hits_pos = 0;
    int hits_neg = 0;

    for (CDCLegendreTrackHit * Hit : cand->getTrackHits()) {
      int curve_sign = Hit->getCurvatureSignWrt(xc, yc);

      if (curve_sign == CDCLegendreTrackCandidate::charge_positive)
        ++hits_pos;
      else if (curve_sign == CDCLegendreTrackCandidate::charge_negative)
        ++hits_neg;
      else {
        B2ERROR(
          "Strange behaviour of CDCLegendreTrackHit::getCurvatureSignWrt");
        exit(EXIT_FAILURE);
      }
    }

    if ((hits_pos != 0) && (hits_neg != 0)) {
      std::vector<CDCLegendreTrackHit*> hitsForNewTrack;
      int chargeNewTrack;
      if (hits_pos > hits_neg)
        chargeNewTrack = CDCLegendreTrackCandidate::charge_negative;
      else
        chargeNewTrack = CDCLegendreTrackCandidate::charge_positive;

      for (CDCLegendreTrackHit * hit : cand->getTrackHits()) {
        int curve_sign = hit->getCurvatureSignWrt(xc, yc);
        if (curve_sign == chargeNewTrack) hitsForNewTrack.push_back(hit);
      }

      for (CDCLegendreTrackHit * hit : hitsForNewTrack) {
        cand->removeHit(hit);
        hit->setUsed(CDCLegendreTrackHit::not_used);
      }

      if (hitsForNewTrack.size() > 5) {
        CDCLegendreTrackCandidate* newTracklet = m_cdcLegendreTrackCreator->createLegendreTracklet(hitsForNewTrack);
        newTracklet->reestimateCharge();
        m_cdcLegendreTrackFitter->fitTrackCandidateFast(newTracklet);
      } else {
        hitsForNewTrack.clear();
      }

      cand->reestimateCharge();

      m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand);



    }

  }
}

void CDCLegendreTrackMerger::checkOverlapping()
{

  const int hitsSlightOverlapThreshold = 3;
  const int hitsHeavyOverlapThreshold = 8;
  const double overlapParametersThreshold = 0.1;


  for (std::list<CDCLegendreTrackCandidate*>::iterator it1 =
         m_trackList.begin(); it1 != m_trackList.end(); ++it1) {
    CDCLegendreTrackCandidate* cand1 = *it1;

    double chi2_cand1 = cand1->getChi2() / cand1->getNHits();

    double x0_track1 = cos(cand1->getTheta()) / cand1->getR() + cand1->getReferencePoint().X();
    double y0_track1 = sin(cand1->getTheta()) / cand1->getR() + cand1->getReferencePoint().Y();


    std::list<CDCLegendreTrackCandidate*>::iterator it2 = std::next(it1);
    while (it2 != m_trackList.end()) {
      CDCLegendreTrackCandidate* cand2 = *it2;
      ++it2;

      bool isSlightOverlapping = false;
      bool __attribute__((unused)) isHeavyOverlapping = false;

      m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand2);
      double chi2_cand2 = cand2->getChi2() / cand2->getNHits();

      double x0_track2 = cos(cand2->getTheta()) / cand2->getR() + cand2->getReferencePoint().X();
      double y0_track2 = sin(cand2->getTheta()) / cand2->getR() + cand2->getReferencePoint().Y();


      if ((fabs(x0_track1 - x0_track2) / x0_track1 < overlapParametersThreshold) &&
          (fabs(y0_track1 - y0_track2) / y0_track1 < overlapParametersThreshold) &&
          ((fabs(cand2->getR()) / fabs(fabs(cand2->getR()) - fabs(cand1->getR())) < overlapParametersThreshold) ||
           (fabs(cand1->getR()) / fabs(fabs(cand2->getR()) - fabs(cand1->getR())) < overlapParametersThreshold)))
        isHeavyOverlapping = true;
      else {
        int n_overlapp = 0;
        double R = fabs(1. / cand1->getR());
        for (CDCLegendreTrackHit * hit : cand2->getTrackHits()) {
          double x0_hit = hit->getOriginalWirePosition().X();
          double y0_hit = hit->getOriginalWirePosition().Y();
          double dist = fabs(R - sqrt(SQR(x0_track1 - x0_hit) + SQR(y0_track1 - y0_hit))) - hit->getDriftTime();
          if (dist < hit->getDeltaDriftTime() * 3.)n_overlapp++;
        }
        if (n_overlapp > hitsSlightOverlapThreshold) {
          isSlightOverlapping = true;
          if (n_overlapp > hitsHeavyOverlapThreshold) isHeavyOverlapping = true;
        } else {
          n_overlapp = 0;
          R = fabs(1. / cand2->getR());
          for (CDCLegendreTrackHit * hit : cand1->getTrackHits()) {
            double x0_hit = hit->getOriginalWirePosition().X();
            double y0_hit = hit->getOriginalWirePosition().Y();
            double dist = fabs(R - sqrt(SQR(x0_track2 - x0_hit) + SQR(y0_track2 - y0_hit))) - hit->getDriftTime();
            if (dist < hit->getDeltaDriftTime() * 3.)n_overlapp++;
          }
          if (n_overlapp > hitsSlightOverlapThreshold) {
            isSlightOverlapping = true;
            if (n_overlapp > hitsHeavyOverlapThreshold) isHeavyOverlapping = true;

          }
        }

        if (not isSlightOverlapping) continue;
        else {
          B2INFO("Overlapping detected!");

          //  here we compare 2 conditions:
          //  1. merge tracks into one;
          //  2. remove overlapping hits;

          //  another approach:
          //  for each overlapping hit calculate distance to each track and then apply weights


          bool approach1;

          approach1 = true;
          if (approach1) {


            std::vector<CDCLegendreTrackHit*> c_list_temp;
            for (CDCLegendreTrackHit * hit : cand2->getTrackHits()) {
              c_list_temp.push_back(hit);
            }
            for (CDCLegendreTrackHit * hit : cand1->getTrackHits()) {
              c_list_temp.push_back(hit);
            }

            std::pair<double, double> ref_point = std::make_pair(0., 0.);
            std::pair<double, double> track_par = std::make_pair(-999, -999);
            double chi2_temp;
            m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp, track_par, ref_point) ;

            //clear bad hits from merged tracks
            double R = fabs(1. / track_par.second);
            double x0_track = cos(track_par.first) / track_par.second + ref_point.first;
            double y0_track = sin(track_par.first) / track_par.second + ref_point.first;
            c_list_temp.erase(std::remove_if(c_list_temp.begin(), c_list_temp.end(),
            [&R, &x0_track, &y0_track](CDCLegendreTrackHit * hit) {
              double x0_hit = hit->getOriginalWirePosition().X();
              double y0_hit = hit->getOriginalWirePosition().Y();
              double dist = fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftTime();
              if ((dist > hit->getDriftTime()) || (dist > hit->getDeltaDriftTime() * 2.)) {
                return true;
              } else {
                return false;
              }
            }), c_list_temp.end());

            chi2_temp = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp, track_par, ref_point) / c_list_temp.size();

            B2INFO("chi2_temp: " << chi2_temp << "; chi2_track1: " << chi2_cand1 << "; chi2_track2: " << chi2_cand2);

//        if (chi2_temp < (chi2_track1 + chi2_track2)) {
            if ((chi2_temp < 1.)/* ||
            (chi2_temp < chi2_cand1) ||
            (chi2_temp < chi2_cand2)*/) {

              mergeTracks(cand1, cand2);
              m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand1);
//          cand1->clearBadHits();
              m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand1);

              B2INFO("Overlapping is resolved!");


            }

          }


        }

      }

    }
  }
}


bool CDCLegendreTrackMerger::earlyCandidateMerge(std::pair < std::vector<CDCLegendreTrackHit*>,
                                                 std::pair<double, double> > & candidate, std::set<CDCLegendreTrackHit*>& hits_set,
                                                 bool fitTracksEarly)
{
  bool merged = false;
  //      cdcLegendreTrackFitter->fitTrackCandidateStepped(&candidate);
  std::pair<double, double> ref_point = std::make_pair(0., 0.);
  //      cdcLegendrePatternChecker->checkCandidate(&candidate);

  if (candidate.first.size() > 0) {
    double chi2_cand;
    m_cdcLegendreTrackFitter->fitTrackCandidateFast(&candidate, ref_point, chi2_cand);
    //      cdcLegendrePatternChecker->clearBadHits(&candidate, ref_point);

    double x0_cand = cos(candidate.second.first) / candidate.second.second + ref_point.first;
    double y0_cand = sin(candidate.second.first) / candidate.second.second + ref_point.second;


    //loop over all candidates
    for (std::list<CDCLegendreTrackCandidate*>::iterator it1 =
           m_trackList.begin(); it1 != m_trackList.end(); ++it1) {
      CDCLegendreTrackCandidate* cand1 = *it1;

      bool make_merge __attribute__((unused)) = false;

      double chi2_track = cand1->getChi2();

      double x0_track = cos(cand1->getTheta()) / cand1->getR() + cand1->getReferencePoint().X();
      double y0_track = sin(cand1->getTheta()) / cand1->getR() + cand1->getReferencePoint().Y();

      //        if((fabs(x0_track-x0_cand)/x0_track < 0.1)&&(fabs(y0_track-y0_cand)/y0_track < 0.1))make_merge = true;
      //        else

      {
        int n_overlapp = 0;
        double R = fabs(1. / cand1->getR());
        for (CDCLegendreTrackHit * hit : candidate.first) {
          double x0_hit = hit->getOriginalWirePosition().X();
          double y0_hit = hit->getOriginalWirePosition().Y();
          double dist = fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftTime();
          if (dist < hit->getDriftTime() * 3.)n_overlapp++;
        }
        if (n_overlapp > 3)make_merge = true;
        else {
          n_overlapp = 0;
          R = fabs(1. / candidate.second.second);
          for (CDCLegendreTrackHit * hit : cand1->getTrackHits()) {
            double x0_hit = hit->getOriginalWirePosition().X();
            double y0_hit = hit->getOriginalWirePosition().Y();
            double dist = fabs(R - sqrt(SQR(x0_cand - x0_hit) + SQR(y0_cand - y0_hit))) - hit->getDriftTime();
            if (dist < hit->getDriftTime() * 3.)n_overlapp++;
          }
          if (n_overlapp > 3)make_merge = true;

        }
      }

      if (true/*make_merge*//*false*/) {
        std::vector<CDCLegendreTrackHit*> c_list_temp;
        std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > candidate_temp =
          std::make_pair(c_list_temp, std::make_pair(-999, -999));
        for (CDCLegendreTrackHit * hit : candidate.first) {
          candidate_temp.first.push_back(hit);
        }
        for (CDCLegendreTrackHit * hit : cand1->getTrackHits()) {
          candidate_temp.first.push_back(hit);
        }

        double chi2_temp;
        std::pair<double, double> ref_point_temp = std::make_pair(0., 0.);
        m_cdcLegendreTrackFitter->fitTrackCandidateFast(&candidate_temp, ref_point_temp, chi2_temp);
        if (candidate_temp.first.size() == 0) {
          B2ERROR("BAD MERGERD CANDIDATE SIZE!");
          merged = true;
          break;
        }

        if (chi2_temp < SQR(sqrt(chi2_track) + sqrt(chi2_cand)) * 2.) {

          cand1->setR(candidate_temp.second.second);
          cand1->setTheta(candidate_temp.second.first);
          //            cand1->setReferencePoint(ref_point_temp.first, ref_point_temp.second);
          mergeTracks(cand1, candidate, hits_set);

          if (fitTracksEarly) m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand1, ref_point);
          cand1->setReferencePoint(ref_point.first, ref_point.second);

          merged = true;
        } else make_merge = false;
      }
    }

  }

  return merged;
}


void CDCLegendreTrackMerger::addStereoTracklesToTrack()
{
  /*
   * First, we are calculating position of the center of stereo tracklet, then trying to add tracklet to track and define z-position of tracklet with
   * formula: z = ((s_wire_back-s_wire_forward)_z/(s_wire_back-s_wire_forward)_xy)*(s_wire - ax_wire)_xy
   * Then we can estimate z-position as z=(alpha/2pi)*h, where cos(alpha) = 1 - ((s_wire_x - x_ref)^2 + (s_wire_y - y_ref)^2)/(2 * R^2) ; h = 2pi*R*ctg(theta)
   */
  B2INFO("Number of tracks: " << m_trackList.size());
  B2INFO("Number of stereo tracklets: " << m_stereoTrackletList.size());
  for (CDCLegendreTrackCandidate * track : m_trackList) {

    for (CDCLegendreTrackCandidate * stereoTracklet : m_stereoTrackletList) {

      B2INFO("Number of hits in stereo tracklet: " << stereoTracklet->getNHits());
      double x0_tracklet = 0, y0_tracklet = 0; // holds geometrical center of cluster of stereohits (tracklet)

      for (CDCLegendreTrackHit * hit : stereoTracklet->getTrackHits()) {
        x0_tracklet += hit->getWirePosition().X();
        y0_tracklet += hit->getWirePosition().Y();
      }

      x0_tracklet = x0_tracklet / stereoTracklet->getNHits();
      y0_tracklet = y0_tracklet / stereoTracklet->getNHits();


      double dist = SQR(fabs(fabs(1. / track->getR()) - sqrt(SQR(x0_tracklet - track->getXc()) + SQR(y0_tracklet - track->getYc()))));

      if (dist < 40) {
//        mergeTracks(track, stereoTracklet);

        for (CDCLegendreTrackHit * hit : stereoTracklet->getTrackHits()) {
          track->addHit(hit);
          hit->setUsed(true);
        }

        B2INFO("MERGED!");

        fitStereoTrackletsToTrack(track, stereoTracklet);
      }

    }

  }
}


double CDCLegendreTrackMerger::fitStereoTrackletsToTrack(CDCLegendreTrackCandidate* track, CDCLegendreTrackCandidate* tracklet)
{

  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();

  TVector3 forwardWirePoint; //forward end of the wire
  TVector3 backwardWirePoint; //backward end of the wire
  TVector3 mediumWirePoint; //center of wire
  TVector3 wireVector;  //direction of the wire

  int sLayer = 0;
  int sign = 0;

  for (CDCLegendreTrackHit * hit : tracklet->getTrackHits()) {
    sLayer = hit->getSuperlayerId();
  }

  if (sLayer == 1 || sLayer == 5) sign = -1;
  else sign = 1;

//  sign *= track->getCharge();


  for (CDCLegendreTrackHit * hit : tracklet->getTrackHits()) {
    //forward end of the wire
    forwardWirePoint.SetX(cdcg.wireForwardPosition(hit->getLayerId(), hit->getWireId()).x());
    forwardWirePoint.SetY(cdcg.wireForwardPosition(hit->getLayerId(), hit->getWireId()).y());
    forwardWirePoint.SetZ(cdcg.wireForwardPosition(hit->getLayerId(), hit->getWireId()).z());

    //backward end of the wire
    backwardWirePoint.SetX(cdcg.wireBackwardPosition(hit->getLayerId(), hit->getWireId()).x());
    backwardWirePoint.SetY(cdcg.wireBackwardPosition(hit->getLayerId(), hit->getWireId()).y());
    backwardWirePoint.SetZ(cdcg.wireBackwardPosition(hit->getLayerId(), hit->getWireId()).z());

    mediumWirePoint.SetX(hit->getOriginalWirePosition().x());
    mediumWirePoint.SetY(hit->getOriginalWirePosition().y());
    mediumWirePoint.SetZ(hit->getOriginalWirePosition().z());


    double lWire = fabs(backwardWirePoint.Z() - forwardWirePoint.Z());
    double rWire = sqrt(SQR(backwardWirePoint.x() - forwardWirePoint.x()) + SQR(backwardWirePoint.y() - forwardWirePoint.y()));

    double dist_1 = fabs(1. / track->getR()) - sqrt(SQR(hit->getOriginalWirePosition().X() - track->getXc()) + SQR(hit->getOriginalWirePosition().Y() - track->getYc())) - hit->getDriftTime();
    double dist_2 = fabs(1. / track->getR()) - sqrt(SQR(hit->getOriginalWirePosition().X() - track->getXc()) + SQR(hit->getOriginalWirePosition().Y() - track->getYc())) + hit->getDriftTime();

    double alpha = acos(1. - (SQR(hit->getOriginalWirePosition().X() - track->getReferencePoint().X()) + SQR(hit->getOriginalWirePosition().Y() - track->getReferencePoint().Y())) / (2.*SQR(fabs(1. / track->getR()))));

    //analyzing sign of z position
    double phi_on_track; //azimutal angle of point on thack which is projection of current stereohit on thack

    phi_on_track = track->getTheta() + (3.1415 / 2. - alpha / 2.) * track->getChargeSign();
    double delta_phi = hit->getPhi() - phi_on_track;
    if (delta_phi > 3.1415) delta_phi -= 2.*3.1415;
    if (delta_phi < -3.1415) delta_phi += 2.*3.1415;

    double sign_phi = delta_phi / fabs(delta_phi);

    double sign_final;
    if ((sign_phi >= 0) ^ (sign < 0))
      sign_final = -1;
    else
      sign_final = 1;

    double Zpos_1 = sign_final * (lWire * fabs(dist_1)) / rWire ;
    double Zpos_2 = sign_final * (lWire * fabs(dist_2)) / rWire ;

//    B2INFO("Z position = " << Zpos_1 << " " << Zpos_2 );

    double __attribute__((unused)) theta_1 = (atan2(Zpos_1 , sqrt(SQR(hit->getOriginalWirePosition().X()) + SQR(hit->getOriginalWirePosition().Y()))) + 3.1415 / 2) * 180. / 3.1415 ;
    double __attribute__((unused)) theta_2 = (atan2(Zpos_2 , sqrt(SQR(hit->getOriginalWirePosition().X()) + SQR(hit->getOriginalWirePosition().Y()))) + 3.1415 / 2) * 180. / 3.1415 ;

    //  B2INFO("THETA1 angle = " << theta_1 << " THETA2 angle = " << theta_2 << "; Z1 position = " << Zpos_1 << "; Z2 position = " << Zpos_2 );


//    B2INFO("alpha = " << alpha);

    double __attribute__((unused)) omega_1 = Zpos_1 / (alpha * fabs(1. / track->getR()));
    double __attribute__((unused)) omega_2 = Zpos_2 / (alpha * fabs(1. / track->getR()));

//    double theta_track_1 = (atan(omega_1) + 3.1415/2 ) * 180. / 3.1415;
//    double theta_track_2 = (atan(omega_2) + 3.1415/2 ) * 180. / 3.1415;
    double theta_track_1 = (atan2(Zpos_1 , (alpha * fabs(1. / track->getR()))) + 3.1415 / 2) * 180. / 3.1415;
    double theta_track_2 = (atan2(Zpos_2 , (alpha * fabs(1. / track->getR()))) + 3.1415 / 2) * 180. / 3.1415;

    B2INFO("THETA_track_1 angle = " << theta_track_1 << " THETA_track_2 angle = " << theta_track_2);


  }



  return 0;

}


void CDCLegendreTrackMerger::extendTracklet(CDCLegendreTrackCandidate* tracklet, std::vector<CDCLegendreTrackHit*>& m_AxialHitList)
{
  return ;
  std::vector<CDCLegendreTrackHit*> hits;

  double x0_track = cos(tracklet->getTheta()) / tracklet->getR() + tracklet->getReferencePoint().X();
  double y0_track = sin(tracklet->getTheta()) / tracklet->getR() + tracklet->getReferencePoint().Y();
  double R = fabs(1. / tracklet->getR());
  double dist_min = 999;
  double x_pos, y_pos;


  for (CDCLegendreTrackHit * hit : tracklet->getTrackHits()) {
//      if (hit->isUsed() != CDCLegendreTrackHit::used_in_track || hit->isUsed() != CDCLegendreTrackHit::background) {
    double x0_hit = hit->getOriginalWirePosition().X();
    double y0_hit = hit->getOriginalWirePosition().Y();
    double dist = fabs(fabs(R - sqrt((x0_track - x0_hit) * (x0_track - x0_hit) + (y0_track - y0_hit) * (y0_track - y0_hit))) - hit->getDriftTime());
    if (dist < dist_min) {
      dist_min = dist;
      x_pos = x0_hit + hit->getDriftTime() * cos(atan2(x0_track - x0_hit, y0_track - y0_hit));
      y_pos = y0_hit + hit->getDriftTime() * sin(atan2(x0_track - x0_hit, y0_track - y0_hit));
//          x_pos = x0_hit;
//          y_pos = y0_hit;
//          x_pos = x0_track + R*cos(atan2(x0_track - x0_hit, y0_track - y0_hit));
//          y_pos = y0_track + R*sin(atan2(x0_track - x0_hit, y0_track - y0_hit));
    }
//      }
  }

  TVector3 reference;
  reference.SetZ(0);
//    reference.SetX(tracklet->getTrackHits()[1]->getOriginalWirePosition().X());
//    reference.SetY(tracklet->getTrackHits()[1]->getOriginalWirePosition().X());
  reference.SetX(x_pos);
  reference.SetY(y_pos);

  /*
  int innermostSLayer = tracklet->getInnermostSLayer(2);
  int outermostSLayer = tracklet->getOutermostSLayer(2);
    for (CDCLegendreTrackHit * axialHit : m_AxialHitList) {
        if(axialHit->isUsed() == CDCLegendreTrackHit::used_in_track) continue;
        if((axialHit->getSuperlayerId() > outermostSLayer) || (axialHit->getSuperlayerId() > innermostSLayer)){
          CDCLegendreTrackHit* trackHit = new CDCLegendreTrackHit(*axialHit);
          trackHit->setPosition(axialHit->getOriginalWirePosition() - reference);
          trackHit->setUsed(CDCLegendreTrackHit::not_used);
          hits.push_back(trackHit);
        }
      }*/
  for (CDCLegendreTrackHit * axialHit : tracklet->getTrackHits()) {
    CDCLegendreTrackHit* trackHit = new CDCLegendreTrackHit(*axialHit);
    trackHit->setPosition(axialHit->getOriginalWirePosition() - reference);
    trackHit->setUsed(CDCLegendreTrackHit::not_used);
    hits.push_back(trackHit);
  }

  B2INFO("Size of hits vector: " << hits.size());

  B2INFO("Size of tracklet: " << tracklet->getTrackHits().size());

  std::vector<CDCLegendreTrackHit*> c_list;
  std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > candidate = std::make_pair(c_list, std::make_pair(-999, -999));

  m_cdcLegendreFastHough->FastHoughNormal(&candidate, hits, 1, 0, 8192, -0.15, 0.15, 5);

  B2INFO("Size of candidate: " << candidate.first.size());

  if (candidate.first.size() >= tracklet->getTrackHits().size() - 1) {
    double theta_mean = candidate.second.first;
    double r_mean = candidate.second.second;
    double delta_r = 0.3 / 1024;
    double delta_theta = 3.14 / 1024;
    double theta_bin[2], r_bin[2];
    theta_bin[0] = theta_mean - delta_theta / 2;
    theta_bin[1] = theta_mean + delta_theta / 2;
    r_bin[0] = r_mean - delta_r / 2;
    r_bin[1] = r_mean + delta_r / 2;
    double dist1[2][2], dist2[2][2];

    for (CDCLegendreTrackHit * axialHit : m_AxialHitList) {
      if (axialHit->isUsed() == CDCLegendreTrackHit::used_in_track) continue;
      CDCLegendreTrackHit* trackHit = new CDCLegendreTrackHit(*axialHit);
      for (int t_index = 0; t_index < 2; ++t_index) {
        trackHit->setPosition(axialHit->getOriginalWirePosition() - reference);

        double r_temp = trackHit->getConformalX() * cos(theta_bin[t_index]) + trackHit->getConformalY() * sin(theta_bin[t_index]);

        double r_1 = r_temp + trackHit->getConformalDriftTime();
        double r_2 = r_temp - trackHit->getConformalDriftTime();

        //calculate distances of lines to horizontal bin border
        for (int r_index = 0; r_index < 2; ++r_index) {
          dist1[t_index][r_index] = r_bin[r_index] - r_1;
          dist2[t_index][r_index] = r_bin[r_index] - r_2;
        }
      }

      bool addHit = false;

      //actual voting, based on the distances (test, if line is passing though the bin)
      //curves are assumed to be straight lines, might be a reasonable assumption locally
      if (not((dist1[0][0] > 0 && dist1[0][1] > 0 && dist1[1][0] > 0 && dist1[1][1] > 0) ||
              (dist1[0][0] < 0 && dist1[0][1] < 0 && dist1[1][0] < 0 && dist1[1][1] < 0)))
        addHit = true;
      else if (not((dist2[0][0] > 0 && dist2[0][1] > 0 && dist2[1][0] > 0 && dist2[1][1] > 0) ||
                   (dist2[0][0] < 0 && dist2[0][1] < 0 && dist2[1][0] < 0 && dist2[1][1] < 0)))
        addHit = true;

      if (addHit) tracklet->addHit(axialHit);
      delete trackHit;
    }

  }

  B2INFO("New size of tracklet: " << tracklet->getTrackHits().size());
  m_cdcLegendreTrackFitter->fitTrackCandidateFast(tracklet);
  /*
      std::pair<double, double> ref_point = std::make_pair(0., 0.);


      std::vector<CDCLegendreTrackHit*> hits_in_candidate;

      for(CDCLegendreTrackHit *hit: candidate.first)
      {
        for (CDCLegendreTrackHit * axialHit : m_AxialHitList) {
          if(axialHit->getStoreIndex() == hit->getStoreIndex()) hits_in_candidate.push_back(axialHit);
        }
      }

      m_cdcLegendreTrackCreator->createLegendreTracklet(hits_in_candidate);

  */

  /*    std::vector<CDCLegendreTrackHit*> hitsToAdd;
      double preinitialChi2 = tracklet->getChi2();
      m_cdcLegendreTrackFitter->fitTrackCandidateFast(tracklet);
      double initialChi2 = tracklet->getChi2();

      for(CDCLegendreTrackHit *hit: candidate.first)
      {
        for (CDCLegendreTrackHit * axialHit : m_AxialHitList) {
          if(axialHit->getStoreIndex() == hit->getStoreIndex())
                hitsToAdd.push_back(axialHit);
        }
      }

      hitsToAdd.erase(std::remove_if(hitsToAdd.begin(), hitsToAdd.end(), [&tracklet](CDCLegendreTrackHit * hit)
          {
            for(CDCLegendreTrackHit * axialHit : tracklet->getTrackHits()){
              if(axialHit->getStoreIndex() == hit->getStoreIndex()) return true;
            }
            return false;
          })
          , hitsToAdd.end());



      for(CDCLegendreTrackHit *hit: hitsToAdd){
        tracklet->addHit(hit);
        hit->setUsed(CDCLegendreTrackHit::used_in_track);
      }

      B2INFO("New size of tracklet: " << tracklet->getTrackHits().size());


      m_cdcLegendreTrackFitter->fitTrackCandidateFast(tracklet);

      double newChi2 = tracklet->getChi2();

      B2INFO("Initial Chi2: " << initialChi2 << "; new Chi2: " << newChi2 << ": preinitialChi2: " << preinitialChi2);
  */

  /*    if(2.*newChi2/tracklet->getNAxialHits() > initialChi2/tracklet->getNAxialHits())
      {
        for(CDCLegendreTrackHit *hit: hitsToAdd){
          tracklet->removeHit(hit);
          hit->setUsed(CDCLegendreTrackHit::not_used);
        }
      }
  */

  for (CDCLegendreTrackHit * hit : hits) {
    delete hit;
  }
  hits.clear();



}
