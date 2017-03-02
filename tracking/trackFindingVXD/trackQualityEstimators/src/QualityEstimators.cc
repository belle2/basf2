/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Felix Metzner                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/trackFindingVXD/trackQualityEstimators/QualityEstimators.h"
#include <framework/logging/Logger.h>

#include <TMathBase.h>
#include <TMatrixD.h>
#include <TMatrixDEigen.h>

#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/math/special_functions/sign.hpp>

#include <Eigen/Dense>

#include <list>
#include <iostream>
#include <limits>
#include <stdio.h>
#include <math.h>
#include <fstream>
#include <iomanip>
#include <utility>

using namespace Belle2;
using boost::math::sign;

// Set precision to be used for Eigen Library
typedef double Precision;


std::pair<TVector3, int> QualityEstimators::calcMomentumSeed(bool useBackwards, double setMomentumMagnitude)
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
    fitResults = riemannHelixFit(m_hits);
    // fitResults = helixFit(m_hits, useBackwards, setMomentumMagnitude);
  } catch (FilterExceptions::Straight_Line& anException) {
    B2DEBUG(1, "Exception caught: QualityEstimators::calcMomentumSeed - helixFit said: " << anException.what());
    try {
      fitResults = simpleLineFit3D(m_hits, useBackwards, setMomentumMagnitude);
      B2DEBUG(1, "After catching straight line case in Helix fit, the lineFit has chi2 of " << fitResults.first  <<
              "\nwhile using following hits:\n" << printHits(m_hits) << "with seed: " << fitResults.second.X() << " " << fitResults.second.Y() <<
              " " << fitResults.second.Z() << "\n");

    } catch (FilterExceptions::Straight_Up& anException) {
      try {
        fitResults = circleFit(m_hits, useBackwards, setMomentumMagnitude);
      } catch (FilterExceptions::Straight_Line& anException) {
        B2FATAL("Exception caught: QualityEstimators::calcMomentumSeed - simpleLineFit3D said: " << anException.what());
      }
    }

  } catch (FilterExceptions::Center_Is_Origin& anException) {
    B2INFO("Exception caught: QualityEstimators::calcMomentumSeed - helixFit said: " << anException.what());
    try {
      fitResults = simpleLineFit3D(m_hits, useBackwards, setMomentumMagnitude);
    } catch (FilterExceptions::Straight_Up& anException) {
      try {
        fitResults = circleFit(m_hits, useBackwards, setMomentumMagnitude);
      } catch (FilterExceptions::Straight_Line& anException) {
        B2FATAL("Exception caught: QualityEstimators::calcMomentumSeed - simpleLineFit3D said: " << anException.what());
      }
    }
  } catch (FilterExceptions::Invalid_result_Nan& anException) {
    B2INFO("Exception caught: QualityEstimators::calcMomentumSeed - helixFit said: " << anException.what() <<
           "\nwhile using following hits:\n" << printHits(m_hits));
    try {
      fitResults = simpleLineFit3D(m_hits, useBackwards, setMomentumMagnitude);
    } catch (FilterExceptions::Straight_Up& anException) {
      try {
        fitResults = circleFit(m_hits, useBackwards, setMomentumMagnitude);
      } catch (FilterExceptions::Straight_Line& anException) {
        B2FATAL("Exception caught: QualityEstimators::calcMomentumSeed - simpleLineFit3D said: " << anException.what());
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
    if (getMagneticField() != 0) {
      B2ERROR("QualityEstimators::calcMomentumSeed: segments parallel although field is " << getMagneticField() <<
              "!\nHit0: " << (*m_hits)[0]->hitPosition.X() << "/" << (*m_hits)[0]->hitPosition.Y() << "/" <<
              (*m_hits)[0]->hitPosition.Z() << ", Hit1: " << (*m_hits)[1]->hitPosition.X() << "/" << (*m_hits)[1]->hitPosition.Y() << "/" <<
              (*m_hits)[1]->hitPosition.Z() << ", Hit2: " << (*m_hits)[2]->hitPosition.X() << "/" << (*m_hits)[2]->hitPosition.Y() << "/" <<
              (*m_hits)[2]->hitPosition.Z());
    } else {
      B2DEBUG(5, "QualityEstimators::calcMomentumSeed: segments parallel, but no magnetic field, therefore no problem...\nHit0: " <<
              (*m_hits)[0]->hitPosition.X() << "/" << (*m_hits)[0]->hitPosition.Y() << "/" << (*m_hits)[0]->hitPosition.Z() << ", Hit1: " <<
              (*m_hits)[1]->hitPosition.X() << "/" << (*m_hits)[1]->hitPosition.Y() << "/" << (*m_hits)[1]->hitPosition.Z() << ", Hit2: " <<
              (*m_hits)[2]->hitPosition.X() << "/" << (*m_hits)[2]->hitPosition.Y() << "/" << (*m_hits)[2]->hitPosition.Z());
    }
  }

  return std::make_pair(fitResults.second, signValue); //.first: momentum vector. .second: sign of curvature
}


double QualityEstimators::circleFit(double& pocaPhi, double& pocaD, double& curvature)
{
  if (m_hits == NULL) { B2FATAL(" QualityEstimators::circleFit hits not set, therefore no calculation possible - please check that!"); }

  //thomas: TODO: WARNING this one throws uncaught execeptions
  bool clockwise =
    CalcCurvature(); // Calculates Curvature: True means clockwise, False means counterclockwise.TODO this is not an optimized approach; just to get things to work. CalcCurvature could be integrated into the looping over the hits which CircleFit does anyhow.

  double stopper = 0.000000001; /// WARNING hardcoded values!
  double meanX = 0, meanY = 0, meanX2 = 0, meanY2 = 0, meanR2 = 0, meanR4 = 0, meanXR2 = 0, meanYR2 = 0, meanXY = 0; //mean values
  double r2 = 0, x = 0, y = 0, x2 = 0, y2 = 0; // coords
  double weight;// weight of each hit, so far no difference in hit quality
  double sumWeights = 0, divisor/*, weightNormalizer = 0*/; // sumWeights is sum of weights, divisor is 1/sumWeights;
  double tuningParameter =
    1.; //0.02; // this parameter is for internal tuning of the weights, since at the moment, the error seams highly overestimated at the moment. 1 means no influence of parameter.

  // looping over all hits and do the division afterwards
  for (PositionInfo* hit : *m_hits) {
    weight = 1. / (sqrt(hit->hitSigma.X() * hit->hitSigma.X() + hit->hitSigma.Y() * hit->hitSigma.Y()) * tuningParameter);
    B2DEBUG(100, " current hitSigmaU/V/X/Y: " << hit->sigmaU << "/" << hit->sigmaV << "/" << hit->hitSigma.X() << "/" <<
            hit->hitSigma.Y() << ", weight: " << weight);
    sumWeights += weight;
    if (std::isnan(weight) or std::isinf(weight) == true) { B2ERROR("QualityEstimators::circleFit, chosen sigma is 'nan': " << weight << ", setting arbitrary error: " << stopper << ")"); weight = stopper; }
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
         cosPhi * cosPhi * covYY - kappa * kappa * covR2R2); // returns chi2
}



std::pair<double, TVector3> QualityEstimators::circleFit(std::vector<PositionInfo*> const* hits, bool useBackwards,
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

  TVector3 pTVector, vec2Hit;

  double psi = 0, alfa = 0, beta = 0;

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


  double signValue = sign(curvature);
  if (sign(rValue) == sign(curvature)) {  // check clockwise
    signValue = 1;
  } else {
    signValue = -1;
  }

  xCc = xPoca + signValue * absRadius * invAbsRValue * xPoca;
  yCc = yPoca + signValue * absRadius * invAbsRValue * yPoca;
  B2DEBUG(100, "QualityEstimators::circleFit: phi: " << phiValue << ", psi: " << psi << ", xPoca: " << xPoca << ", yPoca: " << yPoca
          <<
          ", xCc: " << xCc << ", yCc: " << yCc);

  if (useBackwards == false) {
    xHit = hits->back()->hitPosition.X();
    yHit = hits->back()->hitPosition.Y();
  } else {
    xHit = hits->front()->hitPosition.X();
    yHit = hits->front()->hitPosition.Y();
  }

  B2DEBUG(100, "QualityEstimators::circleFit: xHit: " << xHit << ", yHit: " << yHit << ", xCc: " << xCc << ", yCc: " << yCc <<
          ", curvature: " << curvature);

  vec2Hit.SetX(xCc - xHit);
  vec2Hit.SetY(yCc - yHit);

  alfa = vec2Hit.Phi();

  if (curvature /*<*/ > 0) { // clockwise
    beta = alfa - M_PI * 0.5;
  } else {
    beta = alfa + M_PI * 0.5;
  }

  B2DEBUG(100, "QualityEstimators::circleFit: phiValue: " << phiValue << ", psi: " << psi << ", alfa: " << alfa << ", beta: " <<
          beta);

  if (setMomentumMagnitude == 0) { setMomentumMagnitude = calcPt(absRadius); }

  pTVector.SetX(setMomentumMagnitude * cos(beta));
  pTVector.SetY(setMomentumMagnitude * sin(beta));

  return std::make_pair(chi2, pTVector);
}



/** does a tripletFit of the given hits
 * The filter is based on the paper 'A New Three-Dimensional Track Fit with Multiple Scattering'
 * by Andre Schoening et al. https://arxiv.org/abs/1606.04990*/
std::pair<double, TVector3> QualityEstimators::tripletFit(std::vector<PositionInfo*> const* hits)
{
  if (hits == NULL) { B2FATAL(" QualityEstimators::tripletFit hits not set, therefore no calculation possible - please check that!"); }

  /** TODO Check in which order the hits are stored in the vector hits, as we should consider them in the right order,
   *       starting from the innermost hit!
   */

  // TODO Include modification for strong multiple scattering as described in the paper

  const int nTriplets = hits->size() - 2;

  double combinedChi2 = 0.;

  std::vector<double> R3Ds;
  R3Ds.reserve(nTriplets);
  std::vector<double> sigmaR3DSquareds;
  sigmaR3DSquareds.reserve(nTriplets);

  // looping over all triplets
  for (int i = 0; i < nTriplets; i++) {

    // Three hits relevant for curent triplet
    const TVector3 hit0 = hits->at(i)->hitPosition;
    const TVector3 hit1 = hits->at(i + 1)->hitPosition;
    const TVector3 hit2 = hits->at(i + 2)->hitPosition;

    const double d01sq = pow(hit1.X() - hit0.X(), 2) + pow(hit1.Y() - hit0.Y(), 2);
    const double d12sq = pow(hit2.X() - hit1.X(), 2) + pow(hit2.Y() - hit1.Y(), 2);
    const double d02sq = pow(hit2.X() - hit0.X(), 2) + pow(hit2.Y() - hit0.Y(), 2);

    const double d01 = sqrt(d01sq);
    const double d12 = sqrt(d12sq);
    const double d02 = sqrt(d02sq);

    const double z01 = std::abs(hit1.Z() - hit0.Z());
    const double z12 = std::abs(hit2.Z() - hit1.Z());

    const double R_C = (d01 * d12 * d02) / sqrt(-d01sq * d01sq - d12sq * d12sq - d02sq * d02sq + 2 * d01sq * d12sq + 2 * d12sq * d02sq +
                                                2 *
                                                d02sq * d01sq);

    const double Phi1C = 2. * asin(d01 / (2. * R_C));
    const double Phi2C = 2. * asin(d12 / (2. * R_C));
    // TODO Phi1C and Phi2C have 2 solutions (<Pi and >Pi), each, of which the correct one must be chosen!

    const double R3D1C = sqrt(R_C * R_C + (z01 * z01) / (Phi1C * Phi1C));
    const double R3D2C = sqrt(R_C * R_C + (z12 * z12) / (Phi2C * Phi2C));

    const double theta1C = acos(z01 / (Phi1C * R3D1C));
    const double theta2C = acos(z12 / (Phi2C * R3D2C));
    const double theta = (theta1C + theta2C) / 2.;

    double alpha1 = R_C * R_C * Phi1C * Phi1C + z01 * z01;
    alpha1 *= 1. / (0.5 * R_C * R_C * Phi1C * Phi1C * Phi1C / tan(Phi1C / 2.) + z01 * z01);
    double alpha2 = R_C * R_C * Phi2C * Phi2C + z12 * z12;
    alpha2 *= 1. / (0.5 * R_C * R_C * Phi2C * Phi2C * Phi2C / tan(Phi2C / 2.) + z12 * z12);

    const double PhiTilde = - 0.5 * (Phi1C * alpha1 + Phi2C * alpha2);
    const double eta = 0.5 * Phi1C * alpha1 / R3D1C + 0.5 * Phi2C * alpha2 / R3D2C;
    const double ThetaTilde = theta2C - theta1C - (1 - alpha2) / tan(theta2C) + (1 - alpha1) / tan(theta1C);
    const double beta = (1 - alpha2) / (R3D2C * tan(theta2C)) - (1 - alpha1) / (R3D1C * tan(theta1C));

    // Calculation of sigmaMS
    double bField = getMagneticField();

    /** Using average material budged of SVD sensors for approximation of radiation length
     *  Belle II TDR page 156 states a value of 0.57% X_0.
     *  This approximation is a first approach to the problem and must be checked.
     */
    const double XoverX0 = 0.0057 / cos(M_PI / 2. - theta1C);

    double R3D = - (eta * PhiTilde * sin(theta) * sin(theta) + beta * ThetaTilde);
    R3D *= 1. / (eta * eta * sin(theta) * sin(theta) + beta * beta);
    const double b = 4.5 / bField * sqrt(XoverX0);
    const double sigmaMS = b / R3D;

    double sigmaR3DSquared = pow(sigmaMS, 2) / (pow(eta * sin(theta), 2) + pow(beta, 2));

    double Chi2min = pow(beta * PhiTilde - eta * ThetaTilde, 2);
    Chi2min *= 1. / (sigmaMS * sigmaMS * (eta * eta + beta * beta / pow(sin(theta), 2)));

    // store values for combination
    R3Ds.push_back(R3D);
    sigmaR3DSquareds.push_back(sigmaR3DSquared);

    combinedChi2 += Chi2min;
  }

  // Calculate average R3D
  double numerator = 0;
  double denominator = 0;
  for (short i = 0; i < nTriplets; ++i) {
    numerator += R3Ds.at(i) / sigmaR3DSquareds.at(i);
    denominator += 1. / sigmaR3DSquareds.at(i);
  }
  double const averageR3D = numerator / denominator;

  // Compare individual R3Ds with average R3D to improve chi2 as presented at:
  // Connecting the Dots, Vienna, Feb 2016 by A. Schoening
  double globalCompatibilityOfR3Ds = 0;
  for (short i = 0; i < nTriplets; ++i) {
    globalCompatibilityOfR3Ds += pow(R3Ds.at(i) - averageR3D, 2) / sigmaR3DSquareds.at(i);
  }

  double const finalChi2 = combinedChi2 + globalCompatibilityOfR3Ds;

  // TODO return real momentum vector?
  TVector3 pTVector(1, 2, 3);

  return std::make_pair(finalChi2, pTVector);
}


std::pair<double, TVector3> QualityEstimators::riemannHelixFit(const std::vector<PositionInfo*>* hits)
{
  // Based on R. Fruehwirth, A. Strandlie, W. Waltenberger,
  // Nuclear instruments and Methods in Physics Research A 490 (2002) 366-378

  const int nHits = hits->size();

  // Circle Fit

  Eigen::Matrix<Precision, Eigen::Dynamic, Eigen::Dynamic> W = Eigen::Matrix<Precision, Eigen::Dynamic, Eigen::Dynamic>::Zero(nHits,
      nHits);
  Eigen::Matrix<Precision, Eigen::Dynamic, Eigen::Dynamic> Wz = Eigen::Matrix<Precision, Eigen::Dynamic, Eigen::Dynamic>::Zero(nHits,
      nHits);
  Eigen::Matrix<Precision, Eigen::Dynamic, 3> X = Eigen::Matrix<Precision, Eigen::Dynamic, 3>::Zero(nHits, 3);
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> Z = Eigen::Matrix<Precision, Eigen::Dynamic, 1>::Zero(nHits, 1);
  Precision traceOfW = 0.;

  short index = 0;
  for (PositionInfo* hit : *hits) {
    double x = hit->hitPosition.X();
    double y = hit->hitPosition.Y();
    double z = hit->hitPosition.Z();
    double sigmaX = hit->hitSigma.X();
    double sigmaY = hit->hitSigma.Y();
    double sigmaZ = hit->hitSigma.Z();

    double r2 = x * x + y * y;
    double inverseVarianceXY = 1. / sqrt(sigmaX * sigmaX + sigmaY * sigmaY);

    // The following weight matrix W can be improved for cases with multiple scattering
    // by adding a correction term which will make the matrix non-diagonal.
    // However, this requires prior knowledge of the curvature of the track and thus a
    // second iteration (see page 368 of above mentioned source).
    W(index, index) = inverseVarianceXY;
    traceOfW += inverseVarianceXY;

    X(index, 0) = x;
    X(index, 1) = y;
    X(index, 2) = r2;

    // Values for z line fit for extrended Riemann fit
    Wz(index, index) = 1. / sigmaZ;
    Z(index, 0) = z;

    index++;
  }

  Eigen::Matrix<Precision, 1, 3> xBar = Eigen::Matrix<Precision, Eigen::Dynamic, 1>::Ones(nHits, 1).transpose() * W * X / traceOfW;
  Eigen::Matrix<Precision, 3, 3> Vx = X.transpose() * W * X - xBar.transpose() * xBar * traceOfW;

  // Find eigenvector to smallest eigenvalue
  Eigen::EigenSolver<Eigen::Matrix<Precision, 3, 3>> eigencollection(Vx);
  Eigen::Matrix<Precision, 3, 1> eigenvalues = eigencollection.eigenvalues().real();
  Eigen::Matrix<std::complex<Precision>, 3, 3> eigenvectors = eigencollection.eigenvectors();
  Eigen::Matrix<Precision, 3, 1>::Index minRow, minCol;
  eigenvalues.minCoeff(&minRow, &minCol);

  Eigen::Matrix<Precision, 3, 1> n = eigenvectors.col(minRow).real();

  // Calculate results with this eigenvector
  Precision c = - xBar * n;
  Precision x0 = - 0.5 * n(0) / n(2);
  Precision y0 = - 0.5 * n(1) / n(2);
  Precision rho2 = (1 - n(2) * (n(2) + 4 * c)) / (4 * n(2) * n(2));
  Precision rho = sqrt(rho2);

  // Calculation of 3 different versions of a distance d for Chi Squared calculation
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> d = Eigen::Matrix<Precision, Eigen::Dynamic, 1>::Ones(nHits, 1) * c + X * n;
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> d_trans = (d + d.cwiseProduct(X.col(2))) / sqrt(1 - n(2) * n(2));
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> d_r_phi = d_trans.cwiseQuotient((0.5 * X.col(2) / rho).array().asin().cos().matrix());

  // Calculate Chi Squared for circle fit
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> d_over_sigma = W * d_trans;
  Precision chi2 = Eigen::Matrix<Precision, Eigen::Dynamic, 1>::Ones(nHits,
                   1).transpose() * (d_over_sigma.cwiseProduct(d_over_sigma));
  std::cout << "Chi Squared of Riemann Circle fit = " << chi2 << std::endl;


  // Line Fit for extension to Helix Fit
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> a = Eigen::Matrix<Precision, Eigen::Dynamic, 1>::Ones(nHits, 1) * c + n(2) * X.col(2);
  Precision b = n(0) * n(0) + n(1) * n(1);
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> underRoot = b * X.col(2) - a.cwiseProduct(a);
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> root = underRoot.cwiseSqrt();

  Eigen::Matrix<Precision, Eigen::Dynamic, 1> x_pos = (- n(0) * a + n(1) * root) / b;
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> x_neg = (- n(0) * a - n(1) * root) / b;

  Eigen::Matrix<Precision, Eigen::Dynamic, 1> y_pos = (- n(1) * a - n(0) * root) / b;
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> y_neg = (- n(1) * a + n(0) * root) / b;

  // find the correct intersection point (TODO: There might be room for simplification here, because the
  // selection does not have to be coefficientwise!)
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> dist_pos = (x_pos - X.col(0)).cwiseProduct(x_pos - X.col(0)) + (y_pos - X.col(
                                                           1)).cwiseProduct(y_pos - X.col(1));
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> dist_neg = (x_neg - X.col(0)).cwiseProduct(x_neg - X.col(0)) + (y_neg - X.col(
                                                           1)).cwiseProduct(y_neg - X.col(1));

  Eigen::Matrix<Precision, Eigen::Dynamic, 1> x_s = (dist_pos.cwiseEqual(dist_pos.cwiseMin(dist_neg))).select(x_pos, x_neg);
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> y_s = (dist_pos.cwiseEqual(dist_pos.cwiseMin(dist_neg))).select(y_pos, y_neg);

  // Arc length calculation
  Precision x_first = X.col(0)(0) - x0;
  Precision y_first = X.col(1)(0) - y0;
  Precision r_mag_first = sqrt(x_first * x_first + y_first * y_first);
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> x0s = Eigen::Matrix<Precision, Eigen::Dynamic, 1>::Ones(nHits, 1) * x0;
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> y0s = Eigen::Matrix<Precision, Eigen::Dynamic, 1>::Ones(nHits, 1) * y0;
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> r_mags = ((X.col(0) - x0s).cwiseProduct(X.col(0) - x0s) + (X.col(1) - y0s).cwiseProduct(
                                                          X.col(1) - y0s)).cwiseSqrt();

  Eigen::Matrix<Precision, Eigen::Dynamic, 1> arc_angles = (x_first * (X.col(0) - x0s) + y_first * (X.col(1) - y0s)).cwiseQuotient(
                                                             r_mags) / r_mag_first;
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> arc_lengths = rho * arc_angles.array().acos().matrix();

  Eigen::Matrix<Precision, Eigen::Dynamic, 2> A = Eigen::Matrix<Precision, Eigen::Dynamic, 2>::Ones(nHits, 2);
  arc_lengths(0) = 0.;
  A.col(1) = arc_lengths;

  // Linear regression of z on arg_lengths with weight matrix Wz
  Eigen::Matrix<Precision, 2, 1> p = (A.transpose() * Wz * A).inverse() * A.transpose() * Wz * Z;

  // Construction of momentum vector with innermost hit and reconstructed circle center
  Eigen::Matrix<Precision, 3, 1> momVec = Eigen::Matrix<Precision, 3, 1>::Zero();
  momVec(0) = y0 - X.col(1)(nHits - 1);
  momVec(1) = - (x0 - X.col(0)(nHits - 1));

  Precision pT = Precision(calcPt(rho));
  momVec = pT * momVec.normalized();

  Eigen::Matrix<Precision, 3, 1> vec01 = X.row(nHits - 2) - X.row(nHits - 1);
  vec01(2) = Z(nHits - 2) - Z(nHits - 1);
  Precision angle01 = std::acos(vec01.dot(momVec) / momVec.norm() / vec01.norm());
  if (angle01 > 0.5 * M_PI) { momVec *= -1.; }

  // Calculation of a chi2 distributed quantity for the quality of fit of the z component fit.
  Eigen::Matrix<Precision, Eigen::Dynamic, 1> ones = Eigen::Matrix<Precision, Eigen::Dynamic, 1>::Ones(nHits, 1);
  Precision chi2_z = ((Z - p(0) * ones - p(1) * arc_lengths).cwiseQuotient(Wz * ones)).transpose() * ((Z - p(0) * ones - p(
                       1) * arc_lengths).cwiseQuotient(Wz * ones));

  std::cout << "Chi Squared of Riemann z-component fit = " << chi2_z << std::endl;

  Precision pZ = pT * p(1);
  momVec(2) = - pZ;

  return std::make_pair(rho, TVector3(momVec(0), momVec(1), momVec(2)));
}




std::pair<double, TVector3> QualityEstimators::helixFit(const std::vector<PositionInfo*>* hits, bool useBackwards,
                                                        double setMomentumMagnitude)
{
  if (hits == NULL) { B2FATAL(" QualityEstimators::circleFit hits not set, therefore no calculation possible - please check that!"); }

  int nHits = hits->size();

  if (nHits < 3) { B2ERROR(" QualityEstimators::circleFit number of hits too low: " << nHits << " hits, therefore no useful calculation possible - please check that!"); }

  // WARNING this function assumes that hits are sorted and first hit is innermost hit!!!
  /** TODO:
   * current implementations lacks of style. This approach would be perfect for a vectorized approach
   * still missing:
   * - chi2 calculation currently returns -1, chi2 of circleFit and of lineFit has to be combined
   * - faster implementation (do vectorized)
   * - dealing with multiple scattering (Rudi will do an additional momentum-based implementation dealing with that)
   * - implementing Info into tuning parameters
   * - detailed check whether results are okay
   * */

  double  x = 0,
          y = 0,
          z = 0,
          invVarZ = 0, // inverse variance of Z
          r2 = 0, // radius^2
          sumWeights = 0, // the sum of the weightsXY
          inverseVarianceXY = 0; // current inverse of varianceXY

  TMatrixD inverseCovMatrix(nHits, nHits); // carries inverse of the variances for the circle fit in its diagonal elements
  TMatrixD X(nHits, 3); // carries mapped hits, column 0 = x variables, column 1 = y variables, col 2 = r2 variables
  TMatrixD onesC(nHits, 1); // column vector of ones
  TMatrixD onesR(1, nHits); // row vector of ones
  TMatrixD R2(nHits, 1); // column vector of radii^2
  TMatrixD zValues(nHits, 1); // column vector of z values
  TMatrixD invVarZvalues(nHits, 1); // carries inverse of the variances for the line fit

  int index = 0;

  TVector3 seedHit = (*hits).at(0)->hitPosition;
  TVector3 secondHit = (*hits).at(1)->hitPosition; // need this one for definition of direction of flight for the particle
  if (useBackwards == false) { seedHit = (*hits).at(nHits - 1)->hitPosition; secondHit = (*hits).at(nHits - 2)->hitPosition; } // want innermost hit

  for (PositionInfo* hit : *hits) {
    x = hit->hitPosition.X();
    y = hit->hitPosition.Y();
    z = hit->hitPosition.Z();
    invVarZ = 1. / hit->hitSigma.Z();
    if (std::isnan(invVarZ) == true or std::isinf(invVarZ) == true) { B2ERROR("QualityEstimators::helixFit, chosen varZ is 'nan': " << invVarZ << ", setting arbitrary error: " << 0.000001 << ")"); invVarZ = 0.000001; }
    invVarZvalues(index, 0) = invVarZ;
    B2DEBUG(75, "helixFit: hit.X(): " << hit->hitPosition.X() << ", hit.Y(): " << hit->hitPosition.Y() << ", hit.Z(): " <<
            hit->hitPosition.Z() << ", hit.sigmaU: " << hit->sigmaU << ", hit.sigmaV: " << hit->sigmaV << ", hit.hitSigma X/Y/Z: " <<
            hit->hitSigma.X() << "/" << hit->hitSigma.Y() << "/" << hit->hitSigma.Z());

    r2 = x * x + y * y;
    inverseVarianceXY = 1. / sqrt(hit->hitSigma.X() * hit->hitSigma.X() + hit->hitSigma.Y() * hit->hitSigma.Y());
    if (std::isnan(inverseVarianceXY) == true or std::isinf(inverseVarianceXY) == true) { B2ERROR("QualityEstimators::helixFit, chosen inverseVarianceXY is 'nan': " << inverseVarianceXY << ", setting arbitrary error: " << 0.000001 << ")"); inverseVarianceXY = 0.000001; }
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


  bool didNanAppear = false;

  /** Checking if nan values in TMatrixDs */
  auto lambdaCheckMatrix4NAN = [](TMatrixD & aMatrix) -> bool {
    double totalEntries = 0;
    for (int i = 0; i < aMatrix.GetNrows(); ++i)
    {
      for (int j = 0; j < aMatrix.GetNcols(); ++j) {
        totalEntries += aMatrix(i, j);
      }
    }
    return std::isnan(totalEntries);
  };

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
  if (minValueIndex < 0) { B2FATAL("QualityEstimators::helixFit produced eigenValue smaller than 0 (" << minValue << ")!");}

  double distanceOfPlane = 0;
  for (int i = 0; i < nEVs; ++i) {// calculating scalar product by hand
    distanceOfPlane += eigenVectors(i, minValueIndex) * xBar(0, i);
  }
  distanceOfPlane *= -1.;

  double n1 = eigenVectors(0, minValueIndex),
         n2 = eigenVectors(1, minValueIndex),
         n3 = eigenVectors(2, minValueIndex);

  /** In the case of a straight line, the HelixFit can not work. So we check if the plane is "straight up", or the z-normal vector is zero, n3 has the unit of cm(?) */
  if (fabs(n3) < 1e-06) { throw FilterExceptions::Straight_Line(); } /// TODO WARNING: this value for catching straight lines is hardcoded: its resolution should be finer than the possible resolution of the detectors (we assume that the unit is cm)

  /** In the case of the fitted plane being parallel to the x-y plane, helixFit produces a nan pZ. TODO Why */
  if (fabs(n1) < 1e-10 && fabs(n2) < 1e-10) { throw FilterExceptions::Center_Is_Origin(); }

  double a = 1. / (2.*n3); // TODO temporary value

  double xc = -n1 * a; // x coordinate of the origin of the circle
  double yc = -n2 * a; // y coordinate of the origin of the circle

  double rho = sqrt((1. - n3 * n3 - 4.*distanceOfPlane * n3) * (a * a));

  B2DEBUG(25, "helixFit: circle: origin x: " << xc << ", y: " << yc << ", radius: " << rho  << std::endl);

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
        B2DEBUG(10, "The following hits were part of this TC: \n" << printHits(m_hits));
      }

      T(k, 0) = 0;
    }
  }

  T.Sqrt();

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

    s(i, 0) = rho * acos(((radiusX * radiusXb + radiusY * radiusYb) / radiusMag) / radiusMagb);

    if (std::isnan(s(i, 0)) == true) {
      didNanAppear = true;
      B2DEBUG(3, "helixFit: i: " << i << ", s(i) = 'nan', components - rho: " << rho << ", radiusX: " << radiusX << ", radiusY: " <<
              radiusY << ", radiusXb: " << radiusXb << ", radiusYb: " << radiusYb << ", invRadiusMag: " << invRadiusMag << ", radiusMagb: " <<
              radiusMagb << ", xs(i): " << xs(i, 0) << ", ys(i): " << ys(i, 0));
    }
  }

  // Linear Regression for the fit in z direction
  TMatrixD AtGA(2, 2);
  TMatrixD AtG(2, nHits);
  TMatrixD Diag(nHits, nHits);
  double sumWi = 0, sumWiSi = 0, sumWiSi2 = 0, sw = 0;
  for (int i = 0; i < nHits; ++i) {
    sumWi += invVarZvalues(i, 0);
    sw = invVarZvalues(i, 0) * s(i, 0);
    sumWiSi += sw;
    sumWiSi2 += invVarZvalues(i, 0) * s(i, 0) * s(i, 0);
    AtG(0, i) = invVarZvalues(i, 0);
    AtG(1, i) = sw;
    for (int j = 0; j < nHits; ++j) { Diag(i, j) = 0.; }
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

  TMatrixD p = AtGAInv * AtG * zValues; // fitted z value in the first point, tan(lambda)
  if (lambdaCheckMatrix4NAN(p) == true) { B2DEBUG(10, "helixFit: p got 'nan'-entries!"); }

  double thetaVal = (M_PI * 0.5 - atan(p(1, 0)));

  if (std::isnan(thetaVal) == true) {
    didNanAppear = true;
    thetaVal = (hits->at(0)->hitPosition - hits->at(nHits - 1)->hitPosition).Theta(); /// INFO swapped! feb4th2014
    B2DEBUG(3, "helixFit: calculating theta for momentum produced 'nan' -> fallback-solution produces theta: " << thetaVal);
    if (std::isnan(thetaVal) == true) { B2ERROR("helixFit: no usable Theta value could be produced -> serious error telling us that helix fit does not work! bypass is setting the value to 0!"); thetaVal = 0; }
  }

  TVector3 radialVector(xc, yc, 0.); // here it is the center of the circle
  radialVector = radialVector - seedHit; // now it's the radialVector
  radialVector.SetZ(0.);
  double pT = calcPt(rho);
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

  double pZ = calcPt(rho) * p(1, 0) ;

  B2DEBUG(25, "helixFit: radius(rho): " << rho << ", theta: " << thetaVal << ", pT: " << pT << ", pZ: " << pZ << ", pVector.Perp: " <<
          pVector.Perp() << ", pVector.Mag: " << pVector.Mag() << ", fitted zValue: " << p(0, 0));
  TVector3 vectorToSecondHit = secondHit - seedHit;
  vectorToSecondHit.SetZ(0);

  if (((useBackwards == true) && (vectorToSecondHit.Angle(pVector) < M_PI * 0.5)) || ((useBackwards == false)
      && (vectorToSecondHit.Angle(pVector) > M_PI * 0.5))) { pVector *= -1.; }
  pVector.SetZ(-pZ); // now that track carries full momentum

  if (lambdaCheckVector4NAN(pVector) == true) { B2ERROR("helixFit: pVector got 'nan'-entries x/y/z: " << pVector.X() << "/" << pVector.Y() << "/" << pVector.Z()); didNanAppear = true; }

  if (didNanAppear == true && LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1, PACKAGENAME()) == true) {
    B2DEBUG(3, "helixFit: there was a 'nan'-value detected. When using magnetic field of " << getMagneticField() <<
            ", the following hits were part of this TC: \n" << printHits(m_hits) << "\n pVector  x/y/z: " << pVector.X() << "/" << pVector.Y()
            << "/" << pVector.Z());
  }

  if (std::isnan(rho) == true or lambdaCheckVector4NAN(pVector) == true) {
    throw FilterExceptions::Invalid_result_Nan();
  }

  return std::make_pair(rho, pVector);
}


bool QualityEstimators::CalcCurvature()
{
  if (m_hits == NULL)
    B2FATAL(" QualityEstimators::CalcCurvature: hits not set, therefore no calculation possible - please check that!");
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


std::pair<double, TVector3> QualityEstimators::simpleLineFit3D(const std::vector<PositionInfo*>* hits, bool useBackwards,
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
  double Wyi = 0, // weight for Yi
         Wzi = 0, // weight for Zi
         sumWyi = 0, // sum of weights for Yi
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
    Wyi = (1. / (aHit->hitSigma.Y() * aHit->hitSigma.Y()));
    Wzi = (1. / (aHit->hitSigma.Z() * aHit->hitSigma.Z()));

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

  return std::make_pair(chi2, directionVector);
}


std::string QualityEstimators::printHits(const std::vector<PositionInfo*>* hits) const
{
  std::stringstream hitX, hitY, hitZ, sigmaX, sigmaY, sigmaZ;
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
