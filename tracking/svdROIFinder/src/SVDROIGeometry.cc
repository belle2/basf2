/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include "tracking/svdROIFinder/SVDROIGeometry.h"
#include <framework/logging/Logger.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/RKTrackRep.h>
#include <genfit/Track.h>
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
SVDROIGeometry::fillPlaneList(double toleranceZ, double tolerancePhi)
{

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  std::set<Belle2::VxdID> svdLayers = aGeometry.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();

  //  while (itSvdLayers != svdLayers.end()) {
  //in DESY TB the EUDET telescope planes have been associated to SVD layer 7, we do not want ROIs there, therefore:
  while ((itSvdLayers != svdLayers.end()) && (itSvdLayers->getLayerNumber() != 7)) {

    std::set<Belle2::VxdID> svdLadders = aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    while (itSvdLadders != svdLadders.end()) {

      std::set<Belle2::VxdID> svdSensors = aGeometry.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();
      //      B2DEBUG(1, "    svd sensor info " << * (svdSensors.begin()));

      while (itSvdSensors != svdSensors.end()) {
        B2DEBUG(1, "    svd sensor info " << *itSvdSensors);

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

  B2DEBUG(1, "just filled the plane list with " << m_planeList.size() << "planes");
};


void
SVDROIGeometry::appendSelectedPlanes(std::list<ROIDetPlane>* selectedPlanes, TVector3 recoTrackPosition, int layer)
{


  std::list<ROIDetPlane>::iterator itPlanes = m_planeList.begin();

  B2DEBUG(1, " ..-append Planes, checking " << m_planeList.size() << " planes");

  while (itPlanes != m_planeList.end()) {

    if (itPlanes->isSensorInRange(recoTrackPosition, layer))
      selectedPlanes->push_back(*itPlanes);

    itPlanes++;

  }

  B2DEBUG(1, " ..--list of sensor IDs of the selected planes for this track:");
  itPlanes = selectedPlanes->begin();
  while (itPlanes != selectedPlanes->end()) {
    B2DEBUG(1, "     " << (itPlanes->getSensorInfo()));
    itPlanes++;
  }

}

