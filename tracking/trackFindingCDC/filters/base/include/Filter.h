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

#include <tracking/trackFindingCDC/utilities/ParamList.h>

#include <string>
#include <cmath>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class ParamList;

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
       *
       *  This method is deprecated as the exposeParams below uses a less compile heavy equivalent.
       */
      [[deprecated("Use exposeParams below. Same for Findlet::exposeParameters")]]
      virtual void exposeParameters(ModuleParamList* moduleParamList,
                                    const std::string& prefix)
      {
        ParamList paramList;
        this->exposeParams(&paramList, prefix);
        paramList.transferTo(moduleParamList);
      }

      /**
       *  Expose the set of parameters of the filter to the module parameter list.
       *
       *  Note that not all filters have yet exposed their parameters in this way.
       */
      virtual void exposeParams(ParamList* paramList __attribute__((unused)),
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
       *  Base implementation accepts all objects.
       *
       *  @param obj The object to be accepted or rejected.
       *  @return    A finite float value if the object is accepted.
       *             NAN if the object is rejected.
       */
      virtual Weight operator()(const Object& obj __attribute__((unused)))
      {
        return 1;
      }

      /**
       *  Function to evaluate the object.
       *  Base implementation accepts all objects, except nullptr.
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
