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

#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/eventdata/collections/CDCGenHitVector.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A vector of wire hits. Template instance of CDCGenHitVector<CDCRecoHit2D>
    /** See CDCGenHitVector for all methods and details */
    class CDCRecoHit2DVector : public CDCGenHitVector<Belle2::TrackFindingCDC::CDCRecoHit2D> {

    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoHit2DVector() {;}

      /// Empty deconstructor
      ~CDCRecoHit2DVector() {;}

    private:

    }; //class CDCRecoHit2DVector

  } // namespace TrackFindingCDC
} // namespace Belle2

