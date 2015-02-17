/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackMerger.h>
#include <vector>

using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;

void TrackMerger::mergeTracks(TrackCandidate* cand1, TrackCandidate* cand2)
{
  if (cand1 == cand2) return;

  std::vector<TrackHit*>& commonHitListOfTwoTracks = cand1->getTrackHits();
  for (TrackHit * hit : cand2->getTrackHits()) {
    commonHitListOfTwoTracks.push_back(hit);
  }

  // Do delete hits in the old track cands
  cand2->getTrackHits().clear();

  // Sorting is done via pointer addresses (!!). This is not very stable and also not very meaningful (in terms of physical meaning),
  // but it does the job.
  std::sort(commonHitListOfTwoTracks.begin(), commonHitListOfTwoTracks.end());

  // Only use hits once
  commonHitListOfTwoTracks.erase(
    std::unique(commonHitListOfTwoTracks.begin(), commonHitListOfTwoTracks.end()),
    commonHitListOfTwoTracks.end());
}

void TrackMerger::resetHits(TrackCandidate* otherTrackCandidate)
{
  for (TrackHit * hit : otherTrackCandidate->getTrackHits()) {
    hit->setHitUsage(TrackHit::used_in_track);
  }
}

void TrackMerger::doTracksMerging(std::list<TrackCandidate*> trackList, TrackFitter* trackFitter)
{

  // Search for best matches
  unsigned int outerCounter = 0;
  for (TrackCandidate * trackCandidate : trackList) {
    unsigned int innerCounter = 0;
    double prob = 0;
    TrackCandidate* bestCandidate = nullptr;
    for (TrackCandidate * otherTrackCandidate : trackList) {
      if (innerCounter <= outerCounter) {
        innerCounter++;
        continue;
      }

      double probTemp = doTracksFitTogether(trackCandidate, otherTrackCandidate, trackFitter);

      resetHits(otherTrackCandidate);
      resetHits(trackCandidate);

      if (probTemp > prob) {
        prob = probTemp;
        bestCandidate = otherTrackCandidate;
      }

      innerCounter++;
    }

    if (prob > m_minimum_probability_to_be_merged) {
      mergeTracks(bestCandidate, trackCandidate);
      trackFitter->fitTrackCandidateFast(bestCandidate);
    }

    outerCounter++;
  }

  trackList.erase(std::remove_if(trackList.begin(), trackList.end(),
  [&](TrackCandidate * cand) { return (cand->getTrackHits().size() == 0); }),
  trackList.end());

  for (TrackCandidate * trackCandidate : trackList) {
    trackFitter->fitTrackCandidateFast(trackCandidate);
  }
}

TrackMerger::BestMergePartner TrackMerger::calculateBestTrackToMerge(TrackCandidate* trackCandidateToBeMerged, std::list<TrackCandidate*>::iterator start_iterator, std::list<TrackCandidate*>::iterator end_iterator, TrackFitter* trackFitter)
{
  double probabilityToBeMerged = 0;
  TrackCandidate* candidateToMergeBest = nullptr;

  for (auto iterator = start_iterator; iterator != end_iterator; iterator++) {
    TrackCandidate* cand2 = *iterator;
    if (trackCandidateToBeMerged == cand2) {
      continue;
    }


    if (cand2->getTrackHits().size() < 3) {
      continue;
    }

    double probabilityTemp = doTracksFitTogether(trackCandidateToBeMerged, cand2, trackFitter);

    // Reset hits, because we do not want to throw them away if this is not be best candidate to merge
    for (TrackHit * hit : trackCandidateToBeMerged->getTrackHits()) {
      hit->setHitUsage(TrackHit::used_in_track);
    }
    for (TrackHit * hit : cand2->getTrackHits()) {
      hit->setHitUsage(TrackHit::used_in_track);
    }

    if (probabilityToBeMerged < probabilityTemp) {
      probabilityToBeMerged = probabilityTemp;
      candidateToMergeBest = cand2;
    }
  }

  return std::make_pair(candidateToMergeBest, probabilityToBeMerged);
}


void TrackMerger::tryToMergeTrackWithOtherTracks(TrackCandidate* cand1, std::list<TrackCandidate*> trackList, TrackFitter* trackFitter)
{

  B2DEBUG(100, "Merger: Initial nCands = " << trackList.size());

  bool have_merged_something;

  do {
    have_merged_something = false;
    BestMergePartner candidateToMergeBest = calculateBestTrackToMerge(cand1, trackList.begin(), trackList.end(), trackFitter);
    Probability& probabilityWithCandidate = candidateToMergeBest.second;

    if (probabilityWithCandidate > m_minimum_probability_to_be_merged) {
      /*
      //      B2INFO("chi2_best: " << chi2_best << "; chi2_track1: " << chi2_track1);

            double chi2_best_reverse = 999.;
            TrackCandidate* candidateToMergeBestReverse = NULL;
            for (TrackCandidate * cand_temp : m_trackList) {
              if (cand_temp == cand1) continue;
              if (cand_temp == candidateToMergeBest) continue;
              if (cand_temp->getTrackHits().size() < 3) continue;
      //          if(cand_temp->getCandidateType() ==  TrackCandidate::goodTrack) continue;


              m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand_temp);
              double chi2_temp = tryToMergeAndFit(candidateToMergeBest, cand_temp, remove_hits);

              if (chi2_best_reverse > chi2_temp) {
                candidateToMergeBestReverse = cand_temp;
                chi2_best_reverse = chi2_temp;
              }
            }

            if (chi2_best < chi2_best_reverse) {
              mergeTracks(cand1, candidateToMergeBest, remove_hits);
              m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand1);
              it = m_trackList.begin();
              tracks_merged++;
            } else {
              mergeTracks(candidateToMergeBest, candidateToMergeBestReverse, remove_hits);
              m_cdcLegendreTrackFitter->fitTrackCandidateFast(candidateToMergeBest);
              it = m_trackList.begin();
              tracks_merged++;
            }

            //      mergeTracks(cand1, candidateToMergeBest);
            //      m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand1);
      */

      TrackCandidate* bestFitTrackCandidate = candidateToMergeBest.first;
      mergeTracks(cand1, bestFitTrackCandidate);
      have_merged_something = true;
    }

    B2DEBUG(100, "Cand hits vector size = " << cand1->getTrackHits().size());
    B2DEBUG(100, "Cand R = " << cand1->getR());

    trackList.erase(std::remove_if(trackList.begin(), trackList.end(),
    [&](TrackCandidate * cand) { return (cand->getTrackHits().size() < 3); }),
    trackList.end());

  } while (have_merged_something);

  B2DEBUG(100, "Merger: Resulting nCands = " << trackList.size());

  for (TrackCandidate * cand : trackList) {
    for (TrackHit * hit : cand->getTrackHits()) {
      hit->setHitUsage(TrackHit::used_in_track);
    }
  }

}


