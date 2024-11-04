/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <string>
#include <set>
#include <analysis/modules/AnalysisConfiguration/AnalysisConfigurationModule.h>
#include <analysis/utility/AnalysisConfiguration.h>

using namespace std;
using namespace Belle2;



REG_MODULE(AnalysisConfiguration);

AnalysisConfigurationModule::AnalysisConfigurationModule() : Module()
{
  setDescription("This is a supplementary module designed to configure other modules");
  addParam("tupleStyle", m_tupleStyle, R"DOC(This parameter defines style of variables written by all other ntuples tools.
Possible styles on example of PX variable of pi0 from D in decay B -> (D -> pi0 pi) pi0:

- "default": B_D_pi0_PX
- "semilaconic": D_pi0_PX
- "laconic": pi01_PX
  )DOC", std::string("default"));

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
  else if (m_mcMatchingVersion == "Belle")
    AnalysisConfiguration::instance()->useLegacyMCMatching(true);
  else
    B2ERROR("Invalid mcMatchingVersion specified to AnalysisConfiguration: " << m_mcMatchingVersion << "\n"
            "Please choose between BelleII or Belle depending whether you are analysing Belle II or Belle MC.");
}
