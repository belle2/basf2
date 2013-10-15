/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOFACETVECTOR_H
#define CDCRECOFACETVECTOR_H

#include <vector>
#include <algorithm>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoFacet.h>


namespace Belle2 {
  namespace CDCLocalTracking {

    /// A vector of reconstructed facets. Template instance of CDCGenericHitVector<RecoFacet>
    /** See CDCGenericHitVector for all methods and details */
    class CDCRecoFacetVector : public CDCLocalTracking::UsedTObject {
    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoFacetVector() {;}

      /// Empty deconstructor
      ~CDCRecoFacetVector() {;}

      /* ###### poor mans collection template ###### */
    public:
      typedef CDCRecoFacetVector Collection; ///< The type of this class
    private:
      typedef std::vector<Belle2::CDCLocalTracking::CDCRecoFacet> Container; ///< std::vector to be wrapped
    public:
      typedef Container::value_type Item; ///< Value type of this container

    public:
      /// Include all method implementations
#include <tracking/cdcLocalTracking/eventdata/collections/implementation/CDCGenericHitVector.part.h>
    private:
      /// ROOT Macro to make CDCRecoFacetVector a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoFacetVector, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2

#endif // CDCRECOFACETVECTOR_H
