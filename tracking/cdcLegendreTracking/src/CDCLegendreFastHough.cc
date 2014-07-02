/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLegendreTracking/CDCLegendreFastHough.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>
#include <tracking/cdcLegendreTracking/CDCLegendreQuadTree.h>
#include <tracking/cdcLegendreTracking/CDCLegendreConformalPosition.h>

#include <cmath>
#include <algorithm>

using namespace std;
using namespace Belle2;



CDCLegendreFastHough::CDCLegendreFastHough(bool reconstructCurler, bool useHitPrecalculatedR,
                                           int maxLevel, int nbinsTheta, double rMin, double rMax):
/*m_useHitPrecalculatedR(useHitPrecalculatedR),*/ m_reconstructCurler(reconstructCurler), m_maxLevel(maxLevel),
  m_nbinsTheta(nbinsTheta), /*m_rMin(rMin),*/ m_rMax(rMax)
{

  //Initialize look-up table
  double bin_width = m_PI / m_nbinsTheta;
  m_sin_theta = new double[m_nbinsTheta + 1];
  m_cos_theta = new double[m_nbinsTheta + 1];

  for (int i = 0; i <= m_nbinsTheta; ++i) {
    m_sin_theta[i] = sin(i * bin_width);
    m_cos_theta[i] = cos(i * bin_width);
  }

  Rcell = 2. * m_PI * 40. / 384.;

//  m_cdcLegendreConformalPosition = CDCLegendreConformalPosition::Instance();

}


CDCLegendreFastHough::~CDCLegendreFastHough()
{
  delete[] m_sin_theta;
  delete[] m_cos_theta;
}

/*
inline bool CDCLegendreFastHough::sameSign(double n1, double n2,
                                           double n3, double n4)
{
//    if (n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0)
//      return true;
//    else if (n1 < 0 && n2 < 0 && n3 < 0 && n4 < 0)
//      return true;
//    else
//      return false;

  return ((n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0) || (n1 < 0 && n2 < 0 && n3 < 0 && n4 < 0));
}
*/