void TrackMerger::removeStrangeHits(double factor, std::vector<TrackHit*>& trackHits, std::pair<double, double>& track_par, std::pair<double , double>& ref_point)
{

  // Maybe it is better to use the assignment probability here also? -> SimpleFilter
  for (TrackHit * hit : trackHits) {
    double x0_hit = hit->getOriginalWirePosition().X();
    double y0_hit = hit->getOriginalWirePosition().Y();
    double x0_track = cos(track_par.first) / fabs(track_par.second) + ref_point.first;
    double y0_track = sin(track_par.first) / fabs(track_par.second) + ref_point.second;
    double dist = fabs(fabs(1 / fabs(track_par.second) - sqrt((x0_track - x0_hit) * (x0_track - x0_hit) + (y0_track - y0_hit) * (y0_track - y0_hit))) - hit->getDriftLength());
    if (dist > hit->getDriftLength() * factor) {
      hit->setHitUsage(TrackHit::bad);
    }
  }
  trackHits.erase(std::remove_if(trackHits.begin(), trackHits.end(),
  [&](TrackHit * hit) {
    return hit->getHitUsage() == TrackHit::bad;
  }), trackHits.end());
}

double TrackMerger::doTracksFitTogether(TrackCandidate* cand1, TrackCandidate* cand2, TrackFitter* trackFitter)
{
  // Check if the two tracks do have something in common!
  /*TVector3 deltaMomentum = cand1->getMomentumEstimation() - cand2->getMomentumEstimation();
  TVector3 deltaPosition = cand1->getReferencePoint() - cand2->getReferencePoint();

  if(deltaMomentum.Mag() / cand1->getMomentumEstimation().Mag() < 0.01) {
    B2INFO("Quick");
    return 1;
  }*/

  // Build common hit list
  std::vector<TrackHit*> commonHitListOfTwoTracks;
  for (TrackHit * hit : cand1->getTrackHits()) {
    commonHitListOfTwoTracks.push_back(hit);
  }
  for (TrackHit * hit : cand2->getTrackHits()) {
    commonHitListOfTwoTracks.push_back(hit);
  }

  // Sorting is done via pointer addresses (!!). This is not very stable and also not very meaningful (in terms of physical meaning),
  // but it dies the job.
  std::sort(commonHitListOfTwoTracks.begin(), commonHitListOfTwoTracks.end());

  commonHitListOfTwoTracks.erase(
    std::unique(commonHitListOfTwoTracks.begin(), commonHitListOfTwoTracks.end()),
    commonHitListOfTwoTracks.end());

  for (TrackHit * hit : commonHitListOfTwoTracks) {
    hit->setHitUsage(TrackHit::used_in_track);
  }

  // Calculate track parameters
  std::pair<double, double> ref_point = std::make_pair(0., 0.);
  std::pair<double, double> track_par = std::make_pair(-999, -999); // theta; R

  double chi2_temp;

  // Approach the best fit
  chi2_temp = trackFitter->fitTrackCandidateFast(commonHitListOfTwoTracks, track_par, ref_point);
  removeStrangeHits(5, commonHitListOfTwoTracks, track_par, ref_point);
  chi2_temp = trackFitter->fitTrackCandidateFast(commonHitListOfTwoTracks, track_par, ref_point);
  removeStrangeHits(3, commonHitListOfTwoTracks, track_par, ref_point);
  chi2_temp = trackFitter->fitTrackCandidateFast(commonHitListOfTwoTracks, track_par, ref_point);
  removeStrangeHits(1, commonHitListOfTwoTracks, track_par, ref_point);
  chi2_temp = trackFitter->fitTrackCandidateFast(commonHitListOfTwoTracks, track_par, ref_point);
  removeStrangeHits(1, commonHitListOfTwoTracks, track_par, ref_point);
  chi2_temp = trackFitter->fitTrackCandidateFast(commonHitListOfTwoTracks, track_par, ref_point);

  int charge = TrackCandidate::getChargeAssumption(track_par.first, track_par.second, commonHitListOfTwoTracks);

  if (charge == TrackCandidate::charge_two_tracks) {
    return 0;
  }

  // Dismiss this possibility if the hit list size after all the removing of hits is even smaller than the two lists before or if the list is too small
  if (commonHitListOfTwoTracks.size() <= max(cand2->getTrackHits().size(), cand1->getTrackHits().size())
      or commonHitListOfTwoTracks.size() < 15) {
    return 0;
  }

  unsigned int ndf = commonHitListOfTwoTracks.size() - 4;
  return TMath::Prob(chi2_temp * ndf, ndf);
}


