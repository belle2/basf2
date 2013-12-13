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


namespace Belle2 {

  TMVAExpert::TMVAExpert(std::vector<std::string> variables, std::string method, std::string identifier) : m_method(method)
  {

    // Initialize TMVA and ROOT stuff
    TMVA::Tools::Instance();
    m_reader = new TMVA::Reader("!Color:!Silent");
    TString outfileName(identifier + ".root");

    // Get Pointers to VariableManager::Var for every provided variable name
    VariableManager& manager = VariableManager::Instance();
    for (auto & variable : variables) {
      const VariableManager::Var* x =  manager.getVariable(variable);
      if (x == nullptr) {
        B2ERROR("Couldn't find variable " << variable << " via the VariableManager. Check the name!")
        continue;
      }
      m_input.push_back(x);
    }

    // Add variables to the reader, the readers expects that the variable values are stored in m_inputProxy
    m_inputProxy = new float[m_input.size()];
    {
      int i = 0;
      for (auto & var : m_input) {
        m_reader->AddVariable(var->name, &(m_inputProxy[i++]));
      }
    }

    // Build path to weight file
    TString dir    = "weights/";
    TString prefix(identifier);
    TString methodName = TString(m_method) + TString(" method");
    TString weightfile = dir + prefix + TString("_") + TString(m_method) + TString(".weights.xml");

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
    {
      int i = 0;
      for (auto & x : m_input) {
        m_inputProxy[i++] = x->function(particle);
      }
    }

    // Return Signal Probability of given Particle
    // reader->GetEvaluateMVA(m_method, signal_efficiency);
    // reader->GetProba(m_method, signal_fraction);
    return m_reader->GetProba(m_method);

  }

  TMVAExpert::~TMVAExpert()
  {
    delete[] m_inputProxy;
    delete m_reader;
  }

}

