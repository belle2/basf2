/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/TMVAInterface/Teacher.h>
#include <framework/logging/Logger.h>
#include <framework/pcore/ProcHandler.h>

#include <TMVA/Factory.h>
#include <TMVA/Tools.h>

#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TSystem.h>


#include <sstream>

namespace Belle2 {

  namespace TMVAInterface {

    Teacher::Teacher(std::string prefix, std::string workingDirectory, std::string target, std::vector<Method> methods, bool useExistingData) : m_prefix(prefix),
      m_workingDirectory(workingDirectory), m_methods(methods), m_file(nullptr), m_tree("", DataStore::c_Persistent)
    {

      // Change the workling directory to the user defined working directory
      std::string oldDirectory = gSystem->WorkingDirectory();
      gSystem->ChangeDirectory(m_workingDirectory.c_str());

      // Get Pointer to Variable::Manager::Var for the provided target name
      Variable::Manager& manager = Variable::Manager::Instance();
      m_target_var =  manager.getVariable(target);
      if (m_target_var == nullptr) {
        B2FATAL("Couldn't find target variable " << target << " via the Variable::Manager. Check the name!")
      }
      m_target = 0;

      // Create new tree which stores the input and target variable
      const auto& variables = m_methods[0].getVariables();
      m_input.resize(variables.size());

      // If we want to use existing data we open the file in UPDATE mode,
      // otherwise we recreate the file (overwrite it!)
      std::string mode = "RECREATE";
      if (useExistingData) {
        mode = "UPDATE";
      }

      m_file = TFile::Open((m_prefix + ".root").c_str(), mode.c_str());
      m_file->cd();
      std::string tree_name = m_prefix + "_tree";

      // Search for an existing tree in the file
      TTree* tree = nullptr;
      if (useExistingData) {
        m_file->GetObject((tree_name).c_str(), tree);
      }

      if (tree == nullptr) {

        if (useExistingData)
          B2WARNING("Couldn't find existing data, create new tree")

          tree = new TTree(tree_name.c_str(), tree_name.c_str());

        for (unsigned int i = 0; i < variables.size(); ++i)
          tree->Branch(Variable::makeROOTCompatible(variables[i]->name).c_str(), &m_input[i]);
        tree->Branch(Variable::makeROOTCompatible(m_target_var->name).c_str(), &m_target);

      } else {
        for (unsigned int i = 0; i < variables.size(); ++i)
          tree->SetBranchAddress(Variable::makeROOTCompatible(variables[i]->name).c_str(), &m_input[i]);
        tree->SetBranchAddress(Variable::makeROOTCompatible(m_target_var->name).c_str(), &m_target);
      }

      m_tree.registerInDataStore(tree_name, DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
      m_tree.construct();
      m_tree->assign(tree);

      gSystem->ChangeDirectory(oldDirectory.c_str());

    }

    void Teacher::writeTree()
    {
      m_file->cd();
      m_tree->get().Write("", TObject::kOverwrite);
      m_tree->get().SetDirectory(nullptr);
    }

    Teacher::~Teacher()
    {
      if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
        m_tree->get().SetDirectory(nullptr);
        m_file->Close();
      }
    }

    void Teacher::addSample(const Particle* particle)
    {
      // Change the workling directory to the user defined working directory
      std::string oldDirectory = gSystem->WorkingDirectory();
      gSystem->ChangeDirectory(m_workingDirectory.c_str());

      // Fill the tree with the input variables
      const auto& variables = m_methods[0].getVariables();
      for (unsigned int i = 0; i < variables.size(); ++i) {
        m_input[i] = variables[i]->function(particle);
      }

      // The target variable is converted to an integer
      m_target = int(m_target_var->function(particle) + 0.5);
      m_tree->get().Fill();

      gSystem->ChangeDirectory(oldDirectory.c_str());
    }