/* REPLACED BY OTHER METHOD
void TrackMerger::doTracksMerging()
{

  int ii = 0;

  B2DEBUG(100, "Merger: Initial nCands = " << m_trackList.size());

  bool merging_done(false);
  bool remove_hits(true);

  do {

    int tracks_merged = 0;

    //loop over all candidates
    for (std::list<TrackCandidate*>::iterator it1 =
           m_trackList.begin(); it1 != m_trackList.end(); ++it1) {
      TrackCandidate* cand1 = *it1;

//      if(cand1->getCandidateType() ==  TrackCandidate::goodTrack) continue;
      if (cand1->getTrackHits().size() < 3) continue;
//      if (remove_hits && (cand1->getPt() < 0.9)) continue;
      m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand1);

      double chi2_best = 999;
      TrackCandidate* candidateToMergeBest = NULL;

      //loop over remaining candidates
      std::list<TrackCandidate*>::iterator it2 = std::next(it1);
      while (it2 != m_trackList.end()) {
        TrackCandidate* cand2 = *it2;
        ++it2;

//        if(cand2->getCandidateType() ==  TrackCandidate::goodTrack) continue;

        if (cand2->getTrackHits().size() < 3) continue;
//        if (remove_hits && (cand2->getPt() < 0.9)) continue;
        m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand2);
        double chi2_temp = doTracksFitTogether(cand1, cand2, remove_hits);
        for (TrackHit * hit : cand1->getTrackHits()) {
          hit->setHitUsage(TrackHit::used_in_track);
        }
        for (TrackHit * hit : cand2->getTrackHits()) {
          hit->setHitUsage(TrackHit::used_in_track);
        }

        if (chi2_best > chi2_temp) {
          candidateToMergeBest = cand2;
          chi2_best = chi2_temp;
        }

      }

      if (chi2_best < 1.) {
        //      B2INFO("chi2_best: " << chi2_best << "; chi2_track1: " << chi2_track1);

        double chi2_best_reverse = 999.;
        TrackCandidate* candidateToMergeBestReverse = NULL;
        for (TrackCandidate * cand_temp : m_trackList) {
          if (cand_temp == cand1) continue;
          if (cand_temp == candidateToMergeBest) continue;
          if (cand_temp->getTrackHits().size() < 3) continue;
//          if(cand_temp->getCandidateType() ==  TrackCandidate::goodTrack) continue;


          m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand_temp);
          double chi2_temp = doTracksFitTogether(candidateToMergeBest, cand_temp, remove_hits);
          for (TrackHit * hit : candidateToMergeBest->getTrackHits()) {
            hit->setHitUsage(TrackHit::used_in_track);
          }
          for (TrackHit * hit : cand_temp->getTrackHits()) {
            hit->setHitUsage(TrackHit::used_in_track);
          }

          if (chi2_best_reverse > chi2_temp) {
            candidateToMergeBestReverse = cand_temp;
            chi2_best_reverse = chi2_temp;
          }
        }

        if (chi2_best < chi2_best_reverse) {
          mergeTracks(cand1, candidateToMergeBest, remove_hits );
m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand1);
it1 = m_trackList.begin();
tracks_merged++;
} else {
mergeTracks(candidateToMergeBest, candidateToMergeBestReverse, remove_hits);
m_cdcLegendreTrackFitter->fitTrackCandidateFast(candidateToMergeBest);
it1 = m_trackList.begin();
tracks_merged++;
}

//      mergeTracks(cand1, candidateToMergeBest);
//      m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand1);

ii++;
B2DEBUG(100, "ii = " << ii);

}

B2DEBUG(100, "Cand hits vector size = " << cand1->getTrackHits().size());
B2DEBUG(100, "Cand R = " << cand1->getR());

}

if (tracks_merged == 0 && not remove_hits)merging_done = true;
if (tracks_merged == 0 && remove_hits)remove_hits  = false;
//    tracks_merged = 0;
B2DEBUG(100, "tracks_merged = " << tracks_merged)

m_trackList.erase(std::remove_if(m_trackList.begin(), m_trackList.end(),
[&](TrackCandidate * cand) {
return (cand->getTrackHits().size() < 3);
}), m_trackList.end());

} while (not merging_done);

m_trackList.erase(std::remove_if(m_trackList.begin(), m_trackList.end(),
[&](TrackCandidate * cand) {
return (cand->getTrackHits().size() < 6);
}), m_trackList.end());

B2DEBUG(100, "Merger: Resulting nCands = " << m_trackList.size());

for (TrackCandidate * cand : m_trackList) {
for (TrackHit * hit : cand->getTrackHits()) {
hit->setHitUsage(TrackHit::used_in_track);
}
}

}*/


