/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <tracking/svdROIFinder/SVDROIGeometry.h>
#include <framework/logging/Logger.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <set>

using namespace std;
using namespace Belle2;


SVDROIGeometry::SVDROIGeometry()
{

}

SVDROIGeometry::~SVDROIGeometry()
{

}

void
SVDROIGeometry::fillPlaneList(double toleranceZ,
                              double tolerancePhi)
{

  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

  std::set<Belle2::VxdID> svdLayers = geoCache.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();

  //  while (itSvdLayers != svdLayers.end()) {
  //in DESY TB the EUDET telescope planes have been associated to SVD layer 7, we do not want ROIs there, therefore:
  while ((itSvdLayers != svdLayers.end()) && (itSvdLayers->getLayerNumber() != 7)) {

    std::set<Belle2::VxdID> svdLadders = geoCache.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    while (itSvdLadders != svdLadders.end()) {

      std::set<Belle2::VxdID> svdSensors = geoCache.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();

      while (itSvdSensors != svdSensors.end()) {
        B2DEBUG(20, "    svd sensor info " << *itSvdSensors);

        ROIDetPlane plane(*itSvdSensors, toleranceZ, tolerancePhi);
        genfit::SharedPlanePtr sharedPlane(new ROIDetPlane(plane));
        plane.setSharedPlanePtr(sharedPlane);

        m_planeList.push_back(plane);

        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }

  B2DEBUG(20, "just filled the plane list with " << m_planeList.size() << "planes");
};


void
SVDROIGeometry::appendSelectedPlanes(std::list<ROIDetPlane>* selectedPlanes, TVector3 recoTrackPosition, int layer)
{


  std::list<ROIDetPlane>::iterator itPlanes = m_planeList.begin();

  B2DEBUG(20, " ..-append Planes, checking " << m_planeList.size() << " planes");

  while (itPlanes != m_planeList.end()) {

    if (itPlanes->isSensorInRange(recoTrackPosition, layer))
      selectedPlanes->push_back(*itPlanes);

    ++itPlanes;

  }

  B2DEBUG(20, " ..--list of sensor IDs of the selected planes for this track:");
  itPlanes = selectedPlanes->begin();
  while (itPlanes != selectedPlanes->end()) {
    B2DEBUG(20, "     " << (itPlanes->getVxdID()));
    ++itPlanes;
  }

}

