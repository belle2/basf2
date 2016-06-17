/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/methods/TMVA.h>

#include <framework/logging/Logger.h>

#include <TFile.h>
#include <TTree.h>
#include <TPluginManager.h>
#include <TROOT.h>

namespace Belle2 {
  namespace MVA {

    void TMVAOptions::load(const boost::property_tree::ptree& pt)
    {
      int version = pt.get<int>("TMVA_version");
      if (version != 1) {
        B2ERROR("Unkown weightfile version " << std::to_string(version));
        throw std::runtime_error("Unkown weightfile version " + std::to_string(version));
      }
      m_method = pt.get<std::string>("TMVA_method");
      m_type = pt.get<std::string>("TMVA_type");
      m_config = pt.get<std::string>("TMVA_config");
      m_factoryOption = pt.get<std::string>("TMVA_factoryOption");
      m_prepareOption = pt.get<std::string>("TMVA_prepareOption");
      m_workingDirectory = pt.get<std::string>("TMVA_workingDirectory");
      m_prefix = pt.get<std::string>("TMVA_prefix");
    }

    void TMVAOptions::save(boost::property_tree::ptree& pt) const
    {
      pt.put("TMVA_version", 1);
      pt.put("TMVA_method", m_method);
      pt.put("TMVA_type", m_type);
      pt.put("TMVA_config", m_config);
      pt.put("TMVA_factoryOption", m_factoryOption);
      pt.put("TMVA_prepareOption", m_prepareOption);
      pt.put("TMVA_workingDirectory", m_workingDirectory);
      pt.put("TMVA_prefix", m_prefix);
    }

    po::options_description TMVAOptions::getDescription()
    {
      po::options_description description("TMVA options");
      description.add_options()
      ("tmva_method", po::value<std::string>(&m_method), "TMVA Method Name")
      ("tmva_type", po::value<std::string>(&m_type), "TMVA Method Type (e.g. Plugin, BDT, ...)")
      ("tmva_config", po::value<std::string>(&m_config), "TMVA Configuration string for the method")
      ("tmva_factory", po::value<std::string>(&m_factoryOption), "TMVA Factory options passed to TMVAFactory constructor")
      ("tmva_prepare", po::value<std::string>(&m_prepareOption),
       "TMVA Preprare options passed to prepareTrainingAndTestTree function");
      return description;
    }

    void TMVAOptionsClassification::load(const boost::property_tree::ptree& pt)
    {
      TMVAOptions::load(pt);
      transform2probability = pt.get<bool>("TMVA_transform2probability");
    }

    void TMVAOptionsClassification::save(boost::property_tree::ptree& pt) const
    {
      TMVAOptions::save(pt);
      pt.put("TMVA_transform2probability", transform2probability);
    }

    po::options_description TMVAOptionsClassification::getDescription()
    {
      po::options_description description = TMVAOptions::getDescription();
      description.add_options()
      ("tmva_transform2probability", po::value<bool>(&transform2probability), "TMVA Transform output of classifier to a probability");
      return description;
    }

    TMVATeacher::TMVATeacher(const GeneralOptions& general_options, const TMVAOptions& specific_options) : Teacher(general_options),
      specific_options(specific_options) { }

    Weightfile TMVATeacher::trainFactory(TMVA::Factory& factory, std::string& jobName) const
    {
      factory.PrepareTrainingAndTestTree("", specific_options.m_prepareOption);

      if (specific_options.m_type == "Plugins") {
        auto base = std::string("TMVA@@MethodBase");
        auto regexp1 = std::string(".*_") + specific_options.m_method + std::string(".*");
        auto regexp2 = std::string(".*") + specific_options.m_method + std::string(".*");
        auto className = std::string("TMVA::Method") + specific_options.m_method;
        auto ctor1 = std::string("Method") + specific_options.m_method + std::string("(TMVA::DataSetInfo&,TString)");
        auto ctor2 = std::string("Method") + specific_options.m_method + std::string("(TString&,TString&,TMVA::DataSetInfo&,TString&)");
        auto pluginName = std::string("TMVA") + specific_options.m_method;

        gROOT->GetPluginManager()->AddHandler(base.c_str(), regexp1.c_str(), className.c_str(), pluginName.c_str(), ctor1.c_str());
        gROOT->GetPluginManager()->AddHandler(base.c_str(), regexp2.c_str(), className.c_str(), pluginName.c_str(), ctor2.c_str());
      }

      if (!factory.BookMethod(specific_options.m_type, specific_options.m_method, specific_options.m_config)) {
        B2ERROR("TMVA Method with name " + specific_options.m_method + " cannot be booked.");
      }

      factory.TrainAllMethods();
      factory.TestAllMethods();
      factory.EvaluateAllMethods();

      Weightfile weightfile;
      weightfile.addOptions(m_general_options);
      weightfile.addFile("TMVA_Weightfile", std::string("weights/") + jobName + "_" + specific_options.m_method + ".weights.xml");

      return weightfile;

    }

