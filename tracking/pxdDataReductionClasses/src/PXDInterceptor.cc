/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/MeasurementFactory.h>
#include <genfit/AbsTrackRep.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/pxdDataReductionClasses/PXDInterceptor.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

using namespace std;
using namespace Belle2;

PXDInterceptor::PXDInterceptor(const ROIinfo* theROIinfo, double toleranceZ, double tolerancePhi):
  m_theROIinfo(*theROIinfo)
{
  m_theROIGeometry.fillPlaneList(toleranceZ, tolerancePhi);
}

PXDInterceptor::~PXDInterceptor()
{
}

void
PXDInterceptor::fillInterceptList(StoreArray<PXDIntercept>* interceptList, const StoreArray<RecoTrack>& trackList,
                                  RelationArray* recoTrackToPXDIntercepts)
{

  VXD::GeoCache& vxdGeometry = VXD::GeoCache::getInstance();
  std::set<Belle2::VxdID> pxdLayers = vxdGeometry.getLayers(VXD::SensorInfoBase::PXD);

  for (int i = 0; i < trackList.getEntries(); ++i) { //loop over all tracks

    B2DEBUG(1, " %%%%%  track candidate Nr. : " << i + 1);

    if (! trackList[i] ->wasFitSuccessful()) {
      B2DEBUG(1, "%%%%% Fit not successful! discard this RecoTrack");
      continue;
    }

    // extrapolate track to cylinders (PXD layers 1 and 2)
    for (unsigned int pxdLayer = 0; pxdLayer < pxdLayers.size(); pxdLayer++) {

      B2DEBUG(1, " fill intercept List, Layer: " << pxdLayer);
      // get current state of track
      genfit::MeasuredStateOnPlane gfTrackState = trackList[i]->getMeasuredStateOnPlaneFromFirstHit();

      try {
        gfTrackState.extrapolateToCylinder(m_pxdLayerRadius[pxdLayer]);
      }  catch (...) {
        B2DEBUG(1, "extrapolation to cylinder failed");
        continue;
      }

      std::list<ROIDetPlane> selectedPlanes;
      B2DEBUG(1, "    append selected planes, position " << gfTrackState.getPos().X() << ", " << gfTrackState.getPos().Y() << ", " <<
              gfTrackState.getPos().Z());
      m_theROIGeometry.appendSelectedPlanes(&selectedPlanes, gfTrackState.getPos(), pxdLayer + 1);

      B2DEBUG(1, "    append intercepts for track " << i);
      appendIntercepts(interceptList, selectedPlanes, trackList[i], i, recoTrackToPXDIntercepts);
    } //loop on layers
  } //loop on the track list

} //fillInterceptList

void
PXDInterceptor::appendIntercepts(StoreArray<PXDIntercept>* interceptList, std::list<ROIDetPlane> planeList, RecoTrack* recoTrack,
                                 int recoTrackIndex, RelationArray* recoTrackToPXDIntercepts)
{


  PXDIntercept tmpPXDIntercept;

  genfit::Track& gfTrack = RecoTrackGenfitAccess::getGenfitTrack(*recoTrack);

  std::list<ROIDetPlane>::iterator itPlanes = planeList.begin();

  B2DEBUG(1, "appendIntercepts, checking " << planeList.size() << " planes");

  double lambda = 0;


  for (int propDir = -1; propDir <= 1; propDir += 2) {
    gfTrack.getCardinalRep()->setPropDir(propDir);

    while (itPlanes != planeList.end()) {

      genfit::MeasuredStateOnPlane state;

      try {
        state = gfTrack.getFittedState();
        lambda = state.extrapolateToPlane(itPlanes->getSharedPlanePtr());
      }  catch (...) {
        B2DEBUG(1, "extrapolation to plane failed");
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

      interceptList->appendNew(tmpPXDIntercept);

      recoTrackToPXDIntercepts->add(recoTrackIndex, interceptList->getEntries() - 1);

      ++itPlanes;

    }
  }


}