void CDCLegendreFastHough::FastHoughNormal(
  std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* candidate,
  const std::vector<CDCLegendreTrackHit*>& hits, const int level,
  const int theta_min, const int theta_max, const double r_min,
  const double r_max, const unsigned limit)
{
  if (not m_reconstructCurler
      && (r_min * r_max >= 0 && fabs(r_min) > m_rc && fabs(r_max) > m_rc)) {
    return;
  }

  //calculate bin borders of 2x2 bin "histogram"
  int thetaBin[3];
  thetaBin[0] = theta_min;
  thetaBin[1] = theta_min + (theta_max - theta_min) / 2;
  thetaBin[2] = theta_max;

  double r[3];
  r[0] = r_min;
  r[1] = r_min + 0.5 * (r_max - r_min);
  r[2] = r_max;

  //2 x 2 voting plane
  std::vector<CDCLegendreTrackHit*> voted_hits[2][2];
  for (unsigned int i = 0; i < 2; ++i)
    for (unsigned int j = 0; j < 2; ++j)
      voted_hits[i][j].reserve(1024);

  double r_temp, r_1, r_2;
  double dist_1[3][3];
  double dist_2[3][3];

  //Voting within the four bins
  for (CDCLegendreTrackHit * hit : hits) {
    for (int t_index = 0; t_index < 3; ++t_index) {

//      r_temp = CDCLegendreConformalPosition::Instance().getConformalR(hit->getLayerId(), hit->getWireId(), thetaBin[t_index]);
      r_temp = hit->getConformalX() * m_cos_theta[thetaBin[t_index]] +
               hit->getConformalY() * m_sin_theta[thetaBin[t_index]];

      r_1 = r_temp + hit->getConformalDriftTime();
      r_2 = r_temp - hit->getConformalDriftTime();

      //calculate distances of lines to horizontal bin border
      for (int r_index = 0; r_index < 3; ++r_index) {
        dist_1[t_index][r_index] = r[r_index] - r_1;
        dist_2[t_index][r_index] = r[r_index] - r_2;
      }
    }

    //actual voting, based on the distances (test, if line is passing though the bin)
    for (int t_index = 0; t_index < 2; ++t_index) {
      for (int r_index = 0; r_index < 2; ++r_index) {
        //curves are assumed to be straight lines, might be a reasonable assumption locally
        if ((!sameSign(dist_1[t_index][r_index], dist_1[t_index][r_index + 1], dist_1[t_index + 1][r_index], dist_1[t_index + 1][r_index + 1])) &&
            (hit->isUsed() == CDCLegendreTrackHit::not_used))
          voted_hits[t_index][r_index].push_back(hit);
        else if ((!sameSign(dist_2[t_index][r_index], dist_2[t_index][r_index + 1], dist_2[t_index + 1][r_index], dist_2[t_index + 1][r_index + 1])) &&
                 (hit->isUsed() == CDCLegendreTrackHit::not_used))
          voted_hits[t_index][r_index].push_back(hit);
      }
    }

  }

  unsigned int max_value = 0;
  std::pair<int, int> max_value_bin = std::make_pair(0, 0);

  bool binUsed[2][2];
  for (int ii = 0; ii < 2; ii++)
    for (int jj = 0; jj < 2; jj++)
      binUsed[ii][jj] = false;


  bool allow_overlap = false;
  for (int t_index = 0; t_index < 2; ++t_index) {
    for (int r_index = 0; r_index < 2; ++r_index) {
      if (max_value - (sqrt(max_value)) < voted_hits[t_index][r_index].size())
        allow_overlap = true;
    }
  }
  allow_overlap = false;


  for (int t_index = 0; t_index < 2; ++t_index) {
    for (int r_index = 0; r_index < 2; ++r_index) {
      if (max_value  < voted_hits[t_index][r_index].size()) {
        max_value = voted_hits[t_index][r_index].size();
        max_value_bin = std::make_pair(t_index, r_index);
      }
    }
  }


//Processing, which bins are further investigated
  for (int bin_loop = 0; bin_loop < 4; bin_loop++) {
    int t_index = 0;
    int r_index = 0;
    double max_value_temp = 0;
    for (int t_index_temp = 0; t_index_temp < 2; ++t_index_temp) {
      for (int r_index_temp = 0; r_index_temp < 2; ++r_index_temp) {
        if ((max_value_temp  < voted_hits[t_index_temp][r_index_temp].size()) && (!binUsed[t_index_temp][r_index_temp])) {
          max_value_temp = voted_hits[t_index_temp][r_index_temp].size();
          t_index = t_index_temp;
          r_index = r_index_temp;
        }
      }
    }
    binUsed[t_index][r_index] = true;


    //"trick" which allows to use wider bins for higher r values (lower pt tracks)
    int level_diff = 0;
    if (fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) > (m_rMax / 4.)) level_diff = 3;
    else if ((fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) < (m_rMax / 4.)) && (fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) > (2.*m_rMax / 3.)))
      level_diff = 2;
    else if ((fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) < (2.*m_rMax / 3.)) && (fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) > (m_rMax / 2.)))
      level_diff = 1;
    level_diff = 0;

    //      if(max_value_bin != std::make_pair(t_index, r_index))allow_overlap = false;

    //bin must contain more hits than the limit and maximal found track candidate
    /*    if (voted_hits[t_index][r_index].size() >= limit) {

          //if max level of fast Hough is reached, mark candidate and return
          //        if (((!allow_overlap)&&(level == (m_maxLevel - level_diff))) || ((allow_overlap)&&(level == (m_maxLevel - level_diff) + 2))) {
          if (level >= (m_maxLevel - level_diff)) {

            std::vector<CDCLegendreTrackHit*> c_list;
            std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > candidate_temp =
              std::make_pair(c_list, std::make_pair(-999, -999));



            double theta = static_cast<double>(thetaBin[t_index]
                                               + thetaBin[t_index + 1]) / 2. * m_PI / m_nbinsTheta;

            if (not m_reconstructCurler
                && fabs((r[r_index] + r[r_index + 1]) / 2) > m_rc)
              return;

            for (CDCLegendreTrackHit * hit : voted_hits[t_index][r_index]) {
              hit->setUsed(CDCLegendreTrackHit::used_in_cand);
            }
            candidate_temp.first = voted_hits[t_index][r_index];
            candidate_temp.second = std::make_pair(theta,
                                                   (r[r_index] + r[r_index + 1]) / 2.);

            if (candidate_temp.first.size() > 0) m_candidates->push_back(candidate_temp);

          }
        }*/
    if (voted_hits[t_index][r_index].size() >= limit
        && voted_hits[t_index][r_index].size() > candidate->first.size()) {

      //if max level of fast Hough is reached, mark candidate and return
      //        if (((!allow_overlap)&&(level == (m_maxLevel - level_diff))) || ((allow_overlap)&&(level == (m_maxLevel - level_diff) + 2))) {
      if (level >= (m_maxLevel - level_diff)) {
        double theta = static_cast<double>(thetaBin[t_index]
                                           + thetaBin[t_index + 1]) / 2. * m_PI / m_nbinsTheta;

        if (not m_reconstructCurler
            && fabs((r[r_index] + r[r_index + 1]) / 2) > m_rc)
          return;

        candidate->first = voted_hits[t_index][r_index];
        candidate->second = std::make_pair(theta,
                                           (r[r_index] + r[r_index + 1]) / 2.);

      } else {
        //Recursive calling of the function with higher level and smaller box
        //          if (allow_overlap) { //if overlapping allowed make bin borders wider
        if (allow_overlap  && (level >= (m_maxLevel - level_diff - 2)) && max_value_bin == std::make_pair(t_index, r_index)) { //if overlapping allowed make bin borders wider
          double r_1_overlap, r_2_overlap;
          int theta_1_overlap, theta_2_overlap; // here we involve new variables which allows to change bin borders for positive and negative r independently
          r_1_overlap = r[r_index] - fabs(r[r_index + 1] - r[r_index]) / 2.;
          r_2_overlap = r[r_index + 1] + fabs(r[r_index + 1] - r[r_index]) / 2.;
          //            theta_1_overlap = thetaBin[t_index]/* - fabs(thetaBin[t_index + 1] - thetaBin[t_index]) / 2.*/;
          //            theta_2_overlap = thetaBin[t_index + 1]/* + fabs(thetaBin[t_index + 1] - thetaBin[t_index]) / 2.*/;
          //            theta_1_overlap = thetaBin[t_index] - fabs(thetaBin[t_index + 1] - thetaBin[t_index]) / 2.;
          //            theta_2_overlap = thetaBin[t_index + 1] + fabs(thetaBin[t_index + 1] - thetaBin[t_index]) / 2.;
          theta_1_overlap = thetaBin[t_index] - pow(2, m_maxLevel - level_diff - 2 - level);
          theta_2_overlap = thetaBin[t_index + 1] + pow(2, m_maxLevel - level_diff - 2 - level);
          if (theta_1_overlap < 0)theta_1_overlap = thetaBin[t_index];
          if (theta_2_overlap >= m_nbinsTheta)theta_2_overlap = thetaBin[t_index + 1];
          FastHoughNormal(candidate, voted_hits[t_index][r_index], level + 1,
                          theta_1_overlap, theta_2_overlap, r_1_overlap,
                          r_2_overlap, limit);
        } else
          FastHoughNormal(candidate, voted_hits[t_index][r_index], level + 1,
                          thetaBin[t_index], thetaBin[t_index + 1], r[r_index],
                          r[r_index + 1], limit);
      }
    }
  }
