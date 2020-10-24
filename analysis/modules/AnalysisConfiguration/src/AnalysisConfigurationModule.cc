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
#include <set>
#include <analysis/modules/AnalysisConfiguration/AnalysisConfigurationModule.h>
#include <analysis/utility/AnalysisConfiguration.h>

using namespace std;
using namespace Belle2;



REG_MODULE(AnalysisConfiguration)

AnalysisConfigurationModule::AnalysisConfigurationModule() : Module()
{
  setDescription("This is a supplementary module designed to configure other modules");
  addParam("tupleStyle", m_tupleStyle, R"DOCSTRING(This parameter defines style of variables written by all other ntuples tools. \n
  Possible styles on example of PX variable of pi0 from D in decay B->(D->pi0 pi) pi0:\n
  'default': B_D_pi0_PX\n
  'semilaconic': D_pi0_PX\n
  'laconic': pi01_PX\n)DOCSTRING", std::string("default"));

  addParam("mcMatchingVersion", m_mcMatchingVersion, "Specifies what version of mc matching algorithm is going to be used. \n"
           "Possibilities are: BelleII (default) and Belle. The latter should be used when analysing Belle MC.\n"
           "The difference between the algorithms is only in the treatment (identification) of FSR photons. In Belle II MC \n"
           "it is possible to identify FSR photons via special flag set by the generator, while in case of Belle MC such\n"
           "information is not available.", std::string("BelleII"));
}

void AnalysisConfigurationModule::initialize()
{
  if (m_styles.find(m_tupleStyle) == m_styles.end()) {
    B2WARNING("Chosen tuple style '" << m_tupleStyle << "' is not defined. Using default tuple style.");
  }
  AnalysisConfiguration::instance()->setTupleStyle(m_tupleStyle);

  if (m_mcMatchingVersion == "BelleII")
    AnalysisConfiguration::instance()->useLegacyMCMatching(false);
  else if (m_mcMatchingVersion == "Belle" or m_mcMatchingVersion == "MC5")
    AnalysisConfiguration::instance()->useLegacyMCMatching(true);
  else
    B2ERROR("Invalid mcMatchingVersion specified to AnalysisConfiguration: " << m_mcMatchingVersion << "\n"
            "Please choose between BelleII or Belle depending whether you are analysing Belle II or Belle MC.");
}
