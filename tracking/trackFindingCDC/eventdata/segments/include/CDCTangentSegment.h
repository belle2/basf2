/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/collections/CDCTangentVector.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCFacet.h>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A segment consisting of adjacent tangents.
    class  CDCTangentSegment : public CDCTangentVector {

    public:
      /// Default constructor for ROOT compatibility.
      CDCTangentSegment() {;}

      /// Empty deconstructor
      ~CDCTangentSegment() {;}

      /// Takes all distinct tangents from the facets in the path - Note! there is no particular order of the tangents in the segment.
      static CDCTangentSegment condense(const std::vector<const Belle2::TrackFindingCDC::CDCFacet* >& facetPath);


    private:
      /// ROOT Macro to make CDCTangentSegment a ROOT class.
      TRACKFINDINGCDC_SwitchableClassDef(CDCTangentSegment, 1);


    };

  }
}


