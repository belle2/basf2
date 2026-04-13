/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/track/TrackBadBoardFilter.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/topology/CDCWire.h>

#include <tracking/trackingUtilities/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackingUtilities/eventdata/tracks/CDCTrack.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCRecoHit3D.h>


namespace Belle2 {
  namespace TrackingUtilities {

    // helper function to filter CDCTracks which have bad CDC boards where holes
    // in the track are detected
    // @return true is accepted (has bad boards)
    bool trackBadBoardFilter(const TrackingUtilities::CDCTrack& aCDCTrack)
    {
      // tunable parameters
      double arcLengthShift =
        4.5; // arclength to extrapolate to find the bad board, unit cm, correcponding to around 2.25 drift cells (if passing pendicular)
      int minJump = 5;  // minimum layers to be jumped between two hit to trigger search for hits (one board covers 3 layers)

      // first check if bad wires are present
      Belle2::TrackingUtilities::StoreWrappedObjPtr< std::vector<unsigned int> > badBoardsVectorPtr("BadBoardsVector");

      // nothing to do
      if (not badBoardsVectorPtr.isValid() || (*badBoardsVectorPtr).size() == 0) return false;

      // for better readability
      std::vector<unsigned int>& badBoardsVector = *badBoardsVectorPtr;

      // debug
      /*
      std::cout << "storevector " << std::endl;
      for (auto i : badBoardsVector) std::cout << i << std::endl;
      */

      CDC::CDCGeometryPar& geometryPar = CDC::CDCGeometryPar::Instance();

      auto trajectory = aCDCTrack.getStartTrajectory3D();

      Helix localHelix = trajectory.getLocalHelix().helix();
      auto localOrigin = trajectory.getLocalOrigin();

      // detect layer jumps jumping at least minJump layers
      // Assumes hits are ordered! Need check if that is always true, or sorting without check (PS: TrackQualityEstimator also assumes hits are ordered for its filter)
      const CDCRecoHit3D* prevHitPtr = nullptr;
      for (const CDCRecoHit3D& thisHit : aCDCTrack) {
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
          Vector3D pos3DPrev = localHelix.atArcLength2D(sPrevious + arcLengthShift) + localOrigin;
          Vector3D pos3DThis = localHelix.atArcLength2D(sCurrent - arcLengthShift) + localOrigin;

          // direction in case of backcurling tracks
          int dir = (iclayerThis - iclayerPrev) < 0 ? -1 : 1;
          // jump 3 layers to get the onto next board
          unsigned int newlayerThis = iclayerThis - dir * 3;
          unsigned int newlayerPrev = iclayerPrev + dir * 3;

          // find the wire number
          Belle2::CDC::IWire iWireThis = geometryPar.cellId(newlayerThis, pos3DThis);
          Belle2::CDC::IWire iWirePrev = geometryPar.cellId(newlayerPrev, pos3DPrev);

          // get the board number
          auto boardThis = geometryPar.getBoardID(WireID(newlayerThis, iWireThis));
          auto boardPrev = geometryPar.getBoardID(WireID(newlayerPrev, iWirePrev));

          if (std::find(badBoardsVector.begin(), badBoardsVector.end(), boardThis) != badBoardsVector.end()) return true;
          if (std::find(badBoardsVector.begin(), badBoardsVector.end(), boardPrev) != badBoardsVector.end()) return true;


          /*
             std::cout << "bad boards ? " << boardThis << " " << boardPrev << std::endl;
             std::cout << "arc length "  << sPrevious << " b " << sCurrent << std::endl;
             std::cout << "point a " << prevHitPtr->getRecoPos3D() << " point b " << thisHit.getRecoPos3D() << std::endl;
             std::cout << "extrapolated a " << (localHelix.atArcLength2D(sPrevious) + localOrigin) << " b " << (localHelix.atArcLength2D(sCurrent) + localOrigin)<< std::endl;
             std::cout << "new hit" << std::endl;
             std::cout << thisHit.getWire().getISuperLayer() << std::endl;
             std::cout << thisHit.getWire().getILayer() << std::endl;
             std::cout << thisHit.getWire().getICLayer() << std::endl;
             std::cout << thisHit.isAxial() << std::endl;
             std::cout << "old hit" << std::endl;
             std::cout << prevHitPtr->getWire().getISuperLayer() << std::endl;
             std::cout << prevHitPtr->getWire().getILayer() << std::endl;
             std::cout << prevHitPtr->getWire().getICLayer() << std::endl;
             std::cout << prevHitPtr->isAxial() << std::endl;

          */

        }

        prevHitPtr = &thisHit;
      }

      return false;

    }



  }
}
