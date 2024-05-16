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

#include <genfit/MeasuredStateOnPlane.h>

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

      // extrapolate track to cylinders (VXD layers)
      for (unsigned int layer = 0; layer < m_layerRadii.size(); layer++) {
        const unsigned int layerOffset = (m_detector == VXD::SensorInfoBase::SVD ? 3 : 1);

        B2DEBUG(20, " .fill intercept List, Layer: " << layer + layerOffset);
        for (int firstOrLast : {-1, 1}) {
          std::list<ROIDetPlane> selectedPlanes;
          genfit::MeasuredStateOnPlane gfTrackState;
          switch (firstOrLast) {
            case -1:
              gfTrackState = trackList[i]->getMeasuredStateOnPlaneFromFirstHit();
              break;
            case 1:
              gfTrackState = trackList[i]->getMeasuredStateOnPlaneFromLastHit();
              gfTrackState.setPosMom(gfTrackState.getPos(), -gfTrackState.getMom());
              gfTrackState.setChargeSign(-gfTrackState.getCharge());
              break;
            default:
              break;
          }

          try {
            gfTrackState.extrapolateToCylinder(m_layerRadii[layer]);
          } catch (...) {
            B2DEBUG(20, " .-extrapolation to cylinder failed");
            continue;
          }

          B2DEBUG(20, " ..append selected planes, position " << gfTrackState.getPos().X() << ", " << gfTrackState.getPos().Y() << ", " <<
                  gfTrackState.getPos().Z());

          m_theROIGeometry.appendSelectedPlanes(&selectedPlanes, ROOT::Math::XYZVector(gfTrackState.getPos()), layer + layerOffset);
          B2DEBUG(20, "selectedPlanes length now: " << firstOrLast << " the size of the selectedPlanes : " << selectedPlanes.size());
          B2DEBUG(20, " ...append intercepts for track " << i);
          B2DEBUG(20, " ...the size of the selectedPlanes : " << selectedPlanes.size());
          appendIntercepts(interceptList, selectedPlanes, gfTrackState, i, recoTrackToIntercepts, firstOrLast);
        }

      } //loop on layers
    } //loop on the track list

  } //fillInterceptList


  template<class aIntercept>
  void VXDInterceptor<aIntercept>::appendIntercepts(StoreArray<aIntercept>* interceptList,
                                                    std::list<ROIDetPlane> planeList,
                                                    genfit::MeasuredStateOnPlane state,
                                                    int recoTrackIndex, RelationArray* recoTrackToIntercepts, int firstOrLast)
  {
    aIntercept tmpIntercept;

    B2DEBUG(20, " ...-appendIntercepts, checking " << planeList.size() << " planes");

    double lambda = 0;

    std::list<ROIDetPlane>::iterator itPlanes = planeList.begin();
    while (itPlanes != planeList.end()) {
      B2DEBUG(20, " searching in appendIntercepts :  " << (itPlanes->getVxdID()));

      try {
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

      B2DEBUG(20, "extrapolate to plane!!! >>> coordinates with getPos = " << state.getPos().X()
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

} // namespace Belle2
