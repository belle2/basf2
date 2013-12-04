/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//fw
#include "../include/TrackletFilters.h"
#include <framework/logging/Logger.h>

// c++-base/stl:
#include <list>
#include <iostream>
#include <limits>       // std::numeric_limits
#include <stdio.h>
#include <math.h>
#include <fstream>
#include <iomanip>      // std::setprecision
#include <utility>

// root
#include <TMathBase.h>
// #include <TMatrixT.h>
#include <TMatrixD.h>
#include <TMatrixDEigen.h>
// #include <TMatrixDColumn.h>

// boost
#include <boost/math/special_functions/fpclassify.hpp> // abs
#include <boost/math/special_functions/sign.hpp> // sign

// Vc
// #include <Vc/Vc>



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
  for (PositionInfo * aHit : *m_hits) {
    currentVector.SetXYZ(aHit->hitPosition.Perp(), aHit->hitPosition[1], 0.);
    rzHits.push_back(currentVector);
  }
  for (int i = 0; i < m_numHits - 2; ++i) {
    int signValue = m_3hitFilterBox.calcSign(rzHits.at(i), rzHits.at(i + 1), rzHits.at(i + 2));
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


std::pair<TVector3, int> TrackletFilters::calcMomentumSeed(bool useBackwards)
{

  if (m_numHits < 3) {
    B2ERROR("calcInitialValues4TCs: currentTC got " << m_numHits << " hits! At this point only tcs having at least 3 hits should exist!")
  }
  //     hitA = (*hits)[2]->hitPosition;
  TVector3 hitB = (*m_hits)[1]->hitPosition;
  TVector3 hitC = (*m_hits)[0]->hitPosition; // outermost hit and initial value for genfit::TrackCandidate

  hitC -= hitB; // recycling TVector3s, this is segmentBC
  hitB -= (*m_hits)[2]->hitPosition; // this is segmentAB
  hitC.SetZ(0.);
  hitB.SetZ(0.);
  hitC = hitC.Orthogonal();

  std::pair<double, TVector3> helixFitValues = helixFit(m_hits, useBackwards);

  return make_pair(helixFitValues.second, boost::math::sign(hitC * hitB)); //.first: momentum vector. .second: sign of curvature: is > 0 if angle between vectors is < 90°, < 0 else (rule of scalar product)
}


// clap = closest approach of fitted circle to origin
double TrackletFilters::circleFit(double& clapPhi, double& clapR, double& radius)
{
  if (m_hits == NULL) { B2FATAL(" TrackletFilters::circleFit hits not set, therefore no calculation possible - please check that!") }
  double stopper = 0.000000001;
  double meanX = 0, meanY = 0, meanX2 = 0, meanY2 = 0, meanR2 = 0, meanR4 = 0, meanXR2 = 0, meanYR2 = 0, meanXY = 0; //mean values
  double r2 = 0, x = 0, y = 0, x2 = 0, y2 = 0; // coords
  double weight;// weight of each hit, so far no difference in hit quality
  double sumWeights = 0, divisor; // sumWeights is sum of weights, divisor is 1/sumWeights;
  double tuningParameter = 1.; //0.02; // this parameter is for internal tuning of the weights, since at the moment, the error seams highly overestimated at the moment. 1 means no influence of parameter.

  // looping over all hits and do the division afterwards
  for (PositionInfo * hit : *m_hits) {
    weight = 1. / ((hit->sigmaX) * (hit->sigmaX) * tuningParameter);
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
  if (radius < 0.) { radius *= -1.; }
  return sumWeights * (1. + rho * clapR) * (1. + rho * clapR) * (sinPhi * sinPhi * covXX - 2.*sinPhi * cosPhi * covXY + cosPhi * cosPhi * covYY - kappa * kappa * covR2R2); /// returns chi2
}



std::pair<double, TVector3> TrackletFilters::helixFit(const std::vector<Tracking::PositionInfo*>* hits, bool useBackwards)
{
  if (hits == NULL) { B2FATAL(" TrackletFilters::circleFit hits not set, therefore no calculation possible - please check that!") }
  // WARNING this function assumes that hits are sorted and first hit is innermost hit!!!
  /** NOTE:
   * current implementations lacks of style. This approach would be perfect for a vectorized approach
   * still missing:
   * - chi2 calculation currently returns -1, chi2 of circleFit and of lineFit has to be combined
   * - faster implementation (do vectorized)
   * - dealing with multiple scattering (Rudi will do an additional momentum-based implementation dealing with that)
   * - implementing Info into tuning parameters
   * - detailed check whether results are okay
   * */
  double  x = 0, // current x variable
          y = 0,
          z = 0,
          varU = 0, // variance of U
          varV = 0, // variance of V
///         phi = 0, // angle phi  // not used yet, but will be needed for some calculations which are not implemented yet
          r2 = 0, // radius^2
//          tempRadius = 0,
//          r = 0,
///         rPhi = 0,  // not used yet, but will be needed for some calculations which are not implemented yet
          sumWeights = 0, // the sum of the weights U
          inverseVarianceU = 0; // current inverse of variance U
  int nHits = hits->size();
  TMatrixD inverseCovMatrix(nHits, nHits); // carries inverse of varU in its diagonal elements
  TMatrixD X(nHits, 3); // carries mapped hits, column 0 = x variables, column 1 = y variables, col 2 = r2 variables
  TMatrixD onesC(nHits, 1); // column vector of ones
  TMatrixD onesR(1, nHits); // row vector of ones
  TMatrixD R2(nHits, 1); // column vector of radii^2
  TMatrixD zValues(nHits, 1); // column vector of z values
  TMatrixD invVarVvalues(nHits, 1); // column vector of radii^2

  int index = 0;
//  ofstream hitsFileStream;
//  hitsFileStream.open("hitHelixFit.data", std::ios_base::trunc); // trunc=overwrite app=append
  TVector3 seedHit = (*hits).at(0)->hitPosition;
  TVector3 secondHit = (*hits).at(1)->hitPosition; // need this one for definition of direction of flight for the particle
  if (useBackwards == false) { seedHit = (*hits).at(nHits - 1)->hitPosition; secondHit = (*hits).at(nHits - 2)->hitPosition; } // want innermost hit
//  B2ERROR(" useBackwards == " << useBackwards << ", seedHit.Mag()/secondHit.Mag(): " << seedHit.Mag()<<"/"<< secondHit.Mag())

  for (PositionInfo * hit : *hits) { // column vectors now
    x = hit->hitPosition.X();
    y = hit->hitPosition.Y();
    z = hit->hitPosition.Z();
    varU = hit->sigmaX;
    varV = hit->sigmaY;
    invVarVvalues(index, 0) = 1. / varV;
//    B2ERROR(" hit.X(): " << hit->hitPosition.X() << ", hit.Y(): " << hit->hitPosition.Y() << ", hit.Z(): " << hit->hitPosition.Z())

//    hitsFileStream << setprecision(14) << x << " " << y << " " << z << " " << varU << " " << varV << endl;
///   phi = atan2(y , x);  // not used yet, but will be needed for some calculations which are not implemented yet
    r2 = x * x + y * y;
///     rPhi = phi*sqrt(r2); // not used yet, but will be needed for some calculations which are not implemented yet
    inverseVarianceU = 1. / varU;
    sumWeights += inverseVarianceU;
    inverseCovMatrix(index, index) = inverseVarianceU;
    R2(index, 0) = r2;
    X(index, 0) = x;
    X(index, 1) = y;
    zValues(index, 0) = z;
    X(index, 2) = r2;
    onesC(index, 0) = 1;
    onesR(0, index) = 1;

    ++index;
  }

//  hitsFileStream.close();
//  B2INFO("inverseCovMatrix nRows: " << inverseCovMatrix.GetNrows() << ", nCols: " << inverseCovMatrix.GetNcols() )
//  B2INFO("X nRows: " << X.GetNrows() << ", nCols: " << X.GetNcols() )
//  B2INFO("onesC nRows: " << onesC.GetNrows() << ", nCols: " << onesC.GetNcols() )
//  B2INFO("onesR nRows: " << onesR.GetNrows() << ", nCols: " << onesR.GetNcols() )


  /// transform to paraboloid:
  double inverseSumWeights = 1. / sumWeights;
//  B2INFO("inverseSumWeights: " << inverseSumWeights )
  TMatrixD xBar = onesR * inverseCovMatrix * X * inverseSumWeights; // weighed sample mean values

//  for (int i = 0; i < xBar.GetNcols(); ++i) { B2WARNING("xBar Rows: " <<  i << ", value: " << xBar(0,i) ) }
//  TMatrixD xBar = onesR;
//  B2WARNING("xBar = onesR nRows: " << xBar.GetNrows() << ", nCols: " << xBar.GetNcols() )
//  xBar *= inverseCovMatrix;
//  B2WARNING("xBar *= inverseCovMatrix nRows: " << xBar.GetNrows() << ", nCols: " << xBar.GetNcols() )
//  xBar *= X;
//  B2WARNING("xBar *= X nRows: " << xBar.GetNrows() << ", nCols: " << xBar.GetNcols() )
//  xBar *= inverseSumWeights;
//  B2WARNING("xBar = onesRow * inverseCovMatrix * X * inverseSumWeights")
//  B2WARNING("xBar nRows: " << xBar.GetNrows() << ", nCols: " << xBar.GetNcols() )
  TMatrixD transX = X;
  TMatrixD transxBar = xBar;
  transX.Transpose(transX);
  transxBar.Transpose(transxBar);
//  B2WARNING("transX nRows: " << transX.GetNrows() << ", nCols: " << transX.GetNcols() )
//  B2WARNING("transxBar nRows: " << transxBar.GetNrows() << ", nCols: " << transxBar.GetNcols() )
//  TMatrixD Vx_temp1 = transX*inverseCovMatrix;// - transxBar * xBar * sumWeights; /// X'*W*X-xbar'*xbar*sum(wgtu);
//  B2WARNING("Vx_temp1 nRows: " << Vx_temp1.GetNrows() << ", nCols: " << Vx_temp1.GetNcols() )
//  B2WARNING("X nRows: " << X.GetNrows() << ", nCols: " << X.GetNcols() )
//  TMatrixD Vx_temp2 = Vx_temp1 * X;
  TMatrixD weighedSampleCovMatrix = transX * inverseCovMatrix * X - transxBar * xBar * sumWeights;
//  B2WARNING("weighedSampleCovMatrix = X' * inverseCovMatrix * X - xBar' * xBar * sumWeights")
//  B2WARNING("weighedSampleCovMatrix nRows: " << weighedSampleCovMatrix.GetNrows() << ", nCols: " << weighedSampleCovMatrix.GetNcols() )

  /// find eigenvector to smallest eigenvalue
  TMatrixDEigen eigenCollection(weighedSampleCovMatrix);
  TMatrixD eigenValues = eigenCollection.GetEigenValues();
  TMatrixD eigenVectors = eigenCollection.GetEigenVectors();
//  B2WARNING("eigenValues nRows: " << eigenValues.GetNrows() << ", nCols: " << eigenValues.GetNcols() )
//  for (int i = 0; i < eigenValues.GetNcols(); ++i) { B2WARNING("eigenValues Rows: " <<  i << ", value: " << eigenValues(i,i) ) }
//  B2WARNING("eigenVectors nRows: " << eigenVectors.GetNrows() << ", nCols: " << eigenVectors.GetNcols() )

  double minValue = std::numeric_limits<double>::max();
  int minValueIndex = -1;
  int nEVs = eigenValues.GetNcols();
  for (int i = 0; i < nEVs; ++i) {
    if (eigenValues(i, i) < minValue) {
      minValue = eigenValues(i, i);
      minValueIndex = i;
    }
  }
  if (minValueIndex < 0) { B2FATAL("TrackletFilters::helixFit produced no eigenValue smaller than " << minValue << "!")}
//  B2WARNING("chosenMinValue: " << minValue << ", index: " << minValueIndex )
//  TMatrixDColumn eigenVector(eigenVectors,minValueIndex);

  double distanceOfPlane = 0;
  for (int i = 0; i < nEVs; ++i) {
    distanceOfPlane += eigenVectors(i, minValueIndex) * xBar(0, i); // eigenVectors(:,minValueIndex) = normal vector of the fitted plane (the normalized eigenvector of the smalles eigenValue of weighedSampleCovMatrix)
  }
  distanceOfPlane *= -1.;
//  B2WARNING("distanceOfPlane: " << distanceOfPlane)
  double n1 = eigenVectors(0, minValueIndex), n2 = eigenVectors(1, minValueIndex), n3 = eigenVectors(2, minValueIndex);
//  B2WARNING("circle: n1: " << n1 << ", n2: " << n2 << ", n3: " << n3 )
  double a = 1. / (2.*n3); // temporary value
  double xc = -n1 * a; // x coordinate of the origin of the circle
  double yc = -n2 * a; // y coordinate of the origin of the circle
//  double rho2=(1.-n3*n3-4.*distanceOfPlane*n3)*(a*a);
  double rho = sqrt((1. - n3 * n3 - 4.*distanceOfPlane * n3) * (a * a)); // radius of the circle
//  B2WARNING("circle: origin x: " << xc << ", y: " << yc << ", radius: " << rho  << endl)
//  double h=distanceOfPlane+R2*n3; // temporary value

  /// line fit:
//  B2WARNING("R2 nRows: " << R2.GetNrows() << ", nCols: " << R2.GetNcols() )
  TMatrixD H = distanceOfPlane + R2 * n3; // temporary value
//  B2WARNING("H nRows: " << H.GetNrows() << ", nCols: " << H.GetNcols() )
//  for (int i = 0; i < H.GetNrows(); ++i) { B2WARNING("H Rows: " <<  i << ", value: " << H(i,0) ) }
  TMatrixD H2 = H;
  for (int i = 0; i < H2.GetNrows(); ++i) { H2(i, 0) *= H2(i, 0); }
//  B2WARNING("H2 nRows: " << H2.GetNrows() << ", nCols: " << H2.GetNcols() )
  double b = n1 * n1 + n2 * n2; // temporary value
  TMatrixD T = b * R2 - H2; // temporary value
  b = 1. / b;

  for (int i = 0; i < T.GetNrows(); ++i) { T(i, 0) = sqrt(T(i, 0)); }
//  B2WARNING("T nRows: " << T.GetNrows() << ", nCols: " << T.GetNcols() )
//  for (int i = 0; i < T.GetNrows(); ++i) { B2WARNING("T Rows: " <<  i << ", value: " << T(i,0) ) }

  TMatrixD x1 = (-n1 * H + n2 * T) * b;

//  B2WARNING("x1 nRows: " << x1.GetNrows() << ", nCols: " << x1.GetNcols() )
//  for (int i = 0; i < x1.GetNrows(); ++i) { B2WARNING("x1 Rows: " <<  i << ", value: " << x1(i,0) ) }
  TMatrixD y1 = (-n2 * H - n1 * T) * b;
//  B2WARNING("y1 nRows: " << y1.GetNrows() << ", nCols: " << y1.GetNcols() )
//  for (int i = 0; i < y1.GetNrows(); ++i) { B2WARNING("y1 Rows: " <<  i << ", value: " << y1(i,0) ) }
  TMatrixD x2 = (-n1 * H - n2 * T) * b;
//  B2WARNING("x2 nRows: " << x2.GetNrows() << ", nCols: " << x2.GetNcols() )
//  for (int i = 0; i < x2.GetNrows(); ++i) { B2WARNING("x2 Rows: " <<  i << ", value: " << x2(i,0) ) }
  TMatrixD y2 = (-n2 * H + n1 * T) * b;
//  B2WARNING("y2 nRows: " << y2.GetNrows() << ", nCols: " << y2.GetNcols() )
//  for (int i = 0; i < y2.GetNrows(); ++i) { B2WARNING("y2 Rows: " <<  i << ", value: " << y2(i,0) ) }

  double dx1 = 0; // highest deviation of x1 value from estimated line
  double temp = 0;
  for (int i = 0; i < x1.GetNrows(); ++i) {
    temp = abs(x1(i, 0) - X(i, 0));
    if (temp > dx1) { dx1 = temp; }
  }
  double dx2 = 0; // highest deviation of x2 value from estimated line
  for (int i = 0; i < x2.GetNrows(); ++i) {
    temp = abs(x2(i, 0) - X(i, 0));
    if (temp > dx2) { dx2 = temp; }
  }
//  B2WARNING("dx1: " << dx1 << ", dx2: " << dx2)

  double dy1 = 0; // highest deviation of x1 value from estimated line
  temp = 0;
  for (int i = 0; i < y1.GetNrows(); ++i) {
    temp = abs(y1(i, 0) - X(i, 1));
    if (temp > dy1) { dy1 = temp; }
  }
  double dy2 = 0; // highest deviation of x2 value from estimated line
  for (int i = 0; i < y2.GetNrows(); ++i) {
    temp = abs(y2(i, 0) - X(i, 1));
    if (temp > dy2) { dy2 = temp; }
  }
//  B2WARNING("dy1: " << dy1 << ", dy2: " << dy2)

  TMatrixD xs(nHits, 1), ys(nHits, 1);
//  B2WARNING(" sum d1: " << dx1+dy1 << ", d2: " << dx2+dy2)
  if ((dx1 + dy1) < (dx2 + dy2)) { xs = x1; ys = y1; } else { xs = x2; ys = y2; }

  /// radius vectors
  double radiusX = xs(0, 0) - xc;
  double radiusY = ys(0, 0) - yc;
  double radiusMag = sqrt(radiusX * radiusX + radiusY * radiusY);

  TMatrixD s(nHits, 1); // length of arc
  s(0, 0) = 0;
  for (int i = 1; i < nHits; ++i) {
    double radiusXb = xs(i, 0) - xc;
    double radiusYb = ys(i, 0) - yc;
    double radiusMagb = sqrt(radiusXb * radiusXb + radiusYb * radiusYb);
    s(i, 0) = rho * acos(((radiusX * radiusXb + radiusY * radiusYb) / radiusMag) / radiusMagb);
//    B2WARNING("s Rows: " <<  i << ", value: " << s(i,0) )
  }

  /// fit line s versus z

  TMatrixD AtGA(2, 2);
  TMatrixD AtG(2, nHits);
  double sumWi = 0, sumWiSi = 0, sumWiSi2 = 0, sw = 0;
  for (int i = 0; i < nHits; ++i) {
    sumWi += invVarVvalues(i, 0);
    sw = invVarVvalues(i, 0) * s(i, 0);
    sumWiSi += sw;
    sumWiSi2 += invVarVvalues(i, 0) * s(i, 0) * s(i, 0);
    AtG(0, i) = invVarVvalues(i, 0);
    AtG(1, i) = sw;
  }
  AtGA(0, 0) = sumWi;
  AtGA(0, 1) = sumWiSi;
  AtGA(1, 0) = sumWiSi;
  AtGA(1, 1) = sumWiSi2;
  TMatrixD AtGAInv = AtGA;
  AtGAInv.Invert();

  TMatrixD p = AtGAInv * AtG * zValues; // gefittetes z im ersten Punkt, tan(lambda)
//  for (int i = 0; i < 2; ++i) {
//    B2WARNING("p Rows: " <<  i << ", value: " << p(i,0) )
//  }
  double thetaVal = M_PI * 0.5 - atan(p(1, 0)); // WARNING: was M_PI*0.5 - atan(p(1,0)), but values were wrong! double-WARNING: but + atan was wrong too!
//  B2WARNING("thetaVal: " << thetaVal)

  /// calc direction:
//  TVector3 radialVector = (center - seedHit);
// double radiusInCm = radialVector.Perp();
// double pT = m_3hitFilterBox.calcPt(radiusInCm);
// TVector3 pTVector = (pT / radiusInCm) * radialVector.Orthogonal();
// pZ = pT / tan(theta);
// TVector3 pVector = pTVector;
//     TVector3 pVector.SetZ(pZ);

  TVector3 radialVector(xc, yc, 0.); // here it is the center of the circle
  radialVector = radialVector - seedHit; // now it's the radialVector
  radialVector.SetZ(0.);
  double pT = m_3hitFilterBox.calcPt(rho);
  TVector3 pVector = pT  * (radialVector.Orthogonal()).Unit(); // now it is the pT-Vector, therefore without pZ information
  double pZ = pT / tan(thetaVal);
  TVector3 vectorToSecondHit = secondHit - seedHit;
  vectorToSecondHit.SetZ(0);
  if (((useBackwards == true) && (vectorToSecondHit.Angle(pVector) < M_PI * 0.5)) || ((useBackwards == false) && (vectorToSecondHit.Angle(pVector) > M_PI * 0.5))) { pVector *= -1.; }
//   B2WARNING("radius: " << rho << ", theta: " << thetaVal << " ");
//  if ( vectorToSecondHit.Angle(pVector) > M_PI*0.5 ) { pVector *= -1.; }
  pVector.SetZ(pZ); // now that track carries full momentum
//    B2ERROR("again: useBackwards == " << useBackwards << ", seedHit.Mag()/secondHit.Mag(): " << seedHit.Mag()<<"/"<< secondHit.Mag())

//  B2WARNING("Circle: origin x: " << xc << ", y: " << yc << ", radius: " << rho << ", pT: " << pT << ", thetaVal: " << thetaVal << ", p(1,0): " << p(1,0)<< ", pVector: " << pVector.Mag())


//  double xHit = (*hits)[0]->hitPosition.X();
//  double yHit = (*hits)[0]->hitPosition.Y();
//
//  double vecCircleX = xHit - xc;
//  double vecCircleY = yHit - yc;
//  double tangCircleX =  vecCircleY;
//  double tangCircleY = -vecCircleX;
// //   B2WARNING(" hit: x,y: " << xHit<< "," <<yHit << ", circleCenter x,y: " << xc<< "," << yc ", mag hit+tangentenVector: " << sqrt((tangCircleX+xHit)*(tangCircleX+xHit) + (tangCircleY+yHit)*(tangCircleY+yHit)))
//  if ( sqrt((tangCircleX+xHit)*(tangCircleX+xHit) + (tangCircleY+yHit)*(tangCircleY+yHit)) < sqrt(xHit*xHit + yHit*yHit)) {
//    tangCircleX *= -1.;
//    tangCircleY *= -1.;
//  }
//
//  double magTangent = sqrt(tangCircleX*tangCircleX + tangCircleY*tangCircleY);
//  tangCircleX /= magTangent;
//  tangCircleY /= magTangent;
//  B2WARNING(" hit: x,y: " << xHit<< "," <<yHit << ", circleCenter x,y: " << xc<< "," << yc ", mag hit+tangentenVector: " << sqrt((tangCircleX+xHit)*(tangCircleX+xHit) + (tangCircleY+yHit)*(tangCircleY+yHit)))

//    TMatrixD Rs(nHits,1); //
//  for (int i = 1; i < nHits; ++i) {
//    Rs(i,0) = atan2(ys(i,0),xs(i,0));
//    B2WARNING("Rs Rows: " <<  i << ", value: " << Rs(i,0) )
//  }
//
//  TMatrixD Phis(nHits,1); //
//  for (int i = 1; i < nHits; ++i) {
//    Phis(i,0) = sqrt(ys(i,0)*ys(i,0)+xs(i,0)*xs(i,0));
//    B2WARNING("Phis Rows: " <<  i << ", value: " << Rs(i,0) )
//  }

  /// chi2 of circle fit
// Phis=atan2(ys,xs);
// Rs=sqrt(xs.^2+ys.^2);
// RPhis=Rs.*Phis;
// res=(RPhi-RPhis);
// chi2=dot(res.^2,wgtu);
  /// fit line s versus z
// A=[ones(n,1) s(:)];
// G=1./varz';
// AtG=A'.*repmat(G,2,1);
// par=inv(AtG*A)*AtG*z;
// theta=pi/2-atan(par(2));
// zfit=A*par;
  /// res, resZ = residuals
// resz=z-zfit;
// chi2=chi2+dot(resz.^2,G);
//  rv2=[xs(i)-xc; ys(i)-yc];
//  phi=acos(dot(rv1,rv2)/norm(rv1)/norm(rv2));
//  s(i)=rho*phi;
// end

  //function [xc,yc,rho,theta,chi2]=helixfit(x,y,z,varu,varz)
//event0
// event=[1.0045598807718539 0.98064240740937836 0.89077305908203108 2.0833333333333334e-06 2.520833306014538e-06
// 1.6478037593653985 1.5384201678761289 1.4337223846435547 2.0833333333333334e-06 4.0833324988683497e-06
// 2.9125750550834417 2.5197562332487138 2.434219932556152 2.0963745096426766e-06 2.0351833840888175e-05
// 7.0478994508509487 4.787722223489471 5.4111187877655036 4.707051327516743e-06 4.8015625973494028e-05
// 8.9823161930530837 5.4741719839834033 6.7053809738159185 4.707051327516743e-06 4.8015625973494028e-05
// 12.078509159296951 6.1589094409158482 8.6692161655426041 4.707051327516743e-06 4.8015625973494028e-05];
// // convert to column vectors
// x=event(:,1);y=event(:,2);z=event(:,3);varu=event(:,4);varz=event(:,5);n=length(x);
// Phi=atan2(y,x);
// R=sqrt(x.^2+y.^2);
// RPhi=R.*Phi;
// // transform to paraboloid
// R2=R.^2;
// w=R2;
// X=[x y w];
// wgtu=1./varu; // weights, inverse variances
// W=diag(wgtu);
// eins=ones(n,1);
// xbar=eins'*W*X/sum(wgtu);
// VX=X'*W*X-xbar'*xbar*sum(wgtu); // covariance matrix of mean
// // find eigenvector to smallest eigenvalue
// [U,D]=eig(VX); // U unitäre Matrix deren Spalten die Eigenvektoren sind. D diagonalmatrix, deren diagonalelemente die eigenwerte sind
// lam=diag(D);
// [minlam,imin]=min(lam);
// nv=U(:,imin);
// c=-dot(xbar,nv);
// n1=nv(1);
// n2=nv(2);
// n3=nv(3);
// a=2*n3;
// xc=-n1/a;
// yc=-n2/a;
// rho2=(1-n3^2-4*c*n3)/a^2;
// rho=sqrt(rho2);
// h=c+R2*n3;
// t=sqrt((n1^2+n2^2)*R2-h.^2);
// x1=(-n1*h+n2*t)/(n1^2+n2^2);
// y1=(-n2*h-n1*t)/(n1^2+n2^2);
// x2=(-n1*h-n2*t)/(n1^2+n2^2);
// y2=(-n2*h+n1*t)/(n1^2+n2^2);
// dx1=max(abs(x1-x));
// dx2=max(abs(x2-x));
// imin=1;
// if dx2<dx1,imin=2;end
// if imin==1
//  xs=x1;
//  ys=y1;
// else
//  xs=x2;
//  ys=y2;
// end
// // radius vectors
// rv1=[xs(1)-xc; ys(1)-yc]
// s(1)=0;
// for i=2:n
//  rv2=[xs(i)-xc; ys(i)-yc];
//  phi=acos(dot(rv1,rv2)/norm(rv1)/norm(rv2));
//  s(i)=rho*phi;
// end
// // chi2 of circle fit
// Phis=atan2(ys,xs);
// Rs=sqrt(xs.^2+ys.^2);
// RPhis=Rs.*Phis;
// res=(RPhi-RPhis);
// chi2=dot(res.^2,wgtu);
// // fit line s versus z
// A=[ones(n,1) s(:)];
// G=1./varz';
// AtG=A'.*repmat(G,2,1);
// par=inv(AtG*A)*AtG*z;
// theta=pi/2-atan(par(2));
// zfit=A*par;
  // res, resZ = residuals
// resz=z-zfit;
// chi2=chi2+dot(resz.^2,G);
// return

  return make_pair(rho, pVector);
}


/*
TVector3 radialVector = (center - seedHit);
double radiusInCm = radialVector.Perp();
double pT = m_3hitFilterBox.calcPt(radiusInCm);
TVector3 pTVector = (pT / radiusInCm) * radialVector.Orthogonal();

    if (aTC->getCondition() == false) { continue; }
    const vector<VXDTFHit*>& currentHits = aTC->getHits();
    numOfCurrentHits = currentHits.size();

    if (numOfCurrentHits < 3) {
      B2ERROR("calcInitialValues4TCs: currentTC got " << numOfCurrentHits << " hits! At this point only tcs having at least 3 hits should exist!")
    }

    TVector3 hitA, hitB, hitC;
  TVector3 hitA_T, hitB_T, hitC_T; // those with _T are the hits of the transverlal plane
  TVector3 intersection, radialVector, pTVector, pVector; //coords of center of projected circle of trajectory & vector pointing from center to innermost hit
  ThreeHitFilters threeHitFilterBox = ThreeHitFilters();
  TVector3 segAB, segBC, segAC, cpAB, cpBC, nAB, nBC;
  int numOfCurrentHits, signCurvature, pdGCode;
  double radiusInCm, pT, theta, pZ, preFactor; // needed for dPt calculation

    hitA_T = hitA; hitA_T.SetZ(0.);
    hitB_T = hitB; hitB_T.SetZ(0.);
    hitC_T = hitC; hitC_T.SetZ(0.);
    segAB = hitB - hitA;
    segAC = hitC - hitA;
    theta = segAC.Theta();


    segAB.SetZ(0.);
    segBC = hitC_T - hitB_T;
    nBC = segBC.Orthogonal();

    signCurvature = sign(nBC * segAB);

    currentPass->threeHitFilterBox.calcCircleCenter(hitA_T, hitB_T, hitC_T, intersection);
    if (m_KFBackwardFilter == true) {
      TVector3 radialVector = (intersection - hitC);
    } else {
      TVector3 radialVector = (intersection - hitA);
    }

    radiusInCm = aTC->getEstRadius();
    if (radiusInCm  < 0.1 || radiusInCm > 100000.) { // if it is not set, value stays at zero, therefore small check should be enough
      radiusInCm = radialVector.Perp(); // = radius in [cm], sign here not needed. normally: signKappaAB/normAB1
    }

    pT = currentPass->threeHitFilterBox.calcPt(radiusInCm); // pT[GeV/c] = 0.3*B[T]*r[m] = 0.45*r[cm]/100 = 0.45*r*0.01 length of pT
    B2DEBUG(150, "event: " << m_eventCounter << ": calculated pT: " << pT);
    pZ = pT / tan(theta);
    preFactor = pT / radiusInCm;
    TVector3 pTVector = (pT / radiusInCm) * radialVector.Orthogonal() ;

    if (m_KFBackwardFilter == true) {
      if ((hitC + pTVector).Mag() < hitC.Mag()) { pTVector = pTVector * -1; }
    } else {
      if ((hitA + pTVector).Mag() < hitA.Mag()) { pTVector = pTVector * -1; }
    }

    TVector3 pVector = pTVector;
    TVector3 pVector.SetZ(pZ);

    // the sign of curvature determines the charge of the particle, negative sign for curvature means positively charged particle. The signFactor is needed since the sign of PDG-codes are not defined by their charge but by being a particle or an antiparticle

    pdGCode = signCurvature * m_PARAMpdGCode * m_chargeSignFactor;

    if (m_KFBackwardFilter == true) {
      aTC->setInitialValue(hitC, pVector, pdGCode);
    } else {
      aTC->setInitialValue(hitA, pVector, pdGCode);
    }

    B2DEBUG(10, " TC has got momentum/pT of " << pVector.Mag() << "/" << pTVector.Mag() << "GeV and estimated pdgCode " << pdGCode);*/
