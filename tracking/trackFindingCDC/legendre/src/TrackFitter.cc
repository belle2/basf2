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
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>

#include <tracking/trackFindingCDC/numerics/numerics.h>
#include <cmath>
#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

double TrackFitter::fitTrackCandidateFast(
  std::vector<std::pair<std::pair<double, double>, double>>& hits)
{
  double chi2(-999);

  double meanX = 0, meanY = 0, meanX2 = 0, meanY2 = 0, meanR2 = 0, meanR4 = 0, meanXR2 = 0, meanYR2 = 0, meanXY = 0; //mean values
  double r2 = 0, x = 0, y = 0, x2 = 0, y2 = 0; // coords
  double weight;// weight of each hit, so far no difference in hit quality
  double sumWeights = 0, divisor; // sumWeights is sum of weights, divisor is 1/sumWeights;

//    int nhits = hits.size();

  // if with_drift_time if true, uses drift time information in fitting procedure
  // looping over all hits and do the division afterwards
  for (std::pair<std::pair<double, double>, double>& hit : hits) {
    if (hit.second != 0.)weight =  1. / hit.second;
    else weight = 1.;
    //      if (nhits < 15) weight = 1.;
    sumWeights += weight;
    x = hit.first.first;
    y = hit.first.second;
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

  chi2 = sumWeights * (1. + rho * clapR) * (1. + rho * clapR) * (sinPhi * sinPhi * covXX - 2.*sinPhi * cosPhi * covXY + cosPhi *
         cosPhi * covYY - kappa * kappa * covR2R2); /// returns chi2

  //  chi2 = estimateChi2( hits, track_par, ref_point);


  return chi2;
}

double TrackFitter::fitTrackCandidateFast(
  std::vector<const CDCWireHit*>& hits,
  std::pair<double, double>& track_par,
  std::pair<double, double>& ref_point)
{

  double meanX = 0, meanY = 0, meanX2 = 0, meanY2 = 0, meanR2 = 0, meanR4 = 0, meanXR2 = 0, meanYR2 = 0, meanXY = 0; //mean values
  double r2 = 0, x = 0, y = 0, x2 = 0, y2 = 0; // coords
  double weight;// weight of each hit, so far no difference in hit quality
  double sumWeights = 0, divisor; // sumWeights is sum of weights, divisor is 1/sumWeights;

//    int nhits = hits.size();

  // if with_drift_time if true, uses drift time information in fitting procedure
  // looping over all hits and do the division afterwards
  for (const CDCWireHit* hit : hits) {
    if (hit->getRefDriftLength() != 0.)weight =  1. / fabs(hit->getRefDriftLength());
    else weight = 1.;
    //      if (nhits < 15) weight = 1.;
    sumWeights += weight;
    x = hit->getRefPos2D().x();
    y = hit->getRefPos2D().y();
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

  track_par.first = clapPhi + TMath::Pi() / 2.;
  track_par.second = -1. * rho;

  double chi2 = sumWeights * (1. + rho * clapR) * (1. + rho * clapR) * (sinPhi * sinPhi * covXX - 2.*sinPhi * cosPhi * covXY +
                cosPhi *
                cosPhi * covYY - kappa * kappa * covR2R2); /// returns chi2

  return chi2;
}


CDCTrajectory2D  TrackFitter::fitCDCTrackWhithoutRecoPos(CDCTrack& track)
{
  std::vector<const CDCWireHit*> wireHits;
  for (const CDCRecoHit3D& hit : track) {
    wireHits.push_back(&(hit.getWireHit()));
  }

  return fitWireHitsWhithoutRecoPos(wireHits);
}

CDCTrajectory2D TrackFitter::fitWireHitsWhithoutRecoPos(std::vector<const CDCWireHit*>& wireHits)
{
  CDCKarimakiFitter trackFitter;
  CDCObservations2D observations;
  for (const CDCWireHit* wireHit : wireHits) {
    observations.append(wireHit->getRefPos2D().x(), wireHit->getRefPos2D().y(), 0, 1. / fabs(wireHit->getRefDriftLength()));
  }
  CDCTrajectory2D trackTrajectory2D ;
  trackFitter.update(trackTrajectory2D, observations);

  return trackTrajectory2D;
}


