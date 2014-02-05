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

//#include <vector>
//#include <algorithm>

//#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
//#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCWireHit.h>
#include <tracking/cdcLocalTracking/eventdata/collections/CDCGenHitVector.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    //typedef CDCGenHitVector<Belle2::CDCLocalTracking::CDCWireHit> CDCWireHitVector;

    /// A vector of wire hits. Template instance of CDCGenericHitVector<CDCWireHit>
    /** See CDCGenericHitVector for all methods and details */
    class CDCWireHitVector : public CDCGenHitVector<Belle2::CDCLocalTracking::CDCWireHit> {
      //class CDCWireHitVector : public CDCLocalTracking::UsedTObject {
    public:

      /// Default constructor for ROOT compatibility.
      CDCWireHitVector() {;}

      /// Empty deconstructor
      ~CDCWireHitVector() {;}

      /*
      public:
      typedef CDCWireHitVector Collection; ///< The type of this class
      private:
      typedef std::vector<Belle2::CDCLocalTracking::CDCWireHit> Container; ///< std::vector to be wrapped

      public:
      typedef Container::value_type Item; ///< Value type of this container

      public:
      /// Include all method implementations
      #include <tracking/cdcLocalTracking/eventdata/collections/implementation/CDCGenericHitVector.part.h>
      */
    private:
      /// ROOT Macro to make CDCWireHitVector a ROOT class.
      ClassDefInCDCLocalTracking(CDCWireHitVector, 1);

    }; //class


  } // namespace CDCLocalTracking
} // namespace Belle2

#endif // CDCWIREHITVECTOR_H