//    }
//  }
}


void CDCLegendreFastHough::MaxFastHough(const std::vector<CDCLegendreTrackHit*>& hits, const int level, const int theta_min, const int theta_max,
                                        const double r_min, const double r_max)
{

  if (not m_reconstructCurler
      && (r_min * r_max >= 0 && fabs(r_min) > m_rc && fabs(r_max) > m_rc)) {
    return;
  }

  //calculate bin borders of 2x2 bin "histogram"
  int thetaBin[3];
  thetaBin[0] = theta_min;
  thetaBin[1] = theta_min + (theta_max - theta_min) / 2;
  thetaBin[2] = theta_max;

  double r[3];
  r[0] = r_min;
  r[1] = r_min + 0.5 * (r_max - r_min);
  r[2] = r_max;

  //2 x 2 voting plane
  std::vector<CDCLegendreTrackHit*> voted_hits[2][2];
  for (unsigned int i = 0; i < 2; ++i)
    for (unsigned int j = 0; j < 2; ++j)
      voted_hits[i][j].reserve(1024);

  double r_temp, r_1, r_2;
  double dist_1[3][3];
  double dist_2[3][3];

  //Voting within the four bins
  for (CDCLegendreTrackHit * hit : hits) {
    if (hit->isUsed() != CDCLegendreTrackHit::not_used) continue;
    for (int t_index = 0; t_index < 3; ++t_index) {

      r_temp = CDCLegendreConformalPosition::Instance().getConformalR(hit->getLayerId(), hit->getWireId(), thetaBin[t_index]);

      r_1 = r_temp + hit->getConformalDriftTime();
      r_2 = r_temp - hit->getConformalDriftTime();

      //calculate distances of lines to horizontal bin border
      for (int r_index = 0; r_index < 3; ++r_index) {
        dist_1[t_index][r_index] = r[r_index] - r_1;
        dist_2[t_index][r_index] = r[r_index] - r_2;
      }
    }

    //actual voting, based on the distances (test, if line is passing though the bin)
    for (int t_index = 0; t_index < 2; ++t_index) {
      for (int r_index = 0; r_index < 2; ++r_index) {
        //curves are assumed to be straight lines, might be a reasonable assumption locally
        if (!sameSign(dist_1[t_index][r_index], dist_1[t_index][r_index + 1], dist_1[t_index + 1][r_index], dist_1[t_index + 1][r_index + 1]))
          voted_hits[t_index][r_index].push_back(hit);
        else if (!sameSign(dist_2[t_index][r_index], dist_2[t_index][r_index + 1], dist_2[t_index + 1][r_index], dist_2[t_index + 1][r_index + 1]))
          voted_hits[t_index][r_index].push_back(hit);
      }
    }

  }

  bool binUsed[2][2];
  for (int ii = 0; ii < 2; ii++)
    for (int jj = 0; jj < 2; jj++)
      binUsed[ii][jj] = false;

//Processing, which bins are further investigated
  for (int bin_loop = 0; bin_loop < 4; bin_loop++) {
    int t_index = 0;
    int r_index = 0;
    double max_value_temp = 0;
    for (int t_index_temp = 0; t_index_temp < 2; ++t_index_temp) {
      for (int r_index_temp = 0; r_index_temp < 2; ++r_index_temp) {
        if ((max_value_temp  < voted_hits[t_index_temp][r_index_temp].size()) && (!binUsed[t_index_temp][r_index_temp])) {
          max_value_temp = voted_hits[t_index_temp][r_index_temp].size();
          t_index = t_index_temp;
          r_index = r_index_temp;
        }
      }
    }

    binUsed[t_index][r_index] = true;

    //bin must contain more hits than the limit and maximal found track candidate


    //"trick" which allows to use wider bins for higher r values (lower pt tracks)
    int level_diff = 0;
    if (fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) > (m_rMax / 4.)) level_diff = 3;
    else if ((fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) < (m_rMax / 4.)) && (fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) > (2.*m_rMax / 3.)))
      level_diff = 2;
    else if ((fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) < (2.*m_rMax / 3.)) && (fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) > (m_rMax / 2.)))
      level_diff = 1;

