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
#include <cdc/topology/CDCWireLayer.h>

#include <tracking/trackingUtilities/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackingUtilities/eventdata/tracks/CDCTrack.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCRecoHit3D.h>

// TODO: decide if this is the correct place to put this function

namespace Belle2 {
  namespace TrackFindingCDC {

    void addBoardCandsAtLayer(std::vector<unsigned int>& boardCands, const Belle2::TrackingUtilities::Helix& globalHelix,
                              Belle2::CDC::ILayer iclayer, const CDC::CDCGeometryPar& geometryPar)
    {

      double rLayer = Belle2::CDC::CDCWireLayer::getInstance(iclayer)->getRefCylindricalR();
      if (rLayer <= 0) return;

      double arcLength = globalHelix.arcLength2DToCylindricalR(rLayer);

      TVector3 pos3D = globalHelix.atArcLength2D(arcLength);

      // +/- deltaPhi corresponding to 5cm left and right (on circle)
      double deltaPhi = 5. / rLayer;

      for (int i = -1; i <= 1; i++) {
        TVector3 thisPos = pos3D;
        thisPos.SetPhi(pos3D.Phi() + i * deltaPhi);
        const Belle2::CDC::IWire iWire = geometryPar.cellId(iclayer, thisPos);
        const auto board = geometryPar.getBoardID(WireID(iclayer, iWire));
        boardCands.push_back(board);
      }
    }


    bool cdcTrackDeadBoardFilter(const Belle2::TrackingUtilities::CDCTrack& aCDCTrack)
    {
      // tunable parameters.
      int minJump = 5;  // minimum layers to be jumped between two hits to trigger search for bad boards (one board covers 3 layers)

      // first check if bad wires are present
      Belle2::TrackingUtilities::StoreWrappedObjPtr< std::vector<unsigned int> > deadBoardsVectorPtr("CDCDeadBoardsVector");

      // nothing to do if no dead boards have been assigned
      if (not deadBoardsVectorPtr.isValid() || (*deadBoardsVectorPtr).size() == 0) return false;
      // for better readability
      const std::vector<unsigned int>& deadBoardsVector = *deadBoardsVectorPtr;

      const CDC::CDCGeometryPar& geometryPar = CDC::CDCGeometryPar::Instance();

      auto& trajectory = aCDCTrack.getStartTrajectory3D();

      Belle2::TrackingUtilities::Helix globalHelix = trajectory.getLocalHelix().helix();
      auto localOrigin = trajectory.getLocalOrigin();
      double arcLengthOffset = globalHelix.passiveMoveBy(-localOrigin);

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

          // TODO: check for max radius

          // direction in case of backcurling tracks
          int dir = (iclayerThis - iclayerPrev) < 0 ? -1 : 1;
          // jump 3 layers to get the onto next board (1 board covers 3 layers)
          unsigned int newlayerThis = iclayerThis - dir * 3;
          unsigned int newlayerPrev = iclayerPrev + dir * 3;

          std::vector<unsigned int> boardCands;
          boardCands.reserve(6);
          addBoardCandsAtLayer(boardCands, globalHelix, newlayerThis, geometryPar);
          addBoardCandsAtLayer(boardCands, globalHelix, newlayerPrev, geometryPar);

          for (auto iboard : boardCands) {
            //std::cout << iboard << std::endl;
            if (std::find(deadBoardsVector.begin(), deadBoardsVector.end(), iboard) != deadBoardsVector.end()) return true;
          }
        }

        prevHitPtr = &thisHit;
      }

      return false;

    }



  }
}
