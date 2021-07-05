/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mdst/dbobjects/CollisionInvariantMass.h>
#include <tracking/calibration/InvariantMassAlgorithm.h>
#include <tracking/calibration/InvariantMassStandAlone.h>
#include <tracking/calibration/calibTools.h>

#include <Eigen/Dense>

using Eigen::VectorXd;
using Eigen::MatrixXd;

using namespace Belle2;

using Belle2::InvariantMassCalib::getEvents;
using Belle2::InvariantMassCalib::runInvariantMassAnalysis;

//Using boostVector collector for the input
InvariantMassAlgorithm::InvariantMassAlgorithm() : CalibrationAlgorithm("BoostVectorCollector")
{
  setDescription("Collision invariant mass calibration algorithm");
}


/** Create InvarinatMass object */
static TObject* getInvariantMassObj(VectorXd vMass, MatrixXd  vMassUnc, MatrixXd vMassSpread)
{
  auto payload = new CollisionInvariantMass();

  double mass    = vMass(0);
  double unc2    = vMassUnc(0, 0);
  double spread2 = vMassSpread(0, 0);

  payload->setMass(mass, sqrt(unc2), sqrt(spread2));
  TObject* obj  = static_cast<TObject*>(payload);
  return obj;
}



/* Main calibration method calling dedicated functions */
CalibrationAlgorithm::EResult InvariantMassAlgorithm::calibrate()
{
  TTree* tracks = getObjectPtr<TTree>("events").get();
  return runCalibration(tracks,  "CollisionInvariantMass", getEvents,
                        runInvariantMassAnalysis, getInvariantMassObj,
                        m_lossFunctionOuter, m_lossFunctionInner);
}
