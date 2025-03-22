/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>

#include <tracking/trackingUtilities/utilities/StringManipulation.h>

#include <string>

/**
 *  @file
 *  Utility functions to lookup identifier and description for event data objects useful to define module parameters.
 *
 *  The functions are typically called unqualified to be found at compile time by argument dependent lookup.
 */

namespace Belle2 {

  namespace TrackingUtilities {

    class CDCWireHit;
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
