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

#include <tracking/trackFindingCDC/utilities/CompositeProcessingSignalListener.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

// #include <framework/core/ModuleParamList.h>
// #include <framework/logging/Logger.h>

#include <string>
#include <map>
#include <cmath>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {

    /// Base class for filters on a generic object type.
    template<class AObject>
    class Filter : public CompositeProcessingSignalListener {

    public:
      /// Type of the object to be analysed.
      using Object = AObject;

    public:
      /// Make destructor of interface class virtual
      virtual ~Filter() = default;

      /**
       *  Expose the set of parameters of the filter to the module parameter list.
       *
       *  Note that not all filters have yet exposed their parameters in this way.
       */
      virtual void exposeParameters(ModuleParamList* moduleParamList __attribute__((unused)),
                                    const std::string& prefix __attribute__((unused)))
      {
      }

      /// Indicates if the filter requires Monte Carlo information.
      virtual bool needsTruthInformation()
      {
        return false;
      }

    public:
      /**
       *  Function to evaluate the object.
       *  Base implementation rejects all objects.
       *
       *  @param obj The object to be accepted or rejected.
       *  @return    A finit float value if the object is accepted.
       *             NAN if the object is rejected.
       */
      virtual Weight operator()(const Object& obj __attribute__((unused)))
      {
        return NAN;
      }

      /**
       *  Function to evaluate the object.
       *  Base implementation rejects all objects.
       *
       *  @param obj The object to be accepted or rejected.
       *  @return    A finit float value if the object is accepted.
       *             NAN if the object is rejected. Nullptr is always rejected.
       */
      Weight operator()(const Object* obj)
      {
        return obj ? operator()(*obj) : NAN;
      }

    };
  }
}
