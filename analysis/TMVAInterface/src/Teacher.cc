/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/TMVAInterface/Teacher.h>
#include <analysis/VariableManager/Utility.h>
#include <framework/logging/Logger.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/RootMergeable.h>
#include <framework/utilities/Utils.h>
#include <framework/utilities/WorkingDirectoryManager.h>
#include <framework/utilities/MakeROOTCompatible.h>

#include <analysis/TMVAInterface/Config.h>
#include <analysis/TMVAInterface/SPlot.h>
#include <analysis/TMVAInterface/Expert.h>

#include <TMVA/Factory.h>
#include <TMVA/Tools.h>

#include <TFile.h>
#include <TTree.h>
#include <TTreeFormula.h>
#include <TString.h>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/version.hpp>

#include <sstream>
#include <cmath>

namespace Belle2 {

  namespace TMVAInterface {

    Teacher::Teacher(const TeacherConfig& config, bool useExistingData) : m_config(config), m_file(nullptr), m_tree("",
          DataStore::c_Persistent), splot_class(1)
    {
      B2WARNING("TMVAInterface is deprecated and will be removed, please migrate all your code to the new mva package!");
      WorkingDirectoryManager dummy(m_config.getWorkingDirectory());

      // Get Pointers to Variable::Manager::Var for the provided variables
      m_variables = m_config.getVariablesFromManager();
      m_spectators = m_config.getSpectatorsFromManager();

      auto variable_names = m_config.getVariables();
      auto spectator_names = m_config.getSpectators();

      // Create new tree which stores the input and target variable
      m_input.resize(variable_names.size() + spectator_names.size(), 0);

      // If we want to use existing data we open the file in UPDATE mode,
      // otherwise we recreate the file (overwrite it!)
      m_file = TFile::Open(m_config.getFileName().c_str(), (useExistingData) ? "UPDATE" : "RECREATE");
      m_file->cd();

      // Search for an existing tree in the file
      TTree* tree = nullptr;

      // Load tree from existing data
      if (useExistingData) {
        m_file->GetObject(m_config.getTeacherTreeName().c_str(), tree);

        if (not tree) {
          B2INFO("Did not found a tree named " << m_config.getTeacherTreeName() << " searching for another tree.");
          TIter next(m_file->GetListOfKeys());
          while (TObject* obj = next()) {
            if (std::string(obj->GetName()).find("_tree") != std::string::npos) {
              B2INFO("Found tree with name " << obj->GetName());
              m_file->GetObject(obj->GetName(), tree);
              break;
            }
          }
        }

        if (tree) {

          int nentries = tree->GetEntries();
          TBranch* b = tree->GetBranch("__weight__");
          if (not b) {
            B2WARNING("Couldn't find weight branch named __weight__. Adding it automatically");
            float value = 1.0;
            TBranch* newBranch = tree->Branch("__weight__", &value);
            for (int i = 0; i < nentries; ++i)
              newBranch->Fill();
            tree->Write();
          }
        } else {
          B2WARNING("Couldn't find existing data, create new tree: Filename was " << m_config.getFileName() << ", Treename was " <<
                    m_config.getTeacherTreeName());
        }
      }

      // If tree is not available yet create a new one
      if (tree == nullptr) {
        tree = new TTree(m_config.getTeacherTreeName().c_str(), m_config.getTeacherTreeName().c_str());

        for (unsigned int i = 0; i < variable_names.size(); ++i)
          tree->Branch(makeROOTCompatible(variable_names[i]).c_str(), &m_input[i]);
        for (unsigned int i = 0; i < spectator_names.size(); ++i)
          tree->Branch(makeROOTCompatible(spectator_names[i]).c_str(), &m_input[i + variable_names.size()]);
        tree->Branch("__weight__", &m_original_weight);

      }
      m_tree.registerInDataStore(m_config.getTeacherTreeName(), DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
      m_tree.construct();
      m_tree->assign(tree);

      setBranchAddresses();


    }

    void Teacher::enableLowMemoryProfile()
    {

      m_tree->get().SetBasketSize("*", 1600);
      m_tree->get().SetCacheSize(100000);

    }

    void Teacher::setBranchAddresses()
    {

      auto variable_names = m_config.getVariables();
      auto spectator_names = m_config.getSpectators();

      m_tree->get().SetBranchAddress("__weight__", &m_original_weight);
      for (unsigned int i = 0; i < variable_names.size(); ++i)
        m_tree->get().SetBranchAddress(makeROOTCompatible(variable_names[i]).c_str(), &m_input[i]);
      for (unsigned int i = 0; i < spectator_names.size(); ++i)
        m_tree->get().SetBranchAddress(makeROOTCompatible(spectator_names[i]).c_str(), &m_input[i + variable_names.size()]);
    }

    void Teacher::writeTree()
    {
      WorkingDirectoryManager dummy(m_config.getWorkingDirectory());
      m_file->cd();
      m_tree->get().Write("", TObject::kOverwrite);

      const bool writeError = m_file->TestBit(TFile::kWriteError);
      if (writeError) {
        //m_file deleted first so we have a chance of closing it (though that will probably fail)
        delete m_file;
        B2FATAL("A write error occured while saving '" << m_config.getFileName()  << "', please check if enough disk space is available.");
      }

      m_tree->get().SetDirectory(nullptr);
    }

    Teacher::~Teacher()
    {
      if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
        WorkingDirectoryManager dummy(m_config.getWorkingDirectory());
        m_tree->get().SetDirectory(nullptr);
        m_file->Close();
      }
    }


