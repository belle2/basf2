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
#include <TMinuit.h>

using namespace Belle2;

/** Invariant mass. */
static double s_InvariantMass;

/** Invariant-mass error. */
static double s_InvariantMassError;

/* cppcheck-suppress constParameter */
static void fcn(int& npar, double* grad, double& fval, double* par, int iflag)
{
  (void)npar;
  (void)grad;
  (void)iflag;
  double pHER[3] = {par[0], par[1], par[2]};
  double pLER[3] = {par[3], par[4], par[5]};
  double eHER = sqrt(pHER[0] * pHER[0] + pHER[1] * pHER[1] + pHER[2] * pHER[2]
                     + Const::electronMass * Const::electronMass);
  double eLER = sqrt(pLER[0] * pLER[0] + pLER[1] * pLER[1] + pLER[2] * pLER[2]
                     + Const::electronMass * Const::electronMass);
  double pBeam[4] = {eHER + eLER, pHER[0] + pLER[0], pHER[1] + pLER[1],
                     pHER[2] + pLER[2]
                    };
  double invariantMass = sqrt(pBeam[0] * pBeam[0] - pBeam[1] * pBeam[1] -
                              pBeam[2] * pBeam[2] - pBeam[3] * pBeam[3]);
  fval = pow((invariantMass - s_InvariantMass) / s_InvariantMassError, 2);
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
  s_InvariantMass = m_CollisionInvariantMass->getMass();
  s_InvariantMassError = m_CollisionInvariantMass->getMassSpread();
  TMinuit minuit(6);
  minuit.mnparm(0, "PHER_X", 0, 0.01, 0, 0, minuitResult);
  minuit.mnparm(1, "PHER_Y", 0, 0.01, 0, 0, minuitResult);
  minuit.mnparm(2, "PHER_Z", 0, 0.01, 0, 0, minuitResult);
  minuit.mnparm(3, "PLER_X", 0, 0.01, 0, 0, minuitResult);
  minuit.mnparm(4, "PLER_Y", 0, 0.01, 0, 0, minuitResult);
  minuit.mnparm(5, "PLER_Z", 0, 0.01, 0, 0, minuitResult);
  minuit.SetFCN(fcn);
  minuit.mncomd("FIX 1 2 4 5 6", minuitResult);
  minuit.mncomd("MIGRAD 10000", minuitResult);
}
