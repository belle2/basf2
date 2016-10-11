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
#include <genfit/RKTrackRep.h>
#include <genfit/Track.h>
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
                              genfit::Track* theTrack, int theGFTrackIndex,
                              RelationArray* gfTrackToPXDIntercepts)
{

  PXDIntercept tmpPXDIntercept;

  std::list<ROIDetPlane>::iterator itPlanes = m_planeList.begin();

  B2DEBUG(1, "appendIntercepts, checking " << m_planeList.size() << " planes");

  double lambda = 0;

  for (int propDir = -1; propDir <= 1; propDir += 2) {
    theTrack->getCardinalRep()->setPropDir(propDir);

    while (itPlanes != m_planeList.end()) {

      genfit::MeasuredStateOnPlane state;

      try {
        state = theTrack->getFittedState();
        genfit::SharedPlanePtr plane(new ROIDetPlane(*itPlanes)); // TODO: save copying
        lambda = state.extrapolateToPlane(plane);
      }  catch (...) {
        B2DEBUG(1, "extrapolation failed");
        ++itPlanes;
        continue;
      }

      const TVectorD& predictedIntersect = state.getState();
      const TMatrixDSym& covMatrix = state.getCov();

      tmpPXDIntercept.setCoorU(predictedIntersect[3]);
      tmpPXDIntercept.setCoorV(predictedIntersect[4]);
      tmpPXDIntercept.setSigmaU(sqrt(covMatrix(3, 3)));
      tmpPXDIntercept.setSigmaV(sqrt(covMatrix(4, 4)));
      tmpPXDIntercept.setSigmaUprime(sqrt(covMatrix(1, 1)));
      tmpPXDIntercept.setSigmaVprime(sqrt(covMatrix(2, 2)));
      tmpPXDIntercept.setLambda(lambda);
      tmpPXDIntercept.setVxdID(itPlanes->getSensorInfo());


      listToBeFilled->appendNew(tmpPXDIntercept);

      gfTrackToPXDIntercepts->add(theGFTrackIndex, listToBeFilled->getEntries() - 1);

      ++itPlanes;

    }
  }

};

void
ROIGeometry::fillPlaneList()
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
      B2DEBUG(1, "    pxd sensor info " << * (pxdSensors.begin()));

      while (itPxdSensors != pxdSensors.end()) {
        B2DEBUG(1, "    pxd sensor info " << *itPxdSensors);

        ROIDetPlane plane(*itPxdSensors);


        plane.Print();
        m_planeList.push_back(plane);

        ++itPxdSensors;
      }
      ++itPxdLadders;
    }
    ++itPxdLayers;
  }

};
