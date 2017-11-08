/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

// #include <boost/variant/variant_fwd.hpp>

// #include <map>
// #include <vector>
#include <string>
#include <memory>

namespace Belle2 {
  class ModuleParamList;
  namespace TrackFindingCDC {
    /// Variant of types allowed to be forwarded to a filter selected at runtime
    // using FilterParamVariant = boost::variant<bool, int, double, std::string, std::vector<std::string> >;

    /**
     *  Facade for a map of parameters that should be forwarded to a filter selected at runtime.
     *
     *  This class provides an intermediate space for a map of parameters from the steering that
     *  wait to be assigned to a concrete filter instance also selected in the steering.
     *  To achieve this it uses a map of strings to a variant such that it is able to forward
     *  parameters of different types to the filter in a type-safe way.
     *
     *  The present implementation now uses the PImpl idiom to hide the bulk of internal details of
     *  boost::variant from all the users, which reduced the compilation time by a good 20%.
     *
     *  The parameters can still be inspected by getValues(), but the user must include the
     *  implementation (e.g. #include <boost/variant/variant.hpp>) to be able to inspect them.
     */
    class FilterParamMap {

    public:
      /// Default constructor
      FilterParamMap();

      /// Default destructor
      ~FilterParamMap();

      /// Create one parameter in the parameter list for the module with the given name and desciption.
      void
      addParameter(ModuleParamList* moduleParamList, const std::string& name, const std::string& description);

      /// Unpack the map of parameters and forward them to the concrete filter.
      void assignTo(ModuleParamList* filterModuleParamList);

      /// Getter for the currently stored values
      // std::map<std::string, FilterParamVariant> getValues() const;

    private:
      /// Forward declartion of implementation.
      class Impl;

      /// Pointer to implementation hiding the details.
      std::unique_ptr<Impl> m_impl;
    };
  }
}