// UNUSED
/*
 //можно сделать следование по треклетам - берем первый с той стороны, где больше хитов, потом добавляем второй, если норм - добавляем третий и т.д. если не норм - удаляем первый, но последний добавленый оставляем; если хорошо зашло - продолжаем без первого

 struct TrackletToMerge {
   std::vector<TrackHit*> hits;
   int charge;
   int sLayer;
   TrackCandidate* cand;
   int nhits;
 };

 std::vector<TrackletToMerge> listOfTracklets;

 std::vector<TrackHit*> h_list_1[2][5];
 std::vector<TrackHit*> h_list_2[2][5];

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
 for (TrackHit * hit : cand1->getTrackHits()) {
   if((hit->getCurvatureSignWrt(xc1, yc1)==TrackCandidate::charge_positive) && (hit->getSuperlayerId() == jj*2))h_list_1[0][jj].push_back(hit);
   if((hit->getCurvatureSignWrt(xc1, yc1)==TrackCandidate::charge_negative) && (hit->getSuperlayerId() == jj*2))h_list_1[1][jj].push_back(hit);
 }
 for (TrackHit * hit : cand2->getTrackHits()) {
   if((hit->getCurvatureSignWrt(xc2, yc2)==TrackCandidate::charge_positive) && (hit->getSuperlayerId() == jj*2))h_list_2[0][jj].push_back(hit);
   if((hit->getCurvatureSignWrt(xc2, yc2)==TrackCandidate::charge_negative) && (hit->getSuperlayerId() == jj*2))h_list_2[1][jj].push_back(hit);
 }
}


for(int jj=0; jj<5; jj++){
 TrackletToMerge newTracklet;
 newTracklet.sLayer = jj*2;
 newTracklet.charge = TrackCandidate::charge_positive;
 newTracklet.cand = cand1;
 listOfTracklets.push_back(newTracklet);
}
for(int jj=0; jj<5; jj++){
 TrackletToMerge newTracklet;
 newTracklet.sLayer = jj*2;
 newTracklet.charge = TrackCandidate::charge_negative;
 newTracklet.cand = cand1;
 listOfTracklets.push_back(newTracklet);
}
for(int jj=0; jj<5; jj++){
 TrackletToMerge newTracklet;
 newTracklet.sLayer = jj*2;
 newTracklet.charge = TrackCandidate::charge_positive;
 newTracklet.cand = cand2;
 listOfTracklets.push_back(newTracklet);
}
for(int jj=0; jj<5; jj++){
 TrackletToMerge newTracklet;
 newTracklet.sLayer = jj*2;
 newTracklet.charge = TrackCandidate::charge_negative;
 newTracklet.cand = cand2;
 listOfTracklets.push_back(newTracklet);
}

for (TrackHit * hit : cand1->getTrackHits()) {
 int charge_temp = hit->getCurvatureSignWrt(xc1, yc1);
 for(TrackletToMerge& trackletToFill : listOfTracklets){
   if(charge_temp == trackletToFill.charge && hit->getSuperlayerId() == trackletToFill.sLayer && trackletToFill.cand == cand1) trackletToFill.hits.push_back(hit);
 }
}
for (TrackHit * hit : cand2->getTrackHits()) {
 int charge_temp = hit->getCurvatureSignWrt(xc2, yc2);
 for(TrackletToMerge& trackletToFill : listOfTracklets){
   if(charge_temp == trackletToFill.charge && hit->getSuperlayerId() == trackletToFill.sLayer && trackletToFill.cand == cand2) trackletToFill.hits.push_back(hit);
 }
}


for(TrackletToMerge& trackletToFill : listOfTracklets){
 trackletToFill.nhits = trackletToFill.hits.size();
}


 std::vector<TrackHit*> c_list_temp_1;
for(int ii=0; ii<2; ii++){
 for(int jj=0; jj<5; jj++){
   if((h_list_1[ii][jj].size() == 0) && (h_list_2[ii][jj].size() != 0)) {
     for (TrackHit * hit : h_list_2[ii][jj]) {
         c_list_temp_1.push_back(hit);
     }
   }
   if((h_list_1[ii][jj].size() != 0) && (h_list_2[ii][jj].size() == 0)) {
     for (TrackHit * hit : h_list_1[ii][jj]) {
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


 std::vector<TrackHit*> c_list_temp_2;
 for (TrackHit * hit : cand1->getTrackHits()) {
   c_list_temp_2.push_back(hit);
 }
 for (TrackHit * hit : cand2->getTrackHits()) {
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
  std::vector<TrackHit*> c_list_temp_new;
  for (TrackHit * hit : c_list_temp) {
    c_list_temp_new.push_back(hit);
  }
  for (TrackHit * hitToRemove : c_list_temp) {
    c_list_temp_new.erase(std::remove(c_list_temp_new.begin(), c_list_temp_new.end(), hitToRemove), c_list_temp_new.end());
    double chi2_temp_new = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp_new, track_par, ref_point) / c_list_temp_new.size();
    if (chi2_temp_new > chi2_temp) c_list_temp_new.push_back(hitToRemove);
  }
  double chi2_temp_new = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp_new, track_par, ref_point) / c_list_temp_new.size();
  if (chi2_temp > chi2_temp_new) {

          c_list_temp.clear();
          for (TrackHit * hit : c_list_temp_new) {
            c_list_temp.push_back(hit);
          }
          chi2_temp = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp, track_par, ref_point) / c_list_temp.size();
          for(TrackHit* hitToRemove: c_list_temp){
            c_list_temp_new.erase(std::remove(c_list_temp_new.begin(), c_list_temp_new.end(), hitToRemove), c_list_temp_new.end());
            double chi2_temp_new = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp_new, track_par, ref_point) / c_list_temp_new.size();
            if(chi2_temp_new > chi2_temp) c_list_temp_new.push_back(hitToRemove);
          }
          double chi2_temp_new = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp_new, track_par, ref_point) / c_list_temp_new.size();
          if (chi2_temp > chi2_temp_new)
          {
            c_list_temp.clear();
            for (TrackHit * hit : c_list_temp_new) {
              c_list_temp.push_back(hit);
            }
            chi2_temp = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp, track_par, ref_point) / c_list_temp.size();
          }

     chi2_temp = chi2_temp_new;
   }
 }
*/


// UNUSED

