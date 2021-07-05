/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mdst/dbobjects/CollisionBoostVector.h>
#include <tracking/calibration/BoostVectorAlgorithm.h>
#include <tracking/calibration/BoostVectorStandAlone.h>
#include <tracking/calibration/calibTools.h>

#include <Eigen/Dense>

using Eigen::Vector3d;
using Eigen::Matrix3d;

using namespace Belle2;

using Belle2::BoostVectorCalib::getEvents;
using Belle2::BoostVectorCalib::runBoostVectorAnalysis;


BoostVectorAlgorithm::BoostVectorAlgorithm() : CalibrationAlgorithm("BoostVectorCollector")
{
  setDescription("BoostVector calibration algorithm");
}


/** Create BoostVector object */
static TObject* getBoostVectorObj(Vector3d vBoost, Matrix3d  vBoostUnc, Matrix3d /*vBoostSpread*/)
{
  auto payload = new CollisionBoostVector();
  payload->setBoost(toTVector3(vBoost), toTMatrixDSym(vBoostUnc));
  TObject* obj  = static_cast<TObject*>(payload);
  return obj;
}



/* Main calibration method calling dedicated functions */
CalibrationAlgorithm::EResult BoostVectorAlgorithm::calibrate()
{
  TTree* tracks = getObjectPtr<TTree>("events").get();
  return runCalibration(tracks,  "CollisionBoostVector", getEvents,
                        runBoostVectorAnalysis, getBoostVectorObj,
                        m_lossFunctionOuter, m_lossFunctionInner);
}
