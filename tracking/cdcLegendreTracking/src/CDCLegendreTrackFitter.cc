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


void CDCLegendreTrackFitter::fitTrackCandidateStepped(
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

void CDCLegendreTrackFitter::fitTrackCandidateFast(
  std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* track,
  std::pair<double, double>& ref_point,
  double& chi2,
  bool with_drift_time)
{
  /*
    double x, y, w;
    double x_tot=0., y_tot=0., xy_tot=0., x2_tot=0., y2_tot=0., r2_tot=0., r4_tot=0., xr2_tot=0., yr2_tot=0.;
    double w_tot=0.;
    double Crr, Cxy, Cxr, Cyr, Cxx, Cyy;
    double q1, q2;
    double phi, k, delta, rho, d;
    double chi2 ;

    for(CDCLegendreTrackHit* hit : track->first)
    {
  //    w = hit->getDriftTime();
      w = 1.;
      w_tot += w;
      x += hit->getWirePosition().X();
      y += hit->getWirePosition().Y();
      x_tot += x;
      y_tot += y;
      xy_tot += x*y * w;
      x2_tot += x*x * w;
      y2_tot += y*y * w;
      r2_tot += (x*x + y*y) * w;
      xr2_tot += x*(x*x + y*y) * w;
      yr2_tot += y*(x*x + y*y) * w;
      r4_tot += (x*x + y*y)*(x*x + y*y) * w;
    }

    Cxx = x2_tot/w_tot - (x_tot/w_tot)*(x_tot/w_tot);
    Cyy = y2_tot/w_tot - (y_tot/w_tot)*(y_tot/w_tot);
    Cxy = xy_tot/w_tot - (x_tot/w_tot)*(y_tot/w_tot);
    Cxr = xr2_tot/w_tot - (x_tot/w_tot)*(r2_tot/w_tot);
    Cyr = yr2_tot/w_tot - (y_tot/w_tot)*(r2_tot/w_tot);
    Crr = r4_tot/w_tot - (r2_tot/w_tot)*(r2_tot/w_tot);

    q1 = Crr*Cxy - Cxr*Cyr;
    q2 = Crr*(Cxx-Cyy) - Cxr*Cxr + Cyr*Cyr;

    phi = 0.5*atan(2.*q1/q2);
    k = (sin(phi)*Cxr - cos(phi)*Cyr)/Crr;
    delta = -1.*k*(r2_tot/w_tot) + sin(phi)*(x_tot/w_tot) - cos(phi)*(y_tot/w_tot);

    rho = 2.*k/sqrt(1-4.*delta*k);
    d = 2.*delta/(1 + sqrt(1-4.*delta*k));

    cout << "Before: " << track->second.first << "  " << track->second.second << endl;
    track->second.first = phi;
    track->second.second = rho;
    cout << "k=" << k << " delta=" << delta << endl;
    cout << "After: " << track->second.first << " " << track->second.second << endl;

    chi2 = w_tot*(1+rho*d)*(1+rho*d)*(sin(phi)*sin(phi)*Cxx - 2.*sin(phi)*cos(phi)*Cxy + cos(phi)*cos(phi)*Cyy - k*k*Crr);
    cout << "chi2=" << chi2 << ";" << (1+rho*d) << "^2 * " << (sin(phi)*sin(phi)*Cxx - 2.*sin(phi)*cos(phi)*Cxy + cos(phi)*cos(phi)*Cyy - k*k*Crr) << endl;

  */
  double stopper = 0.000000001; /// WARNING hardcoded values!
  double meanX = 0, meanY = 0, meanX2 = 0, meanY2 = 0, meanR2 = 0, meanR4 = 0, meanXR2 = 0, meanYR2 = 0, meanXY = 0; //mean values
  double r2 = 0, x = 0, y = 0, x2 = 0, y2 = 0; // coords
  double weight;// weight of each hit, so far no difference in hit quality
  double sumWeights = 0, divisor, weightNormalizer = 0; // sumWeights is sum of weights, divisor is 1/sumWeights;
  double tuningParameter = 1.; //0.02; // this parameter is for internal tuning of the weights, since at the moment, the error seams highly overestimated at the moment. 1 means no influence of parameter.

  // looping over all hits and do the division afterwards
  for (CDCLegendreTrackHit * hit : track->first) {
    if (hit->getDriftTime() != 0.)weight =  1. / hit->getDriftTime();
    else continue;
    sumWeights += weight;
    x = hit->getWirePosition().X();
    y = hit->getWirePosition().Y();
    x2 = x * x;
    y2 = y * y;
    r2 = x2 + y2;
    meanX += x * weight;
    meanY += y * weight;
    meanXY += x * y * weight;
    meanX2 += x2 * weight;
    meanY2 += y2 * weight;
    meanXR2 += x * r2 * weight;
    meanYR2 += y * r2 * weight;
    meanR2 += r2 * weight;
    meanR4 += r2 * r2 * weight;
  }
  divisor = 1. / sumWeights;
  meanX *= divisor;
  meanY *= divisor;
  meanXY *= divisor;
  meanY2 *= divisor;
  meanX2 *= divisor;
  meanXR2 *= divisor;
  meanYR2 *= divisor;
  meanR2 *= divisor;
  meanR4 *= divisor;
  if (sumWeights == 0)cout << "clist_temp.size = " << track->first.size() << endl;

  // covariances:
  double covXX = meanX2 - meanX * meanX;
  double covXY = meanXY - meanX * meanY;
  double covYY = meanY2 - meanY * meanY;
  double covXR2 = meanXR2 - meanX * meanR2;
  double covYR2 = meanYR2 - meanY * meanR2;
  double covR2R2 = meanR4 - meanR2 * meanR2;

  // q1, q2: helping variables, to make the code more readable
  double q1 = covR2R2 * covXY - covXR2 * covYR2;
  double q2 = covR2R2 * (covXX - covYY) - covXR2 * covXR2 + covYR2 * covYR2;

  double clapPhi = 0.5 * atan2(2. * q1 , q2); // physical meaning: phi value of the point of closest approach of the fitted circle to the origin

  double sinPhi = sin(clapPhi);
  double cosPhi = cos(clapPhi);
  double kappa = (sinPhi * covXR2 - cosPhi * covYR2) / covR2R2;
  double delta = -kappa * meanR2 + sinPhi * meanX - cosPhi * meanY;
  double rootTerm = sqrt(1. - 4.*delta * kappa);
  double rho = 2.*kappa / (rootTerm); // rho = curvature in X-Y-plane = 1/radius of fitting circle, used for pT-calculation
  double clapR = 2.*delta / (1. + rootTerm);
  double radius = -1. / rho;
//  if (radius < 0.) { radius *= -1.; }

  double ref_x = cos(clapPhi - m_PI / 2.) * clapR;
  double ref_y = sin(clapPhi - m_PI / 2.) * clapR;

  ref_point.first = ref_x;
  ref_point.second = ref_y;

  cout << "Before: " << track->second.first << "	" << track->second.second << endl;
  track->second.first = clapPhi + m_PI / 2.;
  track->second.second = 1. / radius;
  cout << "k=" << sumWeights << " delta=" << divisor << endl;
  cout << "After: " << track->second.first << "	" << track->second.second << endl;

  chi2 = sumWeights * (1. + rho * clapR) * (1. + rho * clapR) * (sinPhi * sinPhi * covXX - 2.*sinPhi * cosPhi * covXY + cosPhi * cosPhi * covYY - kappa * kappa * covR2R2)/*track->first.size()*/; /// returns chi2
  cout << chi2 << endl;
}

void CDCLegendreTrackFitter::fitTrackCandidateFast(
  CDCLegendreTrackCandidate* track,
  std::pair<double, double>& ref_point,
  bool with_drift_time)
{

  double stopper = 0.000000001; /// WARNING hardcoded values!
  double meanX = 0, meanY = 0, meanX2 = 0, meanY2 = 0, meanR2 = 0, meanR4 = 0, meanXR2 = 0, meanYR2 = 0, meanXY = 0; //mean values
  double r2 = 0, x = 0, y = 0, x2 = 0, y2 = 0; // coords
  double weight;// weight of each hit, so far no difference in hit quality
  double sumWeights = 0, divisor, weightNormalizer = 0; // sumWeights is sum of weights, divisor is 1/sumWeights;
  double tuningParameter = 1.; //0.02; // this parameter is for internal tuning of the weights, since at the moment, the error seams highly overestimated at the moment. 1 means no influence of parameter.
  double radius_track, xc_track, yc_track;

  if (!with_drift_time) {
    // looping over all hits and do the division afterwards
    for (CDCLegendreTrackHit * hit : track->getTrackHits()) {
      if (hit->getDriftTime() != 0.)weight =  1. / hit->getDriftTime();
      else continue;
      sumWeights += weight;
      x = hit->getWirePosition().X();
      y = hit->getWirePosition().Y();
      x2 = x * x;
      y2 = y * y;
      r2 = x2 + y2;
      meanX += x * weight;
      meanY += y * weight;
      meanXY += x * y * weight;
      meanX2 += x2 * weight;
      meanY2 += y2 * weight;
      meanXR2 += x * r2 * weight;
      meanYR2 += y * r2 * weight;
      meanR2 += r2 * weight;
      meanR4 += r2 * r2 * weight;
    }
  } else {
    radius_track = fabs(1. / track->getR());
//    xc_track = track->getXc();
//    yc_track = track->getYc();
    xc_track = cos(track->getTheta()) / track->getR() + track->getReferencePoint().X();
    yc_track = sin(track->getTheta()) / track->getR() + track->getReferencePoint().Y();
    for (CDCLegendreTrackHit * hit : track->getTrackHits()) {
//      if (hit->getDriftTime() != 0.)weight =  1. / hit->getDriftTime();
//      else continue;
      weight = 1.;
      sumWeights += weight;

      x = hit->getWirePosition().X();
      y = hit->getWirePosition().Y();
      double R_dist = sqrt(SQR(xc_track - x) + SQR(yc_track - y));
      double dist = radius_track - R_dist;

      if (dist > 0) {
        x = x + hit->getDriftTime() * (R_dist / (x - xc_track));
        y = y + hit->getDriftTime() * (R_dist / (y - yc_track));
      } else {
        x = x - hit->getDriftTime() * (R_dist / (x - xc_track));
        y = y - hit->getDriftTime() * (R_dist / (y - yc_track));
      }

      x2 = x * x;
      y2 = y * y;
      r2 = x2 + y2;
      meanX += x * weight;
      meanY += y * weight;
      meanXY += x * y * weight;
      meanX2 += x2 * weight;
      meanY2 += y2 * weight;
      meanXR2 += x * r2 * weight;
      meanYR2 += y * r2 * weight;
      meanR2 += r2 * weight;
      meanR4 += r2 * r2 * weight;
    }
  }
  divisor = 1. / sumWeights;
  meanX *= divisor;
  meanY *= divisor;
  meanXY *= divisor;
  meanY2 *= divisor;
  meanX2 *= divisor;
  meanXR2 *= divisor;
  meanYR2 *= divisor;
  meanR2 *= divisor;
  meanR4 *= divisor;

  // covariances:
  double covXX = meanX2 - meanX * meanX;
  double covXY = meanXY - meanX * meanY;
  double covYY = meanY2 - meanY * meanY;
  double covXR2 = meanXR2 - meanX * meanR2;
  double covYR2 = meanYR2 - meanY * meanR2;
  double covR2R2 = meanR4 - meanR2 * meanR2;

  // q1, q2: helping variables, to make the code more readable
  double q1 = covR2R2 * covXY - covXR2 * covYR2;
  double q2 = covR2R2 * (covXX - covYY) - covXR2 * covXR2 + covYR2 * covYR2;

  double clapPhi = 0.5 * atan2(2. * q1 , q2); // physical meaning: phi value of the point of closest approach of the fitted circle to the origin

  double sinPhi = sin(clapPhi);
  double cosPhi = cos(clapPhi);
  double kappa = (sinPhi * covXR2 - cosPhi * covYR2) / covR2R2;
  double delta = -kappa * meanR2 + sinPhi * meanX - cosPhi * meanY;
  double rootTerm = sqrt(1. - 4.*delta * kappa);
  double rho = 2.*kappa / (rootTerm); // rho = curvature in X-Y-plane = 1/radius of fitting circle, used for pT-calculation
  double clapR = 2.*delta / (1. + rootTerm);
  double radius = -1. / rho;
//  if (radius < 0.) { radius *= -1.; }

  double ref_x = cos(clapPhi - m_PI / 2.) * clapR;
  double ref_y = sin(clapPhi - m_PI / 2.) * clapR;

  ref_point.first = ref_x;
  ref_point.second = ref_y;
  track->setReferencePoint(ref_x, ref_y);


//  cout << "Before: " << track->second.first << "  " << track->second.second << endl;
  track->setTheta(clapPhi + m_PI / 2.);
  track->setR(1. / radius);
//  cout << "k=" << sumWeights << " delta=" << divisor << endl;
//  cout << "After: " << track->second.first << " " << track->second.second << endl;

  double chi2 = sumWeights * (1. + rho * clapR) * (1. + rho * clapR) * (sinPhi * sinPhi * covXX - 2.*sinPhi * cosPhi * covXY + cosPhi * cosPhi * covYY - kappa * kappa * covR2R2); /// returns chi2
  cout << chi2 << endl;
  track->setChi2(chi2/*track->getNAxialHits()*/);
}