/*
void TrackMerger::mergeTracks(TrackCandidate* cand1,
                              const std::pair<std::vector<TrackHit*>, std::pair<double, double> >& track,
                              std::set<TrackHit*>& )
{

    cand1->setR(
      (cand1->getR() * cand1->getNHits() + track.second.second * track.first.size())
      / (cand1->getNHits() + track.second.second * track.first.size()));
    cand1->setTheta(
      (cand1->getTheta() * cand1->getNHits()
       + track.second.first * track.second.second * track.first.size())
      / (cand1->getNHits() + track.second.second * track.first.size()));

for (TrackHit * hit : track.first) {
      cand1->addHit(hit);
      hit->setHitUsage(TrackHit::used_in_track);

double R = cand1->getRadius();
double x0_track = cand1->getXc() ;
double y0_track = cand1->getYc();
double x0_hit = hit->getOriginalWirePosition().X();
double y0_hit = hit->getOriginalWirePosition().Y();
double dist = SQR(fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftLength());
if (hit->getDriftLength() / 2. < dist) {
  cand1->addHit(hit);
  hit->setHitUsage(TrackHit::used_in_track);
//        hits_set.erase(hit);
}
}

//  m_trackList.remove(cand2);
//  delete cand2;
//  cand2 = NULL;
}*/


/*
double TrackMerger::checkDist(TrackHit* hit, TrackCandidate* track)
{
  double x0_track = track->getXc();
  double y0_track = track->getYc();
  double R = track->getRadius();

  double x0_hit = hit->getOriginalWirePosition().X();
  double y0_hit = hit->getOriginalWirePosition().Y();
  double dist = fabs(fabs(R - sqrt((x0_track - x0_hit) * (x0_track - x0_hit) + (y0_track - y0_hit) * (y0_track - y0_hit))) - hit->getDriftLength());

  return dist;
}*/

/*
void TrackMerger::splitTracks()
{
  for (TrackCandidate * cand : m_trackList) {
//    if(cand->getCandidateType() == TrackCandidate::tracklet) continue;
//    double theta = cand->getTheta();
//    double r = cand->getR();
    double yc = cand->getXc();
    double xc = cand->getYc();

    int hits_pos = 0;
    int hits_neg = 0;

    for (TrackHit * Hit : cand->getTrackHits()) {
      int curve_sign = Hit->getCurvatureSignWrt(xc, yc);

      if (curve_sign == TrackCandidate::charge_positive)
        ++hits_pos;
      else if (curve_sign == TrackCandidate::charge_negative)
        ++hits_neg;
      else {
        B2ERROR(
          "Strange behaviour of TrackHit::getCurvatureSignWrt");
        exit(EXIT_FAILURE);
      }
    }

    if ((hits_pos != 0) && (hits_neg != 0)) {
      std::vector<TrackHit*> hitsForNewTrack;
      int chargeNewTrack;
      if (hits_pos > hits_neg)
        chargeNewTrack = TrackCandidate::charge_negative;
      else
        chargeNewTrack = TrackCandidate::charge_positive;

      for (TrackHit * hit : cand->getTrackHits()) {
        int curve_sign = hit->getCurvatureSignWrt(xc, yc);
        if (curve_sign == chargeNewTrack) hitsForNewTrack.push_back(hit);
      }

      for (TrackHit * hit : hitsForNewTrack) {
//        cand->removeHit(hit);
        hit->setHitUsage(TrackHit::not_used);
      }

      cand->getTrackHits().erase(std::remove_if(cand->getTrackHits().begin(), cand->getTrackHits().end(),
      [&](TrackHit * hit) {
        return hit->getHitUsage() == TrackHit::not_used;
      }), cand->getTrackHits().end());


      if (hitsForNewTrack.size() > 5) {
        TrackCandidate* newTracklet = m_cdcLegendreTrackProcessor->createLegendreTracklet(hitsForNewTrack);
        newTracklet->setCandidateType(TrackCandidate::tracklet);
        newTracklet->reestimateCharge();
        PatternChecker cdcLegendrePatternChecker(m_cdcLegendreTrackProcessor);
        cdcLegendrePatternChecker.checkCandidate(newTracklet);
        cdcLegendrePatternChecker.checkCandidate(cand);
        m_cdcLegendreTrackFitter->fitTrackCandidateFast(newTracklet);
      } else {
        hitsForNewTrack.clear();
      }

      cand->reestimateCharge();

      m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand);




    }

  }
}*/

/*
bool TrackMerger::earlyCandidateMerge(std::pair < std::vector<TrackHit*>,
                                      std::pair<double, double> > & candidate, std::set<TrackHit*>& hits_set,
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
    for (std::list<TrackCandidate*>::iterator it1 =
           m_trackList.begin(); it1 != m_trackList.end(); ++it1) {
      TrackCandidate* cand1 = *it1;

      bool make_merge __attribute__((unused)) = false;

      double chi2_track = cand1->getChi2();

      double x0_track = cand1->getXc();
      double y0_track = cand1->getYc();

      //        if((fabs(x0_track-x0_cand)/x0_track < 0.1)&&(fabs(y0_track-y0_cand)/y0_track < 0.1))make_merge = true;
      //        else

      {
        int n_overlapp = 0;
        double R = cand1->getRadius();
        for (TrackHit * hit : candidate.first) {
          double x0_hit = hit->getOriginalWirePosition().X();
          double y0_hit = hit->getOriginalWirePosition().Y();
          double dist = fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftLength();
          if (dist < hit->getDriftLength() * 3.)n_overlapp++;
        }
        if (n_overlapp > 3)make_merge = true;
        else {
          n_overlapp = 0;
          R = fabs(1. / candidate.second.second);
          for (TrackHit * hit : cand1->getTrackHits()) {
            double x0_hit = hit->getOriginalWirePosition().X();
            double y0_hit = hit->getOriginalWirePosition().Y();
            double dist = fabs(R - sqrt(SQR(x0_cand - x0_hit) + SQR(y0_cand - y0_hit))) - hit->getDriftLength();
            if (dist < hit->getDriftLength() * 3.)n_overlapp++;
          }
          if (n_overlapp > 3)make_merge = true;

        }
      }

      if (true) {
        std::vector<TrackHit*> c_list_temp;
        std::pair<std::vector<TrackHit*>, std::pair<double, double> > candidate_temp =
          std::make_pair(c_list_temp, std::make_pair(-999, -999));
        for (TrackHit * hit : candidate.first) {
          candidate_temp.first.push_back(hit);
        }
        for (TrackHit * hit : cand1->getTrackHits()) {
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
} */

