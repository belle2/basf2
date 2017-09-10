/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/utilities/Param.h>
#include <tracking/trackFindingCDC/utilities/Param.icc.h>

#include <tracking/trackFindingCDC/utilities/ParameterVariant.h>

using namespace Belle2;
using namespace TrackFindingCDC;

/*
 *  Some predefined instances of the concrete parameter.
 *  Instantiation is bundled in this translation unit
 *  such that the various users are relieved of the include
 *  burden of ModuleParamList and boost::python.
 *
 *  Other users may extend this list in a different translation unit,
 *  depending on whether the used types are heavy or not.
 */
template class TrackFindingCDC::Param<bool>;
template class TrackFindingCDC::Param<int>;
template class TrackFindingCDC::Param<unsigned int>;
template class TrackFindingCDC::Param<double>;
template class TrackFindingCDC::Param<std::string>;

template class TrackFindingCDC::Param<std::vector<bool>>;
template class TrackFindingCDC::Param<std::vector<int>>;
template class TrackFindingCDC::Param<std::vector<unsigned int>>;
template class TrackFindingCDC::Param<std::vector<float>>;
template class TrackFindingCDC::Param<std::vector<double>>;
template class TrackFindingCDC::Param<std::vector<std::string>>;

template class TrackFindingCDC::Param<std::map<short, unsigned long>>;
template class TrackFindingCDC::Param<std::map<int, int>>;

template class TrackFindingCDC::Param<std::tuple<double, double, double>>;

template class TrackFindingCDC::Param<ParameterVariant>;
template class TrackFindingCDC::Param<ParameterVariantMap>;
template class TrackFindingCDC::Param<std::vector<ParameterVariantMap> >;
