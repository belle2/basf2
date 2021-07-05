/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/core/ModuleParam.templateDetails.h>

#include <map>
#include <tuple>

template class Belle2::ModuleParam<std::map<short, unsigned long>>;
template class Belle2::ModuleParam<std::map<int, int>>;

template class Belle2::ModuleParam<std::tuple<double, double>>;
template class Belle2::ModuleParam<std::tuple<double, double, double>>;