/*
void TrackMerger::addStereoTracklesToTrack()
{*/
/*
 * First, we are calculating position of the center of stereo tracklet, then trying to add tracklet to track and define z-position of tracklet with
 * formula: z = ((s_wire_back-s_wire_forward)_z/(s_wire_back-s_wire_forward)_xy)*(s_wire - ax_wire)_xy
 * Then we can estimate z-position as z=(alpha/2pi)*h, where cos(alpha) = 1 - ((s_wire_x - x_ref)^2 + (s_wire_y - y_ref)^2)/(2 * R^2) ; h = 2pi*R*ctg(theta)
 */
/*B2INFO("Number of tracks: " << m_trackList.size());
B2INFO("Number of stereo tracklets: " << m_stereoTrackletList.size());
for (TrackCandidate * track : m_trackList) {

  for (TrackCandidate * stereoTracklet : m_stereoTrackletList) {

    B2INFO("Number of hits in stereo tracklet: " << stereoTracklet->getNHits());
    double x0_tracklet = 0, y0_tracklet = 0; // holds geometrical center of cluster of stereohits (tracklet)

    for (TrackHit * hit : stereoTracklet->getTrackHits()) {
      x0_tracklet += hit->getWirePosition().X();
      y0_tracklet += hit->getWirePosition().Y();
    }

    x0_tracklet = x0_tracklet / stereoTracklet->getNHits();
    y0_tracklet = y0_tracklet / stereoTracklet->getNHits();


    double dist = SQR(fabs(track->getRadius() - sqrt(SQR(x0_tracklet - track->getXc()) + SQR(y0_tracklet - track->getYc()))));

    if (dist < 40) {
//        mergeTracks(track, stereoTracklet);

      for (TrackHit * hit : stereoTracklet->getTrackHits()) {
        track->addHit(hit);
        hit->setHitUsage(true);
      }

      B2INFO("MERGED!");

      fitStereoTrackletsToTrack(track, stereoTracklet);
    }

  }

}
}*/
/*
double TrackMerger::fitStereoTrackletsToTrack(TrackCandidate* track, TrackCandidate* tracklet)
{

  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();

  TVector3 forwardWirePoint; //forward end of the wire
  TVector3 backwardWirePoint; //backward end of the wire
  TVector3 mediumWirePoint; //center of wire
  TVector3 wireVector;  //direction of the wire

  int sLayer = 0;
  int sign = 0;

  for (TrackHit * hit : tracklet->getTrackHits()) {
    sLayer = hit->getSuperlayerId();
  }

  if (sLayer == 1 || sLayer == 5) sign = -1;
  else sign = 1;

//  sign *= track->getCharge();


  for (TrackHit * hit : tracklet->getTrackHits()) {
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

    double dist_1 = track->getRadius() - sqrt(SQR(hit->getOriginalWirePosition().X() - track->getXc()) + SQR(hit->getOriginalWirePosition().Y() - track->getYc())) - hit->getDriftLength();
    double dist_2 = track->getRadius() - sqrt(SQR(hit->getOriginalWirePosition().X() - track->getXc()) + SQR(hit->getOriginalWirePosition().Y() - track->getYc())) + hit->getDriftLength();

    double alpha = acos(1. - (SQR(hit->getOriginalWirePosition().X() - track->getReferencePoint().X()) + SQR(hit->getOriginalWirePosition().Y() - track->getReferencePoint().Y())) / (2.*SQR(track->getRadius())));

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

    double __attribute__((unused)) omega_1 = Zpos_1 / (alpha * track->getRadius());
    double __attribute__((unused)) omega_2 = Zpos_2 / (alpha * track->getRadius());

//    double theta_track_1 = (atan(omega_1) + 3.1415/2 ) * 180. / 3.1415;
//    double theta_track_2 = (atan(omega_2) + 3.1415/2 ) * 180. / 3.1415;
    double theta_track_1 = (atan2(Zpos_1 , (alpha * track->getRadius())) + 3.1415 / 2) * 180. / 3.1415;
    double theta_track_2 = (atan2(Zpos_2 , (alpha * track->getRadius())) + 3.1415 / 2) * 180. / 3.1415;

    B2INFO("THETA_track_1 angle = " << theta_track_1 << " THETA_track_2 angle = " << theta_track_2);


  }



  return 0;

}*/

