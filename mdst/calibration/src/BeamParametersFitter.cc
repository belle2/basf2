/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <mdst/calibration/BeamParametersFitter.h>

/* Belle 2 headers. */
#include <framework/database/Database.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBStore.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

/* ROOT headers. */
#include <TLorentzVector.h>
#include <TMinuit.h>
#include <TVectorD.h>

using namespace Belle2;

/** Whether to use momentum components or energy and two angles. */
static bool s_UseMomentum;

/** Invariant mass. */
static double s_InvariantMass;

/** Invariant-mass error. */
static double s_InvariantMassError;

/** Boost vector. */
static TVector3 s_BoostVector;

/** Boost-vector covariance. */
static TMatrixDSym s_BoostVectorInverseCovariance(3);

/** HER direction. */
static TVector3 s_DirectionHER;

/** LER direction. */
static TVector3 s_DirectionLER;

/** Angle error. */
static double s_AngleError;

/**
 * Get momentum by energy, x angle, y angle.
 */
static TLorentzVector getMomentum(double energy, double thetaX, double thetaY,
                                  bool ler)
{
  const double pz = std::sqrt(energy * energy -
                              Const::electronMass * Const::electronMass);
  const double sx = sin(thetaX);
  const double cx = cos(thetaX);
  const double sy = sin(thetaY);
  const double cy = cos(thetaY);
  const double px = sy * cx * pz;
  const double py = -sx * pz;
  TLorentzVector result(px, py, cx * cy * pz, energy);
  if (ler)
    result.RotateY(M_PI);
  return result;
}

/* cppcheck-suppress constParameter */
static void fcn(int& npar, double* grad, double& fval, double* par, int iflag)
{
  (void)npar;
  (void)grad;
  (void)iflag;
  TLorentzVector pHER, pLER;
  if (s_UseMomentum) {
    pHER.SetXYZM(par[0], par[1], par[2], Const::electronMass);
    pLER.SetXYZM(par[3], par[4], par[5], Const::electronMass);
  } else {
    pHER = getMomentum(par[0], par[1], par[2], false);
    pLER = getMomentum(par[3], par[4], par[5], true);
  }
  TLorentzVector pBeam = pHER + pLER;
  TVector3 beamBoost = pBeam.BoostVector();
  TVectorD boostDifference(3);
  boostDifference[0] = beamBoost.X() - s_BoostVector.X();
  boostDifference[1] = beamBoost.Y() - s_BoostVector.Y();
  boostDifference[2] = beamBoost.Z() - s_BoostVector.Z();
  double boostChi2 = s_BoostVectorInverseCovariance.Similarity(boostDifference);
  double invariantMass = pBeam.M();
  double massChi2 = pow((invariantMass - s_InvariantMass) /
                        s_InvariantMassError, 2);
  double angleHER = pHER.Vect().Angle(s_DirectionHER);
  double angleLER = pLER.Vect().Angle(s_DirectionLER);
  double angleChi2 = pow(angleHER / s_AngleError, 2) +
                     pow(angleLER / s_AngleError, 2);
  fval = boostChi2 + massChi2 + angleChi2;
}

void BeamParametersFitter::setupDatabase()
{
  /* DataStore. */
  DataStore::Instance().setInitializeActive(true);
  StoreObjPtr<EventMetaData> eventMetaData;
  eventMetaData.registerInDataStore();
  DataStore::Instance().setInitializeActive(false);
  /* Database. */
  if (eventMetaData.isValid()) {
    eventMetaData->setExperiment(m_IntervalOfValidity.getExperimentLow());
    eventMetaData->setRun(m_IntervalOfValidity.getRunLow());
  } else {
    eventMetaData.construct(1, m_IntervalOfValidity.getRunLow(),
                            m_IntervalOfValidity.getExperimentLow());
  }
  DBStore& dbStore = DBStore::Instance();
  dbStore.update();
  dbStore.updateEvent();
}

