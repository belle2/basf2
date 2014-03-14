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

#include <cmath>

using namespace std;
using namespace Belle2;



CDCLegendreFastHough::CDCLegendreFastHough(bool reconstructCurler,
                                           int maxLevel, int nbinsTheta, double rMin, double rMax):
  m_reconstructCurler(reconstructCurler), m_maxLevel(maxLevel), m_nbinsTheta(nbinsTheta),
  m_rMin(rMin), m_rMax(rMax)
{

  //Initialize look-up table
  double bin_width = m_PI / m_nbinsTheta;
  m_sin_theta = new double[m_nbinsTheta + 1];
  m_cos_theta = new double[m_nbinsTheta + 1];

  for (int i = 0; i <= m_nbinsTheta; ++i) {
    m_sin_theta[i] = sin(i * bin_width);
    m_cos_theta[i] = cos(i * bin_width);
  }

}

void CDCLegendreFastHough::MaxFastHough(
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
      r_temp = hit->getConformalX() * m_cos_theta[thetaBin[t_index]]
               + hit->getConformalY() * m_sin_theta[thetaBin[t_index]];
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

  int max_value = 0;
  std::pair<int, int> max_value_bin = std::make_pair(0, 0);

  for (int t_index = 0; t_index < 2; ++t_index) {
    for (int r_index = 0; r_index < 2; ++r_index) {
      if (max_value  < voted_hits[t_index][r_index].size()) {
        max_value = voted_hits[t_index][r_index].size();
        max_value_bin = std::make_pair(t_index, r_index);
      }
    }
  }

  bool allow_overlap = false;
  for (int t_index = 0; t_index < 2; ++t_index) {
    for (int r_index = 0; r_index < 2; ++r_index) {
      if (max_value - (sqrt(max_value)) < voted_hits[t_index][r_index].size())
        allow_overlap = true;
    }
  }
//  allow_overlap = false;

//Processing, which bins are further investigated
  for (int t_index = 0; t_index < 2; ++t_index) {
    for (int r_index = 0; r_index < 2; ++r_index) {

      //"trick" which allows to use wider bins for higher r values (lower pt tracks)
      int level_diff = 0;
      if (fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) > (m_rMax / 4.)) level_diff = 3;
      else if ((fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) < (m_rMax / 4.)) && (fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) > (2.*m_rMax / 3.)))
        level_diff = 2;
      else if ((fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) < (2.*m_rMax / 3.)) && (fabs(r[r_index] + (r[r_index + 1] - r[r_index]) / 2.) > (m_rMax / 2.)))
        level_diff = 1;
//      level_diff = 0;

//      if(max_value_bin != std::make_pair(t_index, r_index))allow_overlap = false;

      //bin must contain more hits than the limit and maximal found track candidate
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
            MaxFastHough(candidate, voted_hits[t_index][r_index], level + 1,
                         theta_1_overlap, theta_2_overlap, r_1_overlap,
                         r_2_overlap, limit);
          } else
            MaxFastHough(candidate, voted_hits[t_index][r_index], level + 1,
                         thetaBin[t_index], thetaBin[t_index + 1], r[r_index],
                         r[r_index + 1], limit);
        }
      }
    }
  }
}


inline bool CDCLegendreFastHough::sameSign(double n1, double n2,
                                           double n3, double n4)
{
  if (n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0)
    return true;
  else if (n1 < 0 && n2 < 0 && n3 < 0 && n4 < 0)
    return true;
  else
    return false;
}
