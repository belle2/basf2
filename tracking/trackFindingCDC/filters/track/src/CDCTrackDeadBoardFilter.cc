/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/track/CDCTrackDeadBoardFilter.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/topology/CDCWire.h>

#include <tracking/trackingUtilities/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackingUtilities/eventdata/tracks/CDCTrack.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCRecoHit3D.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    // TODO: decide if this is the correct place to put this function

    bool cdcTrackDeadBoardFilter(const Belle2::TrackingUtilities::CDCTrack& aCDCTrack)
    {
      // tunable parameters.
      // TODO: decide if should be put into list of input parameters
      double arcLengthShift =
        4.5; // arclength to extrapolate to find the bad board, unit cm, correcponding to around 2.25 drift cells (if passing pendicular)
      int minJump = 5;  // minimum layers to be jumped between two hits to trigger search for bad boards (one board covers 3 layers)

      // first check if bad wires are present
      Belle2::TrackingUtilities::StoreWrappedObjPtr< std::vector<unsigned int> > deadBoardsVectorPtr("CDCDeadBoardsVector");

      // nothing to do if no dead boards have been assigned
      if (not deadBoardsVectorPtr.isValid() || (*deadBoardsVectorPtr).size() == 0) return false;

      // for better readability
      const std::vector<unsigned int>& deadBoardsVector = *deadBoardsVectorPtr;

      const CDC::CDCGeometryPar& geometryPar = CDC::CDCGeometryPar::Instance();

      auto trajectory = aCDCTrack.getStartTrajectory3D();

      Belle2::TrackingUtilities::Helix localHelix = trajectory.getLocalHelix().helix();
      auto localOrigin = trajectory.getLocalOrigin();

      // detect layer jumps jumping at least minJump layers
      // Assumes hits are ordered! Need check if that is always true, or sorting without check (PS: TrackQualityEstimator also assumes hits are ordered for its filter)
      const Belle2::TrackingUtilities::CDCRecoHit3D* prevHitPtr = nullptr;
      for (const Belle2::TrackingUtilities::CDCRecoHit3D& thisHit : aCDCTrack) {
        if (not prevHitPtr) {
          prevHitPtr = &thisHit;
          continue;
        }

        Belle2::CDC::ILayer iclayerPrev = ((const CDCHit*)*prevHitPtr)->getICLayer(); // signed short
        Belle2::CDC::ILayer iclayerThis = ((const CDCHit*)thisHit)->getICLayer();

        if (abs(iclayerPrev - iclayerThis) >= minJump) {
          double sPrevious = prevHitPtr->getArcLength2D();
          double sCurrent = thisHit.getArcLength2D();

          // may want to filter for too high arclength??? observed up to 45cm arclength differences
          auto pos3DPrev = localHelix.atArcLength2D(sPrevious + arcLengthShift) + localOrigin;
          auto pos3DThis = localHelix.atArcLength2D(sCurrent - arcLengthShift) + localOrigin;

          // direction in case of backcurling tracks
          int dir = (iclayerThis - iclayerPrev) < 0 ? -1 : 1;
          // jump 3 layers to get the onto next board
          unsigned int newlayerThis = iclayerThis - dir * 3;
          unsigned int newlayerPrev = iclayerPrev + dir * 3;

          // find the wire number
          const Belle2::CDC::IWire iWireThis = geometryPar.cellId(newlayerThis, pos3DThis);
          const Belle2::CDC::IWire iWirePrev = geometryPar.cellId(newlayerPrev, pos3DPrev);

          // get the board number
          const auto boardThis = geometryPar.getBoardID(WireID(newlayerThis, iWireThis));
          const auto boardPrev = geometryPar.getBoardID(WireID(newlayerPrev, iWirePrev));

          if (std::find(deadBoardsVector.begin(), deadBoardsVector.end(), boardThis) != deadBoardsVector.end()) return true;
          if (std::find(deadBoardsVector.begin(), deadBoardsVector.end(), boardPrev) != deadBoardsVector.end()) return true;

        }

        prevHitPtr = &thisHit;
      }

      return false;

    }



  }
}