/*

 In BeamParameters, the momenta are represented as (E, theta_x, theta_y).
The cartesian coordinates of the momenta are given by

 |p_x|   |cos(theta_y)  0 sin(theta_y)|   | 1 0            0             |
 |p_y| = |0             1 0           | * | 0 cos(theta_x) -sin(theta_x) |
 |p_x|   |-sin(theta_y) 0 cos(theta_y)|   | 0 sin(theta_x) cos(theta_x)  |

           | 0 |
         * | 0 | ,
           | p |

or, after, matrix multiplication,

 p_x = p * cos(theta_x) * sin(theta_y) ,
 p_y = - p * sin(theta_x) ,
 p_z = p * cos(theta_x) * cos(theta_y) .

 The block form of the full covariance matrix is

    | V_HER | 0     |
V = |-------|-------| ,
    | 0     | V_LER |

where V_HER and V_LER are the covariance matrices of high-energy and
low-energy beam momenta. The directions are assumed to be known exactly,
thus, the covariance matrix has only two non-zero elements:
V_11 = sigma_{E_HER}^2 and V_44 = sigma_{E_LER}^2.

 It is necessary to reproduce the measured variance (note that error represents
energy spread rather than the uncertainty of the mean energy) of collision
invariant mass. The corresponding 1x1 error matrix is given by

               | \partial \sqrt{s} |       | \partial \sqrt{s} | ^ T
V_{\sqrt{s}} = | ----------------- | * V * | ----------------- |     .
               | \partial p_i      |       | \partial p_i      |

Since there are only two non-zero elements, this formula reduces to

                      | \partial \sqrt{s} | ^ 2
\sigma^2_{\sqrt{s}} = | ----------------- |     * sigma_{E_HER}^2
                      | \partial E_HER    |

                        | \partial \sqrt{s} | ^ 2
                      + | ----------------- |     * sigma_{E_LER}^2 .
                        | \partial E_LER    |

 The derivatives are given by

\partial \sqrt{s}
----------------- =
 \partial E_HER

      1                                                          E_HER
= -------- [ E_beam - (p_beam)_x * cos(theta_x) * sin(theta_y) * -----
  \sqrt{s}                                                       p_HER

                                           E_HER
             + (p_beam)_y * sin(theta_x) * -----
                                           p_HER

                                                          E_HER
             - (p_beam)_z * cos(theta_x) * cos(theta_y) * ----- ],
                                                          p_HER

and by similar formula for E_LER.

 Now it is necessary to make some assumption about the relation between
sigma_{E_HER} and sigma_{E_LER}. It is assumed that sigma_{E_HER} = k E_HER
and sigma_{E_LER} = k E_LER.

*/
void BeamParametersFitter::fit()
{
  int minuitResult;
  setupDatabase();
  /* Get p_HER and p_LER from a fit. */
  double herMomentum, herThetaX, herThetaY;
  double lerMomentum, lerThetaX, lerThetaY;
  s_BoostVector = m_CollisionBoostVector->getBoost();
  s_BoostVectorInverseCovariance = m_CollisionBoostVector->getBoostCovariance();
  if (s_BoostVectorInverseCovariance.Determinant() == 0) {
    B2WARNING("Determinant of boost covariance matrix is 0, "
              "using generic inverse covariance matrix for fit.");
    s_BoostVectorInverseCovariance[0][0] = 1.0 / (m_BoostError * m_BoostError);
    s_BoostVectorInverseCovariance[0][1] = 0;
    s_BoostVectorInverseCovariance[0][2] = 0;
    s_BoostVectorInverseCovariance[1][0] = 0;
    s_BoostVectorInverseCovariance[1][1] = 1.0 / (m_BoostError * m_BoostError);
    s_BoostVectorInverseCovariance[1][2] = 0;
    s_BoostVectorInverseCovariance[2][0] = 0;
    s_BoostVectorInverseCovariance[2][1] = 0;
    s_BoostVectorInverseCovariance[2][2] = 1.0 / (m_BoostError * m_BoostError);
  } else {
    s_BoostVectorInverseCovariance.Invert();
  }
  s_InvariantMass = m_CollisionInvariantMass->getMass();
  s_InvariantMassError = m_CollisionInvariantMass->getMassError();
  if (s_InvariantMassError == 0) {
    B2WARNING("Invariant-mass errror is 0, using generic error for fit.");
    s_InvariantMassError = m_InvariantMassError;
  }
  s_DirectionHER.SetX(0);
  s_DirectionHER.SetY(0);
  s_DirectionHER.SetZ(1);
  s_DirectionHER.RotateY(m_AngleHER);
  s_DirectionLER.SetX(0);
  s_DirectionLER.SetY(0);
  s_DirectionLER.SetZ(1);
  s_DirectionLER.RotateY(m_AngleLER + M_PI);
  s_AngleError = m_AngleError;
  s_UseMomentum = m_UseMomentum;
  TMinuit minuit(6);
  if (!m_Verbose)
    minuit.SetPrintLevel(-1);
  minuit.SetFCN(fcn);
  if (m_UseMomentum) {
    minuit.mnparm(0, "PHER_X", 0, 0.01, 0, 0, minuitResult);
    minuit.mnparm(1, "PHER_Y", 0, 0.01, 0, 0, minuitResult);
    minuit.mnparm(2, "PHER_Z", 7, 0.01, 0, 0, minuitResult);
    minuit.mnparm(3, "PLER_X", 0, 0.01, 0, 0, minuitResult);
    minuit.mnparm(4, "PLER_Y", 0, 0.01, 0, 0, minuitResult);
    minuit.mnparm(5, "PLER_Z", -4, 0.01, 0, 0, minuitResult);
    minuit.mncomd("FIX 1 2 4 5", minuitResult);
    minuit.mncomd("MIGRAD 10000", minuitResult);
    minuit.mncomd("RELEASE 1 2 4 5", minuitResult);
    minuit.mncomd("MIGRAD 10000", minuitResult);
  } else {
    minuit.mnparm(0, "PHER_E", 7, 0.01, 0, 0, minuitResult);
    minuit.mnparm(1, "PHER_TX", 0, 0.01, 0, 0, minuitResult);
    minuit.mnparm(2, "PHER_TY", 0, 0.01, 0, 0, minuitResult);
    minuit.mnparm(3, "PLER_E", 4, 0.01, 0, 0, minuitResult);
    minuit.mnparm(4, "PLER_TX", 0, 0.01, 0, 0, minuitResult);
    minuit.mnparm(5, "PLER_TY", 0, 0.01, 0, 0, minuitResult);
    minuit.mncomd("FIX 2 3 5 6", minuitResult);
    minuit.mncomd("MIGRAD 10000", minuitResult);
    minuit.mncomd("RELEASE 2 3 5 6", minuitResult);
    minuit.mncomd("MIGRAD 10000", minuitResult);
    double error;
    minuit.GetParameter(0, herMomentum, error);
    minuit.GetParameter(1, herThetaX, error);
    minuit.GetParameter(2, herThetaY, error);
    minuit.GetParameter(3, lerMomentum, error);
    minuit.GetParameter(4, lerThetaX, error);
    minuit.GetParameter(5, lerThetaY, error);
  }
  /* Calculate error. */
  TLorentzVector pHER = getMomentum(herMomentum, herThetaX, herThetaY, false);
  TLorentzVector pLER = getMomentum(lerMomentum, lerThetaX, lerThetaY, true);
  TLorentzVector pBeam = pHER + pLER;
  double fittedInvariantMass = pBeam.M();
  B2RESULT("Initial invariant mass: " << s_InvariantMass <<
           "; fitted invariant mass: " << fittedInvariantMass);
  double cosThetaX = cos(herThetaX);
  double sinThetaX = sin(herThetaX);
  double cosThetaY = cos(herThetaY);
  double sinThetaY = sin(herThetaY);
  double herPartial =
    (pBeam.E() - pHER.E() / pHER.Vect().Mag() *
     (pBeam.Px() * cosThetaX * sinThetaY - pBeam.Py() * sinThetaX +
      pBeam.Pz() * cosThetaX * cosThetaY)) / fittedInvariantMass;
  cosThetaX = cos(lerThetaX);
  sinThetaX = sin(lerThetaX);
  cosThetaY = cos(lerThetaY + M_PI);
  sinThetaY = sin(lerThetaY + M_PI);
  double lerPartial =
    (pBeam.E() - pLER.E() / pLER.Vect().Mag() *
     (pBeam.Px() * cosThetaX * sinThetaY - pBeam.Py() * sinThetaX +
      pBeam.Pz() * cosThetaX * cosThetaY)) / fittedInvariantMass;
  double sigmaInvariantMass = m_CollisionInvariantMass->getMassSpread();
  double k = sqrt(sigmaInvariantMass * sigmaInvariantMass /
                  (pow(herPartial * pHER.E(), 2) +
                   pow(lerPartial * pLER.E(), 2)));
  double herSpread = k * pHER.E();
  double lerSpread = k * pLER.E();
  B2INFO("Invariant mass spread: " << sigmaInvariantMass);
  B2RESULT("HER energy spread: " << herSpread <<
           "; LER energy spread: " << lerSpread);
  /* Fill beam parameters. */
  m_BeamParameters.setHER(pHER);
  m_BeamParameters.setLER(pLER);
  TMatrixDSym covariance(3);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j)
      covariance[i][j] = 0;
  }
  covariance[0][0] = herSpread * herSpread;
  m_BeamParameters.setCovHER(covariance);
  covariance[0][0] = lerSpread * lerSpread;
  m_BeamParameters.setCovLER(covariance);
}

void BeamParametersFitter::fillVertexData(
  double covarianceXX, double covarianceYY)
{
  setupDatabase();
  m_BeamParameters.setVertex(m_BeamSpot->getIPPosition());
  TMatrixDSym beamSize = m_BeamSpot->getSizeCovMatrix();
  double xScale = sqrt(covarianceXX / beamSize[0][0]);
  double yScale = sqrt(covarianceYY / beamSize[1][1]);
  for (int i = 0; i < 3; ++i) {
    beamSize[0][i] *= xScale;
    beamSize[i][0] *= xScale;
    beamSize[1][i] *= yScale;
    beamSize[i][1] *= yScale;
  }
  m_BeamParameters.setCovVertex(beamSize);
}

void BeamParametersFitter::importBeamParameters()
{
  DBImportObjPtr<BeamParameters> beamParametersImport;
  beamParametersImport.construct(m_BeamParameters);
  beamParametersImport.import(m_IntervalOfValidity);
}
