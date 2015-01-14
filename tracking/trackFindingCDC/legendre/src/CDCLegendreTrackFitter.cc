/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackFitter.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>


#define SQR(x) ((x)*(x)) //we will use it in least squares fit

using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace VXD;
using namespace TrackFindingCDC;
using namespace TrackFindingCDC;

double TrackFitter::fitTrackCandidateFast(
  std::vector<TrackHit*>& hits,
  std::pair<double, double>& track_par,
  std::pair<double, double>& ref_point,
  bool with_drift_time)
{
  if (!m_fitTracks) return 0;

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
      for (TrackHit * hit : hits) {
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
      for (TrackHit * hit : hits) {
        //      if (hit->getDriftLength() != 0.)weight =  1. / hit->getDriftLength();
        //      else continue;
        weight = 1.;
        sumWeights += weight;

        x = hit->getWirePosition().X();
        y = hit->getWirePosition().Y();
        double __attribute__((unused)) R_dist = sqrt(SQR(xc_track - x) + SQR(yc_track - y));
//        double dist = radius_track - R_dist;
        double driftLength = hit->getDriftLength();
        /*
                if (dist > 0) {
                  x = x + hit->getDriftLength() * (R_dist / (x - xc_track));
                  y = y + hit->getDriftLength() * (R_dist / (y - yc_track));
                } else {
                  x = x - hit->getDriftLength() * (R_dist / (x - xc_track));
                  y = y - hit->getDriftLength() * (R_dist / (y - yc_track));
                }
        */

        x = (xc_track * driftLength + x * radius_track) / (driftLength + radius_track);
        y = (yc_track * driftLength + y * radius_track) / (driftLength + radius_track);


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
//    double radius = -1. / rho;
    //  if (radius < 0.) { radius *= -1.; }

    double ref_x = cos(clapPhi - m_PI / 2.) * clapR;
    double ref_y = sin(clapPhi - m_PI / 2.) * clapR;

    ref_point.first = ref_x;
    ref_point.second = ref_y;

    B2DEBUG(100, "============== Fitting info (track candidate as TrackCandidate object) (mode 1) ===============");
    B2DEBUG(100, "Before: theta: " << track_par.first << "; r: " << track_par.second);
    track_par.first = clapPhi + m_PI / 2.;
    track_par.second = -1. * rho;
    B2DEBUG(100, "After:  theta: " << track_par.first << "; r: " << track_par.second);

    chi2 = sumWeights * (1. + rho * clapR) * (1. + rho * clapR) * (sinPhi * sinPhi * covXX - 2.*sinPhi * cosPhi * covXY + cosPhi * cosPhi * covYY - kappa * kappa * covR2R2); /// returns chi2
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
    for (TrackHit * hit : hits) {
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
    for (TrackHit * hit : hits) {
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
    for (TrackHit * hit : hits) {
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

    double ref_x = cos(phi_new - m_PI / 2.) * dist_new;
    double ref_y = sin(phi_new - m_PI / 2.) * dist_new;

    ref_point.first = ref_x;
    ref_point.second = ref_y;

    B2DEBUG(100, "============== Fitting info (track candidate as TrackCandidate object) (mode 2) ===============");
    B2DEBUG(100, "Before: theta: " << track_par.first << "; r: " << track_par.second);
    track_par.first = phi_new + m_PI / 2.;
    track_par.second = -1. * rho;
    B2DEBUG(100, "After:  theta: " << track_par.first << "; r: " << track_par.second);

    chi2 = sum_w * (1. + rho * dist) * (1. + rho * dist) * (sin(phi) * sin(phi) * Cxx - 2.*sin(phi) * cos(phi) * Cxy + cos(phi) * cos(phi) * Cyy - kappa * kappa * Cr2r2); /// returns chi2
    //B2INFO( "chi2: " << chi2);

    //  B2INFO("x: " << x << "; y: " << y << "; xy: " << xy << "; atan(xy): " << atan(xy));

  }

  return chi2 / (hits.size() - 4);

}




void TrackFitter::estimatePolarAngle(TrackCandidate* track)
{
  std::vector<TrackHit*> vectorHits;
  for (TrackHit * hit : track->getTrackHits()) {
    if (not hit->getIsAxial()) {
      vectorHits.push_back(hit);
    }
  }
  B2INFO("Number of stereohits is: " << vectorHits.size());
  if (vectorHits.size() == 0) {
    B2DEBUG(100, "TrackFindingCDC::TrackFitter: track has no stereohits!");
    return;
  }

  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();
  TVector3 forwardWirePoint; //forward end of the wire
  TVector3 backwardWirePoint; //backward end of the wire
  TVector3 mediumWirePoint; //center of wire
  TVector3 wireVector;  //direction of the wire
  double Rcand = track->getRadius();

  int sign(0);

  double thetaMean = 0;

  for (TrackHit * hit : vectorHits) {
    int sLayer = hit->getSuperlayerId();
    if (sLayer == 1 || sLayer == 5) sign = -1;
    else sign = 1;

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


    double dist_1 = Rcand - sqrt(SQR(hit->getOriginalWirePosition().X() - track->getXc()) + SQR(hit->getOriginalWirePosition().Y() - track->getYc())) - hit->getDriftLength();
    double dist_2 = Rcand - sqrt(SQR(hit->getOriginalWirePosition().X() - track->getXc()) + SQR(hit->getOriginalWirePosition().Y() - track->getYc())) + hit->getDriftLength();

    double alpha = acos(1. - (SQR(hit->getOriginalWirePosition().X() - track->getReferencePoint().X()) + SQR(hit->getOriginalWirePosition().Y() - track->getReferencePoint().Y())) / (2.*SQR(Rcand)));

    double phi_on_track = track->getTheta() + (3.1415 / 2. - alpha / 2.) * track->getChargeSign(); //TODO: implement curlers!
    double delta_phi = hit->getPhi() - phi_on_track;
    if (delta_phi > 3.1415) delta_phi -= 3.1415;
    if (delta_phi < -3.1415) delta_phi += 3.1415;



    double sign_phi = delta_phi / fabs(delta_phi);

    sign_phi = track->getChargeSign() * dist_1 / fabs(dist_1) ;

    double sign_final;
    if ((sign_phi >= 0) ^ (sign < 0))
      sign_final = 1;
    else
      sign_final = -1;

//    sign_final = 1;

    double Zpos_1 = sign_final * (lWire * fabs(dist_1)) / rWire ;
    double Zpos_2 = sign_final * (lWire * fabs(dist_2)) / rWire ;

    double __attribute__((unused)) omega_1 = Zpos_1 / (alpha * track->getRadius());
    double __attribute__((unused)) omega_2 = Zpos_2 / (alpha * track->getRadius());

//    double theta_track_1 = (atan(omega_1) + 3.1415/2 ) * 180. / 3.1415;
//    double theta_track_2 = (atan(omega_2) + 3.1415/2 ) * 180. / 3.1415;

    double theta_track_1 = (atan2(Zpos_1 , (alpha * track->getRadius()) /*sqrt(SQR(hit->getOriginalWirePosition().x() - track->getReferencePoint().x()) + SQR(hit->getOriginalWirePosition().y() - track->getReferencePoint().y()))*/) + 3.1415 / 2) * 180. / 3.1415;
    double theta_track_2 = (atan2(Zpos_2 , (alpha * track->getRadius()) /*sqrt(SQR(hit->getOriginalWirePosition().x() - track->getReferencePoint().x()) + SQR(hit->getOriginalWirePosition().y() - track->getReferencePoint().y()))*/) + 3.1415 / 2) * 180. / 3.1415;

    B2INFO("THETA: " << theta_track_1 << "; " << theta_track_2 << " delta_phi: " << delta_phi << "; alpha: " << alpha);

    thetaMean += theta_track_1;
    thetaMean += theta_track_2;

    //        B2WARNING("Distance of stereohit to candidate with angle " << thetaTrack << " is : " << distIdeal << " (" << dist_1 << ", " << dist_2 << ")");
  }

  thetaMean = thetaMean / (2.* vectorHits.size());



  B2INFO("MEAN THETA: " << thetaMean);

  for (TrackHit * hit : vectorHits) {
    double z_pos(0);

    double alpha = acos(1. - (SQR(hit->getOriginalWirePosition().X() - track->getReferencePoint().X()) + SQR(hit->getOriginalWirePosition().Y() - track->getReferencePoint().Y())) / (2.*SQR(Rcand)));

    z_pos = alpha * Rcand / tan(thetaMean * 3.1415 / 180.);

    B2INFO("Z POS: " << z_pos);

    hit->setZReference(z_pos);
  }
}


bool TrackFitter::fitTrackCandWithSVD(TrackCandidate* track, /*std::vector<SVDCluster*>& svdClusters*/ const genfit::TrackCand* vxdTrack)
{
  if (!m_fitTracks) return false;

  std::vector<SVDCluster*> svdClustersTrackCand;
  StoreArray<SVDCluster> svdClusters;
  int nTrackHits = vxdTrack->getNHits();
  B2INFO("TrackCand: NHits: " << nTrackHits);
  for (int ii = 0; ii < nTrackHits; ii++) {
    genfit::TrackCandHit* vxdTrackCandHit = vxdTrack->getHit(ii);
    B2INFO("HitID: " << vxdTrackCandHit->getHitId());
    if (vxdTrackCandHit->getDetId() == Const::SVD) {
      svdClustersTrackCand.push_back(svdClusters[vxdTrackCandHit->getHitId()]);
    }
//    svdClustersTrackCand.push_back( svdClusters[vxdTrackCandHit->getHitId()] );
  }

  B2INFO("Number of SVDClusters for fitting: " << svdClustersTrackCand.size());

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  std::vector<pair<double, double>> svdHitsPos;

  if (svdClustersTrackCand.size() < 1) return false;

  for (SVDCluster * svdUCluster : svdClustersTrackCand) {
    if (not svdUCluster->isUCluster()) continue;
    for (SVDCluster * svdVCluster : svdClustersTrackCand) {
      if (svdVCluster->isUCluster())continue;
      if (svdVCluster->getSensorID() != svdUCluster->getSensorID()) continue;
      float __attribute__((unused)) posSVD = svdUCluster->getPosition();
      VxdID sensorID = svdUCluster->getSensorID();
//      B2INFO("posSVD = " << posSVD);
//      B2INFO("sensorID = " << sensorID);

      const   VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(sensorID);
      TVector3 hitLocal;

      if ((aSensorInfo.getBackwardWidth() > aSensorInfo.getForwardWidth()) == true) {   // isWedgeSensor
        hitLocal.SetX((aSensorInfo.getWidth(svdVCluster->getPosition()) / aSensorInfo.getWidth(0)) * svdUCluster->getPosition());
      } else { // rectangular Sensor
        hitLocal.SetX(svdUCluster->getPosition());
      }

      hitLocal.SetY(svdVCluster->getPosition()); // always correct
      hitLocal.SetZ(0.);
      TVector3 hitSigmaLocal(svdUCluster->getPositionSigma(), svdVCluster->getPositionSigma(), 0);

      TVector3 hitPosition = aSensorInfo.pointToGlobal(hitLocal);
      TVector3 hitSigma = aSensorInfo.vectorToGlobal(hitSigmaLocal);


      double radius = hitPosition.Pt();
      B2INFO("radius: " << radius);

      svdHitsPos.emplace_back(make_pair(hitPosition.X(), hitPosition.Y()));
    }
  }


  double meanX = 0, meanY = 0, meanX2 = 0, meanY2 = 0, meanR2 = 0, meanR4 = 0, meanXR2 = 0, meanYR2 = 0, meanXY = 0; //mean values
  double r2 = 0, x = 0, y = 0, x2 = 0, y2 = 0; // coords
  double weight;// weight of each hit, so far no difference in hit quality
  double sumWeights = 0, divisor; // sumWeights is sum of weights, divisor is 1/sumWeights;

  int nhits(0);
  nhits = track->getTrackHits().size() + svdHitsPos.size();

  // looping over all hits and do the division afterwards
  for (auto & hit : svdHitsPos) {
    weight = 1.;
//      if (nhits < 15) weight = 1.;
    sumWeights += weight;
    x = hit.first;
    y = hit.second;
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

  for (TrackHit * hit : track->getTrackHits()) {
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
//  double radius = track->getR() / (rho * fabs(track->getR()));
//  if (radius < 0.) { radius *= -1.; }

  double ref_x = cos(clapPhi - m_PI / 2.) * clapR;
  double ref_y = sin(clapPhi - m_PI / 2.) * clapR;


  track->setReferencePoint(ref_x, ref_y);

  track->setTheta(clapPhi + m_PI / 2.);
  track->setR(-1. * rho);

  double chi2 = sumWeights * (1. + rho * clapR) * (1. + rho * clapR) * (sinPhi * sinPhi * covXX - 2.*sinPhi * cosPhi * covXY + cosPhi * cosPhi * covYY - kappa * kappa * covR2R2); /// returns chi2
  B2INFO("FIT: trackCand + vxdTrack: chi2 = " << chi2 / (nhits - 4) << "; trackCand: chi2 = " << track->getChi2());

  if ((chi2 / (nhits - 4)) < track->getChi2() * 1.2) {
    track->setChi2(chi2 / (nhits - 4));
    return true;
  } else {
    return false;
  }

}


void TrackFitter::fitTrackCandWithTrack(TrackCandidate* track, /*std::vector<SVDCluster*>& svdClusters*/ genfit::Track* vxdTrack)
{
  for (genfit::TrackPoint * trackPoint : vxdTrack->getPoints()) {
    genfit::AbsMeasurement* trackPointAbsMeasurement = trackPoint->getRawMeasurement();
    int detId = trackPointAbsMeasurement->getDetId();
    int hitId = trackPointAbsMeasurement->getHitId();
    B2INFO("TrackPoint: detId = " << detId << " hitId = " << hitId);
    TVectorD matrix = trackPointAbsMeasurement->getRawHitCoords();
    int nRows = matrix.GetNrows();
    double* elements = matrix.GetMatrixArray();
    B2INFO("NRows: " << nRows);
    for (int ii = 0; ii < nRows; ii++) {
      B2INFO("Element " << ii << " is " << elements[ii]);
    }
    elements = nullptr;
    delete elements;
  }
}

void TrackFitter::performRiemannFit(TrackCandidate* track)
{

  const CDCRiemannFitter& riemannFitter = CDCRiemannFitter::getFitter();
  CDCTrajectory2D trajctory2D = riemannFitter.fit(track->getTrackHits());
  /*
    const CDCKarimakiFitter& karimakiFitter = CDCKarimakiFitter::getFitter();
    CDCTrajectory2D trajctory2D = karimakiFitter.fit(track->getTrackHits());
  */

  float r = trajctory2D.getGlobalCircle().radius();
//  float theta = trajctory2D.getStartMom2D().phi();
  float theta = trajctory2D.getGlobalCircle().tangential().phi();
  float x0 = trajctory2D.getGlobalCircle().perigee().x();
  float y0 = trajctory2D.getGlobalCircle().perigee().y();

  B2INFO("chi2 of fit: " << trajctory2D.getChi2());

  track->setR(1. / r);
  track->setTheta(theta + m_PI / 2.);
  track->setReferencePoint(x0, y0);

  /*
    m_fitTracks = true;

    std::pair<double, double> track_par = std::make_pair(track->getTheta(), track->getR());

    double chi2;
    std::pair<double, double> ref_point;

    chi2 = fitTrackCandidateFast(track->getTrackHits(), track_par, ref_point, false);

    track->setTheta(track_par.first);
    track->setR(track_par.second);
    track->setChi2(chi2);
    track->setReferencePoint(ref_point.first, ref_point.second);

    m_fitTracks = false;
   */
}



double TrackFitter::estimateChi2(std::vector<TrackHit*>& hits,
                                 std::pair<double, double>& track_par, std::pair<double, double>& ref_point)
{
  double chi2 = 0;

  double x0_track = cos(track_par.first) / fabs(track_par.second) + ref_point.first;
  double y0_track = sin(track_par.first) / fabs(track_par.second) + ref_point.second;

  for (TrackHit * hit : hits) {
    double x0_hit = hit->getOriginalWirePosition().X();
    double y0_hit = hit->getOriginalWirePosition().Y();
    double dist = fabs(fabs(1 / fabs(track_par.second) - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftLength());
    chi2 += (dist - hit->getSigmaDriftLength()) / hit->getSigmaDriftLength();
  }

  return chi2 / (hits.size() - 4);
}


void TrackFitter::fitTrackCandidateFast(
  std::pair<std::vector<TrackHit*>, std::pair<double, double> >* track,
  std::pair<double, double>& ref_point,
  double& chi2,
  bool with_drift_time)
{
  if (!m_fitTracks) return;

  std::pair<double, double> track_par = std::make_pair(0., 0.);

  chi2 = fitTrackCandidateFast(track->first, track_par, ref_point, with_drift_time);

  track->second.first = track_par.first;
  track->second.second = track_par.second;
}


void TrackFitter::fitTrackCandidateFast(
  TrackCandidate* track,
  std::pair<double, double>& ref_point,
  bool with_drift_time)
{
  if (!m_fitTracks) return;

  std::pair<double, double> track_par = std::make_pair(track->getTheta(), track->getR());

  double chi2;

  chi2 = fitTrackCandidateFast(track->getTrackHits(), track_par, ref_point, with_drift_time);

  track->setTheta(track_par.first);
  track->setR(track_par.second);
  track->setChi2(chi2);

}


void TrackFitter::fitTrackCandidateFast(TrackCandidate* track,
                                        bool with_drift_time)
{
  if (!m_fitTracks) return;

  std::pair<double, double> track_par = std::make_pair(track->getTheta(), track->getR());

  double chi2;
  std::pair<double, double> ref_point;

  chi2 = fitTrackCandidateFast(track->getTrackHits(), track_par, ref_point, with_drift_time);

  track->setTheta(track_par.first);
  track->setR(track_par.second);
  track->setChi2(chi2);
  track->setReferencePoint(ref_point.first, ref_point.second);

}







void TrackFitter::fitTrackCandidateStepped(
  std::pair<std::vector<TrackHit*>, std::pair<double, double> >* track)
{

  if (!m_fitTracks) return;

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
  for (TrackHit * hit : track->first) {
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

    for (TrackHit * hit : track->first) {
      x0 = hit->getConformalX();
      y0 = hit->getConformalY();
      drift_time = hit->getConformalDriftLength();
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

    for (TrackHit * hit : track->first) {
      x0 = hit->getConformalX();
      y0 = hit->getConformalY();
      drift_time = hit->getConformalDriftLength();
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

  } while (!is_finished);

}


void TrackFitter::fitTrackCandidateNormalSpace(
  std::pair<std::vector<TrackHit*>, std::pair<double, double> >* track,
  std::pair<double, double>* ref_point)
{

  if (!m_fitTracks) return;

  //get theta and r values for each track candidate
  double track_theta = track->second.first;
  double track_r = track->second.second;
  double summ;
  double summ_prev, summ_min;
  double increment_x0, increment_y0, increment_R;
  bool is_finished;
  int direction; //direction of increment: 1 - (1,1); 2 - (1,-1); 3 - (-1,-1); 4 - (-1,1), where (theta,r)
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

  for (TrackHit * hit : track->first) {
    x0_hit = hit->getOriginalWirePosition().X();
    y0_hit = hit->getOriginalWirePosition().Y();
    summ += SQR(fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftLength());
  }

  summ_prev = summ;
  increment_x0 = 1.;
  increment_y0 = 1.;
  increment_R = 1.;
  direction = 1;
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

    for (TrackHit * hit : track->first) {
      x0_hit = hit->getOriginalWirePosition().X();
      y0_hit = hit->getOriginalWirePosition().Y();
      summ += SQR(fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftLength());
    }

    if (direction == 1) {
      summ_min = summ;
      R_min = R;
      x0_track_min = x0_track;
      y0_track_min = y0_track;
    } else if (summ_min > summ) {
      summ_min = summ;
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
        B2DEBUG(100, "r: " << track->second.second << "R: " << R_min);
        B2DEBUG(100, "x0: " << x0_track_min << "y0: " << y0_track_min);
        B2DEBUG(100, "summ: " << summ_min);
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
    }
  } while (!is_finished);

  B2DEBUG(100, "initial: x0: " << x0_initial << "	y0 " << y0_initial << "	R:	" << R_initial);
  B2DEBUG(100, "final:   x0: " << x0_track_min << "	y0 " << y0_track_min << "	R:	" << R_min);
  B2DEBUG(100, "ref.p.:  x0: " << ref_point->first << "	y0 " << ref_point->second);
  B2DEBUG(100, "initial: th: " << track_theta_initial << "	r  " << track_r_initial);
  B2DEBUG(100, "final:   th: " << track->second.first << "	r  " << track->second.second);
}

