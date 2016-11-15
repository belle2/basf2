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
    class CDCConformalHit;
    class CDCWireHitCluster;
    class CDCFacet;
    class CDCSegment2D;
    class CDCSegmentPair;
    class CDCAxialSegmentPair;
    class CDCSegmentTriple;
    class CDCTrack;

    template<class T>
    class WeightedRelation;

    /// Utility class to lookup identifier and description for event data objects useful to define parameters.
    class ClassMnemomics {

    public:
      /// Returns a short name for class CDCWireHit to be used in names of parameters.
      static std::string getParameterName(const CDCWireHit* dispatchTag __attribute__((unused)))
      {
        return "wireHit";
      }

      /// Returns a short description for class CDCWireHit to be used in descriptions of parameters.
      static std::string getParameterDescription(const CDCWireHit* dispatchTag __attribute__((unused)))
      {
        return "wire hit";
      }

      /// Returns a short name for class CDCConformalHit to be used in names of parameters.
      static std::string getParameterName(const CDCConformalHit* dispatchTag __attribute__((unused)))
      {
        return "conformalHit";
      }

      /// Returns a short description for class CDCConformalHit to be used in descriptions of parameters.
      static std::string getParameterDescription(const CDCConformalHit* dispatchTag __attribute__((unused)))
      {
        return "conformal hit";
      }

      /// Returns a short name for class CDCWireHitCluster to be used in names of parameters.
      static std::string getParameterName(const CDCWireHitCluster* dispatchTag __attribute__((unused)))
      {
        return "cluster";
      }

      /// Returns a short description for class CDCWireHitCluster to be used in descriptions of parameters.
      static std::string getParameterDescription(const CDCWireHitCluster* dispatchTag __attribute__((unused)))
      {
        return "wire hit cluster";
      }

      /// Returns a short name for class CDCFacet to be used in names of parameters.
      static std::string getParameterName(const CDCFacet* dispatchTag __attribute__((unused)))
      {
        return "facet";
      }

      /// Returns a short description for class CDCFacet to be used in descriptions of parameters.
      static std::string getParameterDescription(const CDCFacet* dispatchTag __attribute__((unused)))
      {
        return "facet";
      }

      /// Returns a short name for class CDCSegment2D to be used in names of parameters.
      static std::string getParameterName(const CDCSegment2D* dispatchTag __attribute__((unused)))
      {
        return "segment";
      }

      /// Returns a short description for class CDCSegment2D to be used in descriptions of parameters.
      static std::string getParameterDescription(const CDCSegment2D* dispatchTag __attribute__((unused)))
      {
        return "segment";
      }

      /// Returns a short name for class CDCSegmentPair to be used in names of parameters.
      static std::string getParameterName(const CDCSegmentPair* dispatchTag __attribute__((unused)))
      {
        return "segmentPair";
      }

      /// Returns a short description for class CDCSegmentPair to be used in descriptions of parameters.
      static std::string getParameterDescription(const CDCSegmentPair* dispatchTag __attribute__((unused)))
      {
        return "segment pair";
      }

      /// Returns a short name for class CDCAxialSegmentPair to be used in names of parameters.
      static std::string getParameterName(const CDCAxialSegmentPair* dispatchTag __attribute__((unused)))
      {
        return "axialSegmentPair";
      }

      /// Returns a short description for class CDCAxialSegmentPair to be used in descriptions of parameters.
      static std::string getParameterDescription(const CDCAxialSegmentPair* tag __attribute__((unused)))
      {
        return "axial segment pair";
      }

      /// Returns a short name for class CDCSegmentTriple to be used in names of parameters.
      static std::string getParameterName(const CDCSegmentTriple* dispatchTag __attribute__((unused)))
      {
        return "segmentTriple";
      }

      /// Returns a short description for class CDCSegmentTriple to be used in descriptions of parameters.
      static std::string getParameterDescription(const CDCSegmentTriple* dispatchTag __attribute__((unused)))
      {
        return "segment triple";
      }

      /// Returns a short name for class CDCTrack to be used in names of parameters.
      static std::string getParameterName(const CDCTrack* dispatchTag __attribute__((unused)))
      {
        return "track";
      }

      /// Returns a short description for class CDCTrack to be used in descriptions of parameters.
      static std::string getParameterDescription(const CDCTrack* dispatchTag __attribute__((unused)))
      {
        return "track";
      }

      /// Returns a short name for class WeightedRelation<T> to be used in names of parameters.
      template <class T>
      static std::string getParameterName(const WeightedRelation<T>* dispatchTag __attribute__((unused)))
      {
        return getParameterName((T*)(nullptr)) + "Relation";
      }

      /// Returns a short description for class WeightedRelation<T> to be used in descriptions.
      template <class T>
      static std::string getParameterDescription(const WeightedRelation<T>* dispatchTag __attribute__((unused)))
      {
        return getParameterDescription((T*)(nullptr)) + " relation";
      }

    };

  }
}
