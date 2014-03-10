/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/TMVAInterface/Teacher.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/logging/Logger.h>

#include <TMVA/Factory.h>
#include <TMVA/Tools.h>

#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TROOT.h>

namespace Belle2 {

  namespace TMVAInterface {

    Teacher::Teacher(std::string identifier, std::string target, std::vector<Method> methods) : m_identifier(identifier), m_methods(methods)
    {

      const auto& variables = m_methods[0].getVariables();
      m_input.resize(variables.size());

      // Get Pointer to VariableManager::Var for the provided target name
      VariableManager& manager = VariableManager::Instance();
      m_target_var =  manager.getVariable(target);
      if (m_target_var == nullptr) {
        B2ERROR("Couldn't find target variable " << target << " via the VariableManager. Check the name!")
      }
      m_target = 0;

      m_signal_tree = new TTree((m_identifier + "_signal_tree").c_str(), (m_identifier + "_signal_tree").c_str());
      m_bckgrd_tree = new TTree((m_identifier + "_bckgrd_tree").c_str(), (m_identifier + "_bckgrd_tree").c_str());

      for (unsigned int i = 0; i < variables.size(); ++i) {
        m_signal_tree->Branch(variables[i]->name.c_str(), &m_input[i], "F");
        m_bckgrd_tree->Branch(variables[i]->name.c_str(), &m_input[i], "F");
      }
      m_signal_tree->Branch(m_target_var->name.c_str(), &m_target, "F");
      m_bckgrd_tree->Branch(m_target_var->name.c_str(), &m_target, "F");

    }

    Teacher::~Teacher()
    {
      delete m_signal_tree;
      delete m_bckgrd_tree;
    }

    void Teacher::addSample(const Particle* particle)
    {

      const auto& variables = m_methods[0].getVariables();
      for (unsigned int i = 0; i < variables.size(); ++i) {
        m_input[i] = variables[i]->function(particle);
      }
      m_target = m_target_var->function(particle);

      if (m_target > 0.5) {
        m_signal_tree->Fill();
      } else {
        m_bckgrd_tree->Fill();
      }
    }

    void Teacher::train(std::string factoryOption, std::string prepareOption)
    {

      TFile* file = TFile::Open((m_identifier + ".root").c_str(), "RECREATE");
      {
        // Intitialize TMVA and ROOT stuff
        TMVA::Tools::Instance();
        TMVA::Factory factory(m_identifier, file, factoryOption);

        // Add variables to the factory
        for (auto & var : m_methods[0].getVariables()) {
          factory.AddVariable(var->name);
        }

        factory.AddSignalTree(m_signal_tree);
        factory.AddBackgroundTree(m_bckgrd_tree);
        factory.PrepareTrainingAndTestTree(TCut(""), prepareOption);

        for (auto & method : m_methods) {
          factory.BookMethod(method.getType(), method.getName(), method.getConfig());
        }

        factory.TrainAllMethods();
        factory.TestAllMethods();
        factory.EvaluateAllMethods();
      }

      file->Close();
    }
  }
}