    void Teacher::addSample(const Particle* particle, float weight)
    {
      WorkingDirectoryManager dummy(m_config.getWorkingDirectory());

      m_original_weight = weight;

      // Fill the tree with the input variables
      for (unsigned int i = 0; i < m_variables.size(); ++i) {
        if (m_variables[i] != nullptr)
          m_input[i] = m_variables[i]->function(particle);
        if (!std::isfinite(m_input[i])) {
          B2ERROR("Output of variable " << m_variables[i]->name << " is " << m_input[i] << ", please fix it. Candidate will be skipped.");
          return;
        }
      }

      for (unsigned int i = 0; i < m_spectators.size(); ++i) {
        if (m_spectators[i] != nullptr)
          m_input[i + m_variables.size()] = m_spectators[i]->function(particle);
        if (!std::isfinite(m_input[i + m_variables.size()])) {
          B2ERROR("Output of spectator " << m_spectators[i]->name << " is " << m_input[i + m_variables.size()] <<
                  ", please fix it. Candidate will be skipped.");
          return;
        }
      }

      m_tree->get().Fill();

    }

    std::vector<float> Teacher::getRow(unsigned int index)
    {
      m_tree->get().GetEvent(index, 1);
      return m_input;
    }


    void Teacher::addVariable(const std::string& branchName, const std::vector<float>& values)
    {
      WorkingDirectoryManager dummy(m_config.getWorkingDirectory());
      m_tree->get().SetBranchStatus("*", 1);

      std::cout << "Teacher: Set values of branch " << branchName << std::endl;
      int nentries = m_tree->get().GetEntries();
      if (nentries != static_cast<int>(values.size())) {
        B2FATAL("addVariable(" << branchName << ", ...): Not the right number of values provided. The tree contains " <<
                m_tree->get().GetEntries() << " entries, the provided vector contains " << values.size() << ".");
      }

      // The tree will be cloned into the same file.
      m_file->cd();

      // Delete branch if necessary
      TBranch* b = m_tree->get().GetBranch(branchName.c_str());
      if (b) {
        m_tree->get().GetListOfBranches()->Remove(b);
        m_tree->get().GetListOfBranches()->Compress();
        TLeaf* l = m_tree->get().GetLeaf(branchName.c_str());
        if (l) {
          m_tree->get().GetListOfLeaves()->Remove(l);
          m_tree->get().GetListOfLeaves()->Compress();
        }
        m_tree->get().Write();
      }

      // Add new branch and fill it. It is already checked that `values` has enough entries.
      float value;
      TBranch* newBranch = m_tree->get().Branch(branchName.c_str(), &value);
      for (auto& v : values) {
        value = v;
        newBranch->Fill();
      }
      newBranch->ResetAddress();
      m_tree->get().SetBranchStatus("*", 1);
      m_file->Write(branchName.c_str());

    }

