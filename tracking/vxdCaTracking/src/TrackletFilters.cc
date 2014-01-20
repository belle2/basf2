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
#include <math.h>       // isnan, pow 
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


std::pair<TVector3, int> TrackletFilters::calcMomentumSeed(bool useBackwards, double setMomentumMagnitude)
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

  std::pair<double, TVector3> helixFitValues = helixFit(m_hits, useBackwards, setMomentumMagnitude);
  B2DEBUG(10, "calcMomentumSeed: return values of helixFit: first(radius): " << helixFitValues.first << ", second.Mag(): " << helixFitValues.second.Mag())
  int sign = boost::math::sign(hitC * hitB); // sign of curvature: is > 0 if angle between vectors is < 90°, < 0 else (rule of scalar product)
  if (sign == 0) { B2ERROR("trackletFilter::calcMomentumSeed: segments orthogonal! "); sign = 1;}
  return make_pair(helixFitValues.second, sign); //.first: momentum vector. .second: sign of curvature
}


// clap = closest approach of fitted circle to origin
double TrackletFilters::circleFit(double& clapPhi, double& clapR, double& radius)
{
  if (m_hits == NULL) { B2FATAL(" TrackletFilters::circleFit hits not set, therefore no calculation possible - please check that!") }
  double stopper = 0.000000001; /// WARNING hardcoded values!
  double meanX = 0, meanY = 0, meanX2 = 0, meanY2 = 0, meanR2 = 0, meanR4 = 0, meanXR2 = 0, meanYR2 = 0, meanXY = 0; //mean values
  double r2 = 0, x = 0, y = 0, x2 = 0, y2 = 0; // coords
  double weight;// weight of each hit, so far no difference in hit quality
  double sumWeights = 0, divisor, weightNormalizer = 0; // sumWeights is sum of weights, divisor is 1/sumWeights;
  double tuningParameter = 1.; //0.02; // this parameter is for internal tuning of the weights, since at the moment, the error seams highly overestimated at the moment. 1 means no influence of parameter.

  // looping over all hits and do the division afterwards
  for (PositionInfo * hit : *m_hits) {
    weight = 1. / ((hit->sigmaV) * (hit->sigmaV) * tuningParameter);
    B2DEBUG(100, " current hitSigma: " << hit->sigmaU << ", weight: " << weight)
    sumWeights += weight;
    if (hit->sigmaV < stopper) B2FATAL("TrackletFilters::circleFit, chosen sigma is too small (is/threshold: " << hit->sigmaV << "/" << stopper << ")")
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



std::pair<double, TVector3> TrackletFilters::helixFit(const std::vector<PositionInfo*>* hits, bool useBackwards, double setMomentumMagnitude)
{
  if (hits == NULL) { B2FATAL(" TrackletFilters::circleFit hits not set, therefore no calculation possible - please check that!") }

  int nHits = hits->size();

  if (nHits < 3) { B2ERROR(" TrackletFilters::circleFit number of hits too low: " << nHits << " hits, therefore no useful calculation possible - please check that!") }


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
          varV = 0, // variance of V = xy-plane
///         phi = 0, // angle phi  // not used yet, but will be needed for some calculations which are not implemented yet
          r2 = 0, // radius^2
//          tempRadius = 0,
//          r = 0,
///         rPhi = 0,  // not used yet, but will be needed for some calculations which are not implemented yet
          sumWeights = 0, // the sum of the weights V
          inverseVarianceV = 0; // current inverse of variance V

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
    varU = hit->sigmaU;
    varV = hit->sigmaV;
    invVarVvalues(index, 0) = 1. / varV;
    B2DEBUG(10, "helixFit: hit.X(): " << hit->hitPosition.X() << ", hit.Y(): " << hit->hitPosition.Y() << ", hit.Z(): " << hit->hitPosition.Z() << ", hit.sigmaU: " << varU << ", hit.sigmaV: " << varV)

//    hitsFileStream << setprecision(14) << x << " " << y << " " << z << " " << varU << " " << varV << endl;
///   phi = atan2(y , x);  // not used yet, but will be needed for some calculations which are not implemented yet
    r2 = x * x + y * y;
///     rPhi = phi*sqrt(r2); // not used yet, but will be needed for some calculations which are not implemented yet
    inverseVarianceV = 1. / varV; // v carries xy-info, u is for z
    sumWeights += inverseVarianceV;
    inverseCovMatrix(index, index) = inverseVarianceV;
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

  bool didNanAppear = false; // if nan appeared at least once, -> didNanAppear = true;
  /** local lambda-function used for checking TMatrixDs, whether there are nan values included, returns true, if there are */
  auto lambdaCheckMatrix4NAN = [](TMatrixD & aMatrix) -> bool { /// testing c++11 lambda functions...
    double totalEntries = 0;
    for (int i = 0; i < aMatrix.GetNrows(); ++i) {
      for (int j = 0; j < aMatrix.GetNcols(); ++j) {
        totalEntries += aMatrix(i, j);
      }
    }
    return std::isnan(totalEntries);
  }; // should be converted to normal function, since feature could be used much more often...
// TEST for lambdaCheckMatrix4NAN:
//   TMatrixD lambdaTestMatrix(nHits, 1);
//   for (int i = 0; i < nHits; ++i) { lambdaTestMatrix(i, 0) = 1.; }
//   B2DEBUG(175, "lambdaCheckMatrix4NAN(lambdaTestMatrix): " << lambdaCheckMatrix4NAN(lambdaTestMatrix) << " (should be 0)")
//   lambdaTestMatrix(nHits - 2, 0) = sqrt(-1); // producing 'nan'
//   B2DEBUG(175, "lambdaCheckMatrix4NAN(lambdaTestMatrix): " << lambdaCheckMatrix4NAN(lambdaTestMatrix) << " (should be 1)")

  if (lambdaCheckMatrix4NAN(inverseCovMatrix) == true) { B2DEBUG(1, "helixFit: inverseCovMatrix got 'nan'-entries!"); didNanAppear = true; }
  if (lambdaCheckMatrix4NAN(X) == true) { B2DEBUG(1, "helixFit: X got 'nan'-entries!"); didNanAppear = true; }


  /// transform to paraboloid:
  double inverseSumWeights = 1. / sumWeights;
  TMatrixD xBar = onesR * inverseCovMatrix * X * inverseSumWeights; // weighed sample mean values
  if (lambdaCheckMatrix4NAN(xBar) == true) { B2DEBUG(10, "helixFit: xBar got 'nan'-entries!"); didNanAppear = true; }


  TMatrixD transX = X;
  TMatrixD transxBar = xBar;
  transX.Transpose(transX);
  transxBar.Transpose(transxBar);

  TMatrixD weighedSampleCovMatrix = transX * inverseCovMatrix * X - transxBar * xBar * sumWeights;
  if (lambdaCheckMatrix4NAN(weighedSampleCovMatrix) == true) { B2DEBUG(1, "helixFit: weighedSampleCovMatrix got 'nan'-entries!"); didNanAppear = true; }


  /// find eigenvector to smallest eigenvalue
  TMatrixDEigen eigenCollection(weighedSampleCovMatrix);
  TMatrixD eigenValues = eigenCollection.GetEigenValues();
  if (lambdaCheckMatrix4NAN(eigenValues) == true) { B2DEBUG(1, "helixFit: eigenValues got 'nan'-entries!"); didNanAppear = true; }
  TMatrixD eigenVectors = eigenCollection.GetEigenVectors();
  if (lambdaCheckMatrix4NAN(eigenVectors) == true) { B2DEBUG(1, "helixFit: eigenVectors got 'nan'-entries!"); didNanAppear = true; }

  double minValue = std::numeric_limits<double>::max();
  int minValueIndex = -1;
  int nEVs = eigenValues.GetNcols();
  for (int i = 0; i < nEVs; ++i) {
    if (eigenValues(i, i) < minValue) {
      minValue = eigenValues(i, i);
      minValueIndex = i;
    }
  }
  if (minValueIndex < 0) { B2FATAL("TrackletFilters::helixFit produced eigenValue smaller than 0 (" << minValue << ")!")}

  double distanceOfPlane = 0;
  for (int i = 0; i < nEVs; ++i) {// calculating scalar product by hand
    distanceOfPlane += eigenVectors(i, minValueIndex) * xBar(0, i); // eigenVectors(:,minValueIndex) = normal vector of the fitted plane (the normalized eigenvector of the smalles eigenValue of weighedSampleCovMatrix)
//     distanceOfPlane += xBar(0, i) * eigenVectors(i, minValueIndex); // eigenVectors(:,minValueIndex) = normal vector of the fitted plane (the normalized eigenvector of the smalles eigenValue of weighedSampleCovMatrix)
  }
  distanceOfPlane *= -1.;

  double n1 = eigenVectors(0, minValueIndex),
         n2 = eigenVectors(1, minValueIndex),
         n3 = eigenVectors(2, minValueIndex);

//  double aInv = ;
  double a = 1. / (2.*n3); // temporary value

  double xc = -n1 * a; // x coordinate of the origin of the circle
  double yc = -n2 * a; // y coordinate of the origin of the circle
//  double rho2=(1.-n3*n3-4.*distanceOfPlane*n3)*(a*a);

  double rho = sqrt((1. - n3 * n3 - 4.*distanceOfPlane * n3) * (a * a)); // radius of the circle
/// fix dec8,2013:
//  double rho = sqrt((1. - n3 * n3 - 4.*distanceOfPlane * n3) * aInv * aInv); // radius of the circle

  B2DEBUG(10, "helixFit: circle: origin x: " << xc << ", y: " << yc << ", radius: " << rho  << endl)


  /// line fit:
  TMatrixD H = distanceOfPlane + R2 * n3; // temporary value
  if (lambdaCheckMatrix4NAN(H) == true) { B2DEBUG(1, "helixFit: H got 'nan'-entries!"); didNanAppear = true; }

  TMatrixD H2 = H;
  H2.Sqr(); // squares each element
//   for (int i = 0; i < H2.GetNrows(); ++i) { H2(i, 0) *= H2(i, 0); }
  if (lambdaCheckMatrix4NAN(H2) == true) { B2DEBUG(10, "helixFit: H2 got 'nan'-entries!"); didNanAppear = true; }

  double b = n1 * n1 + n2 * n2; // temporary value

  TMatrixD T2 = b * R2 - H2; // temporary value T2 = vector, since b = scalar * vector R2 - Vector H2
  B2DEBUG(10, "helixFit: T.min: " << T2.Min() << ", T.max: " << T2.Max() << ", R2.min: " << R2.Min() << ", R2.max: " << R2.Max() << ", H2.min: " << H2.Min() << ", H2.max: " << H2.Max() << ", H.min: " << H.Min() << ", H.max: " << H.Max() << ", b: " << b)

  if (lambdaCheckMatrix4NAN(T2) == true) {B2DEBUG(1, "helixFit: T got 'nan'-entries!"); didNanAppear = true; }

  TMatrixD T = T2;
  T.Sqrt(); // take square root of all elements
//   for (int i = 0; i < T.GetNrows(); ++i) { T(i, 0) = sqrt(T(i, 0)); }
  if (lambdaCheckMatrix4NAN(T) == true) { B2DEBUG(1, "helixFit: T got 'nan'-entries after Sqrt! Before: T.min: " << T2.Min() << ", T.max: " << T2.Max() << ", after: T.min: " << T.Min() << ", T.max: " << T.Max()); didNanAppear = true; }

  b = 1. / b;


  TMatrixD x1 = (-n1 * H + n2 * T) * b;
  if (lambdaCheckMatrix4NAN(x1) == true) { B2DEBUG(10, " x1 has got 'nan'-values! n1 " << n1 << ", n2 " << n2 << " b " << b); didNanAppear = true;}

  TMatrixD y1 = (-n2 * H - n1 * T) * b;
  if (lambdaCheckMatrix4NAN(y1) == true) { B2DEBUG(10, " y1 has got 'nan'-values! n1 " << n1 << ", n2 " << n2 << " b " << b); didNanAppear = true;}

  TMatrixD x2 = (-n1 * H - n2 * T) * b;
  if (lambdaCheckMatrix4NAN(x2) == true) { B2DEBUG(10, " x2 has got 'nan'-values! n1 " << n1 << ", n2 " << n2 << " b " << b); didNanAppear = true;}

  TMatrixD y2 = (-n2 * H + n1 * T) * b;
  if (lambdaCheckMatrix4NAN(y2) == true) { B2DEBUG(10, " y2 has got 'nan'-values! n1 " << n1 << ", n2 " << n2 << " b " << b); didNanAppear = true;}


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

  double dy1 = 0; // highest deviation of y1 value from estimated line
  temp = 0;
  for (int i = 0; i < y1.GetNrows(); ++i) {
    temp = abs(y1(i, 0) - X(i, 1));
    if (temp > dy1) { dy1 = temp; }
  }
  double dy2 = 0; // highest deviation of y2 value from estimated line
  for (int i = 0; i < y2.GetNrows(); ++i) {
    temp = abs(y2(i, 0) - X(i, 1));
    if (temp > dy2) { dy2 = temp; }
  }

  TMatrixD xs(nHits, 1), ys(nHits, 1);
  B2DEBUG(10, " sum d1: " << dx1 + dy1 << ", d2: " << dx2 + dy2)

  if ((dx1 + dy1) < (dx2 + dy2)) {
    if (lambdaCheckMatrix4NAN(x1) == true or lambdaCheckMatrix4NAN(y1) == true) {
      xs = x2; ys = y2;
      if (lambdaCheckMatrix4NAN(x2) == true or lambdaCheckMatrix4NAN(y2) == true) {
        B2DEBUG(10, "there is 'nan' = " << lambdaCheckMatrix4NAN(x1) << "/" << lambdaCheckMatrix4NAN(y1) << "/" << lambdaCheckMatrix4NAN(x2) << "/" << lambdaCheckMatrix4NAN(y2) << " in x1/y1/x2/y2!")

      }
    } else  {
      xs = x1; ys = y1;
    }
  } else {
    if (lambdaCheckMatrix4NAN(x2) == true or lambdaCheckMatrix4NAN(y2) == true) {
      xs = x1; ys = y1;
      if (lambdaCheckMatrix4NAN(x1) == true or lambdaCheckMatrix4NAN(y1) == true) {
        B2DEBUG(10, "there is 'nan' = " << lambdaCheckMatrix4NAN(x1) << "/" << lambdaCheckMatrix4NAN(y1) << "/" << lambdaCheckMatrix4NAN(x2) << "/" << lambdaCheckMatrix4NAN(y2) << " in x1/y1/x2/y2!")
      }
    } else  {
      xs = x2; ys = y2;
    }
  }

  /// radius vectors
  double radiusX = xs(0, 0) - xc;
  double radiusY = ys(0, 0) - yc;
  double radiusMag = sqrt(radiusX * radiusX + radiusY * radiusY);
  double invRadiusMag = 1. / radiusMag;

  TMatrixD s(nHits, 1); // length of arc
  s(0, 0) = 0;
  for (int i = 1; i < nHits; ++i) {
    double radiusXb = xs(i, 0) - xc;
    double radiusYb = ys(i, 0) - yc;
    double radiusMagb = sqrt(radiusXb * radiusXb + radiusYb * radiusYb);

    s(i, 0) = rho * acos(((radiusX * radiusXb + radiusY * radiusYb) / radiusMag) / radiusMagb); // version 1
//     s(i, 0) = rho * acos(((radiusX * radiusXb + radiusY * radiusYb) * invRadiusMag) / radiusMagb); // version 2
    if (std::isnan(s(i, 0)) == true) {
      didNanAppear = true;
      B2DEBUG(1, "helixFit: i: " << i << ", s(i) = 'nan', components - rho: " << rho << ", radiusX: " << radiusX << ", radiusY: " << radiusY << ", radiusXb: " << radiusXb << ", radiusYb: " << radiusYb << ", invRadiusMag: " << invRadiusMag << ", radiusMagb: " << radiusMagb << ", xs(i): " << xs(i, 0) << ", ys(i): " << ys(i, 0))
//      B2DEBUG(10,
    }
  }

//   if (didNanAppear == true) {
//     stringstream hitOutput;
//     int i = 0;
//     for (PositionInfo * hit : *m_hits) {
//       hitOutput << " hit " << i << ": x/y/sigmaU/sigmaV: " << hit->hitPosition.X() << "/" << hit->hitPosition.Y() << "/" << hit->sigmaU << "/" << hit->sigmaV << endl;
//      ++i;
//     }
//     B2WARNING("helixFit: there was a 'nan'-value detected. The following hits were part of this TC: \n" << hitOutput.str())
//   }

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
    B2DEBUG(10, "hit i: " <<  i << ", sumWi: " << sumWi << ", sw: " << sw << ", sumWiSi: " << sumWiSi << ", sumWiSi2: " << sumWiSi2 << ", s(i): " << s(i, 0) << ", invVarVvalues(i): " << invVarVvalues(i, 0))
  }
  AtGA(0, 0) = sumWi;
  AtGA(0, 1) = sumWiSi;
  AtGA(1, 0) = sumWiSi;
  AtGA(1, 1) = sumWiSi2;
  TMatrixD AtGAInv = AtGA;
  AtGAInv.Invert();
  if (lambdaCheckMatrix4NAN(AtGA) == true) {B2DEBUG(10, "helixFit: AtGA got 'nan'-entries!"); didNanAppear = true; }
  if (lambdaCheckMatrix4NAN(AtGAInv) == true) {B2DEBUG(10, "helixFit: AtGAInv got 'nan'-entries!"); didNanAppear = true; }
  if (lambdaCheckMatrix4NAN(zValues) == true) {B2DEBUG(10, "helixFit: zValues got 'nan'-entries!"); didNanAppear = true; }

  TMatrixD p = AtGAInv * AtG * zValues; // fitted z value in the first point, tan(lambda)
  if (lambdaCheckMatrix4NAN(p) == true) { B2DEBUG(10, "helixFit: p got 'nan'-entries!") }

  double thetaVal = M_PI * 0.5 - atan(p(1, 0)); // WARNING: was M_PI*0.5 - atan(p(1,0)), but values were wrong! double-WARNING: but + atan was wrong too!

  if (std::isnan(thetaVal) == true) {
    didNanAppear = true;
    thetaVal = (hits->at(nHits - 1)->hitPosition - hits->at(0)->hitPosition).Theta();
//    B2DEBUG(1,
    B2DEBUG(1, "helixFit: calculating theta for momentum produced 'nan' -> fallback-solution produces theta: " << thetaVal)
    if (std::isnan(thetaVal) == true) { B2ERROR("helixFit: no usable Theta value could be produced -> serious error telling us that helix fit does not work! bypass is setting the value to 0!"); thetaVal = 0; }
  }

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
  TVector3 pVector;
  if (setMomentumMagnitude == 0) {
    pVector = pT  * (radialVector.Orthogonal()).Unit(); // now it is the pT-Vector, therefore without pZ information
  } else { // means we want to set the magnitude of the momentum artificially
    pVector = setMomentumMagnitude * (radialVector.Orthogonal()).Unit(); // now it is the pT-Vector, therefore without pZ information
  }

  /** local lambda-function used for checking TVector3s, whether there are nan values included, returns true, if there are */
  auto lambdaCheckVector4NAN = [](TVector3 & aVector) -> bool { /// testing c++11 lambda functions...
    return std::isnan(aVector.Mag2()); // if one of them is 'nan', Mag2 will be 'nan' too
  }; // should be converted to normal function, since feature could be used much more often...
  if (lambdaCheckVector4NAN(pVector) == true) { B2ERROR("helixFit: pTVector got 'nan'-entries x/y/z: " << pVector.X() << "/" << pVector.Y() << "/" << pVector.Z()); didNanAppear = true; }

  double pZ = pT / tan(thetaVal);
  B2DEBUG(10, "helixFit: radius(rho): " << rho << ", theta: " << thetaVal << ", pT: " << pT << ", pZ: " << pZ << ", pVector.Perp: " << pVector.Perp() << ", pVector.Mag: " << pVector.Mag() << ", fitted zValue: " << p(1, 0))
  TVector3 vectorToSecondHit = secondHit - seedHit;
  vectorToSecondHit.SetZ(0);
  if (((useBackwards == true) && (vectorToSecondHit.Angle(pVector) < M_PI * 0.5)) || ((useBackwards == false) && (vectorToSecondHit.Angle(pVector) > M_PI * 0.5))) { pVector *= -1.; }
//  if ( vectorToSecondHit.Angle(pVector) > M_PI*0.5 ) { pVector *= -1.; }

  pVector.SetZ(pZ); // now that track carries full momentum
  if (lambdaCheckVector4NAN(pVector) == true) { B2ERROR("helixFit: pVector got 'nan'-entries x/y/z: " << pVector.X() << "/" << pVector.Y() << "/" << pVector.Z()); didNanAppear = true; }

  if (didNanAppear == true) {
    stringstream hitOutput;
    int i = 0;
    for (PositionInfo * hit : *m_hits) {
      hitOutput << " hit " << i << ": x/y/sigmaU/sigmaV: " << hit->hitPosition.X() << "/" << hit->hitPosition.Y() << "/" << hit->sigmaU << "/" << hit->sigmaV << endl;
      ++i;
    }
    B2DEBUG(1, "helixFit: there was a 'nan'-value detected. When using magnetic field of " << m_3hitFilterBox.getMagneticField() << ", the following hits were part of this TC: \n" << hitOutput.str() << "\n pVector  x/y/z: " << pVector.X() << "/" << pVector.Y() << "/" << pVector.Z())
  }

///   if ( pVector.Mag() > 14. or pVector.Mag() < -14. ) { /// DEBUG
//    stringstream hitOutput;
//     int i = 0;
//     for (PositionInfo * hit : *m_hits) {
//       hitOutput << " hit " << i << ": x/y/z/sigmaU/sigmaV: " << hit->hitPosition.X() << "/" << hit->hitPosition.Y() << "/" << hit->hitPosition.Z() << "/" << hit->sigmaU << "/" << hit->sigmaV << endl;
//      ++i;
//     }
//     B2WARNING("helixFit: strange pVector (Mag="<<pVector.Mag()<< ") detected. The following hits were part of this TC: \n" << hitOutput.str() << "\n pVector  x/y/z: " << pVector.X()<<"/"<< pVector.Y()<<"/"<< pVector.Z())
///   }
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
/// convert to column vectors
// x=event(:,1);y=event(:,2);z=event(:,3);varu=event(:,4);varz=event(:,5);n=length(x);
// Phi=atan2(y,x);
// R=sqrt(x.^2+y.^2);
// RPhi=R.*Phi;
/// transform to paraboloid
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
/// radius vectors
// rv1=[xs(1)-xc; ys(1)-yc]
// s(1)=0;
// for i=2:n
//  rv2=[xs(i)-xc; ys(i)-yc];
//  phi=acos(dot(rv1,rv2)/norm(rv1)/norm(rv2));
//  s(i)=rho*phi;
// end
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
  // res, resZ = residuals
// resz=z-zfit;
// chi2=chi2+dot(resz.^2,G);
// return

  return make_pair(rho, pVector);
}



pair<double, TVector3> TrackletFilters::simpleLineFit3D(const vector<PositionInfo*>* hits)
{
  /**
   * Coords:   Sensors:
   * ^        ./| ./| ./|
   * |   ^    | | | | | |
   * |Y /Z    | | | | | |
   * |/       |/  |/  |/
   * -------> X
   *
   * beam parallel to x. Measurement errors in y & z (v&u)
   * With these conditions, the following approach using 2 independent 2D line fits is acceptable (if rotation is the same for all sensors)
   * */
  double chi2 = 0;
  TVector3 directionVector;
  TVector3 meanVal; // xBar, yBar, zBar
  double SSxy = 0, SSxz = 0, SSyy = 0, SSzz = 0; // sum of squares -> SSxy = Sum(x*y)^2 - n*xBar*yBar
  double a = 0, b = 0, c = 0, d = 0; // parameters to be estimated (2 2D fits -> 4 parameters), x1 = a + b*y    x2 = d + c*z
  double chi2xy = 0, chi2xz = 0; // two fits mean two chi2-values, later they can be added
  int nHits = hits->size();

  for (const PositionInfo * aHit : *hits) {
    meanVal += aHit->hitPosition;
  }
  meanVal *= (1. / double(nHits));

  for (const PositionInfo * aHit : *hits) {
    SSxy += aHit->hitPosition.X() * aHit->hitPosition.Y();
  }
  for (const PositionInfo * aHit : *hits) {
    SSxz += aHit->hitPosition.X() * aHit->hitPosition.Z();
  }
  for (const PositionInfo * aHit : *hits) {
    SSyy += aHit->hitPosition.Y() * aHit->hitPosition.Y();
  }
  for (const PositionInfo * aHit : *hits) {
    SSzz += aHit->hitPosition.Z() * aHit->hitPosition.Z();
  }

  SSxy -= nHits * meanVal.X() * meanVal.Y();
  SSxz -= nHits * meanVal.X() * meanVal.Z();
  SSyy -= nHits * meanVal.Y() * meanVal.Y();
  SSzz -= nHits * meanVal.Z() * meanVal.Z();

  // now we use the values from above to calculate parameters for the two 2D-fits:
  b = SSxy / SSyy;
  c = SSxz / SSzz;
  a = meanVal.X() - b * meanVal.Y();
  d = meanVal.X() - d * meanVal.Z();

  // now calculating the chi2s independently...
  for (const PositionInfo * aHit : *hits) {
    chi2xy += (aHit->hitPosition.X() - a - b * aHit->hitPosition.Y()) * (aHit->hitPosition.X() - a - b * aHit->hitPosition.Y()) / aHit->sigmaU;
  }

  for (const PositionInfo * aHit : *hits) {
    chi2xz += (aHit->hitPosition.Z() - d - c * aHit->hitPosition.Z()) * (aHit->hitPosition.Z() - d - c * aHit->hitPosition.Z()) / aHit->sigmaV;
  }

  return make_pair(chi2, directionVector);
}


/*
 NOTE: old version for calculating momentum seed (maybe fallback?)
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
