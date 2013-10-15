/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOFACETSET_H
#define CDCRECOFACETSET_H

#include <algorithm>
#include <set>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoFacet.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// An ordered set of reconstructed facets. Template instance of CDCGenericHitSet<RecoFacet>
    /** See CDCGenericHitSet for all methods and details */
    class CDCRecoFacetSet : public CDCLocalTracking::UsedTObject {
    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoFacetSet() {;}

      /// Empty deconstructor
      ~CDCRecoFacetSet() {;}

      /* ###### poor mans set template ###### */
    public:
      typedef CDCRecoFacetSet Collection; ///< The type of this class
    private:
      typedef std::set<Belle2::CDCLocalTracking::CDCRecoFacet> Container; ///< std::set to be wrapped
    public:
      typedef Container::value_type Item; ///< Value type of this container

    public:
      /// Include all method implementations
#include <tracking/cdcLocalTracking/eventdata/collections/implementation/CDCGenericHitSet.part.h>
    private:
      /// ROOT Macro to make CDCRecoFacetSet a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoFacetSet, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRECOFACETSET_h
