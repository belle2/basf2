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

#include <framework/core/ModuleParam.icc.h>

#include <tracking/trackFindingCDC/utilities/ParameterVariant.h>

#include <map>
#include <vector>
#include <array>
#include <tuple>
#include <string>

namespace Belle2 {
  /*
   *  Some predefined instances of the concrete parameter.
   *  Instantiation is bundled in this translation unit
   *  such that the various users are relieved of the include
   *  burden of ModuleParamList and boost::python.
   *
   *  Other users may extend this list in a different translation unit,
   *  depending on whether the used types are heavy or not.
   */
  extern template class Belle2::ModuleParam<bool>;
  extern template class Belle2::ModuleParam<int>;
  extern template class Belle2::ModuleParam<unsigned int>;
  extern template class Belle2::ModuleParam<double>;
  extern template class Belle2::ModuleParam<std::string>;

  extern template class Belle2::ModuleParam<std::vector<bool>>;
  extern template class Belle2::ModuleParam<std::vector<int>>;
  extern template class Belle2::ModuleParam<std::vector<unsigned int>>;
  extern template class Belle2::ModuleParam<std::vector<float>>;
  extern template class Belle2::ModuleParam<std::vector<double>>;
  extern template class Belle2::ModuleParam<std::vector<std::string>>;

  extern template class Belle2::ModuleParam<std::map<short, unsigned long>>;
  extern template class Belle2::ModuleParam<std::map<int, int>>;

  extern template class Belle2::ModuleParam<std::tuple<double, double>>;
  extern template class Belle2::ModuleParam<std::tuple<double, double, double>>;
}