//      level_diff = 0;

    //bin must contain more hits than the limit and maximal found track candidate
    if (voted_hits[t_index][r_index].size() >= m_limit) {

      //if max level of fast Hough is reached, mark candidate and return
      //        if (((!allow_overlap)&&(level == (m_maxLevel - level_diff))) || ((allow_overlap)&&(level == (m_maxLevel - level_diff) + 2))) {
      if (level >= (m_maxLevel - level_diff)) {

        std::vector<CDCLegendreTrackHit*> c_list;
        std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > candidate_temp =
          std::make_pair(c_list, std::make_pair(-999, -999));



        double theta = static_cast<double>(thetaBin[t_index]
                                           + thetaBin[t_index + 1]) / 2. * m_PI / m_nbinsTheta;

        if (not m_reconstructCurler
            && fabs((r[r_index] + r[r_index + 1]) / 2) > m_rc)
          return;

        voted_hits[t_index][r_index].erase(std::remove_if(voted_hits[t_index][r_index].begin(), voted_hits[t_index][r_index].end(), [](CDCLegendreTrackHit * hit) {return hit->isUsed() != CDCLegendreTrackHit::not_used;}), voted_hits[t_index][r_index].end());

        for (CDCLegendreTrackHit * hit : voted_hits[t_index][r_index]) {
          hit->setUsed(CDCLegendreTrackHit::used_in_cand);
        }

        candidate_temp.first = voted_hits[t_index][r_index];
        candidate_temp.second = std::make_pair(theta,
                                               (r[r_index] + r[r_index + 1]) / 2.);

        if (candidate_temp.first.size() > m_limit) m_candidates->push_back(candidate_temp);

      } else {
        //Recursive calling of the function with higher level and smaller box
        MaxFastHough(voted_hits[t_index][r_index], level + 1, thetaBin[t_index], thetaBin[t_index + 1],
                     r[r_index], r[r_index + 1]);

      }
    }
  }

}


