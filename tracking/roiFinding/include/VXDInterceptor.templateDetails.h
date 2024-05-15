/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/roiFinding/VXDInterceptor.h>
#include <tracking/roiFinding/ROIDetPlane.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

namespace Belle2 {

  template<class aIntercept>
  void VXDInterceptor<aIntercept>::fillInterceptList(StoreArray<aIntercept>* interceptList,
                                                     const StoreArray<RecoTrack>& trackList,
                                                     RelationArray* recoTrackToIntercepts)
  {
    for (int i = 0; i < trackList.getEntries(); ++i) { //loop over all tracks

      B2DEBUG(20, " %%%%%  track candidate Nr. : " << i + 1);

      if (! trackList[i] ->wasFitSuccessful()) {
        B2DEBUG(20, "%%%%% Fit not successful! discard this RecoTrack");
        continue;
      }

      // extrapolate track to cylinders (SVD layers)
      for (unsigned int layer = 0; layer < m_layerRadii.size(); layer++) {
        const unsigned int layerOffset = (m_detector == VXD::SensorInfoBase::SVD ? 3 : 1);

        B2DEBUG(20, " .fill intercept List, Layer: " << layer + layerOffset);
        // get current state of track
        genfit::MeasuredStateOnPlane gfTrackState = trackList[i]->getMeasuredStateOnPlaneFromFirstHit();

        try {
          gfTrackState.extrapolateToCylinder(m_layerRadii[layer]);
        }  catch (...) {
          B2DEBUG(20, " .-extrapolation to cylinder failed");
          continue;
        }

        std::list<ROIDetPlane> selectedPlanes;
        B2DEBUG(20, " ..append selected planes, position " << gfTrackState.getPos().X() << ", " << gfTrackState.getPos().Y() << ", " <<
                gfTrackState.getPos().Z());
        m_theROIGeometry.appendSelectedPlanes(&selectedPlanes, ROOT::Math::XYZVector(gfTrackState.getPos()), layer + layerOffset);

        B2DEBUG(20, " ...append intercepts for track " << i);
        appendIntercepts(interceptList, selectedPlanes, trackList[i], i, recoTrackToIntercepts);
      } //loop on layers
    } //loop on the track list

  } //fillInterceptList


  template<class aIntercept>
  void VXDInterceptor<aIntercept>::appendIntercepts(StoreArray<aIntercept>* interceptList,
                                                    std::list<ROIDetPlane> planeList, RecoTrack* recoTrack,
                                                    int recoTrackIndex, RelationArray* recoTrackToIntercepts)
  {
    aIntercept tmpIntercept;

    const genfit::Track& gfTrack = RecoTrackGenfitAccess::getGenfitTrack(*recoTrack);

    B2DEBUG(20, " ...-appendIntercepts, checking " << planeList.size() << " planes");

    double lambda = 0;

    for (int propDir = -1; propDir <= 1; propDir += 2) {
      gfTrack.getCardinalRep()->setPropDir(propDir);
      std::list<ROIDetPlane>::iterator itPlanes = planeList.begin();

      while (itPlanes != planeList.end()) {

        genfit::MeasuredStateOnPlane state;

        try {
          state = gfTrack.getFittedState();
          lambda = state.extrapolateToPlane(itPlanes->getSharedPlanePtr());
        }  catch (...) {
          B2DEBUG(20, " ...-extrapolation to plane failed");
          ++itPlanes;
          continue;
        }

        const TVectorD& predictedIntersect = state.getState();
        const TMatrixDSym& covMatrix = state.getCov();

        tmpIntercept.setCoorU(predictedIntersect[3]);
        tmpIntercept.setCoorV(predictedIntersect[4]);
        tmpIntercept.setSigmaU(sqrt(covMatrix(3, 3)));
        tmpIntercept.setSigmaV(sqrt(covMatrix(4, 4)));
        tmpIntercept.setSigmaUprime(sqrt(covMatrix(1, 1)));
        tmpIntercept.setSigmaVprime(sqrt(covMatrix(2, 2)));
        tmpIntercept.setLambda(lambda);
        tmpIntercept.setVxdID(itPlanes->getVxdID());
        tmpIntercept.setUprime(predictedIntersect[1]);
        tmpIntercept.setVprime(predictedIntersect[2]);

        B2DEBUG(20, "coordinates with getPos = " << state.getPos().X()
                << ", " << state.getPos().Y()
                << ", " << state.getPos().Z());
        B2DEBUG(20, "coordinates with predInter = " << predictedIntersect[3]
                << ", " << predictedIntersect[4]);
        B2DEBUG(20, "momentum with getMom = " << state.getMom().X()
                << ", " << state.getMom().Y()
                << ", " << state.getMom().Z());
        B2DEBUG(20, "U/V prime momentum with getMom = " << state.getMom().Z() / state.getMom().X()
                << ", " << state.getMom().Z() / state.getMom().Y());
        B2DEBUG(20, "U/V prime momentum with predInter = " << predictedIntersect[1]
                << ", " << predictedIntersect[2]);

        interceptList->appendNew(tmpIntercept);

        recoTrackToIntercepts->add(recoTrackIndex, interceptList->getEntries() - 1);

        ++itPlanes;

      }
    }

    // Manually set the propagation direction back to 0, which represents auto-modus.
    // This might be expected to be the case by the following algorithms without checking
    // or setting the value themselves.
    gfTrack.getCardinalRep()->setPropDir(0);


  }

} // namespace Belle2
