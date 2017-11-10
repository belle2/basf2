/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/base/FilterParamMap.h>

#include <tracking/trackFindingCDC/utilities/ParameterVariant.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/core/ModuleParam.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace Belle2 {
  namespace TrackFindingCDC {
    using FilterParamVariant = boost::variant<bool, int, double, std::string, std::vector<std::string> >;
  }
}

// Instantiate the module parameter
template class Belle2::ModuleParam<std::map<std::string, FilterParamVariant> >;

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Define the implementation
    class FilterParamMap::Impl {
    public:
      Impl() = default;

      void addParameter(ModuleParamList* moduleParamList, const std::string& name, const std::string& description)
      {
        moduleParamList->addParameter(name,
                                      m_param_filterParameters,
                                      description,
                                      m_param_filterParameters);
      }

      void assignTo(ModuleParamList* filterModuleParamList)
      {
        AssignParameterVisitor::update(filterModuleParamList, m_param_filterParameters);
      }

      std::map<std::string, FilterParamVariant> getValues() const
      {
        return m_param_filterParameters;
      }

    private:
      std::map<std::string, FilterParamVariant> m_param_filterParameters;
    };
  }
}

FilterParamMap::FilterParamMap() :
  m_impl(std::make_unique<Impl>())
{
}

FilterParamMap::~FilterParamMap() = default;

void
FilterParamMap::addParameter(ModuleParamList* moduleParamList, const std::string& name, const std::string& description)
{
  m_impl->addParameter(moduleParamList, name, description);
}

void
FilterParamMap::assignTo(ModuleParamList* filterModuleParamList)
{
  m_impl->assignTo(filterModuleParamList);
};

// std::map<std::string, FilterParamVariant> FilterParamMap::getValues() const
// {
//   return m_impl->getValues();
// }