/*
void TrackMerger::extendTracklet(TrackCandidate* tracklet, std::vector<TrackHit*>& m_AxialHitList)
{
  return ;
  std::vector<TrackHit*> hits;

  double x0_track = tracklet->getXc();
  double y0_track = tracklet->getYc();
  double R = tracklet->getRadius();
  double dist_min = 999;
  double x_pos, y_pos;


  for (TrackHit * hit : tracklet->getTrackHits()) {
//      if (hit->isUsed() != TrackHit::used_in_track || hit->isUsed() != TrackHit::background) {
    double x0_hit = hit->getOriginalWirePosition().X();
    double y0_hit = hit->getOriginalWirePosition().Y();
    double dist = fabs(fabs(R - sqrt((x0_track - x0_hit) * (x0_track - x0_hit) + (y0_track - y0_hit) * (y0_track - y0_hit))) - hit->getDriftLength());
    if (dist < dist_min) {
      dist_min = dist;
      x_pos = x0_hit + hit->getDriftLength() * cos(atan2(x0_track - x0_hit, y0_track - y0_hit));
      y_pos = y0_hit + hit->getDriftLength() * sin(atan2(x0_track - x0_hit, y0_track - y0_hit));
//          x_pos = x0_hit;
//          y_pos = y0_hit;
//          x_pos = x0_track + R*cos(atan2(x0_track - x0_hit, y0_track - y0_hit));
//          y_pos = y0_track + R*sin(atan2(x0_track - x0_hit, y0_track - y0_hit));
    }
//      }
  }

//  x_pos = tracklet->getReferencePoint().X();
//  y_pos = tracklet->getReferencePoint().Y();

  TVector3 reference;
  reference.SetZ(0);
//    reference.SetX(tracklet->getTrackHits()[1]->getOriginalWirePosition().X());
//    reference.SetY(tracklet->getTrackHits()[1]->getOriginalWirePosition().X());

//  try this
//  x_pos = tracklet->getReferencePoint().X();
//  y_pos = tracklet->getReferencePoint().Y();

  reference.SetX(x_pos);
  reference.SetY(y_pos);


  double dist_hit_min = 999.;
  TVector3 pos(0, 0, 0);

  for (TrackHit * hit : tracklet->getTrackHits()) {
    if (not hit->getIsAxial()) continue;

    double dist_hit = hit->getOriginalWirePosition().Pt();

    if (dist_hit < dist_hit_min) {
      dist_hit_min = dist_hit;
      pos.SetXYZ(hit->getOriginalWirePosition().X(), hit->getOriginalWirePosition().Y(), 0);
    }

  }

//  x_pos = pos.X();
//  y_pos = pos.Y();


  int innermostSLayer = tracklet->getInnermostSLayer(2);
  int outermostSLayer = tracklet->getOutermostSLayer(2);
  for (TrackHit * axialHit : m_AxialHitList) {
    if (axialHit->getHitUsage() == TrackHit::used_in_track) continue;
    if ((axialHit->getSuperlayerId() > outermostSLayer) || (axialHit->getSuperlayerId() < innermostSLayer)) {
//      TrackHit* trackHit = new TrackHit(*axialHit);
//      trackHit->setPosition(axialHit->getOriginalWirePosition() - reference);
//      trackHit->setHitUsage(TrackHit::not_used);
      hits.push_back(axialHit);
    }
  }

  for (TrackHit * hit : tracklet->getTrackHits()) {
    hits.push_back(hit);
  }


  std::vector<TrackHit*> c_list1;
  std::pair<std::vector<TrackHit*>, std::pair<double, double> > cand = std::make_pair(c_list1, std::make_pair(-999, -999));

  std::pair<double, double> refPoint = std::make_pair(x_pos, y_pos);
  m_cdcLegendreFastHough->FastHoughCustomReferencePoint(&cand, hits, refPoint, 1, 0, 2048, -0.15, 0.15, 9);


  for (TrackHit * hit : cand.first) {
    if (hit->getHitUsage() != TrackHit::used_in_track) {
      hit->setHitUsage(TrackHit::used_in_track);
      tracklet->addHit(hit);
    }
  }


  tracklet->getTrackHits().erase(
    std::unique(tracklet->getTrackHits().begin(), tracklet->getTrackHits().end()),
    tracklet->getTrackHits().end());


  return;


  for (TrackHit * axialHit : tracklet->getTrackHits()) {
    TrackHit* trackHit = new TrackHit(*axialHit);
    trackHit->setPosition(axialHit->getOriginalWirePosition() - reference);
    trackHit->setHitUsage(TrackHit::not_used);
    hits.push_back(trackHit);
  }

  B2INFO("Size of hits vector: " << hits.size());

  B2INFO("Size of tracklet: " << tracklet->getTrackHits().size());

  std::vector<TrackHit*> c_list;
  std::pair<std::vector<TrackHit*>, std::pair<double, double> > candidate = std::make_pair(c_list, std::make_pair(-999, -999));

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

    for (TrackHit * axialHit : m_AxialHitList) {
      if (axialHit->getHitUsage() == TrackHit::used_in_track) continue;
      TrackHit* trackHit = new TrackHit(*axialHit);
      for (int t_index = 0; t_index < 2; ++t_index) {
        trackHit->setPosition(axialHit->getOriginalWirePosition() - reference);

        double r_temp = trackHit->getConformalX() * cos(theta_bin[t_index]) + trackHit->getConformalY() * sin(theta_bin[t_index]);

        double r_1 = r_temp + trackHit->getConformalDriftLength();
        double r_2 = r_temp - trackHit->getConformalDriftLength();

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

    std::vector<TrackHit*> hitsToAdd;
    double preinitialChi2 = tracklet->getChi2();
    m_cdcLegendreTrackFitter->fitTrackCandidateFast(tracklet);
    double initialChi2 = tracklet->getChi2();

    for(TrackHit *hit: candidate.first)
    {
      for (TrackHit * axialHit : m_AxialHitList) {
        if(axialHit->getStoreIndex() == hit->getStoreIndex())
              hitsToAdd.push_back(axialHit);
      }
    }

    hitsToAdd.erase(std::remove_if(hitsToAdd.begin(), hitsToAdd.end(), [&tracklet](TrackHit * hit)
        {
          for(TrackHit * axialHit : tracklet->getTrackHits()){
            if(axialHit->getStoreIndex() == hit->getStoreIndex()) return true;
          }
          return false;
        })
        , hitsToAdd.end());



    for(TrackHit *hit: hitsToAdd){
      tracklet->addHit(hit);
      hit->setHitUsage(TrackHit::used_in_track);
    }

    B2INFO("New size of tracklet: " << tracklet->getTrackHits().size());


    m_cdcLegendreTrackFitter->fitTrackCandidateFast(tracklet);

    double newChi2 = tracklet->getChi2();

    B2INFO("Initial Chi2: " << initialChi2 << "; new Chi2: " << newChi2 << ": preinitialChi2: " << preinitialChi2);
*/

