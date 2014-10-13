/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCWIREHITVECTOR_H
#define CDCWIREHITVECTOR_H

#include <tracking/cdcLocalTracking/eventdata/entities/CDCWireHit.h>
#include <tracking/cdcLocalTracking/eventdata/collections/CDCGenHitVector.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// A vector of wire hits. Template instance of CDCGenHitVector<CDCWireHit>
    /** See CDCGenHitVector for all methods and details */
    class CDCWireHitVector : public CDCGenHitVector<Belle2::CDCLocalTracking::CDCWireHit> {

    public:

      /// Default constructor for ROOT compatibility.
      CDCWireHitVector() {;}

      /// Empty deconstructor
      ~CDCWireHitVector() {;}

    private:
      /// ROOT Macro to make CDCWireHitVector a ROOT class.
      CDCLOCALTRACKING_SwitchableClassDef(CDCWireHitVector, 1);

    }; //class


  } // namespace CDCLocalTracking
} // namespace Belle2

#endif // CDCWIREHITVECTOR_H
