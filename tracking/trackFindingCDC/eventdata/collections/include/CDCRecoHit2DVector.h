/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOHIT2DVECTOR_H
#define CDCRECOHIT2DVECTOR_H

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoHit2D.h>
#include <tracking/cdcLocalTracking/eventdata/collections/CDCGenHitVector.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// A vector of wire hits. Template instance of CDCGenHitVector<CDCRecoHit2D>
    /** See CDCGenHitVector for all methods and details */
    class CDCRecoHit2DVector : public CDCGenHitVector<Belle2::CDCLocalTracking::CDCRecoHit2D> {

    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoHit2DVector() {;}

      /// Empty deconstructor
      ~CDCRecoHit2DVector() {;}

    private:
      /// ROOT Macro to make CDCRecoHit2DVector a ROOT class.
      CDCLOCALTRACKING_SwitchableClassDef(CDCRecoHit2DVector, 1);

    }; //class CDCRecoHit2DVector

  } // namespace CDCLocalTracking
} // namespace Belle2

#endif // CDCRECOHIT2DVECTOR_H
