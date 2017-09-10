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

#include <tracking/trackFindingCDC/utilities/Param.h>
#include <tracking/trackFindingCDC/utilities/ParamBase.h>

#include <map>
#include <string>
#include <vector>
#include <memory>

namespace Belle2 {
  class ModuleParamList;
  namespace TrackFindingCDC {
    /**
     *  This class provides a more light weight substitute for the similar ModuleParamList.
     *
     *  It is used to communicate parameters between findlets and filters potential
     *  across multiple levels to ultimatly publish them as to the ModuleParamList
     *
     *  Rational:
     *  The ModuleParamList and the ModuleParams have a deep entanglement with boost::python.
     *  In using the ModuleParamList as an interface type to organise the propagation of
     *  parameter we save the compile time dependency to boost::python resulting in a big
     *  reduction in build time of the CDC track finder.
     */
    class ParamList {
    public:
      /// Default constructor
      ParamList();

      /// Default destructor
      ~ParamList();

      /// Add a non-force parameter for the given variable.
      template <typename T>
      void
      addParameter(const std::string& name, T& variable, const std::string& description, T value);

      /// Add a force parameter for the given variable.
      template <typename T>
      void addParameter(const std::string& name, T& variable, const std::string& description);

      /// Check whether a parameter with the given name exists.
      bool hasParameter(const std::string& name) const;

      /// Get a reference to the parameter.
      template<typename T>
      Param<T>& getParameter(const std::string& name);

      /// Return a vector of all registered parameter names.
      std::vector<std::string> getParameterNames() const;

      /// Return the description of the parameter with the given name.
      std::string getParameterDescription(const std::string& name) const;

      /// Add all contained parameters to the given module parameters
      void transferTo(ModuleParamList* moduleParamList);

    private:
      /// Memory for the stored parameters.
      std::map<std::string, std::unique_ptr<ParamBase> > m_paramsByName;
    };
  }
}