void CDCLegendreFastHough::MaxFastHoughHighPt(const std::vector<CDCLegendreTrackHit*>& hits, const int theta_min, const int theta_max,
                                              double r_min, double r_max, int level)
{

  //B2DEBUG(100, "MaxFastHoughHighPt: " << theta_min << " " << theta_max << " " << r_min << " " << r_max);
  level++;

  if (not m_reconstructCurler
      && (r_min * r_max >= 0 && fabs(r_min) > m_rc && fabs(r_max) > m_rc)) {
//    return;
    r_min = -1. * m_rc;
    r_max = m_rc;
  }

  //B2DEBUG(100, "CREATING arrays r and theta");

  bool lastBin;

  int nbins_theta;
  int nbins_r;

  double delta_r;

  delta_r = 4. * Rcell / (4. / ((r_max + r_min) * (r_max + r_min)) - 4.*Rcell * Rcell);

  if ((delta_r < 0) || (delta_r > 1))B2FATAL("Bad delta_r value: " << delta_r << ". Please check limits in CDCLegendreFastHough::MaxFastHoughHighPt() or switch to CDCLegendreFastHough::MaxFastHough()");

  //B2DEBUG(100, "DELTA r: " << delta_r << "; r_max-r_min: " << fabs(r_max - r_min));

  //calculate bin borders of 2x2 bin "histogram"
  if (fabs(r_max - r_min) > delta_r) {
    lastBin = false;
    //B2DEBUG(100, "NORMAL binning");
    nbins_r = 2;
    nbins_theta = 2;
  } else {
    lastBin = true;
    //B2DEBUG(100, "1xN binning");
    nbins_r = 1;
    nbins_theta = (theta_max - theta_min) / 4;
  }

  double r[nbins_r + 1];
  int thetaBin[nbins_theta + 1];

  if (!lastBin) {
    r[0] = r_min;
    r[1] = r_min + 0.5 * (r_max - r_min);
    r[2] = r_max;
    thetaBin[0] = theta_min;
    thetaBin[1] = theta_min + (theta_max - theta_min) / 2;
    thetaBin[2] = theta_max;
  } else {
    r[0] = r_min;
    r[1] = r_max;

    for (int t_index = 0; t_index < nbins_theta + 1; t_index++) {
      thetaBin[t_index] = theta_min + t_index;
    }
  }

  //B2DEBUG(100, "Number of bins: " << nbins_r << "x" << nbins_theta);
  //B2DEBUG(100, "CREATING voting plane");

  int nhitsToReserve;
  nhitsToReserve = 2 * hits.size();
  //voting plane
  /*  std::vector<CDCLegendreTrackHit*> voted_hits[nbins_theta][nbins_r];
    for (int i = 0; i < nbins_theta; ++i)
      for (int j = 0; j < nbins_r; ++j)
        voted_hits[i][j].reserve(nhitsToReserve);
  */
  std::vector<CDCLegendreTrackHit*>** voted_hits;
  voted_hits = new std::vector<CDCLegendreTrackHit*>* [nbins_theta];
  for (int i = 0; i < nbins_theta; ++i) {
    voted_hits[i] = new std::vector<CDCLegendreTrackHit*>[nbins_r];
    for (int j = 0; j < nbins_r; ++j)
      voted_hits[i][j].reserve(nhitsToReserve);
  }

//B2DEBUG(100, "CREATING distance arrays");

  double r_temp, r_1, r_2;

  double dist_1[nbins_theta + 1][nbins_r + 1];
  double dist_2[nbins_theta + 1][nbins_r + 1];

  //B2DEBUG(100, "VOTING");

  int hit_counter = 0;
  //Voting within the four bins
  for (CDCLegendreTrackHit * hit : hits) {
    hit_counter++;
    //B2DEBUG(100, "PROCCESSING hit " << hit_counter << " of " << nhitsToReserve);
    if (hit->isUsed() != CDCLegendreTrackHit::not_used) continue;
    for (int t_index = 0; t_index < nbins_theta + 1; ++t_index) {

      r_temp = CDCLegendreConformalPosition::InstanceTrusted().getConformalR(hit->getLayerId(), hit->getWireId(), thetaBin[t_index]);

      r_1 = r_temp + hit->getConformalDriftTime();
      r_2 = r_temp - hit->getConformalDriftTime();

      //calculate distances of lines to horizontal bin border
      for (int r_index = 0; r_index < nbins_r + 1; ++r_index) {
        dist_1[t_index][r_index] = r[r_index] - r_1;
        dist_2[t_index][r_index] = r[r_index] - r_2;
      }
    }

    //B2DEBUG(100, "VOTING for hit " << hit_counter << " of " << nhitsToReserve);
    //actual voting, based on the distances (test, if line is passing though the bin)
    for (int t_index = 0; t_index < nbins_theta; ++t_index) {
      for (int r_index = 0; r_index < nbins_r; ++r_index) {
        //curves are assumed to be straight lines, might be a reasonable assumption locally
        if (!sameSign(dist_1[t_index][r_index], dist_1[t_index][r_index + 1], dist_1[t_index + 1][r_index], dist_1[t_index + 1][r_index + 1])) {
          //B2DEBUG(100, "INSERT hit in " << t_index << ";" << r_index << " bin");
          voted_hits[t_index][r_index].push_back(hit);
        } else if (!sameSign(dist_2[t_index][r_index], dist_2[t_index][r_index + 1], dist_2[t_index + 1][r_index], dist_2[t_index + 1][r_index + 1])) {
          //B2DEBUG(100, "INSERT hit in " << t_index << ";" << r_index << " bin");
          voted_hits[t_index][r_index].push_back(hit);
        }
      }
    }
    //B2DEBUG(100, "MOVING to next hit");
  }

  //B2DEBUG(100, "END of voting");

  bool binUsed[nbins_theta][nbins_r];
  for (int ii = 0; ii < nbins_theta; ++ii)
    for (int jj = 0; jj < nbins_r; jj++)
      binUsed[ii][jj] = false;


  //B2DEBUG(100, "PROCESSING bins");

//Processing, which bins are further investigated
  for (int bin_loop = 0; bin_loop < nbins_r * nbins_theta; bin_loop++) {
    int t_index = 0;
    int r_index = 0;
    double max_value_temp = 0;
    for (int t_index_temp = 0; t_index_temp < nbins_theta; ++t_index_temp) {
      for (int r_index_temp = 0; r_index_temp < nbins_r; ++r_index_temp) {
        if ((max_value_temp  < voted_hits[t_index_temp][r_index_temp].size()) && (!binUsed[t_index_temp][r_index_temp])) {
          max_value_temp = voted_hits[t_index_temp][r_index_temp].size();
          t_index = t_index_temp;
          r_index = r_index_temp;
        }
      }
    }

    binUsed[t_index][r_index] = true;

    //bin must contain more hits than the limit and maximal found track candidate

    /*
        //"trick" which allows to use wider bins for higher r values (lower pt tracks)
        int level_diff = 0;
        if (fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) > (m_rMax / 4.)) level_diff = 3;
        else if ((fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) < (m_rMax / 4.)) && (fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) > (2.*m_rMax / 3.)))
          level_diff = 2;
        else if ((fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) < (2.*m_rMax / 3.)) && (fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) > (m_rMax / 2.)))
          level_diff = 1;

    //      level_diff = 0;
    */
    //bin must contain more hits than the limit and maximal found track candidate
    if (voted_hits[t_index][r_index].size() >= m_limit) {

      //B2DEBUG(100, "GOOD bin " << r_index << "  " << t_index << " " << voted_hits[t_index][r_index].size());


      //if max level of fast Hough is reached, mark candidate and return
      //        if (((!allow_overlap)&&(level == (m_maxLevel - level_diff))) || ((allow_overlap)&&(level == (m_maxLevel - level_diff) + 2))) {
      if (lastBin) {

        //B2DEBUG(100, "CREATING candidate");

        std::vector<CDCLegendreTrackHit*> c_list;
        std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > candidate_temp =
          std::make_pair(c_list, std::make_pair(-999, -999));



        double theta = static_cast<double>(thetaBin[t_index]
                                           + thetaBin[t_index + 1]) / 2. * m_PI / m_nbinsTheta;

        if (not m_reconstructCurler
            && fabs((r[r_index] + r[r_index + 1]) / 2) > m_rc) {
          for (int i = 0; i < nbins_theta; ++i) {
            delete[] voted_hits[i];
          }
          delete[] voted_hits;

          return;
        }

        voted_hits[t_index][r_index].erase(std::remove_if(voted_hits[t_index][r_index].begin(), voted_hits[t_index][r_index].end(), [](CDCLegendreTrackHit * hit) {return hit->isUsed() != CDCLegendreTrackHit::not_used;}), voted_hits[t_index][r_index].end());

        for (CDCLegendreTrackHit * hit : voted_hits[t_index][r_index]) {
          hit->setUsed(CDCLegendreTrackHit::used_in_cand);
        }

        candidate_temp.first = voted_hits[t_index][r_index];
        candidate_temp.second = std::make_pair(theta,
                                               (r[r_index] + r[r_index + 1]) / 2.);

        if (candidate_temp.first.size() > m_limit) m_candidates->push_back(candidate_temp);

//        B2INFO("LEVEL: " << level << "; r=" << (r[r_index] + r[r_index + 1]) / 2.);
      } else {
        //B2DEBUG(100, "RECURSIVE call");

        //Recursive calling of the function with higher level and smaller box
        MaxFastHoughHighPt(voted_hits[t_index][r_index], thetaBin[t_index], thetaBin[t_index + 1],
                           r[r_index], r[r_index + 1], level);

      }
    }
  }

  for (int i = 0; i < nbins_theta; ++i) {
    delete[] voted_hits[i];
  }
  delete[] voted_hits;


}
