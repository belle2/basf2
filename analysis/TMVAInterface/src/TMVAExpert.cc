/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/TMVAInterface/TMVAExpert.h>
#include <framework/datastore/DataStore.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleInfo.h>

#include <algorithm>
#include <string>

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

namespace Belle2 {

  TMVAExpert::TMVAExpert(std::string identifier, std::string method, std::vector<std::string> variables) : m_method(method)
  {

    // Initialize TMVA and ROOT stuff
    TMVA::Tools::Instance();
    m_reader = new TMVA::Reader("!Color:!Silent");

    // Build path to weight file
    TString dir    = "weights/";
    TString prefix(identifier);
    TString methodName = TString(m_method) + TString(" method");
    TString weightfile = dir + prefix + TString("_") + TString(m_method) + TString(".weights.xml");

    // Read out variables
    std::vector<std::string> varnames;
    try {
      boost::property_tree::ptree pt;
      boost::property_tree::xml_parser::read_xml(std::string(weightfile), pt);
      for (const auto & f : pt.get_child("MethodSetup.Variables")) {
        if (f.first.data() != std::string("Variable")) continue;
        varnames.push_back(f.second.get<std::string>("<xmlattr>.Expression"));
      }
    } catch (const std::exception& ex) {
      B2ERROR("There was an error during the readout of the file " <<  weightfile << " : " << ex.what())
    }

    // Check if varnames match given variable names, or if variables vector is empty
    if (not variables.empty()) {
      if (variables.size() != varnames.size()) {
        B2ERROR("Given number of variables doesn't match number of variables the weightfile was trained with!");
      }
      for (unsigned int i = 0; i < variables.size(); ++i) {
        if (variables[i] != varnames[i]) {
          B2WARNING("Given variable " << variables[i] << " doesn't match the corresponding variable " << varnames[i] << " from the training")
        }
      }
    } else {
      variables = varnames;
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
    }

    // Add variables to the reader, the readers expects that the variable values are stored in m_inputProxy
    for (auto & pair : m_input) {
      m_reader->AddVariable(pair.first->name, &pair.second);
    }

    // For the NeuroBayes method we load the TMVA::NeuroBayes interface at runtime as via the TPluginManager of ROOT
    if (m_method == "NeuroBayes") {
      gPluginMgr->AddHandler("TMVA@@MethodBase", ".*_NeuroBayes.*", "TMVA::MethodNeuroBayes", "TMVANeuroBayes", "MethodNeuroBayes(DataSetInfo&,TString)");
      gPluginMgr->AddHandler("TMVA@@MethodBase", ".*NeuroBayes.*", "TMVA::MethodNeuroBayes", "TMVANeuroBayes",  "MethodNeuroBayes(TString&,TString&,DataSetInfo&,TString&)");
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

