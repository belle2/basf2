/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/utilities/ModuleParamInstances.h>

#include <framework/core/ModuleParam.icc.h>

#include <map>
#include <vector>
#include <array>
#include <tuple>
#include <string>

using namespace Belle2;
using namespace TrackFindingCDC;

template class Belle2::ModuleParam<bool>;
template class Belle2::ModuleParam<int>;
template class Belle2::ModuleParam<unsigned int>;
template class Belle2::ModuleParam<double>;
template class Belle2::ModuleParam<std::string>;

template class Belle2::ModuleParam<std::vector<bool>>;
template class Belle2::ModuleParam<std::vector<int>>;
template class Belle2::ModuleParam<std::vector<unsigned int>>;
template class Belle2::ModuleParam<std::vector<float>>;
template class Belle2::ModuleParam<std::vector<double>>;
template class Belle2::ModuleParam<std::vector<std::string>>;

template class Belle2::ModuleParam<std::map<short, unsigned long>>;
template class Belle2::ModuleParam<std::map<int, int>>;

template class Belle2::ModuleParam<std::tuple<double, double, double>>;
