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
#include <boost/foreach.hpp> // for getGlobalizedHiterrors
#include <vxd/geometry/GeoCache.h> // for getGlobalizedHiterrors
#include <vxd/geometry/SensorInfoBase.h> // for getGlobalizedHiterrors
#include <vxd/dataobjects/VxdID.h> // for getGlobalizedHiterrors
#include <framework/logging/Logger.h> // for B2WARNING

using namespace std;
using namespace Belle2;
using namespace Tracking;

vector< vector< pair<double, double> > > Tracking::getGlobalizedHitErrors()
{

  vector< vector< pair<double, double> > > errorContainer;

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  set< VxdID > layers = aGeometry.getLayers(); // SensorInfoBase::SensorType sensortype=SensorInfoBase::VXD
  BOOST_FOREACH(VxdID layer, layers) {
    vector< pair<double, double> > layerErrors;
    const set<VxdID>& ladders = aGeometry.getLadders(layer);
    BOOST_FOREACH(VxdID ladder, ladders) {
      const set<VxdID>& sensors = aGeometry.getSensors(ladder);

      VxdID sensorID = *sensors.begin();

      const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(sensorID);

      double sigmaU = aSensorInfo.getUPitch(0.); // error at center of sensor
      TVector3 localError = TVector3(sigmaU, 0., 0.);
      TVector3 globalError = aSensorInfo.vectorToGlobal(localError);

      layerErrors.push_back(make_pair(globalError.X(), globalError.Y()));
    }

    errorContainer.push_back(layerErrors);
  }

  B2DEBUG(1, " Tracking::getGlobalizedHitErrors, " << errorContainer.size() << " layers stored...")

  return errorContainer;
}

vector< pair<double, double> > Tracking::getHitErrors()
{

  vector< pair<double, double> > errorContainer;

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  set< VxdID > layers = aGeometry.getLayers(); // SensorInfoBase::SensorType sensortype=SensorInfoBase::VXD
  BOOST_FOREACH(VxdID layer, layers) { // only one sensor of each layer is enough. taking first sensor of first ladder of each layer
    const set<VxdID>& ladders = aGeometry.getLadders(layer);
    VxdID ladder = *ladders.begin();
    const set<VxdID>& sensors = aGeometry.getSensors(ladder);
    VxdID sensor = *sensors.begin();
    const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(sensor);

    double sigmaU = aSensorInfo.getUPitch(0.); // error at center of sensor
    double sigmaV = aSensorInfo.getVPitch(0.); // error at center of sensor
    errorContainer.push_back(make_pair(sigmaU, sigmaV));
  }

  B2DEBUG(1, " Tracking::getHitErrors, " << errorContainer.size() << " layers stored...")
  return errorContainer;
}