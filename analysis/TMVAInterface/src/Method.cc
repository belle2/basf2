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

namespace Belle2 {
  namespace TMVAInterface {

    Method::Method(std::string name, std::string type, std::string config, std::vector<std::string> variables)
    {

      m_name = name;
      m_config = config;

      // Automatically load Plugin if necessary. The given name has to correspond to the method name
      if (type == "Plugin") {
        gPluginMgr->AddHandler("TMVA@@MethodBase", (std::string(".*_") + name + std::string(".*")).c_str(), (std::string("TMVA::Method") + name).c_str(),
                               (std::string("TMVA") + name).c_str(), (std::string("Method") + name + std::string("(DataSetInfo&,TString)")).c_str());
        gPluginMgr->AddHandler("TMVA@@MethodBase", (std::string(".*") + name + std::string(".*")).c_str(), (std::string("TMVA::Method") + name).c_str(),
                               (std::string("TMVA") + name).c_str(), (std::string("Method") + name + std::string("(TString&,TString&,DataSetInfo&,TString&)")).c_str());
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
