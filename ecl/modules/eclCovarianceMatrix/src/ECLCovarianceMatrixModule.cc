/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module calculates the covariance matrix for a shower.             *
 * The matrix will depend on the shower region (FWD, Bartel, BWD) and     *
 * possibly on the hypothesis.                                            *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclCovarianceMatrix/ECLCovarianceMatrixModule.h>

// FRAMEWORK
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ECL
#include <ecl/dataobjects/ECLShower.h>

// ROOT
#include <TVector3.h>
#include <TMatrixFSym.h>
#include <TMath.h>

// NAMESPACES
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLCovarianceMatrix)
REG_MODULE(ECLCovarianceMatrixPureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
ECLCovarianceMatrixModule::ECLCovarianceMatrixModule() : Module()
{
  // Set description
  setDescription("ECLCovarianceMatrix: Calculates ECL shower covariance matrix");
  setPropertyFlags(c_ParallelProcessingCertified);

}

ECLCovarianceMatrixModule::~ECLCovarianceMatrixModule()
{
}

void ECLCovarianceMatrixModule::initialize()
{
  // Register in datastore
  StoreArray<ECLShower> eclShowers(eclShowerArrayName());
  eclShowers.registerInDataStore();
}

void ECLCovarianceMatrixModule::beginRun()
{
  // Do not use this for Database updates, they will not follow the concept of a "run"
  ;
}

void ECLCovarianceMatrixModule::event()
{
  // Input array
  StoreArray<ECLShower> eclShowers(eclShowerArrayName());

  // loop over all ECLShowers
  for (auto& eclShower : eclShowers) {

    // Get the corrected and calibrated values for E, theta and phi.
    // For now use the existing class members.
    const double energy = eclShower.getEnergy();
    const double theta  = eclShower.getTheta();

    // Only diagonal elements are filled - this needs to be checked (TF)
    // That means E, theta and phi are assumed to be ~uncorrelated.
    // Note that does not mean that px, py, pz and E are uncorrelated!
    float ErrorMatrix[3] = {
      static_cast<float>(errorEnergy(energy)),
      static_cast<float>(errorTheta(energy, theta)),
      static_cast<float>(errorPhi(energy, theta))
    };

    eclShower.setError(ErrorMatrix);

  }

}

void ECLCovarianceMatrixModule::endRun()
{
  ;
}

void ECLCovarianceMatrixModule::terminate()
{
  ;
}

// ----------------------------------------------------------------
// Px
double ECLCovarianceMatrixModule::Px(double energy, double theta, double phi)
{
  return energy * sin(theta) * cos(phi);
}

// Py
double ECLCovarianceMatrixModule::Py(double energy, double theta, double phi)
{
  return energy * sin(theta) * sin(phi);
}

// Pz
double ECLCovarianceMatrixModule::Pz(double energy, double theta)
{
  return energy * cos(theta);
}

// Fill here Error on Energy
double ECLCovarianceMatrixModule::errorEnergy(double energy)
{
  const double sigmaE = 0.01 * (0.047 / energy + 1.105 / sqrt(sqrt(energy)) + 0.8563) * energy;
  //double sigmaE = 0.01 * E * sqrt(squ(0.066 / E) + squ(0.81) / sqrt(E) + squ(1.34)) ;
  //sigmaE / E = 0.066% / E +- 0.81% / (E)^(1/4)  +- 1.34%//NIM A441, 401(2000)
  return sigmaE;
}

// Fill here Error on Theta
double ECLCovarianceMatrixModule::errorTheta(double energy, double theta)
{
  const double sigmaX = 0.1 * (0.27 + 3.4 / sqrt(energy) + 1.8 / sqrt(sqrt(energy))) ;
  //sigmaX (mm)=  0.27 + 3.4/ (E)^(1/2) + 1.8 / (E)^(1/4) ;E (GeV)

  const double zForward  =  196.2;
  const double zBackward = -102.2;
  const double Rbarrel   =  125.0;

  const double theta_f = atan2(Rbarrel, zForward);
  const double theta_b = atan2(Rbarrel, zBackward);

  if (theta < theta_f) {
    return sigmaX * squ(cos(theta)) / zForward;
  } else if (theta > theta_b) {
    return sigmaX * squ(cos(theta)) / (-1 * zBackward);
  } else {
    return sigmaX * sin(theta) / Rbarrel;
  }
}

// Fill here Error on Phi
double ECLCovarianceMatrixModule::errorPhi(double energy, double theta)
{
  double sigmaX = 0.1 * (0.27 + 3.4 / sqrt(energy) + 1.8 / sqrt(sqrt(energy))) ;
  //sigmaX (mm)=  0.27 + 3.4/ (E)^(1/2) + 1.8 / (E)^(1/4) ;E (GeV)

  const double zForward  =  196.2;
  const double zBackward = -102.2;
  const double Rbarrel   =  125.0;

  const double theta_f = atan2(Rbarrel, zForward);
  const double theta_b = atan2(Rbarrel, zBackward);

  if (theta < theta_f) {
    return sigmaX / (zForward * tan(theta));
  } else if (theta > theta_b) {
    return sigmaX / (zBackward * tan(theta));
  } else {
    return sigmaX / Rbarrel;
  }
}

// For filling error matrix on Px,Py and Pz here : Vishal
void ECLCovarianceMatrixModule::readErrorMatrix(double energy, double theta,
                                                double phi, TMatrixFSym& errorMatrix)
{
  const double energyError = ECLCovarianceMatrixModule::errorEnergy(energy);
  const double thetaError =  ECLCovarianceMatrixModule::errorTheta(energy, theta);
  const double phiError  =   ECLCovarianceMatrixModule::errorPhi(energy, theta);

  TMatrixFSym  errEcl(3);   // 3x3 initialize to zero
  errEcl[0][0] = energyError * energyError; // Energy
  errEcl[1][0] = 0;
  errEcl[1][1] = phiError * phiError; // Phi
  errEcl[2][0] = 0;
  errEcl[2][0] = 0;
  errEcl[2][1] = 0;
  errEcl[2][2] = thetaError * thetaError; // Theta

  TMatrixF  jacobian(4, 3);
  const double cosPhi = cos(phi);
  const double sinPhi = sin(phi);
  const double cosTheta = cos(theta);
  const double sinTheta = sin(theta);
  const double E = energy;

  jacobian[0][0] =            cosPhi * sinTheta;
  jacobian[0][1] = -1.0 * E * sinPhi * sinTheta;
  jacobian[0][2] =        E * cosPhi * cosTheta;
  jacobian[1][0] =            sinPhi * sinTheta;
  jacobian[1][1] =        E * cosPhi * sinTheta;
  jacobian[1][2] =        E * sinPhi * cosTheta;
  jacobian[2][0] =                     cosTheta;
  jacobian[2][1] =                          0.0;
  jacobian[2][2] = -1.0 * E          * sinTheta;
  jacobian[3][0] =                          1.0;
  jacobian[3][1] =                          0.0;
  jacobian[3][2] =                          0.0;
  TMatrixFSym errCart(4);
  errCart = errEcl.Similarity(jacobian);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j <= i ; j++) {
      errorMatrix[i][j] = errCart[i][j];
    }
  }
}

