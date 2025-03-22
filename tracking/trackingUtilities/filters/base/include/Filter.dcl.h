/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/filters/base/Filter.fwd.h>

#include <tracking/trackingUtilities/utilities/CompositeProcessingSignalListener.h>

#include <tracking/trackingUtilities/numerics/Weight.h>

#include <string>

#include <vector>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackingUtilities {

    /// Base class for filters on a generic object type.
    template<class AObject>
    class Filter : public CompositeProcessingSignalListener {

    public:
      /// Type of the object to be analysed.
      using Object = AObject;

      /// Mark this class as the basic interface
      using Interface = Filter<AObject>;

    public:
      /// Default constructor
      Filter();

      /// Make destructor of interface class virtual
      virtual ~Filter();

      /// Indicates if the filter requires Monte Carlo information.
      virtual bool needsTruthInformation();

    public:
      /**
       *  Function to evaluate the object.
       *  Base implementation accepts all objects.
       *
       *  @param obj The object to be accepted or rejected.
       *  @return    A finite float value if the object is accepted.
       *             NAN if the object is rejected.
       */
      virtual Weight operator()(const Object& obj);

      /**
       *  Function to evaluate the object.
       *  Base implementation accepts all objects, except nullptr.
       *
       *  @param obj The object to be accepted or rejected.
       *  @return    A finit float value if the object is accepted.
       *             NAN if the object is rejected. Nullptr is always rejected.
       */
      Weight operator()(const Object* obj);

      /**
       * Function to evaluate a vector of objects
       * Base implementation applies the function to each object. Can be optimized for MVA filters
       * @param objs A vector of pointers to objects
       * @return A vector of float or NAN values. See above
       */
      virtual std::vector<float> operator()(const std::vector <Object*>& objs);
    };
  }
}
