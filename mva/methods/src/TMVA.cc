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
#include <TPluginManager.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>

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
      ("tmva_working_directory", po::value<std::string>(&m_workingDirectory), "TMVA working directory which stores e.g. TMVA.root")
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

    TMVATeacher::TMVATeacher(const GeneralOptions& general_options, const TMVAOptions& _specific_options) : Teacher(general_options),
      specific_options(_specific_options) { }

#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
    Weightfile TMVATeacher::trainFactory(TMVA::Factory& factory, TMVA::DataLoader& data_loader, std::string& jobName) const
#else
    Weightfile TMVATeacher::trainFactory(TMVA::Factory& factory, std::string& jobName) const
#endif
    {
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
      data_loader.PrepareTrainingAndTestTree("", specific_options.m_prepareOption);
#else
      factory.PrepareTrainingAndTestTree("", specific_options.m_prepareOption);
#endif

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

#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
      if (!factory.BookMethod(&data_loader, specific_options.m_type, specific_options.m_method, specific_options.m_config)) {
#else
      if (!factory.BookMethod(specific_options.m_type, specific_options.m_method, specific_options.m_config)) {
#endif
        B2ERROR("TMVA Method with name " + specific_options.m_method + " cannot be booked.");
      }

      Weightfile weightfile;
      std::string logfilename = weightfile.generateFileName(".log");

      // Pipe stdout into a logfile to get TMVA output, which contains valueable information
      // which cannot be retreived otherwise!
      // Hence we do some black magic here
      // TODO Using ROOT_VERSION 6.08  this should be possible without this workaround
      auto logfile = open(logfilename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
      auto saved_stdout = dup(STDOUT_FILENO);
      dup2(logfile, 1);

      factory.TrainAllMethods();
      factory.TestAllMethods();
      factory.EvaluateAllMethods();

      // Reset original output
      dup2(saved_stdout, STDOUT_FILENO);
      close(saved_stdout);
      close(logfile);


      weightfile.addOptions(m_general_options);
      weightfile.addFile("TMVA_Weightfile", std::string("TMVA/weights/") + jobName + "_" + specific_options.m_method + ".weights.xml");
      weightfile.addFile("TMVA_Logfile", logfilename);

      // We have to parse the TMVA output to get the feature importances, there is no other way currently
      std::string begin = "Ranking input variables (method specific)";
      std::string end = "-----------------------------------";
      std::string line;
      std::ifstream file(logfilename, std::ios::in);
      std::map<std::string, float> feature_importances;
      int state = 0;
      while (std::getline(file, line)) {
        if (state == 0 && line.find(begin) != std::string::npos) {
          state = 1;
          continue;
        }
        if (state >= 1 and state <= 4) {
          state++;
          continue;
        }
        if (state == 5) {
          if (line.find(end) != std::string::npos)
            break;
          std::vector<std::string> strs;
          boost::split(strs, line, boost::is_any_of(":"));
          std::string variable = strs[2];
          boost::trim(variable);
          variable = Belle2::invertMakeROOTCompatible(variable);
          float importance = std::stof(strs[3]);
          feature_importances[variable] = importance;
        }
      }
      weightfile.addFeatureImportance(feature_importances);

      return weightfile;

    }


    TMVATeacherClassification::TMVATeacherClassification(const GeneralOptions& general_options,
                                                         const TMVAOptionsClassification& _specific_options) : TMVATeacher(general_options, _specific_options),
      specific_options(_specific_options) { }

    Weightfile TMVATeacherClassification::train(Dataset& training_data) const
    {

      unsigned int numberOfFeatures = training_data.getNumberOfFeatures();
      unsigned int numberOfSpectators = training_data.getNumberOfSpectators();
      unsigned int numberOfEvents = training_data.getNumberOfEvents();

      std::string directory = specific_options.m_workingDirectory;
      if (specific_options.m_workingDirectory.empty()) {
        char* directory_template = strdup("/tmp/Basf2TMVA.XXXXXX");
        directory = mkdtemp(directory_template);
        free(directory_template);
      }

      WorkingDirectoryManager dummy(directory);

      std::string jobName = specific_options.m_prefix;
      if (jobName.empty())
        jobName = "TMVA";
      TFile classFile((jobName + ".root").c_str(), "RECREATE");
      classFile.cd();

      TMVA::Tools::Instance();
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
      TMVA::DataLoader data_loader(jobName);
#endif
      TMVA::Factory factory(jobName, &classFile, specific_options.m_factoryOption);


      // Add variables to the factory
      for (auto& var : m_general_options.m_variables) {
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
        data_loader.AddVariable(Belle2::makeROOTCompatible(var));
#else
        factory.AddVariable(Belle2::makeROOTCompatible(var));
#endif
      }

      // Add variables to the factory
      for (auto& var : m_general_options.m_spectators) {
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
        data_loader.AddSpectator(Belle2::makeROOTCompatible(var));
#else
        factory.AddSpectator(Belle2::makeROOTCompatible(var));
#endif
      }

#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
      data_loader.SetWeightExpression(Belle2::makeROOTCompatible(m_general_options.m_weight_variable));
#else
      factory.SetWeightExpression(Belle2::makeROOTCompatible(m_general_options.m_weight_variable));
#endif

      TTree* signal_tree = new TTree("signal_tree", "signal_tree");
      TTree* background_tree = new TTree("background_tree", "background_tree");

      for (unsigned int iFeature = 0; iFeature < numberOfFeatures; ++iFeature) {
        signal_tree->Branch(Belle2::makeROOTCompatible(m_general_options.m_variables[iFeature]).c_str(),
                            &training_data.m_input[iFeature]);
        background_tree->Branch(Belle2::makeROOTCompatible(m_general_options.m_variables[iFeature]).c_str(),
                                &training_data.m_input[iFeature]);
      }

      for (unsigned int iSpectator = 0; iSpectator < numberOfSpectators; ++iSpectator) {
        signal_tree->Branch(Belle2::makeROOTCompatible(m_general_options.m_spectators[iSpectator]).c_str(),
                            &training_data.m_spectators[iSpectator]);
        background_tree->Branch(Belle2::makeROOTCompatible(m_general_options.m_spectators[iSpectator]).c_str(),
                                &training_data.m_spectators[iSpectator]);
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

#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
      data_loader.AddSignalTree(signal_tree);
      data_loader.AddBackgroundTree(background_tree);
      auto weightfile = trainFactory(factory, data_loader, jobName);
#else
      factory.AddSignalTree(signal_tree);
      factory.AddBackgroundTree(background_tree);
      auto weightfile = trainFactory(factory, jobName);
#endif

      weightfile.addOptions(specific_options);
      weightfile.addSignalFraction(training_data.getSignalFraction());

      delete signal_tree;
      delete background_tree;

      if (specific_options.m_workingDirectory.empty()) {
        boost::filesystem::remove_all(directory);
      }

      return weightfile;

    }

    TMVATeacherRegression::TMVATeacherRegression(const GeneralOptions& general_options,
                                                 const TMVAOptionsRegression& _specific_options) : TMVATeacher(general_options, _specific_options),
      specific_options(_specific_options) { }

    Weightfile TMVATeacherRegression::train(Dataset& training_data) const
    {

      unsigned int numberOfFeatures = training_data.getNumberOfFeatures();
      unsigned int numberOfSpectators = training_data.getNumberOfSpectators();
      unsigned int numberOfEvents = training_data.getNumberOfEvents();

      std::string directory = specific_options.m_workingDirectory;
      if (specific_options.m_workingDirectory.empty()) {
        char* directory_template = strdup("/tmp/Basf2TMVA.XXXXXX");
        directory = mkdtemp(directory_template);
        free(directory_template);
      }

      WorkingDirectoryManager dummy(directory);

      std::string jobName = specific_options.m_prefix;
      if (jobName.empty())
        jobName = "TMVA";
      TFile classFile((jobName + ".root").c_str(), "RECREATE");
      classFile.cd();

      TMVA::Tools::Instance();
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
      TMVA::DataLoader data_loader(jobName);
#endif
      TMVA::Factory factory(jobName, &classFile, specific_options.m_factoryOption);

      // Add variables to the factory
      for (auto& var : m_general_options.m_variables) {
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
        data_loader.AddVariable(Belle2::makeROOTCompatible(var));
#else
        factory.AddVariable(Belle2::makeROOTCompatible(var));
#endif
      }

      // Add variables to the factory
      for (auto& var : m_general_options.m_spectators) {
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
        data_loader.AddSpectator(Belle2::makeROOTCompatible(var));
#else
        factory.AddSpectator(Belle2::makeROOTCompatible(var));
#endif
      }

#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
      data_loader.AddTarget(Belle2::makeROOTCompatible(m_general_options.m_target_variable));
#else
      factory.AddTarget(Belle2::makeROOTCompatible(m_general_options.m_target_variable));
#endif


      TTree* regression_tree = new TTree("regression_tree", "regression_tree");

      for (unsigned int iFeature = 0; iFeature < numberOfFeatures; ++iFeature) {
        regression_tree->Branch(Belle2::makeROOTCompatible(m_general_options.m_variables[iFeature]).c_str(),
                                &training_data.m_input[iFeature]);
      }
      for (unsigned int iSpectator = 0; iSpectator < numberOfSpectators; ++iSpectator) {
        regression_tree->Branch(Belle2::makeROOTCompatible(m_general_options.m_spectators[iSpectator]).c_str(),
                                &training_data.m_spectators[iSpectator]);
      }
      regression_tree->Branch(Belle2::makeROOTCompatible(m_general_options.m_target_variable).c_str(),
                              &training_data.m_target);

      regression_tree->Branch("__weight__", &training_data.m_weight);

      for (unsigned int iEvent = 0; iEvent < numberOfEvents; ++iEvent) {
        training_data.loadEvent(iEvent);
        regression_tree->Fill();
      }

#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
      data_loader.AddRegressionTree(regression_tree);
      data_loader.SetWeightExpression(Belle2::makeROOTCompatible(m_general_options.m_weight_variable), "Regression");

      auto weightfile = trainFactory(factory, data_loader, jobName);
#else
      factory.AddRegressionTree(regression_tree);
      factory.SetWeightExpression(Belle2::makeROOTCompatible(m_general_options.m_weight_variable), "Regression");

      auto weightfile = trainFactory(factory, jobName);
#endif
      weightfile.addOptions(specific_options);

      delete regression_tree;

      if (specific_options.m_workingDirectory.empty()) {
        boost::filesystem::remove_all(directory);
      }

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
        m_expert->AddVariable(Belle2::makeROOTCompatible(general_options.m_variables[i]), &m_input_cache[i]);
      }

      if (weightfile.containsElement("TMVA_Logfile")) {
        std::string custom_weightfile = weightfile.generateFileName("logfile");
        weightfile.getFile("TMVA_Logfile", custom_weightfile);
      }

    }

    void TMVAExpertClassification::load(Weightfile& weightfile)
    {

      weightfile.getOptions(specific_options);
      expert_signalFraction = weightfile.getSignalFraction();

      // TMVA parses the method type for plugins out of the weightfile name, so we must ensure that it has the expected format
      std::string custom_weightfile = weightfile.generateFileName(std::string("_") + specific_options.m_method + ".weights.xml");
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
        B2INFO("Registered new TMVA Plugin named " << pluginName);
      }

      if (!m_expert->BookMVA(specific_options.m_method, custom_weightfile)) {
        B2FATAL("Could not set up expert! Please see preceding error message from TMVA!");
      }

    }

    void TMVAExpertRegression::load(Weightfile& weightfile)
    {

      weightfile.getOptions(specific_options);

      // TMVA parses the method type for plugins out of the weightfile name, so we must ensure that it has the expected format
      std::string custom_weightfile = weightfile.generateFileName(std::string("_") + specific_options.m_method + ".weights.xml");
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
        B2INFO("Registered new TMVA Plugin named " << pluginName);
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
