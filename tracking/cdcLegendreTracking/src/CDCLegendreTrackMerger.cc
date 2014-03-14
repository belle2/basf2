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

#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackFitter.h>
#include <cmath>

#define SQR(x) ((x)*(x))

using namespace std;
using namespace Belle2;

CDCLegendreTrackMerger::CDCLegendreTrackMerger(
  std::list<CDCLegendreTrackCandidate*>& trackList, CDCLegendreTrackFitter* cdcLegendreTrackFitter):
  m_trackList(trackList), m_cdcLegendreTrackFitter(cdcLegendreTrackFitter)
{

}

void CDCLegendreTrackMerger::mergeTracks(CDCLegendreTrackCandidate* cand1, CDCLegendreTrackCandidate* cand2)
{

  cand1->setR(
    (cand1->getR() * cand1->getNHits() + cand2->getR() * cand2->getNHits())
    / (cand1->getNHits() + cand2->getNHits()));
  cand1->setTheta(
    (cand1->getTheta() * cand1->getNHits()
     + cand2->getTheta() * cand2->getNHits())
    / (cand1->getNHits() + cand2->getNHits()));

  for (CDCLegendreTrackHit * hit : cand2->getTrackHits()) {
    cand1->addHit(hit);
    hit->setUsed(true);
  }

  m_trackList.remove(cand2);
  delete cand2;
  cand2 = NULL;
}

void CDCLegendreTrackMerger::mergeTracks(CDCLegendreTrackCandidate* cand1,
                                         const std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >& track,
                                         std::set<CDCLegendreTrackHit*>& hits_set)
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

void CDCLegendreTrackMerger::MergeTracks()
{
  //loop over all full candidates
  /*  for (std::list<CDCLegendreTrackCandidate*>::iterator it1 =
        m_fullTrackList.begin(); it1 != m_fullTrackList.end(); ++it1) {
      CDCLegendreTrackCandidate* cand1 = *it1;

      //loop over remaining candidates

      std::list<CDCLegendreTrackCandidate*>::iterator it2 = boost::next(it1);
      while (it2 != m_fullTrackList.end()) {
        CDCLegendreTrackCandidate* cand2 = *it2;
        ++it2;

        //check if the two tracks lie next to each other
        if (fabs(cand1->getR() - cand2->getR()) < 0.03
            && fabs(cand1->getTheta() - cand2->getTheta()) < 0.15)

          mergeTracks(cand1, cand2);
      }

      for (std::list<CDCLegendreTrackCandidate*>::iterator it_short =
          m_shortTrackList.begin(); it_short != m_shortTrackList.end(); ++it_short) {
        CDCLegendreTrackCandidate* cand_short = *it_short;

        //check if the two tracks lie next to each other
        if (fabs(cand1->getR() - cand_short->getR()) < 0.03
            && fabs(cand1->getTheta() - cand_short->getTheta()) < 0.15)

          mergeTracks(cand1, cand_short);
      }

      for (std::list<CDCLegendreTrackCandidate*>::iterator it_short =
          m_trackletTrackList.begin(); it_short != m_trackletTrackList.end(); ++it_short) {
        CDCLegendreTrackCandidate* cand_short = *it_short;

        //check if the two tracks lie next to each other
        if (fabs(cand1->getR() - cand_short->getR()) < 0.03
            && fabs(cand1->getTheta() - cand_short->getTheta()) < 0.15)

          mergeTracks(cand1, cand_short);
      }

    }

    //loop over all short candidates
    for (std::list<CDCLegendreTrackCandidate*>::iterator it1 =
        m_shortTrackList.begin(); it1 != m_shortTrackList.end(); ++it1) {
      CDCLegendreTrackCandidate* cand1 = *it1;

      //loop over remaining candidates

      std::list<CDCLegendreTrackCandidate*>::iterator it2 = boost::next(it1);
      while (it2 != m_shortTrackList.end()) {
        CDCLegendreTrackCandidate* cand2 = *it2;
        ++it2;

        //check if the two tracks lie next to each other
        if (fabs(cand1->getR() - cand2->getR()) < 0.03
            && fabs(cand1->getTheta() - cand2->getTheta()) < 0.15)

          mergeTracks(cand1, cand2);
      }

      for (std::list<CDCLegendreTrackCandidate*>::iterator it_short =
          m_trackletTrackList.begin(); it_short != m_trackletTrackList.end(); ++it_short) {
        CDCLegendreTrackCandidate* cand_short = *it_short;

        //check if the two tracks lie next to each other
        if (fabs(cand1->getR() - cand_short->getR()) < 0.03
            && fabs(cand1->getTheta() - cand_short->getTheta()) < 0.15)

          mergeTracks(cand1, cand_short);
      }

    }*/

//  for (CDCLegendreTrackCandidate * trackCand : m_fullTrackList) {
//    m_trackList.push_back(new CDCLegendreTrackCandidate(*trackCand));
//  }
//  for (CDCLegendreTrackCandidate * trackCand : m_shortTrackList) {
//    m_trackList.push_back(new CDCLegendreTrackCandidate(*trackCand));
//  }

//  std::copy(m_fullTrackList.begin(), m_fullTrackList.end(), std::back_inserter(m_trackList));
//  std::copy(m_shortTrackList.begin(), m_shortTrackList.end(), std::back_inserter(m_trackList));

//  m_trackList.insert( m_trackList.end(), m_fullTrackList.begin(), m_fullTrackList.end() );
//  m_trackList.insert( m_trackList.end(), m_shortTrackList.begin(), m_shortTrackList.end() );

}

