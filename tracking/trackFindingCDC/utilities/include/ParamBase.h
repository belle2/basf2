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

#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    /**
     *  Basic type erased parameter for the parameter facade.
     *  @sa ParamList
     */
    class ParamBase {
    public:
      /// Constructor from name, description and forced indicator
      ParamBase(const std::string& name, const std::string& description, bool forced);

      /// Virtual destructor for this interface
      virtual ~ParamBase();

      /// Add this parameter to the given module parameters
      virtual void transferTo(ModuleParamList*) = 0;

      /// Getter for the name of the parameter
      const std::string& getName() const;

      /// Getter for the description of the parameter
      const std::string& getDescription() const;

      /// Indicator whether the parameter is considered forced for the receiving module
      bool isForced() const;
    private:
      /// Memory for the name
      std::string m_name;

      /// Memory for the description
      std::string m_description;

      /// Memory for the forced indicator
      bool m_forced;
    };
  }
}
