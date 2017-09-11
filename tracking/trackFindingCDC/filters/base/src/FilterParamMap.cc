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

#include <tracking/trackFindingCDC/utilities/ParamList.icc.h>
#include <tracking/trackFindingCDC/utilities/Param.h>
#include <tracking/trackFindingCDC/utilities/Param.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace Belle2 {
  namespace TrackFindingCDC {
    using FilterParamVariant = boost::variant<bool, int, double, std::string, std::vector<std::string> >;

    // Instantiate the module parameter
    template class Param<std::map<std::string, FilterParamVariant> >;

    /// Define the implementation
    class FilterParamMap::Impl {
    public:
      Impl() = default;

      void addParameter(ParamList* paramList, const std::string& name, const std::string& description)
      {
        paramList->addParameter(name,
                                m_param_filterParameters,
                                description,
                                m_param_filterParameters);
      }

      void assignTo(ParamList* filterParamList)
      {
        AssignParameterVisitor::update(filterParamList, m_param_filterParameters);
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
FilterParamMap::addParameter(ParamList* paramList, const std::string& name, const std::string& description)
{
  m_impl->addParameter(paramList, name, description);
}

void
FilterParamMap::assignTo(ParamList* filterParamList)
{
  m_impl->assignTo(filterParamList);
};

std::map<std::string, FilterParamVariant> FilterParamMap::getValues() const
{
  return m_impl->getValues();
}
