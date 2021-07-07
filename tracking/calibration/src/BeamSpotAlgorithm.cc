/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mdst/dbobjects/BeamSpot.h>
#include <tracking/calibration/BeamSpotAlgorithm.h>
#include <tracking/calibration/BeamSpotStandAlone.h>
#include <tracking/calibration/calibTools.h>

#include <Eigen/Dense>

using Eigen::Vector3d;
using Eigen::Matrix3d;

using namespace Belle2;

using Belle2::BeamSpotCalib::getEvents;
using Belle2::BeamSpotCalib::runBeamSpotAnalysis;

BeamSpotAlgorithm::BeamSpotAlgorithm() : CalibrationAlgorithm("BeamSpotCollector")
{
  setDescription("BeamSpot calibration algorithm");
}


/** Create BS object */
static TObject* getBeamSpotObj(Vector3d ipVtx, Matrix3d  ipVtxUnc, Matrix3d  sizeMat)
{
  auto payload = new BeamSpot();
  payload->setIP(toTVector3(ipVtx), toTMatrixDSym(ipVtxUnc));
  payload->setSizeCovMatrix(toTMatrixDSym(sizeMat));
  TObject* obj  = static_cast<TObject*>(payload);
  return obj;
}




/* Main calibration method calling dedicated functions */
CalibrationAlgorithm::EResult BeamSpotAlgorithm::calibrate()
{
  TTree* tracks = getObjectPtr<TTree>("events").get();
  return runCalibration(tracks,  "BeamSpot", getEvents,
                        runBeamSpotAnalysis, getBeamSpotObj,
                        m_lossFunctionOuter, m_lossFunctionInner);
}
