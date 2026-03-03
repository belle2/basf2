/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/core/ModuleParam.templateDetails.h>

#include <map>
#include <tuple>

template class Belle2::ModuleParam<std::map<short, unsigned long>>;
template class Belle2::ModuleParam<std::map<int, int>>;

template class Belle2::ModuleParam<std::tuple<double, double>>;
template class Belle2::ModuleParam<std::tuple<double, double, double>>;
