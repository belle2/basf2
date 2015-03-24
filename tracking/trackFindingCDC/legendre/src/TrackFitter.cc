/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/TrackFitter.h>

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>
#include <cmath>
#include <TMath.h>
#include <tracking/trackFindingCDC/numerics/numerics.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

double TrackFitter::fitTrackCandidateFast(
  std::vector<TrackHit*>& hits,
  std::pair<double, double>& track_par,
  std::pair<double, double>& ref_point,
  bool with_drift_time)
{
  int mode(1);
  double chi2(-999);
  if (mode == 1) {

    double meanX = 0, meanY = 0, meanX2 = 0, meanY2 = 0, meanR2 = 0, meanR4 = 0, meanXR2 = 0, meanYR2 = 0, meanXY = 0; //mean values
    double r2 = 0, x = 0, y = 0, x2 = 0, y2 = 0; // coords
    double weight;// weight of each hit, so far no difference in hit quality
    double sumWeights = 0, divisor; // sumWeights is sum of weights, divisor is 1/sumWeights;

//    int nhits = hits.size();

    // if with_drift_time if true, uses drift time information in fitting procedure
    if (!with_drift_time) {
      // looping over all hits and do the division afterwards
      for (TrackHit* hit : hits) {
        if (hit->getDriftLength() != 0.)weight =  1. / hit->getDriftLength();
        else weight = 1.;
        //      if (nhits < 15) weight = 1.;
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
      double radius_track = fabs(1. / track_par.second);
      //    xc_track = track->getXc();
      //    yc_track = track->getYc();
      double xc_track = cos(track_par.first) / track_par.second + ref_point.first;
      double yc_track = sin(track_par.first) / track_par.second + ref_point.second;
      for (TrackHit* hit : hits) {
        //      if (hit->getDriftLength() != 0.)weight =  1. / hit->getDriftLength();
        //      else continue;
        weight = 1.;
        sumWeights += weight;

        x = hit->getWirePosition().X();
        y = hit->getWirePosition().Y();
        double __attribute__((unused)) R_dist = sqrt(square(xc_track - x) + square(yc_track - y));
        double dist = radius_track - R_dist;
        double driftLength = hit->getDriftLength();

        if (dist > 0) {
          x = x + driftLength * ((x - xc_track) / R_dist);
          y = y + driftLength * ((y - yc_track) / R_dist);
        } else {
          x = x - driftLength * ((x - xc_track) / R_dist);
          y = y - driftLength * ((y - yc_track) / R_dist);
        }


//        x = (xc_track * driftLength + x * radius_track) / (driftLength + radius_track);
//        y = (yc_track * driftLength + y * radius_track) / (driftLength + radius_track);
//

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

    double clapPhi = 0.5 * atan2(2. * q1 ,
                                 q2); // physical meaning: phi value of the point of closest approach of the fitted circle to the origin

    double sinPhi = sin(clapPhi);
    double cosPhi = cos(clapPhi);
    double kappa = (sinPhi * covXR2 - cosPhi * covYR2) / covR2R2;
    double delta = -kappa * meanR2 + sinPhi * meanX - cosPhi * meanY;
    double rootTerm = sqrt(1. - 4.*delta * kappa);
    double rho = 2.*kappa / (rootTerm); // rho = curvature in X-Y-plane = 1/radius of fitting circle, used for pT-calculation
    double clapR = 2.*delta / (1. + rootTerm);
//    double radius = -1. / rho;
    //  if (radius < 0.) { radius *= -1.; }

    double ref_x = cos(clapPhi - TMath::Pi() / 2.) * clapR;
    double ref_y = sin(clapPhi - TMath::Pi() / 2.) * clapR;

    ref_point.first = ref_x;
    ref_point.second = ref_y;

    B2DEBUG(100, "============== Fitting info (track candidate as TrackCandidate object) (mode 1) ===============");
    B2DEBUG(100, "Before: theta: " << track_par.first << "; r: " << track_par.second);
    track_par.first = clapPhi + TMath::Pi() / 2.;
    track_par.second = -1. * rho;
    B2DEBUG(100, "After:  theta: " << track_par.first << "; r: " << track_par.second);

    chi2 = sumWeights * (1. + rho * clapR) * (1. + rho * clapR) * (sinPhi * sinPhi * covXX - 2.*sinPhi * cosPhi * covXY + cosPhi *
           cosPhi * covYY - kappa * kappa * covR2R2); /// returns chi2
    B2DEBUG(100, "chi2: " << chi2);

    //  chi2 = estimateChi2( hits, track_par, ref_point);




    //----------------------------
    /*

        meanX = 0; meanY = 0; meanX2 = 0; meanY2 = 0; meanR2 = 0; meanR4 = 0; meanXR2 = 0; meanYR2 = 0; meanXY = 0; //mean values
        r2 = 0; x = 0; y = 0; x2 = 0; y2 = 0; // coords
        weight = 0;// weight of each hit, so far no difference in hit quality
        sumWeights = 0; divisor = 0; // sumWeights is sum of weights, divisor is 1/sumWeights;


        double radius_track = fabs(1. / track_par.second);
        double xc_track = cos(track_par.first) / track_par.second + ref_point.first;
        double yc_track = sin(track_par.first) / track_par.second + ref_point.second;
        for (TrackHit * hit : hits) {
          weight = 1.;
          sumWeights += weight;

          x = hit->getWirePosition().X();
          y = hit->getWirePosition().Y();
          double R_dist = sqrt(SQR(xc_track - x) + SQR(yc_track - y));
          double dist = R_dist - radius_track;
          double driftLength = hit->getDriftLength();

    //      if (dist > 0) {
    //        x = x + hit->getDriftLength() * (R_dist / (x - xc_track));
    //        y = y + hit->getDriftLength() * (R_dist / (y - yc_track));
    //      } else {
    //        x = x - hit->getDriftLength() * (R_dist / (x - xc_track));
    //        y = y - hit->getDriftLength() * (R_dist / (y - yc_track));
    //      }

          if (dist > 0) {
            x = (xc_track * driftLength + x*radius_track)/(driftLength + radius_track);
            y = (yc_track * driftLength + y*radius_track)/(driftLength + radius_track);
          } else {
            x = (xc_track * driftLength + x*radius_track)/(radius_track - driftLength );
            y = (yc_track * driftLength + y*radius_track)/(radius_track - driftLength );
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
        covXX = meanX2 - meanX * meanX;
        covXY = meanXY - meanX * meanY;
        covYY = meanY2 - meanY * meanY;
        covXR2 = meanXR2 - meanX * meanR2;
        covYR2 = meanYR2 - meanY * meanR2;
        covR2R2 = meanR4 - meanR2 * meanR2;

        // q1, q2: helping variables, to make the code more readable
        q1 = covR2R2 * covXY - covXR2 * covYR2;
        q2 = covR2R2 * (covXX - covYY) - covXR2 * covXR2 + covYR2 * covYR2;

        clapPhi = 0.5 * atan2(2. * q1 , q2); // physical meaning: phi value of the point of closest approach of the fitted circle to the origin

        sinPhi = sin(clapPhi);
        cosPhi = cos(clapPhi);
        kappa = (sinPhi * covXR2 - cosPhi * covYR2) / covR2R2;
        delta = -kappa * meanR2 + sinPhi * meanX - cosPhi * meanY;
        rootTerm = sqrt(1. - 4.*delta * kappa);
        rho = 2.*kappa / (rootTerm); // rho = curvature in X-Y-plane = 1/radius of fitting circle, used for pT-calculation
        clapR = 2.*delta / (1. + rootTerm);
    //    double radius = -1. / rho;
      //  if (radius < 0.) { radius *= -1.; }

        ref_x = cos(clapPhi - m_PI / 2.) * clapR;
        ref_y = sin(clapPhi - m_PI / 2.) * clapR;

        ref_point.first = ref_x;
        ref_point.second = ref_y;

        B2DEBUG(100, "============== Fitting info (track candidate as TrackCandidate object) (mode 1) ===============");
        B2DEBUG(100, "Before: theta: " << track_par.first << "; r: " << track_par.second);
        track_par.first = clapPhi + m_PI / 2.;
        track_par.second = -1. * rho;
        B2DEBUG(100, "After:  theta: " << track_par.first << "; r: " << track_par.second);

        chi2 = sumWeights * (1. + rho * clapR) * (1. + rho * clapR) * (sinPhi * sinPhi * covXX - 2.*sinPhi * cosPhi * covXY + cosPhi * cosPhi * covYY - kappa * kappa * covR2R2); /// returns chi2
        B2DEBUG(100, "chi2: " << chi2);
    */

  }

  if (1. / track_par.second < 10.) mode = 2;

  if (mode == 2) {



    double x(0), x2(0), y(0), y2(0), xy(0), r2(0), r4(0), xr2(0), yr2(0), sum_w(0);
    for (TrackHit* hit : hits) {
      ;
      TVector3 hitPosition = hit->getWirePosition();

      double w = hit->getDriftLength();
      if (w == 0) w = 0.001;
      w = 1;
      w = 1. / w;
      double x_hit = hitPosition.X();
      double y_hit = hitPosition.Y();
      double r2_temp = x_hit * x_hit + y_hit * y_hit;

      x += x_hit * w;
      y += y_hit * w;
      x2 += x_hit * x_hit * w;
      y2 += y_hit * y_hit * w;
      xy += x_hit * y_hit * w;
      xr2 += x_hit * r2_temp * w;
      yr2 += y_hit * r2_temp * w;
      r2 += r2_temp * w;
      r4 += r2_temp * r2_temp * w;

      sum_w += w;

    }

    x = x / sum_w;
    y = y / sum_w;
    x2 = x2 / sum_w;
    y2 = y2 / sum_w;
    xy = xy / sum_w;
    xr2 = xr2 / sum_w;
    yr2 = yr2 / sum_w;
    r2 = r2 / sum_w;
    r4 = r4 / sum_w;
    /*
      double Cxx(0), Cxy(0), Cyy(0), Cxr2(0), Cyr2(0), Cr2r2(0);

      Cxx = x2 - x*x;
      Cxy = xy - x*y;
      Cyy = y2 - y*y;
      Cxr2 = xr2 - x*r2;
      Cyr2 = yr2 - y*r2;
      Cr2r2 = r4 - r2*r2;

      double q1(0), q2(0);

      q1 = Cr2r2 * Cxy - Cxr2 * Cyr2;
      q2 = Cr2r2 * (Cxx - Cyy) - Cxr2 * Cxr2 + Cyr2 * Cyr2;

      double phi(0), kappa(0), delta(0);
      phi = atan2(2. * q1, q2)/2.;
      kappa = (sin(phi)*Cxr2 - cos(phi)*Cyr2)/Cr2r2;
      delta = -kappa*r2 + sin(phi)*x - cos(phi)*y;

      double rho(0), dist(0);

      rho = 2.*kappa/sqrt(1-4.*delta*kappa);
      dist = 2.*delta/(1+sqrt(1-4.*delta*kappa));

      double radius = -1. / rho;
    //  if (radius < 0.) { radius *= -1.; }

      double ref_x = cos(phi - m_PI / 2.) * dist;
      double ref_y = sin(phi - m_PI / 2.) * dist;

      ref_point.first = ref_x;
      ref_point.second = ref_y;

      B2DEBUG(100, "============== Fitting info (track candidate as TrackCandidate object) ===============");
      B2DEBUG(100, "Before: theta: " << track_par.first << "; r: " << track_par.second);
      track_par.first = phi + m_PI / 2.;
      track_par.second = 1. / radius;
      B2DEBUG(100, "After:  theta: " << track_par.first << "; r: " << track_par.second);

      double chi2 = 1.;//sumWeights * (1. + rho * dist) * (1. + rho * dist) * (sinPhi * sinPhi * covXX - 2.*sinPhi * cosPhi * covXY + cosPhi * cosPhi * covYY - kappa * kappa * covR2R2); /// returns chi2
      B2DEBUG(100, "chi2: " << chi2);

    //  chi2 = estimateChi2( hits, track_par, ref_point);
      B2INFO("params: ---------")
      B2INFO("rho*dist: " << rho*dist);
      B2INFO("x: " << x << "; y: " << y << "; xy: " << xy << "; acos(xy/x*y):" << asin(x*y/xy));
    */

    double __attribute__((unused)) angle_rot(0);
    angle_rot = atan2(2.*xy, x2 - y2) / 2. ;

    double new_x(x), new_y(y);//, rotation(tan(xy));

    x = 0;
    y = 0;
    x2 = 0;
    y2 = 0;
    sum_w = 0;
    for (TrackHit* hit : hits) {
      ;
      TVector3 hitPosition = hit->getWirePosition();

      double w = hit->getDriftLength();
      if (w == 0) w = 0.001;
      w = 1;
      w = 1. / w;
      double x_hit = hitPosition.X() - new_x; // operation of system translation
      double y_hit = hitPosition.Y() - new_y;

      //    double x_hit = cos(angle_rot)*x_translation - sin(angle_rot) * y_translation;
      //    double y_hit = sin(angle_rot)*x_translation + cos(angle_rot) * y_translation;


//      double r2_temp = x_hit * x_hit + y_hit * y_hit;

      x += x_hit * w;
      y += y_hit * w;
      x2 += x_hit * x_hit * w;
      y2 += y_hit * y_hit * w;
      xy += x_hit * y_hit * w;

      sum_w += w;

    }

    angle_rot = atan2(2.*xy, y2 - x2) / 2. ;
    x = 0;
    y = 0;
    x2 = 0;
    y2 = 0;
    xy = 0;
    xr2 = 0;
    yr2 = 0;
    r2 = 0;
    r4 = 0;
    sum_w = 0;
    for (TrackHit* hit : hits) {
      ;
      TVector3 hitPosition = hit->getWirePosition();

      double w = hit->getDriftLength();
      if (w == 0) w = 0.001;
      w = 1;
      w = 1. / w;
      double x_translation = hitPosition.X() - new_x; // operation of system translation
      double y_translation = hitPosition.Y() - new_y;

      double x_hit = x_translation;//cos(angle_rot)*x_translation - sin(angle_rot) * y_translation;
      double y_hit = y_translation;//sin(angle_rot)*x_translation + cos(angle_rot) * y_translation;


      double r2_temp = x_hit * x_hit + y_hit * y_hit;

      x += x_hit * w;
      y += y_hit * w;
      x2 += x_hit * x_hit * w;
      y2 += y_hit * y_hit * w;
      xy += x_hit * y_hit * w;
      xr2 += x_hit * r2_temp * w;
      yr2 += y_hit * r2_temp * w;
      r2 += r2_temp * w;
      r4 += r2_temp * r2_temp * w;

      sum_w += w;

    }

    x = x / sum_w;
    y = y / sum_w;
    x2 = x2 / sum_w;
    y2 = y2 / sum_w;
    xy = xy / sum_w;
    xr2 = xr2 / sum_w;
    yr2 = yr2 / sum_w;
    r2 = r2 / sum_w;
    r4 = r4 / sum_w;

    double Cxx(0), Cxy(0), Cyy(0), Cxr2(0), Cyr2(0), Cr2r2(0);

    Cxx = x2 - x * x;
    Cxy = xy - x * y;
    Cyy = y2 - y * y;
    Cxr2 = xr2 - x * r2;
    Cyr2 = yr2 - y * r2;
    Cr2r2 = r4 - r2 * r2;

    double q1(0), q2(0);

    q1 = Cr2r2 * Cxy - Cxr2 * Cyr2;
    q2 = Cr2r2 * (Cxx - Cyy) - Cxr2 * Cxr2 + Cyr2 * Cyr2;

    double phi(0), kappa(0), delta(0);
    phi = atan2(2. * q1, q2) / 2.;
    kappa = (sin(phi) * Cxr2 - cos(phi) * Cyr2) / Cr2r2;
    delta = -kappa * r2 + sin(phi) * x - cos(phi) * y;

    double rho(0), dist(0);

    rho = 2.*kappa / sqrt(1 - 4.*delta * kappa);
    dist = 2.*delta / (1 + sqrt(1 - 4.*delta * kappa));

    double A(0), B(0), U(0), u(0), C(0), Delta_parr(0), Delta_perp(0);

    u = 1 + rho * dist;
    Delta_perp = new_x * sin(phi) - new_y * cos(phi) + dist;
    Delta_parr = new_x * cos(phi) + new_y * sin(phi);
    A = 2.*Delta_perp + rho * (Delta_parr * Delta_parr + Delta_perp * Delta_perp);
    B = rho * new_x + u * sin(phi);
    C = -1.*rho * new_y + u * cos(phi);
    U = sqrt(1 + rho * A);

    double phi_new = atan2(B, C);
    double dist_new = A / (1 + U);





//    double radius = track_par.second / (rho * fabs(track_par.second));
    //  if (radius < 0.) { radius *= -1.; }
    //  if (radius < 0.) { radius *= -1.; }

    double ref_x = cos(phi_new - TMath::Pi() / 2.) * dist_new;
    double ref_y = sin(phi_new - TMath::Pi() / 2.) * dist_new;

    ref_point.first = ref_x;
    ref_point.second = ref_y;

    B2DEBUG(100, "============== Fitting info (track candidate as TrackCandidate object) (mode 2) ===============");
    B2DEBUG(100, "Before: theta: " << track_par.first << "; r: " << track_par.second);
    track_par.first = phi_new + TMath::Pi() / 2.;
    track_par.second = -1. * rho;
    B2DEBUG(100, "After:  theta: " << track_par.first << "; r: " << track_par.second);

    chi2 = sum_w * (1. + rho * dist) * (1. + rho * dist) * (sin(phi) * sin(phi) * Cxx - 2.*sin(phi) * cos(phi) * Cxy + cos(phi) * cos(
                                                              phi) * Cyy - kappa * kappa * Cr2r2); /// returns chi2
    //B2INFO( "chi2: " << chi2);

    //  B2INFO("x: " << x << "; y: " << y << "; xy: " << xy << "; atan(xy): " << atan(xy));

  }

//  if(with_drift_time) B2INFO( "chi2: " << chi2 / (hits.size() - 3));

  return chi2 / (hits.size() - 3);

}


void TrackFitter::fitTrackCandidateFast(TrackCandidate* track,
                                        bool with_drift_time)
{

  std::pair<double, double> ref_point = make_pair(track->getReferencePoint().X(), track->getReferencePoint().Y());
  std::pair<double, double> track_par = std::make_pair(track->getTheta(), track->getR());

  double chi2 = fitTrackCandidateFast(track->getTrackHits(), track_par, ref_point, with_drift_time);
  if (not with_drift_time) chi2 = fitTrackCandidateFast(track->getTrackHits(), track_par, ref_point, true);

  track->setTheta(track_par.first);
  track->setR(track_par.second);
  track->setChi2(chi2);
  track->setReferencePoint(ref_point.first, ref_point.second);
}
