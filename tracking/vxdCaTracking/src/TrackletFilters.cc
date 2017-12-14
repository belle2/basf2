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
#include "tracking/vxdCaTracking/TrackletFilters.h"
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
#include <TMatrixD.h>
#include <TMatrixDEigen.h>

// boost
#include <boost/math/special_functions/fpclassify.hpp> // abs
#include <boost/math/special_functions/sign.hpp> // sign

// Vc
// #include <Vc/Vc>



using namespace std;
using namespace Belle2;
using boost::math::sign;



bool TrackletFilters::ziggZaggXY()
{
  if (m_hits == NULL) B2FATAL(" TrackletFilters::ziggZaggXY hits not set, therefore no calculation possible - please check that!");
  list<int> chargeSigns;
  bool isZiggZagging = false; // good: not ziggZagging
  for (int i = 0; i < m_numHits - 2; ++i) {
    int signValue = m_3hitFilterBox.calcSign(m_hits->at(i)->hitPosition, m_hits->at(i + 1)->hitPosition,
                                             m_hits->at(i + 2)->hitPosition);
    chargeSigns.push_back(signValue);
  }
  chargeSigns.sort();
  chargeSigns.unique();
  if (int(chargeSigns.size()) != 1) {
    isZiggZagging = true;
  }
  return isZiggZagging;
}



bool TrackletFilters::ziggZaggXYWithSigma()
{
  if (m_hits == NULL)
    B2FATAL(" TrackletFilters::ziggZaggXYWithSigma: hits not set, therefore no calculation possible - please check that!");
  list<int> chargeSigns;
  bool isZiggZagging = false; // good: not ziggZagging
  for (int i = 0; i < m_numHits - 2; ++i) {
    int signValue = m_3hitFilterBox.calcSign(m_hits->at(i)->hitPosition, m_hits->at(i + 1)->hitPosition, m_hits->at(i + 2)->hitPosition,
                                             m_hits->at(i)->hitSigma, m_hits->at(i + 1)->hitSigma, m_hits->at(i + 2)->hitSigma);
    chargeSigns.push_back(signValue);
  }
  chargeSigns.remove(0);    //removes approximately (calcSign defines what approximately means) straight segments.
  chargeSigns.sort();
  chargeSigns.unique();
  if (int(chargeSigns.size()) > 1) {  //size can be 1 or 0, if all the elements were '0' before
    isZiggZagging = true;
  }
  return isZiggZagging;
}



bool TrackletFilters::ziggZaggRZ()
{
  if (m_hits == NULL) B2FATAL(" TrackletFilters::ziggZaggRZ: hits not set, therefore no calculation possible - please check that!");
  list<int> chargeSigns;
  bool isZiggZagging = false; // good: not ziggZagging
  vector<TVector3> rzHits;
  TVector3 currentVector;
  for (PositionInfo* aHit : *m_hits) {
    currentVector.SetXYZ(aHit->hitPosition.Perp(), aHit->hitPosition[1], 0.);
    rzHits.push_back(currentVector);
  }
  for (int i = 0; i < m_numHits - 2; ++i) {
    int signValue = m_3hitFilterBox.calcSign(rzHits.at(i), rzHits.at(i + 1), rzHits.at(i + 2));
    chargeSigns.push_back(signValue);
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
    B2ERROR("calcInitialValues4TCs: currentTC got " << m_numHits <<
            " hits! At this point only tcs having at least 3 hits should exist!");
  }
  //     hitA = (*hits)[2]->hitPosition;
  TVector3 hitB = (*m_hits)[1]->hitPosition;
  TVector3 hitC = (*m_hits)[0]->hitPosition; // outermost hit and initial value for genfit::TrackCandidate

  hitC -= hitB; // recycling TVector3s, this is segmentBC
  hitB -= (*m_hits)[2]->hitPosition; // this is segmentAB
  hitC.SetZ(0.);
  hitB.SetZ(0.);
  hitC = hitC.Orthogonal();

  std::pair<double, TVector3> fitResults;

  try {
    fitResults = helixFit(m_hits, useBackwards, setMomentumMagnitude);
  } catch (FilterExceptions::Straight_Line& firstException) {
    B2DEBUG(1, "Exception caught: TrackletFilters::calcMomentumSeed - helixFit said: " << firstException.what());
    try {
      fitResults = simpleLineFit3D(m_hits, useBackwards, setMomentumMagnitude);
      B2DEBUG(1, "After catching straight line case in Helix fit, the lineFit has chi2 of " << fitResults.first  <<
              "\nwhile using following hits:\n" << printHits(m_hits) << "with seed: " << fitResults.second.X() << " " << fitResults.second.Y() <<
              " " << fitResults.second.Z() << "\n");

    } catch (FilterExceptions::Straight_Up& secondException) {
      try {

        fitResults = circleFit(m_hits, useBackwards, setMomentumMagnitude);
      } catch (FilterExceptions::Straight_Line& thirdException) {
        B2FATAL("Exception caught: TrackletFilters::calcMomentumSeed - simpleLineFit3D said: " << thirdException.what());
      }
    }

  } catch (FilterExceptions::Center_Is_Origin& fourthException) {
    B2INFO("Exception caught: TrackletFilters::calcMomentumSeed - helixFit said: " << fourthException.what());
    try {
      fitResults = simpleLineFit3D(m_hits, useBackwards, setMomentumMagnitude);
    } catch (FilterExceptions::Straight_Up& fifthException) {
      try {
        fitResults = circleFit(m_hits, useBackwards, setMomentumMagnitude);
      } catch (FilterExceptions::Straight_Line& sixthException) {
        B2FATAL("Exception caught: TrackletFilters::calcMomentumSeed - simpleLineFit3D said: " << sixthException.what());
      }
    }
  } catch (FilterExceptions::Invalid_result_Nan& seventhException) {
    B2INFO("Exception caught: TrackletFilters::calcMomentumSeed - helixFit said: " << seventhException.what() <<
           "\nwhile using following hits:\n" << printHits(m_hits));
    try {
      fitResults = simpleLineFit3D(m_hits, useBackwards, setMomentumMagnitude);
    } catch (FilterExceptions::Straight_Up& eighthException) {
      try {
        fitResults = circleFit(m_hits, useBackwards, setMomentumMagnitude);
      } catch (FilterExceptions::Straight_Line& ninthException) {
        B2FATAL("Exception caught: TrackletFilters::calcMomentumSeed - simpleLineFit3D said: " << ninthException.what());
      }
    }
  }
  B2DEBUG(10, "calcMomentumSeed: return values of Fit: first(radius): " << fitResults.first << ", second.Mag(): " <<
          fitResults.second.Mag());
  int signValue = boost::math::sign(hitC *
                                    hitB); // sign of curvature: is > 0 if angle between vectors is < 90°, < 0 else (rule of scalar product)
  if (signValue == 0) {
    // means that 3 hits are completely in a line, if magnetic field is off, this can occur and therefore does not need to produce an error
    signValue = 1;
    if (m_3hitFilterBox.getMagneticField() != 0) {
      B2ERROR("trackletFilter::calcMomentumSeed: segments parallel although field is " << m_3hitFilterBox.getMagneticField() <<
              "!\nHit0: " << (*m_hits)[0]->hitPosition.X() << "/" << (*m_hits)[0]->hitPosition.Y() << "/" <<
              (*m_hits)[0]->hitPosition.Z() << ", Hit1: " << (*m_hits)[1]->hitPosition.X() << "/" << (*m_hits)[1]->hitPosition.Y() << "/" <<
              (*m_hits)[1]->hitPosition.Z() << ", Hit2: " << (*m_hits)[2]->hitPosition.X() << "/" << (*m_hits)[2]->hitPosition.Y() << "/" <<
              (*m_hits)[2]->hitPosition.Z());
    } else {
      B2DEBUG(5, "trackletFilter::calcMomentumSeed: segments parallel, but no magnetic field, therefore no problem...\nHit0: " <<
              (*m_hits)[0]->hitPosition.X() << "/" << (*m_hits)[0]->hitPosition.Y() << "/" << (*m_hits)[0]->hitPosition.Z() << ", Hit1: " <<
              (*m_hits)[1]->hitPosition.X() << "/" << (*m_hits)[1]->hitPosition.Y() << "/" << (*m_hits)[1]->hitPosition.Z() << ", Hit2: " <<
              (*m_hits)[2]->hitPosition.X() << "/" << (*m_hits)[2]->hitPosition.Y() << "/" << (*m_hits)[2]->hitPosition.Z());
    }
  }
  return make_pair(fitResults.second, signValue); //.first: momentum vector. .second: sign of curvature
}


