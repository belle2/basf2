/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/TMVAInterface/TMVAExpert.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/datastore/DataStore.h>

#include <TMVA/Reader.h>
#include <TMVA/Tools.h>
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TRandom.h>
#include <TPluginManager.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <string>


namespace Belle2 {

  TMVAExpert::TMVAExpert(std::string identifier, std::string method) : m_method(method)
  {

    // Initialize TMVA and ROOT stuff
    TMVA::Tools::Instance();
    m_reader = new TMVA::Reader("!Color:!Silent");

    // Build path to weight file
    TString dir    = "weights/";
    TString prefix(identifier);
    TString methodName = TString(m_method) + TString(" method");
    TString weightfile = dir + prefix + TString("_") + TString(m_method) + TString(".weights.xml");

    // Read out variables and method type and name
    std::vector<std::string> variables;
    try {
      boost::property_tree::ptree pt;
      boost::property_tree::xml_parser::read_xml(std::string(weightfile), pt);

      for (const auto & f : pt.get_child("MethodSetup.Variables")) {
        if (f.first.data() != std::string("Variable")) continue;
        variables.push_back(f.second.get<std::string>("<xmlattr>.Expression"));
      }

      // For plugin methods we load the TMVA::PluginName interface at runtime as via the TPluginManager of ROOT
      std::string methodString = pt.get<std::string>("MethodSetup.<xmlattr>.Method");
      size_t pos = methodString.find(':');
      if (pos < std::string::npos) {
        std::string type = methodString.substr(0, pos);
        std::string name = methodString.substr(pos + 2, std::string::npos);
        if (type == "Plugins") {
          gPluginMgr->AddHandler("TMVA@@MethodBase", (std::string(".*_") + name + std::string(".*")).c_str(), (std::string("TMVA::Method") + name).c_str(),
                                 (std::string("TMVA") + name).c_str(), (std::string("Method") + name + std::string("(DataSetInfo&,TString)")).c_str());
          gPluginMgr->AddHandler("TMVA@@MethodBase", (std::string(".*") + name + std::string(".*")).c_str(), (std::string("TMVA::Method") + name).c_str(),
                                 (std::string("TMVA") + name).c_str(), (std::string("Method") + name + std::string("(TString&,TString&,DataSetInfo&,TString&)")).c_str());
        }
      }

    } catch (const std::exception& ex) {
      B2ERROR("There was an error during the readout of the file " <<  weightfile << " : " << ex.what())
    }

    // Get Pointers to VariableManager::Var for every provided variable name
    VariableManager& manager = VariableManager::Instance();
    for (auto & variable : variables) {
      const VariableManager::Var* x =  manager.getVariable(variable);
      if (x == nullptr) {
        B2ERROR("Couldn't find variable " << variable << " via the VariableManager. Check the name!")
        continue;
      }
      m_input.insert(std::make_pair(x, 0));
      m_reader->AddVariable(x->name, &m_input[x]);
    }

    if (!m_reader->BookMVA(m_method, weightfile)) {
      B2FATAL("Could not set up expert! Please see preceding error message from TMVA!");
    }

  }

  float TMVAExpert::analyse(const Particle* particle)
  {
    for (auto & pair : m_input) {
      pair.second = pair.first->function(particle);
    }

    // Return Signal Probability of given Particle
    // reader->GetEvaluateMVA(m_method, signal_efficiency);
    // reader->GetProba(m_method, signal_fraction);
    return m_reader->GetProba(m_method);

  }

  TMVAExpert::~TMVAExpert()
  {
    delete m_reader;
  }

}