void CDCLegendreTrackMerger::MergeCurler()
{
  /*
    //loop over all candidates
    for (std::list<CDCLegendreTrackCandidate*>::iterator it1 =
           m_trackList.begin(); it1 != m_trackList.end(); ++it1) {
      CDCLegendreTrackCandidate* cand1 = *it1;

      //check only curler
      if (fabs(cand1->getR()) > m_rc) {

        //loop over remaining candidates
        std::list<CDCLegendreTrackCandidate*>::iterator it2 = boost::next(it1);
        while (it2 != m_trackList.end()) {
          CDCLegendreTrackCandidate* cand2 = *it2;
          ++it2;

          if (fabs(cand2->getR()) > m_rc) {

            //check if the two tracks lie next to each other
            if (fabs(cand1->getR() - cand2->getR()) < 0.03
                && fabs(cand1->getTheta() - cand2->getTheta()) < 0.15)
              return;
            //mergeTracks(cand1, cand2);
          }
        }
      }
    }
    */

  bool merged = false;
  bool make_merge = false;
  std::pair<double, double> ref_point = std::make_pair(0., 0.);

  double chi2_track1, chi2_track2;
  double x0_track1, y0_track1;
  double x0_track2, y0_track2;
  //loop over all candidates
  for (std::list<CDCLegendreTrackCandidate*>::iterator it1 =
         m_trackList.begin(); it1 != m_trackList.end(); ++it1) {
    CDCLegendreTrackCandidate* cand1 = *it1;

    chi2_track1 = cand1->getChi2();

    double x0_track1 = cos(cand1->getTheta()) / cand1->getR() + cand1->getReferencePoint().X();
    double y0_track1 = sin(cand1->getTheta()) / cand1->getR() + cand1->getReferencePoint().Y();

    //loop over remaining candidates
    std::list<CDCLegendreTrackCandidate*>::iterator it2 = std::next(it1);
    while (it2 != m_trackList.end()) {
      CDCLegendreTrackCandidate* cand2 = *it2;
      ++it2;

      chi2_track2 = cand2->getChi2();

      double x0_track2 = cos(cand2->getTheta()) / cand2->getR() + cand2->getReferencePoint().X();
      double y0_track2 = sin(cand2->getTheta()) / cand2->getR() + cand2->getReferencePoint().Y();


//    if((fabs(x0_track1-x0_track2)/x0_track1 < 0.1)&&(fabs(y0_track1-y0_track2)/y0_track1 < 0.1))make_merge = true;
//    else
      {
        int n_overlapp = 0;
        double R = fabs(1. / cand1->getR());
        for (CDCLegendreTrackHit * hit : cand2->getTrackHits()) {
          double x0_hit = hit->getOriginalWirePosition().X();
          double y0_hit = hit->getOriginalWirePosition().Y();
          double dist = fabs(R - sqrt(SQR(x0_track1 - x0_hit) + SQR(y0_track1 - y0_hit))) - hit->getDriftTime();
          if (dist < hit->getDriftTime() * 3.)n_overlapp++;
        }
        if (n_overlapp > 3)make_merge = true;
        else {
          n_overlapp = 0;
          R = fabs(1. / cand2->getR());
          for (CDCLegendreTrackHit * hit : cand1->getTrackHits()) {
            double x0_hit = hit->getOriginalWirePosition().X();
            double y0_hit = hit->getOriginalWirePosition().Y();
            double dist = fabs(R - sqrt(SQR(x0_track2 - x0_hit) + SQR(y0_track2 - y0_hit))) - hit->getDriftTime();
            if (dist < hit->getDriftTime() * 3.)n_overlapp++;
          }
          if (n_overlapp > 3)make_merge = true;

        }
      }

      if (true/*make_merge*/) {
        std::vector<CDCLegendreTrackHit*> c_list_temp;
        std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > candidate_temp =
          std::make_pair(c_list_temp, std::make_pair(-999, -999));
        for (CDCLegendreTrackHit * hit : cand2->getTrackHits()) {
          candidate_temp.first.push_back(hit);
        }
        for (CDCLegendreTrackHit * hit : cand1->getTrackHits()) {
          candidate_temp.first.push_back(hit);
        }

        double chi2_temp;
        std::pair<double, double> ref_point_temp = std::make_pair(0., 0.);
        m_cdcLegendreTrackFitter->fitTrackCandidateFast(&candidate_temp, ref_point_temp, chi2_temp);
        if (chi2_temp < SQR(sqrt(chi2_track1) + sqrt(chi2_track2)) * 2.) {

          mergeTracks(cand1, cand2);
          m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand1, ref_point, true);
          cand1->setReferencePoint(ref_point.first, ref_point.second);
          it2 = std::next(it1);
          /*        merged = true;
                  cand1->clearBadHits(ref_point);
                  cout << "MERGED!" << endl;
                  break;
          */

        } else make_merge = false;
      }

    }
    m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand1, ref_point);
    m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand1, ref_point, true);

  }

}

