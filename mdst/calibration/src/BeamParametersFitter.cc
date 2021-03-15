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
#include <framework/database/DBStore.h>
#include <framework/gearbox/Const.h>

/* ROOT headers. */
#include <TLorentzVector.h>
#include <TMinuit.h>
#include <TVectorD.h>

using namespace Belle2;

/** Invariant mass. */
static double s_InvariantMass;

/** Invariant-mass error. */
static double s_InvariantMassError;

/** Boost vector. */
static TVector3 s_BoostVector;

/** Boost-vector covariance. */
static TMatrixDSym s_BoostVectorInverseCovariance(3);

/* cppcheck-suppress constParameter */
static void fcn(int& npar, double* grad, double& fval, double* par, int iflag)
{
  (void)npar;
  (void)grad;
  (void)iflag;
  TLorentzVector pHER, pLER;
  pHER.SetXYZM(par[0], par[1], par[2], Const::electronMass);
  pLER.SetXYZM(par[3], par[4], par[5], Const::electronMass);
  TLorentzVector pBeam = pHER + pLER;
  TVector3 beamBoost = pBeam.BoostVector();
  TVectorD boostDifference(3);
  boostDifference[0] = beamBoost.X() - s_BoostVector.X();
  boostDifference[1] = beamBoost.Y() - s_BoostVector.Y();
  boostDifference[2] = beamBoost.Z() - s_BoostVector.Z();
  double boostChi2 = s_BoostVectorInverseCovariance.Similarity(boostDifference);
  double invariantMass = pBeam.M();
  fval = pow((invariantMass - s_InvariantMass) / s_InvariantMassError, 2) +
         boostChi2;
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
  s_BoostVectorInverseCovariance.Invert();
  s_InvariantMass = m_CollisionInvariantMass->getMass();
  s_InvariantMassError = m_CollisionInvariantMass->getMassSpread();
  TMinuit minuit(6);
  minuit.mnparm(0, "PHER_X", 0, 0.01, 0, 0, minuitResult);
  minuit.mnparm(1, "PHER_Y", 0, 0.01, 0, 0, minuitResult);
  minuit.mnparm(2, "PHER_Z", 7, 0.01, 0, 0, minuitResult);
  minuit.mnparm(3, "PLER_X", 0, 0.01, 0, 0, minuitResult);
  minuit.mnparm(4, "PLER_Y", 0, 0.01, 0, 0, minuitResult);
  minuit.mnparm(5, "PLER_Z", -4, 0.01, 0, 0, minuitResult);
  minuit.SetFCN(fcn);
  minuit.mncomd("FIX 1 2 4 5", minuitResult);
  minuit.mncomd("MIGRAD 10000", minuitResult);
}
