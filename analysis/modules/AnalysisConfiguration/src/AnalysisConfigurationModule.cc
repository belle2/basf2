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
  addParam("tupleStyle", m_tupleStyle, R"DOCSTRING(Style of the tuples. \n
  Possible styles on example of PX variable of pi0 from D in decay B->(D->pi0 pi) pi0:\n
  'Default': B_D_pi0_PX\n
  'Laconic': pi01_PX)DOCSTRING", std::string("Default"));
}

void AnalysisConfigurationModule::initialize()
{
  AnalysisConfiguration::instance()->setTupleStyle(m_tupleStyle);
}
