/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/utilities/ParameterVariant.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/core/ModuleParam.templateDetails.h>

#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

AssignParameterVisitor::AssignParameterVisitor(ModuleParamList* moduleParamList,
                                               const std::string& paramName)
  : m_moduleParamList(moduleParamList)
  , m_paramName(paramName)
{
}

/// Assign the value to a keyword in the dictionary
template <class T>
void AssignParameterVisitor::operator()(const T& t) const
{
  B2DEBUG(200, "Received parameter of type " << ModuleParam<T>::TypeInfo());
  m_moduleParamList->getParameter<T>(m_paramName).setDefaultValue(t);
}

// Make explicit template function instances of
// namespace Belle2 {
//   namespace TrackingFindingCDC {
//     /// template<typename T> AssignParameterVisitor::operator()(const T&) const;
//     /// for each of the parameter types in ParameterVariant
//     void instantiate_AssignParameterVisitor_operator() __attribute__((unused));
//     void instantiate_AssignParameterVisitor_operator()
//     {
//       ModuleParamList moduleParamList;
//       const std::string name = "";
//       ParameterVariant value;
//       AssignParameterVisitor psv(&moduleParamList, name);
//       boost::apply_visitor(psv, value);
//     }
//   }
// }
// Does the same as above but explicit. The above however reacts to changes in the variant automatically.
// Make explicit template instances for each of the parameter types in FilterParamVariant
template void AssignParameterVisitor::operator()(const bool&) const;
template void AssignParameterVisitor::operator()(const int&) const;
template void AssignParameterVisitor::operator()(const double&) const;
template void AssignParameterVisitor::operator()(const std::string&) const;
template void AssignParameterVisitor::operator()(const std::vector<double>&) const;
template void AssignParameterVisitor::operator()(const std::vector<std::string>&) const;

template class Belle2::ModuleParam<TrackFindingCDC::ParameterVariant>;
template class Belle2::ModuleParam<TrackFindingCDC::ParameterVariantMap>;
template class Belle2::ModuleParam<std::vector<TrackFindingCDC::ParameterVariantMap> >;
