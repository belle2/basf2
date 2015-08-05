/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/TMVAInterface/Method.h>
#include <framework/logging/Logger.h>

#include <TPluginManager.h>

namespace Belle2 {
  namespace TMVAInterface {
    Method::Method(const std::string& name, const std::string& type, const std::string& config) : m_name(name), m_type(type),
      m_config(config)
    {

      if (m_config.find("CreateMVAPdfs") == std::string::npos) {
        B2WARNING("Config string of method " << name << " doesn't contain option CreateMVAPdfs,"
                  "but this option is required to transform the output of the trained method into a probability."
                  "Hint: Probabely you want to set NbinsMVAPdfs too!");
      } else if (m_config.find("NbinsMVAPdf=") == std::string::npos) {
        B2WARNING("Config string of method " << name << " doesn't contain option NbinsMVAPdf,"
                  "I strongly recommend to set this option to a value between 50 and 200, the default value of TMVA is sqrt(NEvents) which is way too high for bigger samples!"
                  "A save choice would be NbinsMVAPdf=100 (which means your classifier will output in the end 100 different values if you transform its output to a probability");
      }

      // Automatically load Plugin if necessary. The given name has to correspond to the method name
      if (m_type == "Plugins" or m_type == "Plugin") {
        m_type = "Plugins";
        auto base = std::string("TMVA@@MethodBase");
        auto regexp1 = std::string(".*_") + m_name + std::string(".*");
        auto regexp2 = std::string(".*") + m_name + std::string(".*");
        auto className = std::string("TMVA::Method") + m_name;
        auto ctor1 = std::string("Method") + m_name + std::string("(DataSetInfo&,TString)");
        auto ctor2 = std::string("Method") + m_name + std::string("(TString&,TString&,DataSetInfo&,TString&)");
        auto pluginName = (m_name == "MockPlugin") ? std::string("analysis_TMVA") + m_name : std::string("TMVA") + m_name;

        gPluginMgr->AddHandler(base.c_str(), regexp1.c_str(), className.c_str(), pluginName.c_str(), ctor1.c_str());
        gPluginMgr->AddHandler(base.c_str(), regexp2.c_str(), className.c_str(), pluginName.c_str(), ctor2.c_str());

        m_type_enum = TMVA::Types::kPlugins;
        B2INFO("Loaded plugin " << m_name);
      } else {
        m_type_enum = TMVA::Types::Instance().GetMethodType(m_type);
      }
    }
  }
}
