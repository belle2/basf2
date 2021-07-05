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

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <string>

/**
 *  @file
 *  Utility functions to lookup identifier and description for event data objects useful to define moduel parameters.
 *
 *  The functions are typically called unqualified to be found at compile time by aragument dependent lookup.
 */

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCWireHit;
    class CDCWireHitCluster;
    class CDCFacet;
    class CDCSegment2D;
    class CDCSegmentPair;
    class CDCAxialSegmentPair;
    class CDCSegmentTriple;
    class CDCTrack;

    /// Returns a short name for class CDCWireHit to be used in names of parameters.
    inline std::string getClassMnemomicParameterName(const CDCWireHit* dispatchTag __attribute__((unused)))
    {
      return "wireHit";
    }

    /// Returns a short description for class CDCWireHit to be used in descriptions of parameters.
    inline std::string getClassMnemomicParameterDescription(const CDCWireHit* dispatchTag __attribute__((unused)))
    {
      return "wire hit";
    }

    /// Returns a short name for class CDCWireHitCluster to be used in names of parameters.
    inline std::string getClassMnemomicParameterName(const CDCWireHitCluster* dispatchTag __attribute__((unused)))
    {
      return "cluster";
    }

    /// Returns a short description for class CDCWireHitCluster to be used in descriptions of parameters.
    inline std::string getClassMnemomicParameterDescription(const CDCWireHitCluster* dispatchTag __attribute__((unused)))
    {
      return "wire hit cluster";
    }

    /// Returns a short name for class CDCFacet to be used in names of parameters.
    inline std::string getClassMnemomicParameterName(const CDCFacet* dispatchTag __attribute__((unused)))
    {
      return "facet";
    }

    /// Returns a short description for class CDCFacet to be used in descriptions of parameters.
    inline std::string getClassMnemomicParameterDescription(const CDCFacet* dispatchTag __attribute__((unused)))
    {
      return "facet";
    }

    /// Returns a short name for class CDCSegment2D to be used in names of parameters.
    inline std::string getClassMnemomicParameterName(const CDCSegment2D* dispatchTag __attribute__((unused)))
    {
      return "segment";
    }

    /// Returns a short description for class CDCSegment2D to be used in descriptions of parameters.
    inline std::string getClassMnemomicParameterDescription(const CDCSegment2D* dispatchTag __attribute__((unused)))
    {
      return "segment";
    }

    /// Returns a short name for class CDCSegmentPair to be used in names of parameters.
    inline std::string getClassMnemomicParameterName(const CDCSegmentPair* dispatchTag __attribute__((unused)))
    {
      return "segmentPair";
    }

    /// Returns a short description for class CDCSegmentPair to be used in descriptions of parameters.
    inline std::string getClassMnemomicParameterDescription(const CDCSegmentPair* dispatchTag __attribute__((unused)))
    {
      return "segment pair";
    }

    /// Returns a short name for class CDCAxialSegmentPair to be used in names of parameters.
    inline std::string getClassMnemomicParameterName(const CDCAxialSegmentPair* dispatchTag __attribute__((unused)))
    {
      return "axialSegmentPair";
    }

    /// Returns a short description for class CDCAxialSegmentPair to be used in descriptions of parameters.
    inline std::string getClassMnemomicParameterDescription(const CDCAxialSegmentPair* tag __attribute__((unused)))
    {
      return "axial segment pair";
    }

    /// Returns a short name for class CDCSegmentTriple to be used in names of parameters.
    inline std::string getClassMnemomicParameterName(const CDCSegmentTriple* dispatchTag __attribute__((unused)))
    {
      return "segmentTriple";
    }

    /// Returns a short description for class CDCSegmentTriple to be used in descriptions of parameters.
    inline std::string getClassMnemomicParameterDescription(const CDCSegmentTriple* dispatchTag __attribute__((unused)))
    {
      return "segment triple";
    }

    /// Returns a short name for class CDCTrack to be used in names of parameters.
    inline std::string getClassMnemomicParameterName(const CDCTrack* dispatchTag __attribute__((unused)))
    {
      return "track";
    }

    /// Returns a short description for class CDCTrack to be used in descriptions of parameters.
    inline std::string getClassMnemomicParameterDescription(const CDCTrack* dispatchTag __attribute__((unused)))
    {
      return "track";
    }

    /// Returns a short name for class WeightedRelation<T> to be used in names of parameters.
    template <class T>
    inline std::string getClassMnemomicParameterName(const WeightedRelation<T>* dispatchTag __attribute__((unused)))
    {
      return getClassMnemomicParameterName((T*)(nullptr)) + "Relation";
    }

    /// Returns a short description for class WeightedRelation<T> to be used in descriptions.
    template <class T>
    inline std::string getClassMnemomicParameterDescription(const WeightedRelation<T>* dispatchTag __attribute__((unused)))
    {
      return getClassMnemomicParameterDescription((T*)(nullptr)) + " relation";
    }

    /// Returns a short name for class WeightedRelation<From, To> to be used in names of parameters.
    template <class From, class To>
    inline std::string getClassMnemomicParameterName(const WeightedRelation<From, To>* dispatchTag __attribute__((unused)))
    {
      return prefixed(getClassMnemomicParameterName((From*)(nullptr)) + "To",
                      getClassMnemomicParameterName((To*)(nullptr)) + "Relation");
    }

    /// Returns a short description for class WeightedRelation<From, To> to be used in descriptions.
    template <class From, class To>
    inline std::string getClassMnemomicParameterDescription(const WeightedRelation<From, To>* dispatchTag __attribute__((unused)))
    {
      return getClassMnemomicParameterDescription((From*)(nullptr)) + " to " +
             getClassMnemomicParameterDescription((To*)(nullptr)) + " relation";
    }

  }
}
