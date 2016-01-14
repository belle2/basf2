/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCWireHit;
    class ConformalCDCWireHit;
    class CDCWireHitCluster;
    class CDCFacet;
    class CDCRecoSegment2D;
    class CDCSegmentPair;
    class CDCSegmentTriple;
    class CDCTrack;

    /// Interface for a minimal algorithm part that wants to expose some parameters to a module
    class ClassMnemomics {

    public:
      /// Returns a short name for class CDCWireHit to be used in names of parameters.
      static std::string getParameterName(const CDCWireHit*) { return "wireHit"; }

      /// Returns a short description for class CDCWireHit to be used in descriptions of parameters.
      static std::string getParameterDescription(const CDCWireHit*) { return "wire hit"; }



      /// Returns a short name for class ConformalCDCWireHit to be used in names of parameters.
      static std::string getParameterName(const ConformalCDCWireHit*) { return "conformalHit"; }

      /// Returns a short description for class ConformalCDCWireHit to be used in descriptions of parameters.
      static std::string getParameterDescription(const ConformalCDCWireHit*) { return "conformal hit"; }



      /// Returns a short name for class CDCWireHitCluster to be used in names of parameters.
      static std::string getParameterName(const CDCWireHitCluster*) { return "cluster"; }

      /// Returns a short description for class CDCWireHitCluster to be used in descriptions of parameters.
      static std::string getParameterDescription(const CDCWireHitCluster*) { return "wire hit cluster"; }



      /// Returns a short name for class CDCFacet to be used in names of parameters.
      static std::string getParameterName(const CDCFacet*) { return "facet"; }

      /// Returns a short description for class CDCFacet to be used in descriptions of parameters.
      static std::string getParameterDescription(const CDCFacet*) { return "facet"; }



      /// Returns a short name for class CDCRecoSegment2D to be used in names of parameters.
      static std::string getParameterName(const CDCRecoSegment2D*) { return "segment"; }

      /// Returns a short description for class CDCRecoSegment2D to be used in descriptions of parameters.
      static std::string getParameterDescription(const CDCRecoSegment2D*) { return "segment"; }



      /// Returns a short name for class CDCSegmentPair to be used in names of parameters.
      static std::string getParameterName(const CDCSegmentPair*) { return "segmentPair"; }

      /// Returns a short description for class CDCSegmentPair to be used in descriptions of parameters.
      static std::string getParameterDescription(const CDCSegmentPair*) { return "segment pair"; }



      /// Returns a short name for class CDCSegmentTriple to be used in names of parameters.
      static std::string getParameterName(const CDCSegmentTriple*) { return "segmentTriple"; }

      /// Returns a short description for class CDCSegmentTriple to be used in descriptions of parameters.
      static std::string getParameterDescription(const CDCSegmentTriple*) { return "segment triple"; }



      /// Returns a short name for class CDCTrack to be used in names of parameters.
      static std::string getParameterName(const CDCTrack*) { return "track"; }

      /// Returns a short description for class CDCTrack to be used in descriptions of parameters.
      static std::string getParameterDescription(const CDCTrack*) { return "track"; }


    };

  } //end namespace TrackFindingCDC
} //end namespace Belle2
