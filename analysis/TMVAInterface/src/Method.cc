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

#include <TSystem.h>
#include <TROOT.h>
#include <TPluginManager.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>

namespace Belle2 {
  namespace TMVAInterface {

    Method::Method(std::istream& istream)
    {

      // Read out the variables and the method used for the training from the given xml-stream.
      // If the format of the TMVA xml-weightfile changes we need to change the code only in the next block
      std::vector<std::string> variables;
      std::string method;
      try {
        boost::property_tree::ptree pt;
        boost::property_tree::xml_parser::read_xml(istream, pt);
        for (const auto & f : pt.get_child("MethodSetup.Variables")) {
          if (f.first.data() != std::string("Variable")) continue;
          variables.push_back(f.second.get<std::string>("<xmlattr>.Expression"));
        }
        method = pt.get<std::string>("MethodSetup.<xmlattr>.Method");
      } catch (const std::exception& ex) {
        B2ERROR("There was an error while scanning the stream of the file for the used variables and the used method: " << ex.what())
      }

      // Set name and type of the method.
      // If the method is of the form Plugins::* its a plugin method. -> (*, "Plugin")
      // Else it's a builtin method -> therfore split method into (name, type) pair
      std::string prefix("Plugins::");
      std::string name;
      std::string type;
      if (method.compare(0,  prefix.size(), prefix) == 0) {
        name = method.substr(prefix.size(), method.size());
        type = "Plugin";
      } else {
        size_t pos = method.find(':');
        if (pos == std::string::npos) {
          B2ERROR("Couldn't read name of method out of given stream of the weightfile")
        }
        type = method.substr(0, pos);
        name = method.substr(pos + 2, std::string::npos);
      }
      init(name, type, std::string(), variables);

    }

    Method::Method(std::string name, std::string type, std::string config, std::vector<std::string> variables)
    {
      init(name, type, config, variables);
    }

    void Method::init(std::string name, std::string type, std::string config, std::vector<std::string> variables)
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
