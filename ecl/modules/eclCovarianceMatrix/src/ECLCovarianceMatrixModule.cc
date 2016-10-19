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
#include <framework/datastore/StoreArray.h>

// ECL
#include <ecl/dataobjects/ECLShower.h>

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
ECLCovarianceMatrixModule::ECLCovarianceMatrixModule() : Module(), m_eclShowers(eclShowerArrayName())
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
  m_eclShowers.registerInDataStore(eclShowerArrayName());
}

void ECLCovarianceMatrixModule::beginRun()
{
  // Do not use this for Database updates, they will not follow the concept of a "run"
  ;
}

void seterrormatrix(ECLShower& eclShower)
{
  // Get the corrected and calibrated values for E, theta and phi.
  // For now use the existing class members.
  const double energy = eclShower.getEnergy();
  const double theta  = eclShower.getTheta();

  // Only diagonal elements are filled - this needs to be checked (TF)
  // That means E, theta and phi are assumed to be ~uncorrelated.
  // Note that does not mean that px, py, pz and E are uncorrelated!
  const double iE = 1.0 / energy;
  const double iE12 = sqrt(iE);
  const double iE14 = sqrt(iE12);
  const double sigmaE = 0.01 * (0.047 * iE + 1.105 * iE14 + 0.8563) * energy;
  const double sigmaX = 0.1 * (0.27 + 3.4 * iE12 + 1.8 * iE14);

  const double zForward = 196.2;
  const double zBackward = -102.2;
  const double Rbarrel = 125.0;

  const double theta_f = atan2(Rbarrel, zForward);
  const double theta_b = atan2(Rbarrel, zBackward);
  double sigmaPhi = 1.0 / Rbarrel;

  if (theta < theta_f) {
    sigmaPhi = (1 / zForward) / tan(theta);
  } else if (theta > theta_b) {
    sigmaPhi = (1 / zBackward) / tan(theta);
  }
  sigmaPhi *= sigmaX;

  double sigmaTheta = sigmaPhi * sin(theta);

  double covMatrix[6] = {sigmaE * sigmaE, 0.0, sigmaPhi * sigmaPhi, 0.0, 0.0, sigmaTheta * sigmaTheta};

  eclShower.setCovarianceMatrix(covMatrix);
}

void ECLCovarianceMatrixModule::event()
{
  // loop over all ECLShowers
  TClonesArray* ca = m_eclShowers.getPtr();
  TObject** a = ca->GetObjectRef();
  for (int i = 0, imax = ca->GetEntries(); i < imax; i++)
    seterrormatrix(*static_cast<ECLShower*>(a[i]));

}

void ECLCovarianceMatrixModule::endRun()
{
  ;
}

void ECLCovarianceMatrixModule::terminate()
{
  ;
}
