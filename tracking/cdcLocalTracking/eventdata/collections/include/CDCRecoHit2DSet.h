/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOHIT2DSET_H
#define CDCRECOHIT2DSET_H

#include <algorithm>
#include <set>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoHit2D.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// An ordered set of reconstructed facets. Template instance of CDCGenericHitSet<CDCRecoHit2D>
    /** See CDCGenericHitSet for all methods and details */
    class CDCRecoHit2DSet : public CDCLocalTracking::UsedTObject {
    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoHit2DSet() {;}

      /// Empty deconstructor
      ~CDCRecoHit2DSet() {;}

      /* ###### poor mans set template ###### */
    public:
      typedef CDCRecoHit2DSet Collection; ///< The type of this class
    private:
      typedef std::set<Belle2::CDCLocalTracking::CDCRecoHit2D> Container; ///< std::set to be wrapped
    public:
      typedef Container::value_type Item; ///< Value type of this container

    public:
      /// Include all method implementations
#include <tracking/cdcLocalTracking/eventdata/collections/implementation/CDCGenericHitSet.part.h>
    private:
      /// ROOT Macro to make CDCRecoHit2DSet a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoHit2DSet, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRECOHIT2DSET_H
