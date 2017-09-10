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

#include <tracking/trackFindingCDC/utilities/ParamBase.h>

#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {

    /**
     *  Concrete typed parameter for the parameter facade.
     *  @sa ParamList
     */
    template<class T>
    class Param : public ParamBase {
    public:
      /// Construct a non-forced parameter.
      Param(const std::string& name, T* variable, const std::string& description, T value);

      /// Construct a forced parameter.
      Param(const std::string& name, T* variable, const std::string& description);

      /// Default destructor
      ~Param();

      /// Get the current value of the variable
      T getValue();

      /// Reset the value of the contained variable.
      void setValue(T t);

      /// Reset the default value and the value of the contained variable.
      void setDefaultValue(T t);

      /// Add this parameter to the given module parameters
      void transferTo(ModuleParamList* moduleParamList) override;

    private:
      /// Pointer to the destination of the parameter value.
      T* m_variable;

      /// Default value of the parameter.
      T m_defaultValue;
    };
  }
}
