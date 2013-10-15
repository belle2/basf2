/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCWIREHITPTRSET_H
#define CDCWIREHITPTRSET_H

#include <algorithm>
#include <set>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCWireHit.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// An ordered set of reconstructed facets. Template instance of CDCGenericHitSet<const CDCWireHit *>
    /** See CDCGenericHitSet for all methods and details */
    class CDCWireHitPtrSet : public CDCLocalTracking::UsedTObject {
    public:

      /// Default constructor for ROOT compatibility.
      CDCWireHitPtrSet() {;}

      /// Empty deconstructor
      ~CDCWireHitPtrSet() {;}

      /* ###### poor mans collection template ###### */
    public:
      typedef CDCWireHitPtrSet Collection; ///< The type of this class
    private:
      typedef std::set<const Belle2::CDCLocalTracking::CDCWireHit*> Container;  ///< std::set to be wrapped
    public:
      typedef Container::value_type Item; ///< Value type of this container

    public:
      /// Include all method implementations
#include <tracking/cdcLocalTracking/eventdata/collections/implementation/CDCGenericHitSet.part.h>
    private:
      /// ROOT Macro to make CDCWireHitPtrSet a ROOT class.
      ClassDefInCDCLocalTracking(CDCWireHitPtrSet, 1);


    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCWIREHITPTRSET_H