    TMVATeacherClassification::TMVATeacherClassification(const GeneralOptions& general_options,
                                                         const TMVAOptionsClassification& specific_options) : TMVATeacher(general_options, specific_options),
      specific_options(specific_options) { }

    Weightfile TMVATeacherClassification::train(Dataset& training_data) const
    {

      unsigned int numberOfFeatures = training_data.getNumberOfFeatures();
      unsigned int numberOfEvents = training_data.getNumberOfEvents();

      WorkingDirectoryManager dummy(specific_options.m_workingDirectory);

      std::string jobName = specific_options.m_prefix;
      if (jobName.empty())
        jobName = "TMVA";
      TFile classFile((jobName + ".root").c_str(), "RECREATE");
      classFile.cd();

      TMVA::Tools::Instance();
      TMVA::Factory factory(jobName, &classFile, specific_options.m_factoryOption);

      // Add variables to the factory
      for (auto& var : m_general_options.m_variables) {
        factory.AddVariable(Belle2::Variable::makeROOTCompatible(var));
      }

      factory.SetWeightExpression(Belle2::Variable::makeROOTCompatible(m_general_options.m_weight_variable));

      TTree* signal_tree = new TTree("signal_tree", "signal_tree");
      TTree* background_tree = new TTree("background_tree", "background_tree");

      for (unsigned int iFeature = 0; iFeature < numberOfFeatures; ++iFeature) {
        signal_tree->Branch(Belle2::Variable::makeROOTCompatible(m_general_options.m_variables[iFeature]).c_str(),
                            &training_data.m_input[iFeature]);
        background_tree->Branch(Belle2::Variable::makeROOTCompatible(m_general_options.m_variables[iFeature]).c_str(),
                                &training_data.m_input[iFeature]);
      }

      signal_tree->Branch("__weight__", &training_data.m_weight);
      background_tree->Branch("__weight__", &training_data.m_weight);

      for (unsigned int iEvent = 0; iEvent < numberOfEvents; ++iEvent) {
        training_data.loadEvent(iEvent);
        if (training_data.m_isSignal) {
          signal_tree->Fill();
        } else {
          background_tree->Fill();
        }
      }

      factory.AddSignalTree(signal_tree);
      factory.AddBackgroundTree(background_tree);

      auto weightfile = trainFactory(factory, jobName);
      weightfile.addOptions(specific_options);
      weightfile.addSignalFraction(training_data.getSignalFraction());

      delete signal_tree;
      delete background_tree;

      return weightfile;

    }

    TMVATeacherRegression::TMVATeacherRegression(const GeneralOptions& general_options,
                                                 const TMVAOptionsRegression& specific_options) : TMVATeacher(general_options, specific_options),
      specific_options(specific_options) { }

    Weightfile TMVATeacherRegression::train(Dataset& training_data) const
    {

      unsigned int numberOfFeatures = training_data.getNumberOfFeatures();
      unsigned int numberOfEvents = training_data.getNumberOfEvents();

      WorkingDirectoryManager dummy(specific_options.m_workingDirectory);

      std::string jobName = specific_options.m_prefix;
      if (jobName.empty())
        jobName = "TMVA";
      TFile classFile((jobName + ".root").c_str(), "RECREATE");
      classFile.cd();

      TMVA::Tools::Instance();
      TMVA::Factory factory(jobName, &classFile, specific_options.m_factoryOption);

      // Add variables to the factory
      for (auto& var : m_general_options.m_variables) {
        factory.AddVariable(Belle2::Variable::makeROOTCompatible(var));
      }
      factory.AddTarget(Belle2::Variable::makeROOTCompatible(m_general_options.m_target_variable));


      TTree* regression_tree = new TTree("regression_tree", "regression_tree");

      for (unsigned int iFeature = 0; iFeature < numberOfFeatures; ++iFeature) {
        regression_tree->Branch(Belle2::Variable::makeROOTCompatible(m_general_options.m_variables[iFeature]).c_str(),
                                &training_data.m_input[iFeature]);
      }
      regression_tree->Branch(Belle2::Variable::makeROOTCompatible(m_general_options.m_target_variable).c_str(),
                              &training_data.m_target);

      regression_tree->Branch("__weight__", &training_data.m_weight);

      for (unsigned int iEvent = 0; iEvent < numberOfEvents; ++iEvent) {
        training_data.loadEvent(iEvent);
        regression_tree->Fill();
      }

      factory.AddRegressionTree(regression_tree);
      factory.SetWeightExpression(Belle2::Variable::makeROOTCompatible(m_general_options.m_weight_variable), "Regression");

      auto weightfile = trainFactory(factory, jobName);
      weightfile.addOptions(specific_options);

      delete regression_tree;

      return weightfile;

    }

    void TMVAExpert::load(Weightfile& weightfile)
    {

      // Initialize TMVA and ROOT stuff
      TMVA::Tools::Instance();

      m_expert = std::unique_ptr<TMVA::Reader>(new TMVA::Reader("!Color:!Silent"));

      GeneralOptions general_options;
      weightfile.getOptions(general_options);
      m_input_cache.resize(general_options.m_variables.size(), 0);
      for (unsigned int i = 0; i < general_options.m_variables.size(); ++i) {
        m_expert->AddVariable(Belle2::Variable::makeROOTCompatible(general_options.m_variables[i]), &m_input_cache[i]);
      }

    }

