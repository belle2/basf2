/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/TrackletFilters.h"
#include <TMathBase.h>
#include <math.h>
#include <boost/foreach.hpp>
#include <list>
#include <iostream>
#include <framework/logging/Logger.h>
#include <stdio.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Tracking;

/// TODO: evtly do a 'nan'-check for return values


bool TrackletFilters::ziggZaggXY()
{
  if (m_hits == NULL) B2FATAL(" TrackletFilters::ziggZaggXY hits not set, therefore no calculation possible - please check that!")
    list<int> chargeSigns;
  bool isZiggZagging = false; // good: not ziggZagging
  for (int i = 0; i < m_numHits - 2; ++i) {
    int signValue = m_3hitFilterBox.calcSign(m_hits->at(i)->hitPosition, m_hits->at(i + 1)->hitPosition, m_hits->at(i + 2)->hitPosition);
    chargeSigns.push_back(signValue);
//    cout << "zzXY charge was: " << signValue << endl;
  }
  chargeSigns.sort();
  chargeSigns.unique();
  if (int(chargeSigns.size()) != 1) {
    isZiggZagging = true;
  }
  return isZiggZagging;
}



bool TrackletFilters::ziggZaggRZ()
{
  if (m_hits == NULL) B2FATAL(" TrackletFilters::ziggZaggRZ hits not set, therefore no calculation possible - please check that!")
    list<int> chargeSigns;
  bool isZiggZagging = false; // good: not ziggZagging
  vector<TVector3> rzHits;
  TVector3 currentVector;
  BOOST_FOREACH(PositionInfo * aHit, *m_hits) {
    currentVector.SetXYZ(aHit->hitPosition.Perp(), aHit->hitPosition[1], 0.);
    rzHits.push_back(currentVector);
  }
  for (int i = 0; i < m_numHits - 2; ++i) {
    int signValue = m_3hitFilterBox.calcSign(rzHits[i], rzHits[i + 1], rzHits[i + 2]);
    chargeSigns.push_back(signValue);
//    cout << "zzRZ charge was: " << signValue << endl;
  }
  chargeSigns.sort();
  chargeSigns.unique();
  if (int(chargeSigns.size()) != 1) {
    isZiggZagging = true;
  }
  return isZiggZagging;
}


// clap = closest approach of fitted circle to origin
double TrackletFilters::circleFit(double& clapPhi, double& clapR, double& radius)
{
  if (m_hits == NULL) B2FATAL(" TrackletFilters::circleFit hits not set, therefore no calculation possible - please check that!")
    double stopper = 0.000000001;
  double meanX = 0, meanY = 0, meanX2 = 0, meanY2 = 0, meanR2 = 0, meanR4 = 0, meanXR2 = 0, meanYR2 = 0, meanXY = 0; //mean values
  double r2 = 0, x = 0, y = 0, x2 = 0, y2 = 0; // coords
  double weight;// weight of each hit, so far no difference in hit quality
  double sumWeights = 0, divisor; // sumWeights is sum of weights, divisor is 1/sumWeights;

  // looping over all hits and do the division afterwards
  BOOST_FOREACH(PositionInfo * hit, *m_hits) {
    weight = 1. / ((hit->sigmaX) * (hit->sigmaX));
    B2DEBUG(100, " current hitSigma: " << hit->sigmaX << ", weight: " << weight)
    sumWeights += weight;
    if (hit->sigmaX < stopper) B2FATAL("TrackletFilters::circleFit, chosen sigma is too small (is/threshold: " << hit->sigmaX << "/" << stopper << ")")
      x = hit->hitPosition.X();
    y = hit->hitPosition.Y();
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

  clapPhi = 0.5 * atan2(2. * q1 , q2); // physical meaning: phi value of the point of closest approach of the fitted circle to the origin

  double sinPhi = sin(clapPhi);
  double cosPhi = cos(clapPhi);
  double kappa = (sinPhi * covXR2 - cosPhi * covYR2) / covR2R2;
  double delta = -kappa * meanR2 + sinPhi * meanX - cosPhi * meanY;
  double rootTerm = sqrt(1. - 4.*delta * kappa);
  double rho = 2.*kappa / (rootTerm); // rho = curvature in X-Y-plane = 1/radius of fitting circle, used for pT-calculation
  clapR = 2.*delta / (1. + rootTerm);
  radius = 1. / rho;
  double chi2 = sumWeights * (1. + rho * clapR) * (1. + rho * clapR) * (sinPhi * sinPhi * covXX - 2.*sinPhi * cosPhi * covXY + cosPhi * cosPhi * covYY - kappa * kappa * covR2R2);
  return chi2;
}
