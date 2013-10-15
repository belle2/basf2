/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOTANGENTSET_H
#define CDCRECOTANGENTSET_H

#include <algorithm>
#include <set>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoTangent.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// An ordered set of reconstructed facets. Template instance of CDCGenericHitSet<CDCRecoTangent>
    /** See CDCGenericHitSet for all methods and details */
    class CDCRecoTangentSet : public CDCLocalTracking::UsedTObject {
    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoTangentSet() {;}

      /// Empty deconstructor
      ~CDCRecoTangentSet() {;}

      /* ###### poor mans set template ###### */
    public:
      typedef CDCRecoTangentSet Collection; ///< The type of this class
    private:
      typedef std::set<Belle2::CDCLocalTracking::CDCRecoTangent> Container; ///< std::set to be wrapped
    public:
      typedef Container::value_type Item; ///< Value type of this container

    public:
      /// Include all method implementations
#include <tracking/cdcLocalTracking/eventdata/collections/implementation/CDCGenericHitSet.part.h>
    private:
      /// ROOT Macro to make CDCRecoTangentSet a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoTangentSet, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRECOTANGENTSET_H
