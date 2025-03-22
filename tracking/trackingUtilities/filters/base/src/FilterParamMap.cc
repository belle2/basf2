/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

      /// Add name, description and values to the module's parameter list
      void addParameter(ModuleParamList* moduleParamList, const std::string& name, const std::string& description)
      {
        moduleParamList->addParameter(name,
                                      m_param_filterParameters,
                                      description,
                                      m_param_filterParameters);
      }

      /// Assign values to the module-parameters in the module's list
      void assignTo(ModuleParamList* filterModuleParamList)
      {
        AssignParameterVisitor::update(filterModuleParamList, m_param_filterParameters);
      }

      /// Retrieve the parameter (name,value) pairs
      std::map<std::string, FilterParamVariant> getValues() const
      {
        return m_param_filterParameters;
      }

    private:
      /// Parameter values
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
