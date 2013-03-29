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

using namespace std;
using namespace Belle2;

/// TODO: evtly do a 'nan'-check for return values

TrackletFilters::TrackletFilters(vector<TVector3> hits):
  m_hits(hits)
{
  m_numHits = m_hits.size();
}



void TrackletFilters::resetValues(vector<TVector3> hits)
{
  m_hits = hits;
  m_numHits = hits.size();
}



bool TrackletFilters::ziggZaggXY()
{
  list<int> chargeSigns;
  bool isZiggZagging = false; // good: not ziggZagging
  for (int i = 0; i < m_numHits - 2; ++i) {
    int signValue = m_3hitFilterBox.calcSign(m_hits[i], m_hits[i + 1], m_hits[i + 2]);
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
  list<int> chargeSigns;
  bool isZiggZagging = false; // good: not ziggZagging
  vector<TVector3> rzHits;
  TVector3 currentVector;
  BOOST_FOREACH(TVector3 aHit, m_hits) {
    currentVector.SetXYZ(aHit.Perp(), aHit[1], 0.);
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
double TrackletFilters::circleFit(double& clapPhi, double& clapR)
{
  double meanX = 0, meanY = 0, meanX2 = 0, meanY2 = 0, meanR2 = 0, meanR4 = 0, meanXR2 = 0, meanYR2 = 0, meanXY = 0; //mean values
  double r2 = 0, x = 0, y = 0, x2 = 0, y2 = 0, divisor = 1. / m_numHits; // coords and divisor which is the same for all of them

  // looping over all hits and do the division afterwards
  BOOST_FOREACH(TVector3 hit, m_hits) {
    x = hit.X();
    y = hit.Y();
    x2 = x * x;
    y2 = y * y;
    r2 = x2 + y2;
    meanX += x;
    meanY += y;
    meanXY += x * y;
    meanX2 += x2;
    meanY2 += y2;
    meanXR2 += x * r2;
    meanYR2 += y * r2;
    meanR2 += r2;
    meanR4 += r2 * r2;
  }
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

  clapPhi = 0.5 * atan(2. * q1 / q2); // physical meaning: phi value of the point of closest approach of the fitted circle to the origin

  double sinPhi = sin(clapPhi);
  double cosPhi = cos(clapPhi);
  double kappa = (sinPhi * covXR2 - cosPhi * covYR2) / covR2R2;
  double delta = -kappa * meanR2 + sinPhi * meanX - cosPhi * meanY;
  double rootTerm = sqrt(1. - 4.*delta * kappa);
  double rho = 2.*kappa / (rootTerm); // rho = 1/curvature in X-Y-plane = radius of fitting circle, used for pT-calculation
  double dist = 2.*delta / (1 + rootTerm);
  clapR = dist; // WARNING currently not sure whether this is indeed right...
  double chi2 = m_numHits * (1 + rho * dist) * (1 + rho * dist) * (sinPhi * sinPhi * covXX - 2.*sinPhi * cosPhi * covXY + cosPhi * cosPhi * covYY - kappa * kappa * covR2R2);
  return chi2;
}


// if you do not want to have the coordinates of the point of closest approach, use this one
double TrackletFilters::circleFit()
{
  double phiValue, rValue;
  return circleFit(phiValue, rValue);
}