    std::vector<float> Teacher::getVariable(const std::string& branchName)
    {

      WorkingDirectoryManager dummy(m_config.getWorkingDirectory());

      int nentries = m_tree->get().GetEntries();
      std::vector<float> values(nentries);

      float object;
      m_tree->get().SetBranchStatus("*", 0);
      m_tree->get().SetBranchStatus(branchName.c_str(), 1);
      m_tree->get().SetBranchAddress(branchName.c_str(), &object);
      for (int i = 0; i < nentries; ++i) {
        m_tree->get().GetEvent(i);
        values[i] = object;
      }
      m_tree->get().SetBranchStatus("*", 1);

      // Reset all branch addresses
      setBranchAddresses();

      return values;
    }

    std::set<int> Teacher::getDistinctIntegerValues(const std::string& branchName)
    {

      int nentries = m_tree->get().GetEntries();
      std::set<int> distinct_values;

      if (m_tree->get().GetBranch(branchName.c_str()) == 0) {
        return distinct_values;
      }

      float object;
      m_tree->get().SetBranchStatus("*", 0);
      m_tree->get().SetBranchStatus(branchName.c_str(), 1);
      m_tree->get().SetBranchAddress(branchName.c_str(), &object);
      for (int i = 0; i < nentries; ++i) {
        m_tree->get().GetEvent(i);
        distinct_values.insert(std::lround(object));
      }
      m_tree->get().SetBranchStatus("*", 1);

      // Reset all branch addresses
      setBranchAddresses();

      return distinct_values;
    }

    TTree* Teacher::getClassTree(const std::string& target, int classID)
    {
      WorkingDirectoryManager dummy(m_config.getWorkingDirectory());

      // Copy Events from original tree to new signal and background tree.
      // Unfortunately this is necessary because TMVA internally uses vectors to store the data,
      // therefore TMVA looses its out-of-core capability.
      // The options nTrain_Background and nTest_Background (same for *_Signal) are applied
      // after this transformation to vectors, therefore they're too late to prevent a allocation of huge amount of memory
      // if one has many background events.

      TCut cut;
      if (target != "")
        cut = (std::string("abs(") + makeROOTCompatible(target) + " - " + std::to_string(classID) +
               std::string(") < 1e-2")).c_str();
      m_tree->get().SetBranchStatus("*", 1);
      TTree* tree = m_tree->get().CopyTree(cut);
      tree->SetBranchStatus("*", 1);

      if (tree->GetEntries() == 0) {
        B2WARNING("Tree containing class " << classID << " has no entries!");
      }
      return tree;
    }

