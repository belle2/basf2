/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

AssignParameterVisitor::AssignParameterVisitor(ModuleParamList* moduleParamList,
                                               const std::string& paramName)
  : m_moduleParamList(moduleParamList)
  , m_paramName(paramName)
{
}

template <class T>
void AssignParameterVisitor::operator()(const T& t) const
{
  B2DEBUG(200, "Received parameter of type" << PyObjConvUtils::Type<T>::name());
  m_moduleParamList->getParameter<T>(m_paramName).setDefaultValue(t);
}

// Make explicit template function instances of
// namespace Belle2 {
//   namespace TrackingFindingCDC {
//     /// template<typename T> AssignParameterVisitor::operator()(const T&) const;
//     /// for each of the parameter types in FilterParamVariant
//     void instantiate_AssignParameterVisitor_operator() __attribute__((unused));
//     void instantiate_AssignParameterVisitor_operator()
//     {
//       ModuleParamList moduleParamList;
//       const std::string name = "";
//       AssignParameterVisitor::FilterParamVariant value;
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
template void AssignParameterVisitor::operator()(const std::vector<std::string>&) const;
