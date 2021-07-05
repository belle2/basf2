/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include "tracking/pxdDataReductionClasses/ROIGeometry.h"
#include <framework/logging/Logger.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <tracking/pxdDataReductionClasses/ROIDetPlane.h>

#include <set>

using namespace std;
using namespace Belle2;

ROIGeometry::ROIGeometry()
{

}

ROIGeometry::~ROIGeometry()
{

}


void
ROIGeometry::fillPlaneList(double toleranceZ, double tolerancePhi)
{


  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  std::set<Belle2::VxdID> pxdLayers = aGeometry.getLayers(VXD::SensorInfoBase::PXD);
  std::set<Belle2::VxdID>::iterator itPxdLayers = pxdLayers.begin();

  //  while (itPxdLayers != pxdLayers.end()) {
  //in DESY TB the EUDET telescope planes have been associated to PXD layer 7, we do not want ROIs there, therefore:
  while ((itPxdLayers != pxdLayers.end()) && (itPxdLayers->getLayerNumber() != 7)) {

    std::set<Belle2::VxdID> pxdLadders = aGeometry.getLadders(*itPxdLayers);
    std::set<Belle2::VxdID>::iterator itPxdLadders = pxdLadders.begin();

    while (itPxdLadders != pxdLadders.end()) {

      std::set<Belle2::VxdID> pxdSensors = aGeometry.getSensors(*itPxdLadders);
      std::set<Belle2::VxdID>::iterator itPxdSensors = pxdSensors.begin();
      B2DEBUG(20, "    pxd sensor info " << * (pxdSensors.begin()));

      while (itPxdSensors != pxdSensors.end()) {
        B2DEBUG(20, "    pxd sensor info " << *itPxdSensors);

        ROIDetPlane plane(*itPxdSensors, toleranceZ, tolerancePhi);
        genfit::SharedPlanePtr sharedPlane(new ROIDetPlane(plane));
        plane.setSharedPlanePtr(sharedPlane);

        m_planeList.push_back(plane);

        ++itPxdSensors;
      }
      ++itPxdLadders;
    }
    ++itPxdLayers;
  }

  B2DEBUG(20, "just filled the plane list with " << m_planeList.size() << "planes");
};


void
ROIGeometry::appendSelectedPlanes(std::list<ROIDetPlane>* selectedPlanes, TVector3 recoTrackPosition, int layer)
{


  std::list<ROIDetPlane>::iterator itPlanes = m_planeList.begin();

  B2DEBUG(20, "append Planes, checking " << m_planeList.size() << " planes");

  while (itPlanes != m_planeList.end()) {

    if (itPlanes->isSensorInRange(recoTrackPosition, layer))
      selectedPlanes->push_back(*itPlanes);

    ++itPlanes;

  }

  B2DEBUG(20, " list of sensor IDs of the selected planes for this track:");
  itPlanes = selectedPlanes->begin();
  while (itPlanes != selectedPlanes->end()) {
    B2DEBUG(20, "     " << (itPlanes->getVxdID()));
    ++itPlanes;
  }

}
