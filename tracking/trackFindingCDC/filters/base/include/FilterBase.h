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

#include <tracking/trackFindingCDC/findlets/base/ProcessingSignalListener.h>
#include <tracking/trackFindingCDC/ca/CellWeight.h>

#include <framework/core/ModuleParamList.h>
#include <framework/logging/Logger.h>
#include <string>
#include <map>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Base class for filters on a generic object type.
    template<class AObject>
    class FilterBase : public ProcessingSignalListener {

    public:
      /// Type of the object to be analysed.
      typedef AObject Object;

    public:
      /** Expose the set of parameters of the filter to the module parameter list.
       *
       *  Note that not all filters have yet exposed their parameters in this way.
       */
      virtual void exposeParameters(ModuleParamList*, const std::string& = "")
      {
      }

      /** Set a map of parameter key and string values.
       *  Meaning depends on the specific filter class implementation.
       */
      void setParameters(const std::map<std::string, std::string>& parameterMap)
      {
        for (const std::pair<std::string, std::string>& keyValue : parameterMap) {
          setParameter(keyValue.first, keyValue.second);
        }
      }

      /** Sets the parameter of key to the given value.
       *  Base implementation emits a warning for each parameter that is given to it.
       *  Derived class should forward parameters to their superclass if they do not expect them,
       *  such that unexpected parameters propagate to this method.*/
      virtual void setParameter(const std::string& key, const std::string& value)
      {
        // Base has no parameters.
        B2WARNING("Filter received unexpected parameter " << key << " = " << value);
      }

      /** Returns a map of keys to descriptions describing the individual parameters of the filter.
       */
      virtual std::map<std::string, std::string> getParameterDescription()
      {
        return std::map<std::string, std::string>();
      }

      /// Indicates if the filter requires Monte Carlo information.
      virtual bool needsTruthInformation()
      {
        return false;
      }

    public:
      /** Function to evaluate the object.
       *  Base implementation rejects all objects.
       *
       *  @param obj The object to be accepted or rejected.
       *  @return    A finit float value if the object is accepted.
       *             NOT_A_CELL if the object is rejected.
       */
      virtual CellWeight operator()(const Object&)
      {
        return NOT_A_CELL;
      }

      /** Function to evaluate the object.
       *  Base implementation rejects all objects.
       *
       *  @param obj The object to be accepted or rejected.
       *  @return    A finit float value if the object is accepted.
       *             NOT_A_CELL if the object is rejected. Nullptr is always rejected.
       */
      CellWeight operator()(const Object* obj)
      {
        return obj ? operator()(*obj) : NOT_A_CELL;
      }

    };
  }
}
