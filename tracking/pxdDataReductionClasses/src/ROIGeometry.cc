/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include "tracking/pxdDataReductionClasses/ROIGeometry.h"
#include <framework/logging/Logger.h>
#include <RKTrackRep.h>
#include <GFTrack.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <set>

using namespace std;
using namespace Belle2;

ROIGeometry::ROIGeometry()
{

  fillPlaneList();

}

ROIGeometry::~ROIGeometry()
{

}


void
ROIGeometry::appendIntercepts(StoreArray<PXDIntercept>* listToBeFilled,
                              RKTrackRep* theTrack, int theGFTrackCandIndex,
                              RelationArray* gfTrackCandToPXDIntercepts)
{

  TVectorD predictedIntersect;
  TMatrixDSym covMatrix;
  PXDIntercept tmpPXDIntercept;

  std::list<ROIDetPlane>::iterator itPlanes = m_planeList.begin();

  B2DEBUG(1, "appendIntercepts, checking " << m_planeList.size() << " planes");

  double lambda = 0;

  while (itPlanes != m_planeList.end()) {

    for (int propDir = -1; propDir <= 1; propDir += 2) {
      theTrack->setPropDir(propDir);
      try {
        lambda = theTrack->extrapolate(*itPlanes, predictedIntersect, covMatrix);
      }  catch (...) {
        //      B2WARNING("extrapolation failed");
        itPlanes++;
        continue;
      }

      tmpPXDIntercept.setCoorU(predictedIntersect[3]);
      tmpPXDIntercept.setCoorV(predictedIntersect[4]);
      tmpPXDIntercept.setSigmaU(sqrt(covMatrix(3, 3)));
      tmpPXDIntercept.setSigmaV(sqrt(covMatrix(4, 4)));
      tmpPXDIntercept.setSigmaUprime(sqrt(covMatrix(1, 1)));
      tmpPXDIntercept.setSigmaVprime(sqrt(covMatrix(2, 2)));
      tmpPXDIntercept.setLambda(lambda);
      //      tmpPXDIntercept.setPlane(*itPlanes);
      tmpPXDIntercept.setVxdID(itPlanes->getSensorInfo());

      listToBeFilled->appendNew(tmpPXDIntercept);

      gfTrackCandToPXDIntercepts->add(theGFTrackCandIndex, listToBeFilled->getEntries() - 1);

      itPlanes++;

    }
  }

};

void
ROIGeometry::fillPlaneList()
{


  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  std::set<Belle2::VxdID> pxdLayers = aGeometry.getLayers(VXD::SensorInfoBase::PXD);
  std::set<Belle2::VxdID>::iterator itPxdLayers = pxdLayers.begin();

  while (itPxdLayers != pxdLayers.end()) {

    std::set<Belle2::VxdID> pxdLadders = aGeometry.getLadders(*itPxdLayers);
    std::set<Belle2::VxdID>::iterator itPxdLadders = pxdLadders.begin();

    while (itPxdLadders != pxdLadders.end()) {

      std::set<Belle2::VxdID> pxdSensors = aGeometry.getSensors(*itPxdLadders);
      std::set<Belle2::VxdID>::iterator itPxdSensors = pxdSensors.begin();

      while (itPxdSensors != pxdSensors.end()) {
        B2DEBUG(1, "    pxd sensor info " << *itPxdSensors);

        ROIDetPlane plane(*itPxdSensors);

        plane.Print();
        m_planeList.push_back(plane);

        itPxdSensors++;
      }
      itPxdLadders++;
    }
    itPxdLayers++;
  }

};