// For filling error matrix on Px,Py and Pz here 7x 7
void ECLCovarianceMatrixModule::readErrorMatrix7x7(double energy, double theta,
                                                   double phi, TMatrixFSym& errorMatrix)
{
  const double energyError = ECLCovarianceMatrixModule::errorEnergy(energy);
  const double thetaError  = ECLCovarianceMatrixModule::errorTheta(energy, theta);
  const double phiError    = ECLCovarianceMatrixModule::errorPhi(energy, theta);

  TMatrixFSym  errEcl(3);   // 3x3 initialize to zero
  errEcl[0][0] = energyError * energyError; // Energy
  errEcl[1][0] = 0;
  errEcl[1][1] = phiError * phiError; // Phi
  errEcl[2][0] = 0;
  errEcl[2][0] = 0;
  errEcl[2][1] = 0;
  errEcl[2][2] = thetaError * thetaError; // Theta

  TMatrixF jacobian(4, 3);
  const double cosPhi = cos(phi);
  const double sinPhi = sin(phi);
  const double cosTheta = cos(theta);
  const double sinTheta = sin(theta);
  const double E = energy;

  jacobian[0][0] =            cosPhi * sinTheta;
  jacobian[0][1] = -1.0 * E * sinPhi * sinTheta;
  jacobian[0][2] =        E * cosPhi * cosTheta;
  jacobian[1][0] =            sinPhi * sinTheta;
  jacobian[1][1] =        E * cosPhi * sinTheta;
  jacobian[1][2] =        E * sinPhi * cosTheta;
  jacobian[2][0] =                     cosTheta;
  jacobian[2][1] =                          0.0;
  jacobian[2][2] = -1.0 * E          * sinTheta;
  jacobian[3][0] =                          1.0;
  jacobian[3][1] =                          0.0;
  jacobian[3][2] =                          0.0;
  TMatrixFSym errCart(4);
  errCart = errEcl.Similarity(jacobian);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j <= i ; j++) {
      errorMatrix[i][j] = errCart[i][j];
    }
  }
  for (int i = 4; i <= 6; ++i) {
    errorMatrix[i][i] = 1.0;
  }

}
