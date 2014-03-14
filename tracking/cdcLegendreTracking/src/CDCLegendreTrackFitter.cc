/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLegendreTracking/CDCLegendreTrackFitter.h>
#include <framework/datastore/StoreArray.h>

#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>

#include <cstdlib>
#include <iomanip>
#include <string>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>
#include <cmath>
#include <iostream>


#define SQR(x) ((x)*(x)) //we will use it in least squares fit

using namespace std;
using namespace Belle2;
//using namespace CDC;


void CDCLegendreTrackFitter::fitTrackCandidate(
  std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* track)
{
  //get theta and r values for each track candidate
  double track_theta = track->second.first;
  double track_r = track->second.second;
  double summ;
  double alpha, beta; //line parameters
  double x0, y0, drift_time;
  double summ_prev, summ_min;
  double delta_track_theta, delta_track_r;
  double prev_track_theta, prev_track_r;
  double increment_sign_theta, increment_sign_r;
  bool is_finished;
  int direction, direction_min; //direction of increment: 1 - (1,1); 2 - (1,-1); 3 - (-1,-1); 4 - (-1,1), where (theta,r)

  //define starting squares sum
  alpha = -1. / tan(track_theta);
  beta = track_r / sin(track_theta);
  summ = 0.;
  BOOST_FOREACH(CDCLegendreTrackHit * hit, track->first) {
    x0 = hit->getConformalX();
    y0 = hit->getConformalY();
    summ += SQR(x0 - (x0 + alpha * (y0 - beta)) / (SQR(alpha + 1))) + SQR(y0 - (alpha * x0 + alpha * alpha * y0 + beta) / (SQR(alpha + 1)));
  }

  summ_prev = summ;
  prev_track_theta = track_theta;
  prev_track_r = track_r;
  increment_sign_theta = 1.;
  increment_sign_r = 1.;
  delta_track_theta = m_PI / (m_nbinsTheta * 2.);
  delta_track_r = (m_rMax - m_rMin) / (m_nbinsTheta * 2.);
  direction = 1;
  direction_min = 0;
  summ_min = 999.;

  is_finished = false;
  do {
    summ = 0.;
    track_theta = prev_track_theta + increment_sign_theta * delta_track_theta;
    track_r = prev_track_r + increment_sign_r * delta_track_r;
    alpha = -1. / tan(track_theta);
    beta = track_r / sin(track_theta);

    BOOST_FOREACH(CDCLegendreTrackHit * hit, track->first) {
      x0 = hit->getConformalX();
      y0 = hit->getConformalY();
      drift_time = hit->getConformalDriftTime();
//      summ += (x0-(x0+alpha*(y0-beta))/((alpha+1)*(alpha+1)))*(x0-(x0+alpha*(y0-beta))/((alpha+1)*(alpha+1)))
//            + (y0-(alpha*x0+alpha*alpha*y0+beta)/((alpha+1)*(alpha+1)))*(y0-(alpha*x0+alpha*alpha*y0+beta)/((alpha+1)*(alpha+1)));
      summ += SQR(fabs(cos(track_theta) * (y0 - alpha * x0 - beta))/*-drift_time*/);
    }

    if (direction == 1) {
      summ_min = summ;
      direction_min = direction;
    } else if (summ_min > summ) {
      summ_min = summ;
      direction_min = direction;
    }

    if (direction != 4) {
      direction++;
      switch (direction) {
        case 1:
          increment_sign_theta = 1;
          increment_sign_r = 1;
          break;
        case 2:
          increment_sign_theta = 1;
          increment_sign_r = -1;
          break;
        case 3:
          increment_sign_theta = -1;
          increment_sign_r = -1;
          break;
        case 4:
          increment_sign_theta = -1;
          increment_sign_r = 1;
          break;
        default:
          is_finished = true;
      }
    } else {
      if (summ_prev > summ_min) {
        direction = 1;
        increment_sign_theta = 1;
        increment_sign_r = 1;
        summ_prev = summ_min;
        switch (direction_min) {
          case 1:
            prev_track_theta = prev_track_theta + 1 * delta_track_theta;
            prev_track_r = prev_track_r + 1 * delta_track_r;
            break;
          case 2:
            prev_track_theta = prev_track_theta + 1 * delta_track_theta;
            prev_track_r = prev_track_r + -1 * delta_track_r;
            break;
          case 3:
            prev_track_theta = prev_track_theta + -1 * delta_track_theta;
            prev_track_r = prev_track_r + -1 * delta_track_r;
            break;
          case 4:
            prev_track_theta = prev_track_theta + -1 * delta_track_theta;
            prev_track_r = prev_track_r + 1 * delta_track_r;
            break;
        }
      } else is_finished = true;
    }

//    printf("%i,   %f\n",direction,summ);
  } while (!is_finished);

//now applying least square fitting with drift time

  increment_sign_theta = 1.;
  increment_sign_r = 1.;
  delta_track_theta = m_PI / (m_nbinsTheta * 5.);
  delta_track_r = (m_rMax - m_rMin) / (m_nbinsTheta * 5.);
  direction = 1;
  summ_prev = summ;
  prev_track_theta = track_theta;
  prev_track_r = track_r;

  is_finished = false;
  do {
    summ = 0.;
    track_theta = prev_track_theta + increment_sign_theta * delta_track_theta;
    track_r = prev_track_r + increment_sign_r * delta_track_r;
    alpha = -1. / tan(track_theta);
    beta = track_r / sin(track_theta);

    BOOST_FOREACH(CDCLegendreTrackHit * hit, track->first) {
      x0 = hit->getConformalX();
      y0 = hit->getConformalY();
      drift_time = hit->getConformalDriftTime();
      summ += SQR(fabs(cos(track_theta) * (y0 - alpha * x0 - beta)) - drift_time);
    }

    if (summ_prev < summ) {
      if (direction < 4) {
        direction++;
        switch (direction) {
          case 1:
            increment_sign_theta = 1;
            increment_sign_r = 1;
            break;
          case 2:
            increment_sign_theta = 1;
            increment_sign_r = -1;
            break;
          case 3:
            increment_sign_theta = -1;
            increment_sign_r = -1;
            break;
          case 4:
            increment_sign_theta = -1;
            increment_sign_r = 1;
            break;
          default:
            is_finished = true;
        }
      } else is_finished = true;
    } else {
      direction = 1;
      increment_sign_theta = 1;
      increment_sign_r = 1;
      summ_prev = summ;
      prev_track_theta = track_theta;
      prev_track_r = track_r;
    }


    if (is_finished) {
      track->second.first = track_theta;
      track->second.second = track_r;
    }

//    printf("%i,   %f\n",direction,summ);
  } while (!is_finished);

}


