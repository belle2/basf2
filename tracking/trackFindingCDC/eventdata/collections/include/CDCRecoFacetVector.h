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

#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoFacet.h>
#include <tracking/trackFindingCDC/eventdata/collections/CDCGenHitVector.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A vector of wire hits. Template instance of CDCGenHitVector<CDCRecoFacet>
    /** See CDCGenHitVector for all methods and details */
    class CDCRecoFacetVector : public CDCGenHitVector<Belle2::TrackFindingCDC::CDCRecoFacet> {

    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoFacetVector() {;}

      /// Empty deconstructor
      ~CDCRecoFacetVector() {;}

    private:
      /// ROOT Macro to make CDCRecoFacetVector a ROOT class.
      TRACKFINDINGCDC_SwitchableClassDef(CDCRecoFacetVector, 1);

    }; //class CDCRecoFacetVector

  } // namespace TrackFindingCDC
} // namespace Belle2

#endif // CDCRECOFACETVECTOR_H
