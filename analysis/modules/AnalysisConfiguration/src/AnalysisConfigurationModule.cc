/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <string>
#include <analysis/modules/AnalysisConfiguration/AnalysisConfigurationModule.h>
#include <analysis/utility/AnalysisConfiguration.h>

using namespace std;
using namespace Belle2;



REG_MODULE(AnalysisConfiguration)

AnalysisConfigurationModule::AnalysisConfigurationModule() : Module()
{
  setDescription("This is a supplimentary module designed to configure other modules");
  addParam("tuple_style", m_TupleStyle, "Style of the tuples", std::string("Default"));
}

void AnalysisConfigurationModule::initialize()
{
  AnalysisConfiguration::instance()->setTupleStyle(m_TupleStyle);
  //c.setTupleStyle(m_TupleStyle);
}
