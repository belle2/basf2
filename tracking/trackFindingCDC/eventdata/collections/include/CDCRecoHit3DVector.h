/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/BasicTypes.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/collections/CDCGenHitVector.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A vector of wire hits. Template instance of CDCGenHitVector<CDCRecoHit3D>
    /** See CDCGenHitVector for all methods and details */
    class CDCRecoHit3DVector : public CDCGenHitVector<Belle2::TrackFindingCDC::CDCRecoHit3D> {

    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoHit3DVector() {;}

      /// Empty deconstructor
      ~CDCRecoHit3DVector() {;}

    }; //class CDCRecoHit3DVector

  } // namespace TrackFindingCDC
} // namespace Belle2