    void Teacher::train(std::string factoryOption, std::string prepareOption, unsigned long int maxEventsPerClass)
    {

      // Change the workling directory to the user defined working directory
      std::string oldDirectory = gSystem->WorkingDirectory();
      gSystem->ChangeDirectory(m_workingDirectory.c_str());

      m_file->cd();

      std::map<int, unsigned long int> cluster_count;
      TBranch* targetBranch  = m_tree->get().GetBranch(Variable::makeROOTCompatible(m_target_var->name).c_str());
      targetBranch->SetAddress(&m_target);

      unsigned long int nevent = m_tree->get().GetEntries();
      for (unsigned long int i = 0; i < nevent; i++) {
        targetBranch->GetEvent(i);
        auto it = cluster_count.find(m_target);
        if (it == cluster_count.end())
          cluster_count[m_target] = 1;
        else
          it->second++;
      }

      // Calculate the total number of events
      std::vector<int> classesWhichReachedMaximum;
      unsigned long int total = 0;
      for (auto & x : cluster_count) {
        if (maxEventsPerClass != 0 and x.second > maxEventsPerClass) {
          classesWhichReachedMaximum.push_back(x.first);
        }
        total += x.second;
      }

      // Add to the output config xml file the different clusters and their fractions
      boost::property_tree::ptree pt;
      for (auto & x : cluster_count) {
        boost::property_tree::ptree node;
        node.put("ID", x.first);
        node.put("Count", x.second);
        node.put("MaxEventUsedInTraining", maxEventsPerClass);
        node.put("Fraction", static_cast<double>(x.second) / total);
        pt.add_child("Setup.Clusters.Cluster", node);
      }


      // Remove constant variables from all methods,
      // this is a common problem using TMVA. TMVA checks if a variable is constant
      // and exits the programm if this is the case!
      // We need to prevent this, therefore we remove all constant variables here.
      std::vector<std::string> cleaned_variables;
      for (auto & x : m_methods[0].getVariables()) {
        std::string varname = Variable::makeROOTCompatible(x->name);
        if (m_tree->get().GetMinimum(varname.c_str()) < m_tree->get().GetMaximum(varname.c_str())) {
          cleaned_variables.push_back(x->name);
        } else {
          B2WARNING("Removed variable " << x->name << " from TMVA training because it's constant!")
        }
      }
      for (auto & method : m_methods) {
        method = Method(method.getName(), method.getTypeAsString(), method.getConfig(), cleaned_variables);
      }

      for (auto & x : m_methods[0].getVariables()) {
        boost::property_tree::ptree node;
        node.put("Name", x->name);
        pt.add_child("Setup.Variables.Variable", node);
      }

      if (cluster_count.size() <= 1) {
        B2ERROR("Found less than two clusters in sample, no training necessary!")
        return;
      } else if (cluster_count.size() ==  2) {
        int maxId = cluster_count.begin()->first;
        for (const auto & pair : cluster_count) {
          if (pair.first > maxId)
            maxId = pair.first;
        }
        auto node = trainClass(factoryOption, prepareOption, cluster_count, maxEventsPerClass, maxId);
        pt.add_child("Setup.Trainings.Training", node);
      } else {
        for (const auto & pair : cluster_count) {
          auto node = trainClass(factoryOption, prepareOption, cluster_count, maxEventsPerClass, pair.first);
          pt.add_child("Setup.Trainings.Training", node);
        }
      }

      boost::property_tree::xml_writer_settings<char> settings('\t', 1);
      boost::property_tree::xml_parser::write_xml(m_prefix + ".config", pt, std::locale(), settings);
      gSystem->ChangeDirectory(oldDirectory.c_str());
    }

    boost::property_tree::ptree Teacher::trainClass(std::string factoryOption, std::string prepareOption, std::map<int, unsigned long int>& cluster_count, unsigned long int maxEventsPerClass, int signalClass)
    {

      std::stringstream signal;
      signal << signalClass;
      std::string jobName = m_prefix + "_" + signal.str();

      TFile classFile((jobName + ".root").c_str(), "RECREATE");
      classFile.cd();

      boost::property_tree::ptree node;

      TMVA::Tools::Instance();
      TMVA::Factory factory(jobName, &classFile, factoryOption);

      // Add variables to the factory
      for (auto & var : m_methods[0].getVariables()) {
        factory.AddVariable(Variable::makeROOTCompatible(var->name));
      }

      auto signalCut = TCut((Variable::makeROOTCompatible(m_target_var->name) + " == " + signal.str()).c_str());
      unsigned long int signalEvents = (maxEventsPerClass == 0) ? cluster_count[signalClass] : maxEventsPerClass;

      auto backgroundCut = TCut((Variable::makeROOTCompatible(m_target_var->name) + " != " + signal.str()).c_str());
      unsigned long int backgroundEvents = 0;
      if (maxEventsPerClass == 0) {
        for (const auto & pair : cluster_count) {
          if (pair.first != signalClass)
            backgroundEvents += pair.second;
        }
      } else {
        backgroundEvents = maxEventsPerClass;
      }

      // Copy Events from original tree to new signal and background tree.
      // Unfortunatly this is necessary because TMVA internally uses vectors to store the data,
      // therefore TMVA looses its out-of-core capability.
      // The options nTrain_Background and nTest_Background (same for *_Signal) are applied
      // after this transformation to vectors, therefore they're too late to prevent a allocation of huge amount of memory
      // if one has many backgruond events.
      factory.AddSignalTree(m_tree->get().CopyTree(signalCut)->CopyTree("", "", signalEvents));
      factory.AddBackgroundTree(m_tree->get().CopyTree(backgroundCut)->CopyTree("", "", backgroundEvents));
      factory.PrepareTrainingAndTestTree("", prepareOption);

      // Append the trained methods to the config xml file
      for (auto & method : m_methods) {
        boost::property_tree::ptree method_node;
        method_node.put("SignalID", signalClass);
        method_node.put("MethodName", method.getName());
        method_node.put("MethodType", method.getTypeAsString());
        method_node.put("Samplefile", m_prefix + ".root");
        method_node.put("Weightfile", std::string("weights/") + jobName + std::string("_") + method.getName() + std::string(".weights.xml"));
        factory.BookMethod(method.getType(), method.getName(), method.getConfig());
        node.add_child("Methods.Method", method_node);
      }

      factory.TrainAllMethods();
      factory.TestAllMethods();
      factory.EvaluateAllMethods();

      return node;

    }

  }
}

