/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOSEGMENT3D_H
#define CDCRECOSEGMENT3D_H

#include "CDCRecoSegment2D.h"

#include <tracking/cdcLocalTracking/eventdata/collections/CDCRecoHit3DVector.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// A segment consisting of three dimensional reconstructed hits.
    class CDCRecoSegment3D : public CDCRecoHit3DVector {

    public:
      /// Default constructor for ROOT compatibility.
      CDCRecoSegment3D() {;}

      /// Empty deconstructor
      ~CDCRecoSegment3D() {;}

      static CDCRecoSegment3D reconstruct(const CDCRecoSegment2D& segment2D, const CDCTrajectory2D& trajectory2D) {
        CDCRecoSegment3D segment3D;
        for (const CDCRecoHit2D & recoHit2D : segment2D) {
          CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit2D, trajectory2D);
          segment3D.push_back(recoHit3D);
        }
        return segment3D;

      }


    private:
      /// ROOT Macro to make CDCRecoSegment3D a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoSegment3D, 1);


    };

  }
}


#endif //CDCRECOSEGMENT3D_H



