/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOHIT3DVECTOR_H
#define CDCRECOHIT3DVECTOR_H

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoHit3D.h>
#include <tracking/cdcLocalTracking/eventdata/collections/CDCGenHitVector.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// A vector of wire hits. Template instance of CDCGenHitVector<CDCRecoHit3D>
    /** See CDCGenHitVector for all methods and details */
    class CDCRecoHit3DVector : public CDCGenHitVector<Belle2::CDCLocalTracking::CDCRecoHit3D> {

    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoHit3DVector() {;}

      /// Empty deconstructor
      ~CDCRecoHit3DVector() {;}

    public:
      FloatType getSquaredZDist(const CDCTrajectorySZ& trajectorySZ) const;

    private:
      /// ROOT Macro to make CDCRecoHit3DVector a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoHit3DVector, 1);

    }; //class CDCRecoHit3DVector

  } // namespace CDCLocalTracking
} // namespace Belle2

#endif // CDCRECOHIT3DVECTOR_H