void CDCLegendreTrackFitter::fitTrackCandidateNormalSpace(
  std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* track,
  std::pair<double, double>* ref_point)
{
  //get theta and r values for each track candidate
  double track_theta = track->second.first;
  double track_r = track->second.second;
  double summ;
  double alpha, beta; //line parameters
  double x0, y0, drift_time;
  double summ_prev, summ_min;
  double delta_track_theta, delta_track_r;
  double prev_track_theta, prev_track_r;
  double increment_x0, increment_y0, increment_R;
  bool is_finished;
  int direction, direction_min; //direction of increment: 1 - (1,1); 2 - (1,-1); 3 - (-1,-1); 4 - (-1,1), where (theta,r)
  double R, R_prev, x0_track, y0_track, x0_track_prev, y0_track_prev, x0_hit, y0_hit;
  double R_min, x0_track_min, y0_track_min;
  double x0_initial, y0_initial, track_theta_initial, track_r_initial, R_initial;

  //define starting squares sum
  summ = 0.;
  R = fabs(1. / track->second.second);
  x0_track = cos(track->second.first) / track->second.second;
  y0_track = sin(track->second.first) / track->second.second;
  x0_initial =  x0_track;
  y0_initial =  y0_track;
  track_theta_initial = track_theta;
  track_r_initial = track_r;
  R_initial = R;

  BOOST_FOREACH(CDCLegendreTrackHit * hit, track->first) {
    x0_hit = hit->getOriginalWirePosition().X();
    y0_hit = hit->getOriginalWirePosition().Y();
    summ += SQR(fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftTime());
  }

  summ_prev = summ;
  increment_x0 = 1.;
  increment_y0 = 1.;
  increment_R = 1.;
  direction = 1;
  direction_min = 0;
  summ_min = 999.;
  x0_track_prev = x0_track;
  y0_track_prev = y0_track;
  R_prev = R;

  is_finished = false;
  do {
    summ = 0.;
    x0_track = x0_track_prev + increment_x0 * x0_track_prev * 0.01; // we move (x0,y0) non-lineary - this is temporary solution
    y0_track = y0_track_prev + increment_y0 * x0_track_prev * 0.01;
    R = R_prev + increment_R * R_prev * 0.01;
//    R = R_prev;

    BOOST_FOREACH(CDCLegendreTrackHit * hit, track->first) {
      x0_hit = hit->getOriginalWirePosition().X();
      y0_hit = hit->getOriginalWirePosition().Y();
      summ += SQR(fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftTime());
    }

    if (direction == 1) {
      summ_min = summ;
      direction_min = direction;
      R_min = R;
      x0_track_min = x0_track;
      y0_track_min = y0_track;
    } else if (summ_min > summ) {
      summ_min = summ;
      direction_min = direction;
      R_min = R;
      x0_track_min = x0_track;
      y0_track_min = y0_track;
    }

    if (direction != 12) {
      direction++;
      switch (direction) {
        case 1:
          increment_x0 = 1.;
          increment_y0 = 1.;
          increment_R = 1.;
          break;
        case 2:
          increment_x0 = 1.;
          increment_y0 = -1.;
          increment_R = 1.;
          break;
        case 3:
          increment_x0 = -1.;
          increment_y0 = -1.;
          increment_R = 1.;
          break;
        case 4:
          increment_x0 = -1.;
          increment_y0 = 1.;
          increment_R = 1.;
          break;
        case 5:
          increment_x0 = 1.;
          increment_y0 = 1.;
          increment_R = -1.;
          break;
        case 6:
          increment_x0 = 1.;
          increment_y0 = -1.;
          increment_R = -1.;
          break;
        case 7:
          increment_x0 = -1.;
          increment_y0 = -1.;
          increment_R = -1.;
          break;
        case 8:
          increment_x0 = -1.;
          increment_y0 = 1.;
          increment_R = -1.;
          break;
        case 9:
          increment_x0 = 1.;
          increment_y0 = 1.;
          increment_R = 0.;
          break;
        case 10:
          increment_x0 = 1.;
          increment_y0 = -1.;
          increment_R = 0.;
          break;
        case 11:
          increment_x0 = -1.;
          increment_y0 = -1.;
          increment_R = 0.;
          break;
        case 12:
          increment_x0 = -1.;
          increment_y0 = 1.;
          increment_R = 0.;
          break;
        default:
          is_finished = true;
      }
    } else {
      if (summ_prev > summ_min) {
//        printf("r:  %f; R:  %f\n",track->second.second,R_min);
//        printf("x0: %f; y0: %f\n",x0_track_min,y0_track_min);
//        printf("summ: %f\n",summ_min);
        direction = 1;
        increment_x0 = 1.;
        increment_y0 = 1.;
        increment_R = 1.;
        summ_prev = summ_min;
        x0_track_prev = x0_track_min;
        y0_track_prev = y0_track_min;
        R_prev = R_min;
      } else is_finished = true;
    }

    if (is_finished) {
      ref_point->first = x0_track_min - R_min * cos(atan2(y0_track_min, x0_track_min));
      ref_point->second = y0_track_min - R_min * sin(atan2(y0_track_min, x0_track_min));
      track->second.first = (atan2((y0_track_min - ref_point->second), (x0_track_min - ref_point->first)) >= 0.)
                            ? atan2((y0_track_min - ref_point->second), (x0_track_min - ref_point->first))
                            : m_PI + atan2((y0_track_min - ref_point->second), (x0_track_min - ref_point->first));
      track->second.second = (1. / R_min) * (fabs(track->second.second) / track->second.second); // some playaround with sign of R
//      ref_point->first = 0.;
//      ref_point->second = 0.;
    }
//    printf("%i,   %f\n",direction,summ);
  } while (!is_finished);


  printf("initial: x0: %f	y0 %f	R:	%f\n", x0_initial, y0_initial, R_initial);
  printf("final:   x0: %f	y0 %f	R:	%f\n", x0_track_min, y0_track_min, R_min);
  printf("ref.p.:  x0: %f	y0 %f\n", ref_point->first, ref_point->second);
  printf("initial: th: %f	r  %f\n", track_theta_initial, track_r_initial);
  printf("final:   th: %f	r  %f\n", track->second.first, track->second.second);
}
