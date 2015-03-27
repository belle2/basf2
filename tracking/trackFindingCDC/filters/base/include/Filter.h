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

#include <tracking/trackFindingCDC/algorithms/CellWeight.h>

#include <framework/logging/Logger.h>
#include <string>
#include <map>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Base class for filters on a generic object type.
    template<class Object_>
    class Filter {

    public:
      /// Type of the object to be analysed.
      typedef Object_ Object;

    public:
      /// Constructor of the filter.
      Filter() {;}

      /// Making destructor virtual
      ~Filter() {;}

      /// Initialize the filter before event processing.
      virtual void initialize() {;}

      /// Terminate the filter after event processing.
      virtual void terminate() {;}

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
    };
  }
}