/*    if(2.*newChi2/tracklet->getNAxialHits() > initialChi2/tracklet->getNAxialHits())
    {
      for(TrackHit *hit: hitsToAdd){
        tracklet->removeHit(hit);
        hit->setHitUsage(TrackHit::not_used);
      }
    }
*/
/*
  for (TrackHit * hit : hits) {
    delete hit;
  }
  hits.clear();



}*/

/*
double TrackMerger::selectCoreMergeFit(TrackCandidate* cand1, TrackCandidate* cand2)
{
  std::vector<TrackHit*> hitList1;
  for (TrackHit * hit : cand1->getTrackHits()) {
    if (checkDist(hit, cand1) <  4.*hit->getSigmaDriftLength()) hitList1.push_back(hit);
  }

  std::vector<TrackHit*> hitList2;
  for (TrackHit * hit : cand2->getTrackHits()) {
    if (checkDist(hit, cand2) <  4.*hit->getSigmaDriftLength()) hitList2.push_back(hit);
  }

  std::vector<TrackHit*> c_list_temp;
  for (TrackHit * hit : hitList1) {
    c_list_temp.push_back(hit);
  }
  for (TrackHit * hit : hitList2) {
    c_list_temp.push_back(hit);
  }


  std::pair<double, double> ref_point = std::make_pair(0., 0.);
  std::pair<double, double> track_par = std::make_pair(-999, -999);
  double chi2_temp = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp, track_par, ref_point) / c_list_temp.size();

  return chi2_temp;

} */
/*
void TrackMerger::checkOverlapping()
{

  const int hitsSlightOverlapThreshold = 3;
  const int hitsHeavyOverlapThreshold = 8;
  const double overlapParametersThreshold = 0.1;


  for (std::list<TrackCandidate*>::iterator it1 =
         m_trackList.begin(); it1 != m_trackList.end(); ++it1) {
    TrackCandidate* cand1 = *it1;

    double chi2_cand1 = cand1->getChi2();

    double x0_track1 = cand1->getXc();
    double y0_track1 = cand1->getYc();


    std::list<TrackCandidate*>::iterator it2 = std::next(it1);
    while (it2 != m_trackList.end()) {
      TrackCandidate* cand2 = *it2;
      ++it2;

      bool isSlightOverlapping = false;
      bool __attribute__((unused)) isHeavyOverlapping = false;

      m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand2);
      double chi2_cand2 = cand2->getChi2();

      double x0_track2 = cand2->getXc();
      double y0_track2 = cand2->getYc();


      if ((fabs(x0_track1 - x0_track2) / x0_track1 < overlapParametersThreshold) &&
          (fabs(y0_track1 - y0_track2) / y0_track1 < overlapParametersThreshold) &&
          ((fabs(cand2->getR()) / fabs(fabs(cand2->getR()) - fabs(cand1->getR())) < overlapParametersThreshold) ||
           (fabs(cand1->getR()) / fabs(fabs(cand2->getR()) - fabs(cand1->getR())) < overlapParametersThreshold)))
        isHeavyOverlapping = true;
      else {
        int n_overlapp = 0;
        double R = cand1->getRadius();
        for (TrackHit * hit : cand2->getTrackHits()) {
          double x0_hit = hit->getOriginalWirePosition().X();
          double y0_hit = hit->getOriginalWirePosition().Y();
          double dist = fabs(R - sqrt(SQR(x0_track1 - x0_hit) + SQR(y0_track1 - y0_hit))) - hit->getDriftLength();
          if (dist < hit->getSigmaDriftLength() * 3.)n_overlapp++;
        }
        if (n_overlapp > hitsSlightOverlapThreshold) {
          isSlightOverlapping = true;
          if (n_overlapp > hitsHeavyOverlapThreshold) isHeavyOverlapping = true;
        } else {
          n_overlapp = 0;
          R = cand2->getRadius();
          for (TrackHit * hit : cand1->getTrackHits()) {
            double x0_hit = hit->getOriginalWirePosition().X();
            double y0_hit = hit->getOriginalWirePosition().Y();
            double dist = fabs(R - sqrt(SQR(x0_track2 - x0_hit) + SQR(y0_track2 - y0_hit))) - hit->getDriftLength();
            if (dist < hit->getSigmaDriftLength() * 3.)n_overlapp++;
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


            std::vector<TrackHit*> c_list_temp;
            for (TrackHit * hit : cand2->getTrackHits()) {
              c_list_temp.push_back(hit);
            }
            for (TrackHit * hit : cand1->getTrackHits()) {
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
            [&R, &x0_track, &y0_track](TrackHit * hit) {
              double x0_hit = hit->getOriginalWirePosition().X();
              double y0_hit = hit->getOriginalWirePosition().Y();
              double dist = fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftLength();
              if ((dist > hit->getDriftLength()) || (dist > hit->getSigmaDriftLength() * 2.)) {
                return true;
              } else {
                return false;
              }
            }), c_list_temp.end());

            chi2_temp = m_cdcLegendreTrackFitter->fitTrackCandidateFast(c_list_temp, track_par, ref_point) / c_list_temp.size();

            B2INFO("chi2_temp: " << chi2_temp << "; chi2_track1: " << chi2_cand1 << "; chi2_track2: " << chi2_cand2);

//        if (chi2_temp < (chi2_track1 + chi2_track2)) {
            if ((chi2_temp < 1.) ||
            (chi2_temp < chi2_cand1) ||
            (chi2_temp < chi2_cand2)) {

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
}*/
