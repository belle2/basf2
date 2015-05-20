/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#ifndef CDCFACETVECTOR_H
#define CDCFACETVECTOR_H

#include <tracking/trackFindingCDC/eventdata/entities/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/collections/CDCGenHitVector.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A vector of wire hits. Template instance of CDCGenHitVector<CDCFacet>
    /** See CDCGenHitVector for all methods and details */
    class CDCFacetVector : public CDCGenHitVector<Belle2::TrackFindingCDC::CDCFacet> {

    public:

      /// Default constructor for ROOT compatibility.
      CDCFacetVector() {;}

      /// Empty deconstructor
      ~CDCFacetVector() {;}

    private:
      /// ROOT Macro to make CDCFacetVector a ROOT class.
      TRACKFINDINGCDC_SwitchableClassDef(CDCFacetVector, 1);

    }; //class CDCFacetVector

  } // namespace TrackFindingCDC
} // namespace Belle2

#endif // CDCFACETVECTOR_H