    void Teacher::trainSPlot(const std::string& modelFileName, std::vector<std::string> discriminatingVariables,
                             const std::string& weight)
    {
      WorkingDirectoryManager dummy(m_config.getWorkingDirectory());

      std::string factoryOption = "!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification";
      std::string prepareOption = "!V:SplitMode=alternate:MixMode=Block:NormMode=None";

      std::map<std::string, std::vector<float>> discriminatingVariablesMap;

      for (auto& var : discriminatingVariables) {
        discriminatingVariablesMap[var] = getVariable(var);
      }

      TMVAInterface::SPlot splot(modelFileName, discriminatingVariablesMap);
      for (auto& var : discriminatingVariables) {
        splot.plot(m_config.getPrefix(), var);
      }

      addVariable("__weight__splot__", splot.getSPlotWeights());
      addVariable("__weight__signal__cdf__", splot.getSignalCDFWeights());
      addVariable("__weight__signal__pdf__", splot.getSignalPDFWeights());
      addVariable("__weight__background__cdf__", splot.getBackgroundCDFWeights());
      addVariable("__weight__background__pdf__", splot.getBackgroundPDFWeights());

      m_config.addExtraData("SPlotSignalPDF", splot.getSignalPDFBins());
      m_config.addExtraData("SPlotBackgroundPDF", splot.getBackgroundPDFBins());
      m_config.addExtraData("SPlotPDFBinning", splot.getPDFBinning());

      // Perform ordinary sPlot training
      std::string signal_splot_weight = "__weight__ * __weight__splot__";
      std::string background_splot_weight = "__weight__ * (1 - __weight__splot__)";

      if (weight != "") {
        signal_splot_weight += " * (" + weight + ")";
        background_splot_weight += " * (" + weight + ")";
      }

      m_tree->get().SetBranchStatus("*", 1);
      setSPlotClass(1);
      std::cerr << "Use following weight " << signal_splot_weight << " " << background_splot_weight << std::endl;
      train(factoryOption, prepareOption, "", signal_splot_weight, background_splot_weight);

      /*
       * SAC sPlot anti-correlation training
       * The idea is to use the probabilities from the cdf training as boost weights
       * for another splot training, using only the orthogonal part of the data.
       */

      // Perform splot training with __weight__cdf__
      std::string signal_cdf_weight = "__weight__ * __weight__signal__cdf__ / (__weight__background__pdf__ + 0.001 )";
      std::string background_cdf_weight = "__weight__ * (1 - __weight__signal__cdf__) / (__weight__background__pdf__ + 0.001)";

      if (weight != "") {
        signal_cdf_weight += " * (" + weight + ")";
        background_cdf_weight += " * (" + weight + ")";
      }

      m_tree->get().SetBranchStatus("*", 1);
      setSPlotClass(2);
      std::cerr << "Use following weight " << signal_cdf_weight << " " << background_cdf_weight << std::endl;
      train(factoryOption, prepareOption, "", signal_cdf_weight, background_cdf_weight);

      // Perform splot training using inverse classifier-output multiplied with splot weights and cdf_weights
      std::string methodName = m_config.getMethods()[0].getName();
      if (m_config.getMethods().size() > 1) {
        B2WARNING("Train more than one method with advanced sPlot technique, the output of the first method is used as sPlot anti-correlation boost!");
      }

      // Apply second training
      TMVAInterface::ExpertConfig config(m_config.getPrefix(), std::string("."), methodName, 2, 0.5);
      auto expert = std::unique_ptr<TMVAInterface::Expert>(new TMVAInterface::Expert(config, true));

      // Just to be save, we reset all branch addresses before we access them via getRow!
      setBranchAddresses();

      unsigned int nEvents = splot.getSPlotWeights().size();
      std::vector<float> probability(nEvents);
      for (unsigned int i = 0; i < nEvents; ++i) {
        probability[i] = expert->analyse(getRow(i)) * 0.99 + 0.005;
      }
      addVariable("__weight__probability__", probability);

      //SAC sPlot anti-correlation training
      // Michael's Formula?
      std::string boost_weight =
        "(__weight__signal__cdf__ / __weight__probability__ + (1.0 - __weight__signal__cdf__) / (1.0 - __weight__probability__)) / 2.0";
      std::string signal_sac_weight = "__weight__ *  __weight__splot__ * " + boost_weight;
      std::string background_sac_weight = "__weight__ *  (1 - __weight__splot__) * " + boost_weight;

      if (weight != "") {
        signal_sac_weight += " * (" + weight + ")";
        background_sac_weight += " * (" + weight + ")";
      }

      m_tree->get().SetBranchStatus("*", 1);
      setSPlotClass(3);
      std::cerr << "Use following weight " << signal_sac_weight << " " << background_sac_weight << std::endl;
      train(factoryOption, prepareOption, "", signal_sac_weight, background_sac_weight);

      // Enable all branches
      // Otherwise the __weight__splot__ branch seems to be not available in the file.
      m_tree->get().SetBranchStatus("*", 1);

    }