    void TMVAExpertClassification::load(Weightfile& weightfile)
    {

      weightfile.getOptions(specific_options);
      expert_signalFraction = weightfile.getSignalFraction();

      // TMVA parses the method type for plugins out of the weightfile name, so we must ensure that it has the expected format
      std::string custom_weightfile = weightfile.getFileName(std::string("_") + specific_options.m_method + ".weights.xml");
      weightfile.getFile("TMVA_Weightfile", custom_weightfile);

      TMVAExpert::load(weightfile);

      if (specific_options.m_type == "Plugins") {
        auto base = std::string("TMVA@@MethodBase");
        auto regexp1 = std::string(".*_") + specific_options.m_method + std::string(".*");
        auto regexp2 = std::string(".*") + specific_options.m_method + std::string(".*");
        auto className = std::string("TMVA::Method") + specific_options.m_method;
        auto ctor1 = std::string("Method") + specific_options.m_method + std::string("(TMVA::DataSetInfo&,TString)");
        auto ctor2 = std::string("Method") + specific_options.m_method + std::string("(TString&,TString&,TMVA::DataSetInfo&,TString&)");
        auto pluginName = std::string("TMVA") + specific_options.m_method;

        gROOT->GetPluginManager()->AddHandler(base.c_str(), regexp1.c_str(), className.c_str(), pluginName.c_str(), ctor1.c_str());
        gROOT->GetPluginManager()->AddHandler(base.c_str(), regexp2.c_str(), className.c_str(), pluginName.c_str(), ctor2.c_str());
        B2INFO("Registered new TMVA Plugin named " << pluginName)
      }

      if (!m_expert->BookMVA(specific_options.m_method, custom_weightfile)) {
        B2FATAL("Could not set up expert! Please see preceding error message from TMVA!");
      }

    }

    void TMVAExpertRegression::load(Weightfile& weightfile)
    {

      weightfile.getOptions(specific_options);

      // TMVA parses the method type for plugins out of the weightfile name, so we must ensure that it has the expected format
      std::string custom_weightfile = weightfile.getFileName(std::string("_") + specific_options.m_method + ".weights.xml");
      weightfile.getFile("TMVA_Weightfile", custom_weightfile);

      TMVAExpert::load(weightfile);

      if (specific_options.m_type == "Plugins") {
        auto base = std::string("TMVA@@MethodBase");
        auto regexp1 = std::string(".*_") + specific_options.m_method + std::string(".*");
        auto regexp2 = std::string(".*") + specific_options.m_method + std::string(".*");
        auto className = std::string("TMVA::Method") + specific_options.m_method;
        auto ctor1 = std::string("Method") + specific_options.m_method + std::string("(TMVA::DataSetInfo&,TString)");
        auto ctor2 = std::string("Method") + specific_options.m_method + std::string("(TString&,TString&,TMVA::DataSetInfo&,TString&)");
        auto pluginName = std::string("TMVA") + specific_options.m_method;

        gROOT->GetPluginManager()->AddHandler(base.c_str(), regexp1.c_str(), className.c_str(), pluginName.c_str(), ctor1.c_str());
        gROOT->GetPluginManager()->AddHandler(base.c_str(), regexp2.c_str(), className.c_str(), pluginName.c_str(), ctor2.c_str());
        B2INFO("Registered new TMVA Plugin named " << pluginName)
      }

      if (!m_expert->BookMVA(specific_options.m_method, custom_weightfile)) {
        B2FATAL("Could not set up expert! Please see preceding error message from TMVA!");
      }

    }

    std::vector<float> TMVAExpertClassification::apply(Dataset& test_data) const
    {

      std::vector<float> probabilities(test_data.getNumberOfEvents());
      for (unsigned int iEvent = 0; iEvent < test_data.getNumberOfEvents(); ++iEvent) {
        test_data.loadEvent(iEvent);
        for (unsigned int i = 0; i < m_input_cache.size(); ++i)
          m_input_cache[i] = test_data.m_input[i];
        if (specific_options.transform2probability)
          probabilities[iEvent] = m_expert->GetProba(specific_options.m_method, expert_signalFraction);
        else
          probabilities[iEvent] = m_expert->EvaluateMVA(specific_options.m_method);
      }
      return probabilities;

    }

    std::vector<float> TMVAExpertRegression::apply(Dataset& test_data) const
    {

      std::vector<float> prediction(test_data.getNumberOfEvents());
      for (unsigned int iEvent = 0; iEvent < test_data.getNumberOfEvents(); ++iEvent) {
        test_data.loadEvent(iEvent);
        for (unsigned int i = 0; i < m_input_cache.size(); ++i)
          m_input_cache[i] = test_data.m_input[i];
        prediction[iEvent] = m_expert->EvaluateMVA(specific_options.m_method);
      }
      return prediction;

    }

  }
}
