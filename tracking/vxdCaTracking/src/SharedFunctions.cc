/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/SharedFunctions.h"
#include <vxd/geometry/GeoCache.h> // for getGlobalizedHiterrors, getHitErrors
#include <vxd/geometry/SensorInfoBase.h> // for getGlobalizedHiterrors, getHitErrors
#include <vxd/dataobjects/VxdID.h> // for getGlobalizedHiterrors, VxdID
#include <framework/logging/Logger.h> // for B2DEBUG

using namespace std;
// using namespace Belle2;

namespace Belle2 {
  vector< vector< pair<double, double> > > getGlobalizedHitErrors()
  {

    vector< vector< pair<double, double> > > errorContainer;

    VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
    set< VxdID > layers = aGeometry.getLayers(); // SensorInfoBase::SensorType sensortype=SensorInfoBase::VXD
    for (VxdID layer : layers) {
      vector< pair<double, double> > layerErrors;
      const set<VxdID>& ladders = aGeometry.getLadders(layer);
      for (VxdID ladder : ladders) {
        const set<VxdID>& sensors = aGeometry.getSensors(ladder);

        VxdID sensorID = *sensors.begin();

        const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(sensorID);

//         double sigmaU = aSensorInfo.getUPitch(0.); // error at center of sensor /// WARNING ausgetauscht
//        TVector3 localError = TVector3(sigmaU, 0., 0.);
        double sigmaV = aSensorInfo.getVPitch(0.); // error at center of sensor
        TVector3 localError = TVector3(0., sigmaV, 0.);
        TVector3 globalError = aSensorInfo.vectorToGlobal(localError);

        layerErrors.push_back(make_pair(globalError.X(), globalError.Y()));
        B2DEBUG(175, " getGlobalizedHitErrors at layer " << layer << ", ladder " << ladder << ", sigmaX/Y/Z" << globalError.X() << "/" << globalError.Y() << "/" << globalError.Z() <<  " [unit cm?]")
      }

      errorContainer.push_back(layerErrors);
    }

    B2DEBUG(1, " getGlobalizedHitErrors, " << errorContainer.size() << " layers stored...")

    return errorContainer;
  }

  vector< pair<double, double> > getHitErrors()
  {

    vector< pair<double, double> > errorContainer;

    VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
    set< VxdID > layers = aGeometry.getLayers(); // SensorInfoBase::SensorType sensortype=SensorInfoBase::VXD
    for (VxdID layer : layers) { // only one sensor of each layer is enough. taking first sensor of first ladder of each layer
      const set<VxdID>& ladders = aGeometry.getLadders(layer);
      VxdID ladder = *ladders.begin();
      const set<VxdID>& sensors = aGeometry.getSensors(ladder);
      VxdID sensor = *sensors.begin();
      const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(sensor);

      double sigmaU = aSensorInfo.getUPitch(0.); // error at center of sensor  /// WARNING ausgetauscht
      double sigmaV = aSensorInfo.getVPitch(0.); // error at center of sensor  /// WARNING ausgetauscht
//      double sigmaV = aSensorInfo.getUPitch(0.); // error at center of sensor
//       double sigmaU = aSensorInfo.getVPitch(0.); // error at center of sensor
      errorContainer.push_back(make_pair(sigmaU, sigmaV));
      B2DEBUG(5, " getHitErrors at layer " << layer << ", sigmaU/V" << sigmaU << "/" << sigmaV << " [unit cm?]")
    }

    B2DEBUG(1, " getHitErrors, " << errorContainer.size() << " layers stored...")
    return errorContainer;
  }


  void printMyMatrix(TMatrixD& aMatrix, std::stringstream& ss)
  {
    //      std::stringstream printOut;
    for (int nRow = 0; nRow < aMatrix.GetNrows(); nRow++) {
      for (int nCol = 0; nCol < aMatrix.GetNcols(); nCol++) {
        ss << aMatrix(nRow, nCol) << '\t';
      }
      ss << std::endl;
    }
    //      return printOut;
  }

// void printMyMatrix(Eigen::MatrixXd& aMatrix, std::stringstream& ss) {
//      std::stringstream printOut;
//  for (int nRow = 0; nRow < aMatrix.GetNrows(); nRow++) {
//    for (int nCol = 0; nCol < aMatrix.GetNcols(); nCol++) {
//      ss << aMatrix(nRow, nCol) << '\t';
//    }
///     ss << aMatrix << std::endl;
//  }
//      return printOut;
// }
}
