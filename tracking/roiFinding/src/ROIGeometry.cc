/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <tracking/roiFinding/ROIGeometry.h>
#include <tracking/roiFinding/ROIDetPlane.h>
#include <framework/logging/Logger.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <set>

using namespace Belle2;

ROIGeometry::ROIGeometry()
{

}

ROIGeometry::~ROIGeometry()
{

}


void
ROIGeometry::fillPlaneList(double toleranceZ, double tolerancePhi, const VXD::SensorInfoBase::SensorType det)
{

  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

  std::set<Belle2::VxdID> vxdLayers = geoCache.getLayers(det);
  std::set<Belle2::VxdID>::iterator itVXDLayers = vxdLayers.begin();

  while (itVXDLayers != vxdLayers.end()) {

    std::set<Belle2::VxdID> vxdLadders = geoCache.getLadders(*itVXDLayers);
    std::set<Belle2::VxdID>::iterator itVXDLadders = vxdLadders.begin();

    while (itVXDLadders != vxdLadders.end()) {

      std::set<Belle2::VxdID> vxdSensors = geoCache.getSensors(*itVXDLadders);
      std::set<Belle2::VxdID>::iterator itVXDSensors = vxdSensors.begin();
      B2DEBUG(20, "    vxd sensor info " << * (vxdSensors.begin()));

      while (itVXDSensors != vxdSensors.end()) {
        B2DEBUG(20, "    vxd sensor info " << *itVXDSensors);

        ROIDetPlane plane(*itVXDSensors, toleranceZ, tolerancePhi);
        genfit::SharedPlanePtr sharedPlane(new ROIDetPlane(plane));
        plane.setSharedPlanePtr(sharedPlane);

        m_planeList.push_back(plane);

        ++itVXDSensors;
      }
      ++itVXDLadders;
    }
    ++itVXDLayers;
  }

  B2DEBUG(20, "just filled the plane list with " << m_planeList.size() << "planes");
};


void
ROIGeometry::appendSelectedPlanes(std::list<ROIDetPlane>* selectedPlanes, ROOT::Math::XYZVector recoTrackPosition, int layer)
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
