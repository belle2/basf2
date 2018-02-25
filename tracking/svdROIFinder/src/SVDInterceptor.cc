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
#include <tracking/svdROIFinder/SVDInterceptor.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

using namespace std;
using namespace Belle2;

SVDInterceptor::SVDInterceptor(const ROIinfo* theROIinfo, double toleranceZ, double tolerancePhi):
  m_theROIinfo(*theROIinfo)
{
  m_theROIGeometry.fillPlaneList(toleranceZ, tolerancePhi);
}

SVDInterceptor::~SVDInterceptor()
{
}

void
SVDInterceptor::fillInterceptList(StoreArray<SVDIntercept>* interceptList, const StoreArray<RecoTrack>& trackList,
                                  RelationArray* recoTrackToSVDIntercepts)
{

  VXD::GeoCache& vxdGeometry = VXD::GeoCache::getInstance();
  std::set<Belle2::VxdID> svdLayers = vxdGeometry.getLayers(VXD::SensorInfoBase::SVD);

  for (int i = 0; i < trackList.getEntries(); ++i) { //loop over all tracks

    B2DEBUG(1, " %%%%%  track candidate Nr. : " << i + 1);

    if (! trackList[i] ->wasFitSuccessful()) {
      B2DEBUG(1, "%%%%% Fit not successful! discard this RecoTrack");
      continue;
    }

    // extrapolate track to cylinders (SVD layers)
    for (unsigned int svdLayer = 0; svdLayer < svdLayers.size(); svdLayer++) {

      B2DEBUG(1, " .fill intercept List, Layer: " << svdLayer + 3);
      // get current state of track
      genfit::MeasuredStateOnPlane gfTrackState = trackList[i]->getMeasuredStateOnPlaneFromFirstHit();

      try {
        gfTrackState.extrapolateToCylinder(m_svdLayerRadius[svdLayer]);
      }  catch (...) {
        B2DEBUG(1, " .-extrapolation to cylinder failed");
        continue;
      }

      std::list<ROIDetPlane> selectedPlanes;
      B2DEBUG(1, " ..append selected planes, position " << gfTrackState.getPos().X() << ", " << gfTrackState.getPos().Y() << ", " <<
              gfTrackState.getPos().Z());
      m_theROIGeometry.appendSelectedPlanes(&selectedPlanes, gfTrackState.getPos(), svdLayer + 3);

      B2DEBUG(1, " ...append intercepts for track " << i);
      appendIntercepts(interceptList, selectedPlanes, trackList[i], i, recoTrackToSVDIntercepts);
    } //loop on layers
  } //loop on the track list

} //fillInterceptList


void
SVDInterceptor::appendIntercepts(StoreArray<SVDIntercept>* interceptList, std::list<ROIDetPlane> planeList, RecoTrack* recoTrack,
                                 int recoTrackIndex, RelationArray* recoTrackToSVDIntercepts)
{


  SVDIntercept tmpSVDIntercept;

  genfit::Track& gfTrack = RecoTrackGenfitAccess::getGenfitTrack(*recoTrack);

  std::list<ROIDetPlane>::iterator itPlanes = planeList.begin();

  B2DEBUG(1, " ...-appendIntercepts, checking " << planeList.size() << " planes");

  double lambda = 0;


  for (int propDir = -1; propDir <= 1; propDir += 2) {
    gfTrack.getCardinalRep()->setPropDir(propDir);

    while (itPlanes != planeList.end()) {

      genfit::MeasuredStateOnPlane state;

      try {
        state = gfTrack.getFittedState();
        lambda = state.extrapolateToPlane(itPlanes->getSharedPlanePtr());
      }  catch (...) {
        B2DEBUG(1, " ...-extrapolation to plane failed");
        ++itPlanes;
        continue;
      }

      const TVectorD& predictedIntersect = state.getState();
      const TMatrixDSym& covMatrix = state.getCov();

      tmpSVDIntercept.setCoorU(predictedIntersect[3]);
      tmpSVDIntercept.setCoorV(predictedIntersect[4]);
      tmpSVDIntercept.setSigmaU(sqrt(covMatrix(3, 3)));
      tmpSVDIntercept.setSigmaV(sqrt(covMatrix(4, 4)));
      tmpSVDIntercept.setSigmaUprime(sqrt(covMatrix(1, 1)));
      tmpSVDIntercept.setSigmaVprime(sqrt(covMatrix(2, 2)));
      tmpSVDIntercept.setLambda(lambda);
      tmpSVDIntercept.setVxdID(itPlanes->getSensorInfo());

      interceptList->appendNew(tmpSVDIntercept);

      recoTrackToSVDIntercepts->add(recoTrackIndex, interceptList->getEntries() - 1);

      ++itPlanes;

    }
  }


}


