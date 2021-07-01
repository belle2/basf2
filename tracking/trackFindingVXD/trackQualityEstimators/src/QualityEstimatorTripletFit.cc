/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Felix Metzner, Jonas Wagner                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <math.h>
#include <TMath.h>

#include <framework/gearbox/Const.h>

#include <svd/geometry/SensorInfo.h>

#include "tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h"
#include <tracking/trackFindingVXD/utilities/CalcCurvatureSignum.h>


using namespace Belle2;

double QualityEstimatorTripletFit::estimateQuality(std::vector<SpacePoint const*> const& measurements)
{
  const int nTriplets = measurements.size() - 2;

  if (nTriplets < 1) return 0;

  double combinedChi2 = 0.;

  // values required for pt calculation
  m_R3Ds.clear();
  m_R3Ds.reserve(nTriplets);
  m_sigmaR3DSquareds.clear();
  m_sigmaR3DSquareds.reserve(nTriplets);
  m_alphas.clear();
  m_alphas.reserve(nTriplets);
  m_thetas.clear();
  m_thetas.reserve(nTriplets);

  // looping over all triplets
  for (int i = 0; i < nTriplets; i++) {

    // Three hits relevant for curent triplet
    const B2Vector3D& hit0 = measurements.at(i)->getPosition();
    const B2Vector3D& hit1 = measurements.at(i + 1)->getPosition();
    const B2Vector3D& hit2 = measurements.at(i + 2)->getPosition();

    const double d01sq = pow(hit1.X() - hit0.X(), 2) + pow(hit1.Y() - hit0.Y(), 2);
    const double d12sq = pow(hit2.X() - hit1.X(), 2) + pow(hit2.Y() - hit1.Y(), 2);
    const double d02sq = pow(hit2.X() - hit0.X(), 2) + pow(hit2.Y() - hit0.Y(), 2);

    const double d01 = sqrt(d01sq);
    const double d12 = sqrt(d12sq);
    const double d02 = sqrt(d02sq);

    const double z01 = hit1.Z() - hit0.Z();
    const double z12 = hit2.Z() - hit1.Z();

    // equation (8)
    const double R_C = (d01 * d12 * d02) / sqrt(-d01sq * d01sq - d12sq * d12sq - d02sq * d02sq +
                                                2 * d01sq * d12sq + 2 * d12sq * d02sq + 2 * d02sq * d01sq);

    // equations (9)
    const double Phi1C = 2. * asin(d01 / (2. * R_C));
    const double Phi2C = 2. * asin(d12 / (2. * R_C));
    // TODO Phi1C and Phi2C have 2 solutions (<Pi and >Pi), each, of which the correct one must be chosen!

    // equations (10)
    const double R3D1C = sqrt(R_C * R_C + (z01 * z01) / (Phi1C * Phi1C));
    const double R3D2C = sqrt(R_C * R_C + (z12 * z12) / (Phi2C * Phi2C));

    // equations (11)
    const double theta1C = acos(z01 / (Phi1C * R3D1C));
    const double theta2C = acos(z12 / (Phi2C * R3D2C));
    const double theta = (theta1C + theta2C) / 2.;

    // equations (15)
    double alpha1 = R_C * R_C * Phi1C * Phi1C + z01 * z01;
    alpha1 *= 1. / (0.5 * R_C * R_C * Phi1C * Phi1C * Phi1C / tan(Phi1C / 2.) + z01 * z01);
    double alpha2 = R_C * R_C * Phi2C * Phi2C + z12 * z12;
    alpha2 *= 1. / (0.5 * R_C * R_C * Phi2C * Phi2C * Phi2C / tan(Phi2C / 2.) + z12 * z12);

    // equation (18)
    const double PhiTilde = - 0.5 * (Phi1C * alpha1 + Phi2C * alpha2);
    // equation (19)
    const double eta = 0.5 * (Phi1C * alpha1 / R3D1C + Phi2C * alpha2 / R3D2C);

    const double tanTheta1C = tan(theta1C);
    const double tanTheta2C = tan(theta2C);
    // equation (21)
    const double ThetaTilde = theta2C - theta1C - (1 - alpha2) / tanTheta2C + (1 - alpha1) / tanTheta1C;
    // equation (22)
    const double beta = (1 - alpha2) / (R3D2C * tanTheta2C) - (1 - alpha1) / (R3D1C * tanTheta1C);

    // Calculation of sigmaMS
    // First get the orientation of the sensor plate for material budged calculation
    double entranceAngle = theta;
    VxdID::baseType sensorID = measurements.at(i + 1)->getVxdID();
    int detID = measurements.at(i + 1)->getType();
    if (sensorID != 0 and detID == VXD::SensorInfoBase::SVD) {
      const SVD::SensorInfo& sensor = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
      const B2Vector3D& sensorOrigin  = sensor.pointToGlobal(B2Vector3D(0, 0, 0), true);
      const B2Vector3D& sensoru  = sensor.pointToGlobal(B2Vector3D(1, 0, 0), true);
      const B2Vector3D& sensorv  = sensor.pointToGlobal(B2Vector3D(0, 1, 0), true);

      B2Vector3D globalu = sensoru - sensorOrigin;
      B2Vector3D globalv = sensorv - sensorOrigin;
      B2Vector3D normal = globalu.Cross(globalv);

      // Calculate the angle of incidence for the middle hit
      if (sensorOrigin.Angle(normal) > M_PI * 0.5) { normal *= -1.; }
      entranceAngle = (hit1 - hit0).Angle(normal);
    }

    /** Using average material budged of SVD sensors for approximation of radiation length
     *  Belle II TDR page 156 states a value of 0.57% X_0.
     *  This approximation is a first approach to the problem and must be checked.
     */
    const double XoverX0 = m_materialBudgetFactor * 0.0057 / cos(entranceAngle);

    const double sinTheta = sin(theta);
    // equation (23)
    double R3D = - (eta * PhiTilde * sinTheta * sinTheta + beta * ThetaTilde);
    R3D *= 1. / (eta * eta * sinTheta * sinTheta + beta * beta);
    const double b = 4.5 / m_magneticFieldZ * sqrt(XoverX0);

    // Calculation of maximal 3D radius from a maximal p_t cut off value
    // (which is a hyper parameter of the Triplet Fit QE) via conversion
    // using the magnetic field at the origin and the speed of light in the
    // default units cm/ns times 1e-4 due to the units of the magnetic field.
    double R3DmaxCut = m_maxPt / (m_magneticFieldZ * 1e-4 * Const::speedOfLight);
    double R3D_truncated = R3D > R3DmaxCut ? R3DmaxCut : R3D;
    const double sigmaMS = b / R3D_truncated;

    // equation (24)
    double Chi2min = pow(beta * PhiTilde - eta * ThetaTilde, 2);
    Chi2min *= 1. / (sigmaMS * sigmaMS * (eta * eta + beta * beta / (sinTheta * sinTheta)));

    // equation (25)
    double sigmaR3DSquared = sigmaMS * sigmaMS / (eta * eta * sinTheta * sinTheta + beta * beta);

    // bias correction as proposed in section 2.4 of the paper describing this fit. (see above)
    // equation (30)
    double delta = (beta * PhiTilde - eta * ThetaTilde) / (eta * PhiTilde * sinTheta * sinTheta + beta * ThetaTilde);
    if (8 * delta * delta * sinTheta * sinTheta <= 1) {
      // equation (29), the first part is the same as in equation (23)
      R3D *= (0.75 + sqrt(1 - 8 * delta * delta * sinTheta * sinTheta) / 4.);
    }

    // required for optional pt calculation
    m_thetas.push_back(theta);
    m_alphas.push_back((alpha1 + alpha2) / 2.);

    // store values for combination
    m_R3Ds.push_back(R3D);
    m_sigmaR3DSquareds.push_back(sigmaR3DSquared);

    // equation (39)
    combinedChi2 += Chi2min;
  }

  // Calculate average R3D
  double numerator = 0;
  double denominator = 0;
  // c.f. equation (40)
  for (short i = 0; i < nTriplets; ++i) {
    // What is meant:
    // numerator += pow(m_R3Ds.at(i), 3) / m_sigmaR3DSquareds.at(i);
    // denominator += pow(m_R3Ds.at(i), 2) / m_sigmaR3DSquareds.at(i);
    // To save clock cycles, only calculate R^2 / sigma^2 once, and only multiply with the missing R
    double addValue = m_R3Ds.at(i) * m_R3Ds.at(i) / m_sigmaR3DSquareds.at(i);
    numerator += addValue * m_R3Ds.at(i);
    denominator += addValue;
  }
  m_averageR3D = numerator / denominator;

  // Compare individual R3Ds with average R3D to improve chi2 as presented at:
  // Connecting the Dots, Vienna, Feb 2016 by A. Schoening
  double globalCompatibilityOfR3Ds = 0;
  for (short i = 0; i < nTriplets; ++i) {
    globalCompatibilityOfR3Ds += pow(m_R3Ds.at(i) - m_averageR3D, 2) / m_sigmaR3DSquareds.at(i);
  }

  double const finalChi2 = combinedChi2 + globalCompatibilityOfR3Ds;

  m_results.chiSquared = finalChi2;

  return TMath::Prob(finalChi2, 2 * measurements.size() - 5);
}


QualityEstimationResults QualityEstimatorTripletFit::estimateQualityAndProperties(std::vector<SpacePoint const*> const&
    measurements)
{
  // calculate and store chiSquared(calcChiSquared) and curvature(calcCurvature) in m_reuslts.
  QualityEstimatorBase::estimateQualityAndProperties(measurements);
  if (measurements.size() < 3) return m_results;

  m_results.curvatureSign = calcCurvatureSignum(measurements);

  // calculate pt and pt_sigma
  double averageThetaPrime = 0;
  // equation (43)
  for (unsigned short i = 0; i < m_thetas.size(); ++i) {
    averageThetaPrime += m_thetas.at(i) - (m_averageR3D - m_R3Ds.at(i)) * (1 - m_alphas.at(i)) / (tan(m_thetas.at(i)) * m_R3Ds.at(i));
  }
  averageThetaPrime /= m_thetas.size();

  m_results.pt = calcPt(m_averageR3D * sin(averageThetaPrime));
  m_results.pmag = calcPt(m_averageR3D);
  return m_results;
}

