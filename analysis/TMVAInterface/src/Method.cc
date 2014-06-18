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

#include <algorithm>
#include <iostream>

namespace Belle2 {
  namespace TMVAInterface {

    Method::Method(std::string name, std::string type, std::string config, std::vector<std::string> variables)
    {

      m_name = name;
      m_config = config;

      // Automatically load Plugin if necessary. The given name has to correspond to the method name
      if (type == "Plugin") {
        std::string base = "TMVA@@MethodBase";
        std::string regexp1 = std::string(".*_") + name + std::string(".*");
        std::string regexp2 = std::string(".*") + name + std::string(".*");
        std::string className = std::string("TMVA::Method") + name;
        std::string pluginName = std::string("TMVA") + name;
        std::string ctor1 = std::string("Method") + name + std::string("(DataSetInfo&,TString)");
        std::string ctor2 = std::string("Method") + name + std::string("(TString&,TString&,DataSetInfo&,TString&)");

        if (m_name == "MockPlugin") {
          pluginName = std::string("analysis_TMVA") + name;
        }

        gPluginMgr->AddHandler(base.c_str(), regexp1.c_str(), className.c_str(), pluginName.c_str(), ctor1.c_str());
        gPluginMgr->AddHandler(base.c_str(), regexp2.c_str(), className.c_str(), pluginName.c_str(), ctor2.c_str());

        m_type = TMVA::Types::kPlugins;
        B2INFO("Loaded plugin " << name)
      } else {
        m_type = TMVA::Types::Instance().GetMethodType(type);
      }
      m_type_as_string = type;

      // Load variables from the VariableManager
      VariableManager& manager = VariableManager::Instance();
      for (auto & variable : variables) {
        const VariableManager::Var* x =  manager.getVariable(variable);
        if (x == nullptr) {
          B2ERROR("Couldn't find variable " << variable << " via the VariableManager. Check the name!")
          continue;
        }
        m_variables.push_back(x);
      }

    }
  }
}
