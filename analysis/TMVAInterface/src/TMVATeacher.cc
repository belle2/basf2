/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/TMVAInterface/TMVATeacher.h>

/**
 * TMVA Factory uses a private static variable for storing the given output file.
 * This transforms the Factory into a Singleton! And because the variable is private one can do nothing about it.
 * Except for...
 */
#define private public
#include <TMVA/Factory.h>
#undef private
void ROOTGlobalVariableWorkaround(TFile* file)
{
  TMVA::Factory::fgTargetFile = file;
}

#include <framework/datastore/DataStore.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleInfo.h>

#include <algorithm>
#include <string>

#include <TFile.h>
#include <TMVA/Tools.h>
#include <TTree.h>
#include <TString.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TRandom.h>
#include <TPluginManager.h>


namespace Belle2 {

  TMVATeacher::TMVATeacher(std::vector<std::string> variables, std::string target, std::map<std::string, std::string> methods, std::string identifier, std::string factory_option) : m_methods(methods)
  {

    // Intitialize TMVA and ROOT stuff
    TMVA::Tools::Instance();
    TString outfileName(identifier + ".root");
    TString factoryName(identifier);

    m_outputFile = TFile::Open(outfileName, "RECREATE");
    m_factory = new TMVA::Factory(factoryName, m_outputFile, factory_option);

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

    // Get Pointer to VariableManager::Var for the provided target name
    const VariableManager::Var* targetVar =  manager.getVariable(target);
    if (targetVar == nullptr) {
      B2ERROR("Couldn't find variable " << targetVar << " via the VariableManager. Check the name!")
    }
    m_target = targetVar;

    // Add variables to the factory
    for (auto & var : m_input) {
      m_factory->AddVariable(var->name);
    }
    //factory->AddTarget(m_target->name);


  }

  TMVATeacher::~TMVATeacher()
  {
    delete m_factory;
    m_outputFile->Close();
  }

  void TMVATeacher::addSample(const Particle* particle)
  {

    std::vector<double> input;
    for (auto & x : m_input) {
      input.push_back(x->function(particle));
    }
    double target = m_target->function(particle);

    if (target > 0.5) {
      if (gRandom->Rndm() > 0.5) {
        m_factory->AddSignalTrainingEvent(input);
      } else {
        m_factory->AddSignalTestEvent(input);
      }
    } else {
      if (gRandom->Rndm() > 0.5) {
        m_factory->AddBackgroundTrainingEvent(input);
      } else {
        m_factory->AddBackgroundTestEvent(input);
      }
    }
  }

  void TMVATeacher::train()
  {

    ROOTGlobalVariableWorkaround(m_outputFile);
    TCut mycut = "";
    m_factory->PrepareTrainingAndTestTree(mycut, "SplitMode=random:!V");

    // This lambda function checks if provided method name begins with a predefined method
    auto is_method = [](const std::string & name, const std::string & method) -> bool { return name.compare(0, method.size(), method.c_str()) == 0; };

    for (auto & method : m_methods) {
      if (is_method(method.first, "KNN"))      m_factory->BookMethod(TMVA::Types::kKNN, method.first, method.second);
      else if (is_method(method.first, "Fisher"))  m_factory->BookMethod(TMVA::Types::kFisher, method.first, method.second);
      else if (is_method(method.first, "MLP"))     m_factory->BookMethod(TMVA::Types::kMLP, method.first, method.second);
      else if (is_method(method.first, "SVM"))     m_factory->BookMethod(TMVA::Types::kSVM, method.first, method.second);
      else if (is_method(method.first, "BDT"))     m_factory->BookMethod(TMVA::Types::kBDT, method.first, method.second);
      else if (is_method(method.first, "RuleFit")) m_factory->BookMethod(TMVA::Types::kRuleFit, method.first, method.second);
      else if (is_method(method.first, "Cuts"))    m_factory->BookMethod(TMVA::Types::kCuts, method.first, method.second);
      else if (is_method(method.first, "Likelihood")) m_factory->BookMethod(TMVA::Types::kLikelihood, method.first, method.second);
      else if (is_method(method.first, "PDERS"))      m_factory->BookMethod(TMVA::Types::kPDERS, method.first, method.second);
      else if (is_method(method.first, "PDEFoam"))    m_factory->BookMethod(TMVA::Types::kPDEFoam, method.first, method.second);
      else if (is_method(method.first, "HMatrix"))    m_factory->BookMethod(TMVA::Types::kHMatrix, method.first, method.second);
      else if (is_method(method.first, "FDA"))        m_factory->BookMethod(TMVA::Types::kFDA, method.first, method.second);
      else if (is_method(method.first, "CFMlpANN"))   m_factory->BookMethod(TMVA::Types::kCFMlpANN, method.first, method.second);
      else if (is_method(method.first, "TMlpANN"))    m_factory->BookMethod(TMVA::Types::kTMlpANN, method.first, method.second);
      else if (is_method(method.first, "NeuroBayes")) {
        gPluginMgr->AddHandler("TMVA@@MethodBase", ".*_NeuroBayes.*", "TMVA::MethodNeuroBayes", "TMVANeuroBayes", "MethodNeuroBayes(DataSetInfo&,TString)");
        gPluginMgr->AddHandler("TMVA@@MethodBase", ".*NeuroBayes.*", "TMVA::MethodNeuroBayes", "TMVANeuroBayes",  "MethodNeuroBayes(TString&,TString&,DataSetInfo&,TString&)");
        m_factory->BookMethod(TMVA::Types::kPlugins, method.first, method.second);
      } else {
        B2ERROR("Requested Method " << method.first  << " not found.")
      }
    }

    m_factory->TrainAllMethods();
    m_factory->TestAllMethods();
    m_factory->EvaluateAllMethods();
  }

}