    void Teacher::train(const std::string& factoryOption, const std::string& prepareOption, const std::string& target,
                        const std::string& signal_weight,
                        const std::string& background_weight)
    {

      WorkingDirectoryManager dummy(m_config.getWorkingDirectory());
      m_file->cd();

      // Remove constant variables from all methods,
      // this is a common problem using TMVA. TMVA checks if a variable is constant
      // and exits the program if this is the case!
      // We need to prevent this, therefore we remove all constant variables here.
      std::vector<std::string> cleaned_variables;

      for (auto& x : m_config.getVariables()) {
        std::string varname = makeROOTCompatible(x);
        if (m_tree->get().GetMinimum(varname.c_str()) < m_tree->get().GetMaximum(varname.c_str())) {
          cleaned_variables.push_back(x);
        } else {
          B2WARNING("Removed variable " << x << " from TMVA training because it's constant!");
        }
      }

      m_config = TeacherConfig(m_config.getPrefix(), m_config.getTeacherTreeName(), m_config.getWorkingDirectory(), cleaned_variables,
                               m_config.getSpectators(),
                               m_config.getMethods(), m_config.getExtraData());
      m_variables = m_config.getVariablesFromManager();

      std::set<int> classes = getDistinctIntegerValues(target);

      if (classes.empty() and target != "") {
        B2FATAL("Found 0 classes in data.");
      }

      std::map<int, TTree*> class_trees;
      if (classes.empty()) {

        classes.insert(getSPlotClass());
        classes.insert(0);

        class_trees[getSPlotClass()] = getClassTree(target, 0);
        class_trees[0] = getClassTree(target, 0);

      } else {
        for (const auto& value : classes) {
          class_trees[value] = getClassTree(target, value);
        }
      }

      if (classes.size() == 1) {
        B2FATAL("Encountered only 1 class, cannot train anything!");
      }

      if (classes.size() ==  2) {
        int maxId = *classes.begin();
        for (const auto& value : classes) {
          if (value > maxId)
            maxId = value;
        }
        trainClass(class_trees, factoryOption, prepareOption, signal_weight, background_weight, maxId);
      } else {
        for (const auto& value : classes) {
          trainClass(class_trees, factoryOption, prepareOption, signal_weight, background_weight, value);
        }
      }
    }

    void Teacher::trainClassification(const std::string& factoryOption, const std::string& prepareOption, const std::string& target,
                                      const std::string& weight)
    {
      WorkingDirectoryManager dummy(m_config.getWorkingDirectory());

      std::string signal_weight = "__weight__";
      std::string background_weight = "__weight__";

      if (weight != "") {
        signal_weight += " * (" + weight + ")";
        background_weight += " * (" + weight + ")";
      }

      train(factoryOption, prepareOption, target, signal_weight, background_weight);

    }

    double Teacher::sumOfFormula(std::string formula, TTree* tree) const
    {
      double sum = 0;
      std::unique_ptr<TTreeFormula> form(new TTreeFormula("form", formula.c_str(), tree));
      form->GetNdata();
      int nentries = tree->GetEntries();
      for (int i = 0; i < nentries; ++i) {
        tree->GetEvent(i);
        sum += form->EvalInstance();
      }
      return sum;
    }


    void Teacher::trainClass(std::map<int, TTree*> class_trees, std::string factoryOption, std::string prepareOption,
                             std::string signal_weight, std::string background_weight, int signalClass)
    {
      WorkingDirectoryManager dummy(m_config.getWorkingDirectory());

      std::string jobName = m_config.getPrefix() + "_" + std::to_string(signalClass);
      TFile classFile((jobName + ".root").c_str(), "RECREATE");
      classFile.cd();

      TMVA::Tools::Instance();
      TMVA::Factory factory(jobName, &classFile, factoryOption);

      // Add variables to the factory
      for (auto& var : m_config.getVariables()) {
        factory.AddVariable(makeROOTCompatible(var));
      }

      for (auto& var : m_config.getSpectators()) {
        factory.AddSpectator(makeROOTCompatible(var));
      }

      factory.SetSignalWeightExpression(signal_weight);
      factory.SetBackgroundWeightExpression(background_weight);

      factory.AddSignalTree(class_trees[signalClass]);
      double sum_sig = sumOfFormula(signal_weight, class_trees[signalClass]);
      double sum_bck = 0;
      for (const auto& pair : class_trees) {
        if (pair.first != signalClass) {
          factory.AddBackgroundTree(pair.second);
          sum_bck += sumOfFormula(background_weight, pair.second);
        }
      }

      factory.PrepareTrainingAndTestTree("", prepareOption);

      for (const auto& method : m_config.getMethods()) {
        if (!factory.BookMethod(method.getTypeAsString(), method.getName(), method.getConfig())) {
          B2ERROR("TMVA Method with name " + method.getName() + " cannot be booked.");
        }
      }

      factory.TrainAllMethods();
      factory.TestAllMethods();
      factory.EvaluateAllMethods();

      double signalFraction = sum_sig / (sum_sig + sum_bck);
      m_config.save(signalClass, signalFraction);

    }

  }
}