bool CDCLegendreTrackMerger::earlyCandidateMerge(std::pair < std::vector<CDCLegendreTrackHit*>,
                                                 std::pair<double, double> > & candidate, std::set<CDCLegendreTrackHit*>& hits_set,
                                                 bool fitTracksEarly)
{
  bool merged = false;
  //      cdcLegendreTrackFitter->fitTrackCandidateStepped(&candidate);
  std::pair<double, double> ref_point = std::make_pair(0., 0.);
  double chi2_cand;
  //      cdcLegendrePatternChecker->checkCandidate(&candidate);

  if (candidate.first.size() > 0) {
    //        cdcLegendreTrackFitter->fitTrackCandidateFast(&candidate, ref_point, chi2_cand);
    //      cdcLegendrePatternChecker->clearBadHits(&candidate, ref_point);

    /*
          for (CDCLegendreTrackHit* hit: candidate.first){
            double R = fabs(1. / candidate.second.second);
            double x0_track = cos(candidate.second.first) / candidate.second.second + ref_point.first;
            double y0_track = sin(candidate.second.first) / candidate.second.second + ref_point.second;
            double x0_hit = hit->getOriginalWirePosition().X();
            double y0_hit = hit->getOriginalWirePosition().Y();
            double dist = SQR(fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftTime());
            cout << "dist=" << dist << "||" << hit->getDriftTime() <<endl;
          }
          candidate.first.erase(std::remove_if(candidate.first.begin(), candidate.first.end(),
              [&candidate,&ref_point](CDCLegendreTrackHit * hit) {
                    double R = fabs(1. / candidate.second.second);
                    double x0_track = cos(candidate.second.first) / candidate.second.second + ref_point.first;
                    double y0_track = sin(candidate.second.first) / candidate.second.second + ref_point.second;
                    double x0_hit = hit->getOriginalWirePosition().X();
                    double y0_hit = hit->getOriginalWirePosition().Y();
                    double dist = SQR(fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftTime());
                    return hit->getDriftTime()/2. < dist;
                }), candidate.first.end());
          */

    double x0_cand = cos(candidate.second.first) / candidate.second.second + ref_point.first;
    double y0_cand = sin(candidate.second.first) / candidate.second.second + ref_point.second;

    bool make_merge = false;

    double chi2_track;
    //loop over all candidates
    for (std::list<CDCLegendreTrackCandidate*>::iterator it1 =
           m_trackList.begin(); it1 != m_trackList.end(); ++it1) {
      CDCLegendreTrackCandidate* cand1 = *it1;

      chi2_track = cand1->getChi2();

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

      if (/*true*/make_merge/*false*/) {
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
        //          cdcLegendrePatternChecker->clearBadHits(&candidate_temp, ref_point_temp);
        //          cdcLegendreTrackFitter->fitTrackCandidateFast(&candidate_temp, ref_point_temp, chi2_temp);
        //      cout << "clist_temp.size = " << candidate_temp.first.size() << endl;
        //      cout << "chi_cand=" << chi2_cand << " chi2_track=" << chi2_track << " chi2_temp=" << chi2_temp << endl;
        if (candidate_temp.first.size() == 0) {
          merged = true;
          break;
        }

        if (chi2_temp < SQR(sqrt(chi2_track) + sqrt(chi2_cand)) * 3.) {

          cand1->setR(candidate_temp.second.second);
          cand1->setTheta(candidate_temp.second.first);
          //            cand1->setReferencePoint(ref_point_temp.first, ref_point_temp.second);
          mergeTracks(cand1, candidate, hits_set);

          if (fitTracksEarly) m_cdcLegendreTrackFitter->fitTrackCandidateFast(cand1, ref_point);
          cand1->setReferencePoint(ref_point.first, ref_point.second);

          merged = true;
          //            cand1->clearBadHits(ref_point);
          //        cout << "MERGED!" << endl;
          break;
        } else make_merge = false;
      }
    }

  }

  return merged;
}
