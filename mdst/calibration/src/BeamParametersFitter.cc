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
#include <framework/dbobjects/BeamParameters.h>
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

void BeamParametersFitter::fit()
{
  int minuitResult;
  setupDatabase();
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
  s_InvariantMassError = m_CollisionInvariantMass->getMassSpread();
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
  //minuit.SetPrintLevel(-1);
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
  }
  DBImportObjPtr<BeamParameters> beamParameters;
  beamParameters.import(m_IntervalOfValidity);
}