// poca = point of closest approach of fitted circle to origin
double TrackletFilters::circleFit(double& pocaPhi, double& pocaD, double& curvature)
{
  if (m_hits == NULL) { B2FATAL(" TrackletFilters::circleFit hits not set, therefore no calculation possible - please check that!"); }

  //thomas: WARNING this one throws uncaught execeptions
  bool clockwise =
    CalcCurvature(); // Calculates Curvature: True means clockwise, False means counterclockwise.TODO this is not an optimized approach; just to get things to work. CalcCurvature could be integrated into the looping over the hits which CircleFit does anyhow.

  double stopper = 0.000000001; /// WARNING hardcoded values!
  double meanX = 0, meanY = 0, meanX2 = 0, meanY2 = 0, meanR2 = 0, meanR4 = 0, meanXR2 = 0, meanYR2 = 0, meanXY = 0; //mean values
  double sumWeights = 0, divisor/*, weightNormalizer = 0*/; // sumWeights is sum of weights, divisor is 1/sumWeights;
  double tuningParameter =
    1.; //0.02; // this parameter is for internal tuning of the weights, since at the moment, the error seams highly overestimated at the moment. 1 means no influence of parameter.

  // looping over all hits and do the division afterwards
  for (PositionInfo* hit : *m_hits) {
    double weight = 1. / (sqrt(hit->hitSigma.X() * hit->hitSigma.X() + hit->hitSigma.Y() * hit->hitSigma.Y()) * tuningParameter);
    B2DEBUG(100, " current hitSigmaU/V/X/Y: " << hit->sigmaU << "/" << hit->sigmaV << "/" << hit->hitSigma.X() << "/" <<
            hit->hitSigma.Y() << ", weight: " << weight);
    sumWeights += weight;
    if (std::isnan(weight) or std::isinf(weight) == true) { B2ERROR("TrackletFilters::circleFit, chosen sigma is 'nan': " << weight << ", setting arbitrary error: " << stopper << ")"); weight = stopper; }
    double x = hit->hitPosition.X();
    double y = hit->hitPosition.Y();
    double x2 = x * x;
    double y2 = y * y;
    double r2 = x2 + y2;
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

  if (covR2R2 == 0) { throw FilterExceptions::Center_Is_Origin(); } // TODO could be problematic if it is pretty near to 0

  // q1, q2: helping variables, to make the code more readable
  double q1 = covR2R2 * covXY - covXR2 * covYR2;
  double q2 = covR2R2 * (covXX - covYY) - covXR2 * covXR2 + covYR2 * covYR2;

  pocaPhi = 0.5 * atan2(2. * q1 ,
                        q2); // physical meaning: phi value of the point of closest approach of the fitted circle to the origin

  double sinPhi = sin(pocaPhi);
  double cosPhi = cos(pocaPhi);
  double kappa = (sinPhi * covXR2 - cosPhi * covYR2) / covR2R2;
  double delta = -kappa * meanR2 + sinPhi * meanX - cosPhi * meanY;
  double rootTerm = sqrt(1. - 4.*delta * kappa);
  curvature = 2.*kappa / (rootTerm); // rho = curvature in X-Y-plane = 1/radius of fitting circle, used for pT-calculation
  pocaD = 2.*delta / (1. + rootTerm);

  if ((curvature < 0 && clockwise) || (curvature > 0
                                       && !clockwise)) { // Checks if the random Curvature of CircleFit corresponds to CalcCurvature and adjust the results accordingly.
    // this is according to eq. 23 in the paper of Karimäki
    curvature = -curvature;
    pocaPhi = pocaPhi + M_PI;
    pocaD = -pocaD;
    //TODO ..and swap correlation Terms V_rho_phi and V_rho_d (which are not implemented anyway)
  }

  return sumWeights * (1. + curvature * pocaD) * (1. + curvature * pocaD) * (sinPhi * sinPhi * covXX - 2.*sinPhi * cosPhi * covXY +
         cosPhi * cosPhi * covYY - kappa * kappa * covR2R2); /// returns chi2
}



std::pair<double, TVector3> TrackletFilters::circleFit(const std::vector<PositionInfo*>* hits, bool useBackwards,
                                                       double setMomentumMagnitude)
{
  /** The following values are set by the circleFit.
   * - phiValue is the angle between the tangent of the fitted circle at the poca and the x-axis
   * - rValue is the distance between the poca and (0,0).
   *    If its sign is positive, the pT-Vector and the vector to the poca form a right handed system,
   *    its a left handed system if the sign is negative
   * - curvature is the curvature of the fitted circle (= 1/radius)
   *    If its sign is positive, the trajectory is curling clockwise
   *    it is curling counter-clockwise if the sign is negative
   * */
  double phiValue = 0, rValue = 0, curvature = 0;

  // transverse momentum (z-value is 0), vector pointing from the circle origin to the innermost hit:
  TVector3 pTVector, vec2Hit;

  /** The following values are angles for:
   * psi: the vector pointing to the poca,
   * the vector pointing to the innermost hit (iHit)
   * the pTVector
   * (all are measured to the x-axis)
   * */
  double psi = 0, alfa = 0, beta = 0;

  // x and y-values of the poca, the circleCenter and the hit where the pT is calculated for
  double xPoca = 0, yPoca = 0, xCc = 0, yCc = 0, xHit = 0, yHit = 0;

  double chi2 = circleFit(phiValue, rValue, curvature);
  m_radius = 1. / curvature;
  double absRadius = fabs(m_radius);
  double absRValue = fabs(rValue);
  double invAbsRValue = 1. / absRValue;

  if (rValue < 0) {    // check right handed system
    psi = /*fabs(*/phiValue/*)*/ + M_PI * 0.5;
  } else {
    psi = phiValue - M_PI * 0.5;
  }

  xPoca = absRValue * cos(psi);
  yPoca = absRValue * sin(psi);

  /// poca as result:
//  if (0 == false) {
//    pTVector.SetX(xPoca);
//    pTVector.SetY(yPoca);
//    return make_pair(chi2, pTVector);
//  }


  double signValue = sign(curvature);
  if (sign(rValue) == sign(curvature)) {  // check clockwise
    signValue = 1;
  } else {
    signValue = -1;
  }

  xCc = xPoca + signValue * absRadius * invAbsRValue * xPoca;
  yCc = yPoca + signValue * absRadius * invAbsRValue * yPoca;
  B2DEBUG(100, "TrackletFilters::circleFit: phi: " << phiValue << ", psi: " << psi << ", xPoca: " << xPoca << ", yPoca: " << yPoca  <<
          ", xCc: " << xCc << ", yCc: " << yCc);

  /// circleCenter as result:
//  if (0 == false) {
//    pTVector.SetX(xCc);
//    pTVector.SetY(yCc);
//    return make_pair(chi2, pTVector);
//  }

  // last entry of hits is the innermost one
  if (useBackwards == false) {
    xHit = hits->back()->hitPosition.X();
    yHit = hits->back()->hitPosition.Y();
  } else {
    xHit = hits->front()->hitPosition.X();
    yHit = hits->front()->hitPosition.Y();
  }

  B2DEBUG(100, "TrackletFilters::circleFit: xHit: " << xHit << ", yHit: " << yHit << ", xCc: " << xCc << ", yCc: " << yCc <<
          ", curvature: " << curvature);

  /// seedHitPosition as result:
//  if (0 == false) {
//    pTVector.SetX(xHit);
//    pTVector.SetY(yHit);
//    return make_pair(chi2, pTVector);
//  }
  vec2Hit.SetX(xCc - xHit);
  vec2Hit.SetY(yCc - yHit);
  /// vector circleCenter to seedHitPosition as result:
//  if (0 == false) {
//    return make_pair(chi2, vec2Hit);
//  }
  alfa = vec2Hit.Phi();//acos(vec2Hit.X()*fabs(curvature));

  if (curvature /*<*/ > 0) { // clockwise
    beta = alfa - M_PI * 0.5;
  } else {
    beta = alfa + M_PI * 0.5;
  }
//  if ( beta > M_PI ) beta -= 2.*M_PI;
//  if ( beta < -M_PI ) beta += 2.*M_PI;
//  if ( rValue < 0 ) {  // check right handed system
//    beta = alfa + M_PI*0.5;
//  } else {
//    beta = alfa - M_PI*0.5;
//  }
  B2DEBUG(100, "TrackletFilters::circleFit: phiValue: " << phiValue << ", psi: " << psi << ", alfa: " << alfa << ", beta: " << beta);

  if (setMomentumMagnitude ==
      0) { // in this case, we do not want an artificial magnitude for the pT-Vector and calculate the value ourself

    setMomentumMagnitude = calcPt(absRadius);
//    setMomentumMagnitude = calcPt(m_radius);
  }

  pTVector.SetX(setMomentumMagnitude * cos(beta));
  pTVector.SetY(setMomentumMagnitude * sin(beta));

  return make_pair(chi2, pTVector);
}



std::pair<double, TVector3> TrackletFilters::helixFit(const std::vector<PositionInfo*>* hits, bool useBackwards,
                                                      double setMomentumMagnitude)
{
  if (hits == NULL) { B2FATAL(" TrackletFilters::circleFit hits not set, therefore no calculation possible - please check that!"); }

  int nHits = hits->size();

  if (nHits < 3) { B2ERROR(" TrackletFilters::circleFit number of hits too low: " << nHits << " hits, therefore no useful calculation possible - please check that!"); }


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

  double sumWeights = 0;

  TMatrixD inverseCovMatrix(nHits, nHits); // carries inverse of the variances for the circle fit in its diagonal elements
  TMatrixD X(nHits, 3); // carries mapped hits, column 0 = x variables, column 1 = y variables, col 2 = r2 variables
  TMatrixD onesC(nHits, 1); // column vector of ones
  TMatrixD onesR(1, nHits); // row vector of ones
  TMatrixD R2(nHits, 1); // column vector of radii^2
  TMatrixD zValues(nHits, 1); // column vector of z values
  TMatrixD invVarZvalues(nHits, 1); // carries inverse of the variances for the line fit

  int index = 0;
//  ofstream hitsFileStream;
//  hitsFileStream.open("hitHelixFit.data", std::ios_base::trunc); // trunc=overwrite app=append
  TVector3 seedHit = (*hits).at(0)->hitPosition;
  TVector3 secondHit = (*hits).at(1)->hitPosition; // need this one for definition of direction of flight for the particle
  if (useBackwards == false) { seedHit = (*hits).at(nHits - 1)->hitPosition; secondHit = (*hits).at(nHits - 2)->hitPosition; } // want innermost hit
//  B2ERROR(" useBackwards == " << useBackwards << ", seedHit.Mag()/secondHit.Mag(): " << seedHit.Mag()<<"/"<< secondHit.Mag())

  for (PositionInfo* hit : *hits) {  // column vectors now
    double x = hit->hitPosition.X();
    double y = hit->hitPosition.Y();
    double z = hit->hitPosition.Z();
    double invVarZ = 1. / hit->hitSigma.Z();
    if (std::isnan(invVarZ) == true or std::isinf(invVarZ) == true) { B2ERROR("TrackletFilters::helixFit, chosen varZ is 'nan': " << invVarZ << ", setting arbitrary error: " << 0.000001 << ")"); invVarZ = 0.000001; }
    invVarZvalues(index, 0) = invVarZ;
    B2DEBUG(75, "helixFit: hit.X(): " << hit->hitPosition.X() << ", hit.Y(): " << hit->hitPosition.Y() << ", hit.Z(): " <<
            hit->hitPosition.Z() << ", hit.sigmaU: " << hit->sigmaU << ", hit.sigmaV: " << hit->sigmaV << ", hit.hitSigma X/Y/Z: " <<
            hit->hitSigma.X() << "/" << hit->hitSigma.Y() << "/" << hit->hitSigma.Z());

//    hitsFileStream << setprecision(14) << x << " " << y << " " << z << " " << varU << " " << varV << endl;
///   phi = atan2(y , x);  // not used yet, but will be needed for some calculations which are not implemented yet
    double r2 = x * x + y * y;
///     rPhi = phi*sqrt(r2); // not used yet, but will be needed for some calculations which are not implemented yet
    double inverseVarianceXY = 1. / sqrt(hit->hitSigma.X() * hit->hitSigma.X() + hit->hitSigma.Y() * hit->hitSigma.Y());
    if (std::isnan(inverseVarianceXY) == true or std::isinf(inverseVarianceXY) == true) { B2ERROR("TrackletFilters::helixFit, chosen inverseVarianceXY is 'nan': " << inverseVarianceXY << ", setting arbitrary error: " << 0.000001 << ")"); inverseVarianceXY = 0.000001; }
    sumWeights += inverseVarianceXY;
    inverseCovMatrix(index, index) = inverseVarianceXY;
    R2(index, 0) = r2;
    X(index, 0) = x;
    X(index, 1) = y;
    zValues(index, 0) = z;
    X(index, 2) = r2;
    onesC(index, 0) = 1;
    onesR(0, index) = 1;

    ++index;
    B2DEBUG(75, "helixFit: index: " << index << ", invVarZ: " << invVarZ << ", invVarianceXY: " << inverseVarianceXY << ", x: " << x <<
            ", y: " << y << ", z: " << z << ", r2: " << r2);
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
    for (int i = 0; i < aMatrix.GetNrows(); ++i)
    {
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

  if (lambdaCheckMatrix4NAN(inverseCovMatrix) == true) { B2DEBUG(3, "helixFit: inverseCovMatrix got 'nan'-entries!"); didNanAppear = true; }
  if (lambdaCheckMatrix4NAN(X) == true) { B2DEBUG(3, "helixFit: X got 'nan'-entries!"); didNanAppear = true; }


  /// transform to paraboloid:
  double inverseSumWeights = 1. / sumWeights;
  TMatrixD xBar = onesR * inverseCovMatrix * X * inverseSumWeights; // weighed sample mean values
  if (lambdaCheckMatrix4NAN(xBar) == true) { B2DEBUG(3, "helixFit: xBar got 'nan'-entries!"); didNanAppear = true; }


  TMatrixD transX = X;
  TMatrixD transxBar = xBar;
  transX.Transpose(transX);
  transxBar.Transpose(transxBar);

  TMatrixD weighedSampleCovMatrix = transX * inverseCovMatrix * X - transxBar * xBar * sumWeights;
  if (lambdaCheckMatrix4NAN(weighedSampleCovMatrix) == true) { B2DEBUG(3, "helixFit: weighedSampleCovMatrix got 'nan'-entries!"); didNanAppear = true; }


  /// find eigenvector to smallest eigenvalue
  TMatrixDEigen eigenCollection(weighedSampleCovMatrix);
  TMatrixD eigenValues = eigenCollection.GetEigenValues();
  if (lambdaCheckMatrix4NAN(eigenValues) == true) { B2DEBUG(3, "helixFit: eigenValues got 'nan'-entries!"); didNanAppear = true; }
  TMatrixD eigenVectors = eigenCollection.GetEigenVectors();
  if (lambdaCheckMatrix4NAN(eigenVectors) == true) { B2DEBUG(3, "helixFit: eigenVectors got 'nan'-entries!"); didNanAppear = true; }

  double minValue = std::numeric_limits<double>::max();
  int minValueIndex = -1;
  int nEVs = eigenValues.GetNcols();
  for (int i = 0; i < nEVs; ++i) {
    if (eigenValues(i, i) < minValue) {
      minValue = eigenValues(i, i);
      minValueIndex = i;
    }
  }
  if (minValueIndex < 0) { B2FATAL("TrackletFilters::helixFit produced eigenValue smaller than 0 (" << minValue << ")!");}

  double distanceOfPlane = 0;
  for (int i = 0; i < nEVs; ++i) {// calculating scalar product by hand
    distanceOfPlane += eigenVectors(i, minValueIndex) * xBar(0,
                                                             i); // eigenVectors(:,minValueIndex) = normal vector of the fitted plane (the normalized eigenvector of the smalles eigenValue of weighedSampleCovMatrix)
//     distanceOfPlane += xBar(0, i) * eigenVectors(i, minValueIndex); // eigenVectors(:,minValueIndex) = normal vector of the fitted plane (the normalized eigenvector of the smalles eigenValue of weighedSampleCovMatrix)
  }
  distanceOfPlane *= -1.;

  double n1 = eigenVectors(0, minValueIndex),
         n2 = eigenVectors(1, minValueIndex),
         n3 = eigenVectors(2, minValueIndex);

  /** In the case of a straight line, the HelixFit can not work. So we check if the plane is "straight up", or the z-normal vector is zero, n3 has the unit of cm(?) */
  if (fabs(n3) < 1e-06) { throw FilterExceptions::Straight_Line(); } /// WARNING: this value for catching straight lines is hardcoded: its resolution should be finer than the possible resolution of the detectors (we assume that the unit is cm)

  /** In the case of the fitted plane being parallel to the x-y plane, helixFit produces a nan pZ. TODO Why */
  if (fabs(n1) < 1e-10 && fabs(n2) < 1e-10) { throw FilterExceptions::Center_Is_Origin(); }

  double a = 1. / (2.*n3); // temporary value

  double xc = -n1 * a; // x coordinate of the origin of the circle
  double yc = -n2 * a; // y coordinate of the origin of the circle
//  double rho2=(1.-n3*n3-4.*distanceOfPlane*n3)*(a*a);

  double rho = sqrt((1. - n3 * n3 - 4.*distanceOfPlane * n3) * (a * a)); // radius of the circle
/// fix dec8,2013:
//  double rho = sqrt((1. - n3 * n3 - 4.*distanceOfPlane * n3) * aInv * aInv); // radius of the circle

  B2DEBUG(25, "helixFit: circle: origin x: " << xc << ", y: " << yc << ", radius: " << rho  << endl);


  /// line fit:
  TMatrixD H = distanceOfPlane + R2 * n3; // temporary value
  if (lambdaCheckMatrix4NAN(H) == true) { B2DEBUG(3, "helixFit: H got 'nan'-entries!"); didNanAppear = true; }

  TMatrixD H2 = H;
  H2.Sqr(); // squares each element
//   for (int i = 0; i < H2.GetNrows(); ++i) { H2(i, 0) *= H2(i, 0); }
  if (lambdaCheckMatrix4NAN(H2) == true) { B2DEBUG(50, "helixFit: H2 got 'nan'-entries!"); didNanAppear = true; }

  double b = n1 * n1 + n2 * n2; // temporary value

  TMatrixD T2 = b * R2 - H2; // temporary value T2 = vector, since b = scalar * vector R2 - Vector H2
  B2DEBUG(25, "helixFit: T.min: " << T2.Min() << ", T.max: " << T2.Max() << ", R2.min: " << R2.Min() << ", R2.max: " << R2.Max() <<
          ", H2.min: " << H2.Min() << ", H2.max: " << H2.Max() << ", H.min: " << H.Min() << ", H.max: " << H.Max() << ", b: " << b);

  if (lambdaCheckMatrix4NAN(T2) == true) {B2DEBUG(50, "helixFit: T2 got 'nan'-entries!"); didNanAppear = true; }

  TMatrixD T = T2;

  for (int k = 0; k < T.GetNrows(); ++k) { // filtering cases where rounding errors produce negative T-entries
    if (T(k, 0) < 0) {

      //Console Output:
      B2DEBUG(5, "T" << k << " was " << T(k, 0) << " and will manually be set to 0.");
      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 3, PACKAGENAME()) == true) {
        B2DEBUG(10, "The following hits were part of this TC: \n" << printHits(m_hits) << "\n'T' had following entries: " <<
                printMyMatrixstring(T));
      }

      T(k, 0) = 0;
    }
  }

  T.Sqrt(); // take square root of all elements
//   for (int i = 0; i < T.GetNrows(); ++i) { T(i, 0) = sqrt(T(i, 0)); }
  if (lambdaCheckMatrix4NAN(T) == true) { B2DEBUG(3, "helixFit: T got 'nan'-entries after Sqrt! Before: T.min: " << T2.Min() << ", T.max: " << T2.Max() << ", after: T.min: " << T.Min() << ", T.max: " << T.Max()); didNanAppear = true; }

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
  B2DEBUG(25, " sum d1: " << dx1 + dy1 << ", d2: " << dx2 + dy2);

  if ((dx1 + dy1) < (dx2 + dy2)) {
    if (lambdaCheckMatrix4NAN(x1) == true or lambdaCheckMatrix4NAN(y1) == true) {
      xs = x2; ys = y2;
      if (lambdaCheckMatrix4NAN(x2) == true or lambdaCheckMatrix4NAN(y2) == true) {
        B2DEBUG(10, "there is 'nan' = " << lambdaCheckMatrix4NAN(x1) << "/" << lambdaCheckMatrix4NAN(y1) << "/" << lambdaCheckMatrix4NAN(
                  x2) << "/" << lambdaCheckMatrix4NAN(y2) << " in x1/y1/x2/y2!");

      }
    } else  {
      xs = x1; ys = y1;
    }
  } else {
    if (lambdaCheckMatrix4NAN(x2) == true or lambdaCheckMatrix4NAN(y2) == true) {
      xs = x1; ys = y1;
      if (lambdaCheckMatrix4NAN(x1) == true or lambdaCheckMatrix4NAN(y1) == true) {
        B2DEBUG(10, "there is 'nan' = " << lambdaCheckMatrix4NAN(x1) << "/" << lambdaCheckMatrix4NAN(y1) << "/" << lambdaCheckMatrix4NAN(
                  x2) << "/" << lambdaCheckMatrix4NAN(y2) << " in x1/y1/x2/y2!");
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
      B2DEBUG(3, "helixFit: i: " << i << ", s(i) = 'nan', components - rho: " << rho << ", radiusX: " << radiusX << ", radiusY: " <<
              radiusY << ", radiusXb: " << radiusXb << ", radiusYb: " << radiusYb << ", invRadiusMag: " << invRadiusMag << ", radiusMagb: " <<
              radiusMagb << ", xs(i): " << xs(i, 0) << ", ys(i): " << ys(i, 0));
    }
  }

  /// fit line s (= arc length) versus z

  TMatrixD AtGA(2, 2);
  TMatrixD AtG(2, nHits);
  TMatrixDSparse Diag(nHits, nHits);
  double sumWi = 0, sumWiSi = 0, sumWiSi2 = 0;
  for (int i = 0; i < nHits; ++i) {
    sumWi += invVarZvalues(i, 0);
    double sw = invVarZvalues(i, 0) * s(i, 0);
    sumWiSi += sw;
    sumWiSi2 += invVarZvalues(i, 0) * s(i, 0) * s(i, 0);
    AtG(0, i) = invVarZvalues(i, 0);
    AtG(1, i) = sw;
    Diag(i, i) = 1.;
    B2DEBUG(75, "hit i: " <<  i << ", sumWi: " << sumWi << ", sw: " << sw << ", sumWiSi: " << sumWiSi << ", sumWiSi2: " << sumWiSi2 <<
            ", s(i): " << s(i, 0) << ", invVarZvalues(i): " << invVarZvalues(i, 0));
  }
  AtGA(0, 0) = sumWi; // sum of weights
  AtGA(0, 1) = sumWiSi; // sum of weights times arc length
  AtGA(1, 0) = sumWiSi;  // sum of weights times arc length
  AtGA(1, 1) = sumWiSi2;  // sum of weights times arc length ^2
  TMatrixD AtGAInv = AtGA;
  AtGAInv.Invert();
  if (lambdaCheckMatrix4NAN(AtGA) == true) {B2DEBUG(10, "helixFit: AtGA got 'nan'-entries!"); didNanAppear = true; }
  if (lambdaCheckMatrix4NAN(AtGAInv) == true) {B2DEBUG(10, "helixFit: AtGAInv got 'nan'-entries!"); didNanAppear = true; }
  if (lambdaCheckMatrix4NAN(zValues) == true) {B2DEBUG(10, "helixFit: zValues got 'nan'-entries!"); didNanAppear = true; }

  TMatrixD p = AtGAInv * AtG *
               zValues; // fitted z value in the first point, tan(lambda) -> WARNING FIXME why is the first point 1,0 not 0,0? jkl feb8th2014
  if (lambdaCheckMatrix4NAN(p) == true) { B2DEBUG(10, "helixFit: p got 'nan'-entries!"); }

  TMatrixD TAtG = AtG;
  TAtG.T();
  TMatrixD zValuesT = zValues;
  zValuesT.T();

  TMatrixD sigma2M = zValuesT * (Diag * TAtG * AtGAInv * AtG) * zValues;

  double thetaVal = (M_PI * 0.5 - atan(p(1,
                                         0))); // WARNING: was M_PI*0.5 - atan(p(1,0)), but values were wrong! double-WARNING: but + atan was wrong too!
//    double thetaVal = (M_PI * 0.5 - atan2(rho, p(1, 0))); // test feb8th: trying to calculate Thetaval like TVector3.Theta...
/// opposite leg = r, adjacent leg = z
  if (std::isnan(thetaVal) == true) {
    didNanAppear = true;
    thetaVal = (hits->at(0)->hitPosition - hits->at(nHits - 1)->hitPosition).Theta(); /// INFO swapped! feb4th2014
    B2DEBUG(3, "helixFit: calculating theta for momentum produced 'nan' -> fallback-solution produces theta: " << thetaVal);
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
  TVector3 pVector = (radialVector.Orthogonal()).Unit(); // is the direction of the momentum without actual magnitude of the momentum
  if (setMomentumMagnitude == 0) {
    pVector = pT * pVector; // now it is the pT-Vector, therefore without pZ information
  } else { // means we want to set the magnitude of the momentum artificially
    pVector = setMomentumMagnitude * pVector; // now it is the pT-Vector, therefore without pZ information
  }

  /** local lambda-function used for checking TVector3s, whether there are nan values included, returns true, if there are */
  auto lambdaCheckVector4NAN = [](TVector3 & aVector) -> bool { /// testing c++11 lambda functions...
    return std::isnan(aVector.Mag2()); // if one of them is 'nan', Mag2 will be 'nan' too
  }; // should be converted to normal function, since feature could be used much more often...
  if (lambdaCheckVector4NAN(pVector) == true) { B2ERROR("helixFit: pTVector got 'nan'-entries x/y/z: " << pVector.X() << "/" << pVector.Y() << "/" << pVector.Z()); didNanAppear = true; }


//   double pZ = pT / tan(thetaVal);
//  double pZ = -calcPt(rho)*p(1,0); // TODO check that !!!1111eleven
  double pZ = calcPt(rho) * p(1, 0) ;
  // pz = pt / tan (Theta); where Theta is Arctan(s/z), s... arc length in (x,y) = radius * Phi in radians, z... z-distance.
  // So pz = pt * z / s = magneticFieldFactor*rho*z/s, and p(1,0).."fitted z-value" (TODO: check that!) seems to be z/s. => calcPt(rho)p(1,0)
  B2DEBUG(25, "helixFit: radius(rho): " << rho << ", theta: " << thetaVal << ", pT: " << pT << ", pZ: " << pZ << ", pVector.Perp: " <<
          pVector.Perp() << ", pVector.Mag: " << pVector.Mag() << ", fitted zValue: " << p(0, 0));
  TVector3 vectorToSecondHit = secondHit - seedHit;
  vectorToSecondHit.SetZ(0);


  if (((useBackwards == true) && (vectorToSecondHit.Angle(pVector) < M_PI * 0.5)) || ((useBackwards == false)
      && (vectorToSecondHit.Angle(pVector) > M_PI * 0.5))) { pVector *= -1.; }
  pVector.SetZ(-pZ); // now that track carries full momentum

  // Tobias approach
//  if ((useBackwards && vectorToSecondHit.Angle(pVector) < M_PI/2)
//       || (!useBackwards && vectorToSecondHit.Angle(pVector) > M_PI/2)) {
//     pVector *= -1.;
//   }


//  if (((useBackwards == true) && (vectorToSecondHit.Angle(pVector) < M_PI * 0.5)) || ((useBackwards == false) && (vectorToSecondHit.Angle(pVector) > M_PI * 0.5))) { pVector.SetZ(-pZ); /*pVector *= -1.;*/ } else { pVector.SetZ(pZ); }
  // if (((useBackwards == true) && (vectorToSecondHit.Angle(pVector) > M_PI * 0.5)) || ((useBackwards == false) && (vectorToSecondHit.Angle(pVector) < M_PI * 0.5))) { pVector *= -1.; } // edit: Feb4-2014: swapped values...
  if (lambdaCheckVector4NAN(pVector) == true) { B2ERROR("helixFit: pVector got 'nan'-entries x/y/z: " << pVector.X() << "/" << pVector.Y() << "/" << pVector.Z()); didNanAppear = true; }

  if (didNanAppear == true && LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1, PACKAGENAME()) == true) {
    B2DEBUG(3, "helixFit: there was a 'nan'-value detected. When using magnetic field of " << m_3hitFilterBox.getMagneticField() <<
            ", the following hits were part of this TC: \n" << printHits(m_hits) << "\n pVector  x/y/z: " << pVector.X() << "/" << pVector.Y()
            << "/" << pVector.Z());
  }


//     B2WARNING("helixFit: strange pVector (Mag="<<pVector.Mag()<< ") detected. The following hits were part of this TC: \n" << printHits(m_hits) << "\n pVector  x/y/z: " << pVector.X()<<"/"<< pVector.Y()<<"/"<< pVector.Z())
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

  if (std::isnan(rho) == true or lambdaCheckVector4NAN(pVector) == true) {
    throw FilterExceptions::Invalid_result_Nan();
  }
  return make_pair(rho, pVector);
}



pair<double, TVector3> TrackletFilters::simpleLineFit3D(const vector<PositionInfo*>* hits, bool useBackwards,
                                                        double setMomentumMagnitude)
{
  /** Testbeam:
   * Coords:   Sensors:
   * ^        ./| ./| ./|
   * |   ^    | | | | | |
   * |Y /Z    | | | | | |
   * |/       |/  |/  |/
   * -------> X
   *
   * beam parallel to x. Measurement errors in y & z (v&u)
   * With these conditions, the following approach using 2 independent 2D line fits is acceptable (if rotation is the same for all sensors):
   * Modells:
   * Y_i = a*X_i + b        Z_i = c*X_i + d
   * */

  TVector3 directionVector;
  double sumWyi = 0, // sum of weights for Yi
         sumWzi = 0, // sum of weights for Zi
         sumWyiXi = 0, // sum of (y-weights times x-values)
         sumWziXi = 0, // sum of (z-weights times x-values)
         sumWyiYi = 0, // sum of (y-weights times y-values)
         sumWziZi = 0, // sum of (z-weights times z-values)
         sumWyiXiYi = 0, // sum of (y-weights times x-values times y-values)
         sumWziXiZi = 0, // sum of (z-weights times x-values times z-values)
         sumWyiXi2 = 0, // sum of (y-weights times x-values^2)
         sumWziXi2 = 0, // sum of (z-weights times x-values^2)
         detValY = 0, // determinant for norming values - y
         detValZ = 0, // determinant for norming values - z
         slopeY = 0, // = a of model
         slopeZ = 0, // = c of model
         chi2 = 0, // final chi2-value of fit
         interceptY = 0, // b of model, needed only for chi2-calculation
         interceptZ = 0; // d of model, needed only for chi2-calculation

  // NOTE: this approach is not optimal. Maybe can be optimized for less redundancy
  for (const PositionInfo* aHit : *hits) {
    double Wyi = (1. / (aHit->hitSigma.Y() * aHit->hitSigma.Y()));
    double Wzi = (1. / (aHit->hitSigma.Z() * aHit->hitSigma.Z()));

    sumWyi += Wyi;
    sumWzi += Wzi;

    sumWyiXi += Wyi * aHit->hitPosition.X();
    sumWziXi += Wzi * aHit->hitPosition.X();

    sumWyiYi += Wyi * aHit->hitPosition.Y();
    sumWziZi += Wzi * aHit->hitPosition.Z();

    sumWyiXiYi += Wyi * aHit->hitPosition.X() * aHit->hitPosition.Y();
    sumWziXiZi += Wzi * aHit->hitPosition.X() * aHit->hitPosition.Z();

    sumWyiXi2 += Wyi * aHit->hitPosition.X() * aHit->hitPosition.X();
    sumWziXi2 += Wzi * aHit->hitPosition.X() * aHit->hitPosition.X();
  }

  detValY = sumWyiXi2 * sumWyi - sumWyiXi * sumWyiXi;
  if (detValY == 0) {
    throw FilterExceptions::Straight_Up();
  }
  detValY = 1. / detValY; // invert

  detValZ = sumWziXi2 * sumWzi - sumWziXi * sumWziXi;
  if (detValZ == 0) {
    throw FilterExceptions::Straight_Up();
  }
  detValZ = 1. / detValZ; // invert

  slopeY = detValY * (sumWyi * sumWyiXiYi  -  sumWyiXi * sumWyiYi);
  slopeZ = detValZ * (sumWzi * sumWziXiZi  -  sumWziXi * sumWziZi);

  interceptY = detValY * (- sumWyiXi * sumWyiXiYi  +  sumWyiXi2 * sumWyiYi);
  interceptZ = detValZ * (- sumWziXi * sumWziXiZi  +  sumWziXi2 * sumWziZi);

  for (const PositionInfo* aHit : *hits) {  // chi2 of xy-fit and of xz-fit can be combined by adding their values
    chi2 += pow(((aHit->hitPosition.Y() - slopeY * aHit->hitPosition.X() - interceptY) / aHit->hitSigma.Y()) , 2)
            + pow(((aHit->hitPosition.Z() - slopeZ * aHit->hitPosition.X() - interceptZ) / aHit->hitSigma.Z()) , 2);
  }

  m_lineParameters = {slopeY, interceptY, slopeZ, interceptZ}; // storing values for validation

  directionVector.SetXYZ(1, slopeY, slopeZ);

  if (useBackwards == true) { directionVector *= -1.; } // TODO: check that...

  if (setMomentumMagnitude != 0) { directionVector = setMomentumMagnitude * directionVector.Unit(); } // means we want to set the magnitude of the momentum artificially

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
      B2ERROR("calcInitialValues4TCs: currentTC got " << numOfCurrentHits << " hits! At this point only tcs having at least 3 hits should exist!");
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



bool TrackletFilters::CalcCurvature()
{
  if (m_hits == NULL)
    B2FATAL(" TrackletFilters::CalcCurvature: hits not set, therefore no calculation possible - please check that!");
  double sumOfCurvature = 0.;
  for (int i = 0; i < m_numHits - 2; ++i) {
    TVector3 ab = m_hits->at(i)->hitPosition - m_hits->at(i + 1)->hitPosition;
    ab.SetZ(0.);
    TVector3 bc = m_hits->at(i + 1)->hitPosition - m_hits->at(i + 2)->hitPosition;
    bc.SetZ(0.);
    sumOfCurvature += bc.Orthogonal() * ab; //normal vector of m_vecBC times segment of ba
  }
  //B2WARNING(sumOfCurvature);
  if (sumOfCurvature == 0.) {
    throw FilterExceptions::Calculating_Curvature_Failed();
  }
  if (sumOfCurvature > 0.) { return true; }
  else { return false; }
}


std::string TrackletFilters::printHits(const std::vector<PositionInfo*>* hits) const
{
  stringstream hitX, hitY, hitZ, sigmaX, sigmaY, sigmaZ;
  hitX << "xPos: ";
  hitY << "yPos: ";
  hitZ << "zPos: ";
  sigmaX << "xSigma: ";
  sigmaY << "ySigma: ";
  sigmaZ << "zSigma: ";
  for (PositionInfo* hit : *hits) {
    hitX << hit->hitPosition.X() << ", ";
    hitY << hit->hitPosition.Y() << ", ";
    hitZ << hit->hitPosition.Z() << ", ";
    sigmaX << hit->hitSigma.X() << ", ";
    sigmaY << hit->hitSigma.X() << ", ";
    sigmaZ << hit->hitSigma.X() << ", ";
  }
  return hitX.str() + "\n" + hitY.str() + "\n" + hitZ.str() + "\n" + sigmaX.str() + "\n" + sigmaY.str() + "\n" + sigmaZ.str() + "\n";
}
// bool TrackletFilters::CalcCurvature()
// {
//   if (m_hits == NULL) B2FATAL(" TrackletFilters::CalcCurvature: hits not set, therefore no calculation possible - please check that!")
//     int sumOfCurvature = 0;
//   for (int i = 0; i < m_numHits - 2; ++i) {
//     sumOfCurvature += m_3hitFilterBox.calcSign(m_hits->at(i)->hitPosition, m_hits->at(i + 1)->hitPosition, m_hits->at(i + 2)->hitPosition);
//     //We sum over the Signs: a positive value represents a left-oriented (from out to in) curvature, a negative value means having a right-oriented curvature.
//   }
//   if (sumOfCurvature == 0) { throw FilterExceptions::Calculating_Curvature_Failed(); }  //Maybe one should define a more suitable exception; TODO and one could try weighting the Curvature -1,0,1 by the 3D Distance. (if the ==0 case appears too often.)
//   if (sumOfCurvature > 0) { return true; }
//   else { return false; }
// }
