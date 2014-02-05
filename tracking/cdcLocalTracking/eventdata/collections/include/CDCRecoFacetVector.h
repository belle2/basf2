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

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoFacet.h>
#include <tracking/cdcLocalTracking/eventdata/collections/CDCGenHitVector.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// A vector of wire hits. Template instance of CDCGenHitVector<CDCRecoFacet>
    /** See CDCGenHitVector for all methods and details */
    class CDCRecoFacetVector : public CDCGenHitVector<Belle2::CDCLocalTracking::CDCRecoFacet> {

    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoFacetVector() {;}

      /// Empty deconstructor
      ~CDCRecoFacetVector() {;}

    private:
      /// ROOT Macro to make CDCRecoFacetVector a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoFacetVector, 1);

    }; //class CDCRecoFacetVector

  } // namespace CDCLocalTracking
} // namespace Belle2

#endif // CDCRECOFACETVECTOR_H
