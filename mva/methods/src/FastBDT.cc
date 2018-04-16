/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/methods/FastBDT.h>

#include <framework/logging/Logger.h>
#include <sstream>
#include <vector>

// Template specialization to fix NAN sort bug of FastBDT in upto Version 3.2
#if FastBDT_VERSION_MAJOR <= 3 && FastBDT_VERSION_MINOR <= 2
namespace FastBDT {
  template<>
  bool compareIncludingNaN(float i, float j)
  {
    if (std::isnan(i)) {
      if (std::isnan(j)) {
        // If both are NAN i is NOT smaller
        return false;
      } else {
        // In all other cases i is smaller
        return true;
      }
    }
    // If j is NaN the following line will return false,
    // which is fine in our case.
    return i < j;
  }
}
#endif

namespace Belle2 {
  namespace MVA {
    bool isValidSignal(const std::vector<bool>& Signals)
    {
      const auto first = Signals.front();
      for (const auto& value : Signals) {
        if (value != first)
          return true;
      }
      return false;
    }

    void FastBDTOptions::load(const boost::property_tree::ptree& pt)
    {
      int version = pt.get<int>("FastBDT_version");
#if FastBDT_VERSION_MAJOR >= 5
      if (version != 1 and version != 2) {
        B2ERROR("Unkown weightfile version " << std::to_string(version));
        throw std::runtime_error("Unkown weightfile version " + std::to_string(version));
      }
#else
      if (version != 1) {
        B2ERROR("Unkown weightfile version " << std::to_string(version));
        throw std::runtime_error("Unkown weightfile version " + std::to_string(version));
      }
#endif
      m_nTrees = pt.get<int>("FastBDT_nTrees");
      m_nCuts = pt.get<int>("FastBDT_nCuts");
      m_nLevels = pt.get<int>("FastBDT_nLevels");
      m_shrinkage = pt.get<double>("FastBDT_shrinkage");
      m_randRatio = pt.get<double>("FastBDT_randRatio");

#if FastBDT_VERSION_MAJOR >= 5
      if (version > 1) {

        m_flatnessLoss = pt.get<double>("FastBDT_flatnessLoss");
        m_sPlot = pt.get<bool>("FastBDT_sPlot");

        unsigned int numberOfIndividualNCuts = pt.get<unsigned int>("FastBDT_number_individual_nCuts", 0);
        m_individual_nCuts.resize(numberOfIndividualNCuts);
        for (unsigned int i = 0; i < numberOfIndividualNCuts; ++i) {
          m_individual_nCuts[i] = pt.get<unsigned int>(std::string("FastBDT_individual_nCuts") + std::to_string(i));
        }

        m_purityTransformation = pt.get<bool>("FastBDT_purityTransformation");
        unsigned int numberOfIndividualPurityTransformation = pt.get<unsigned int>("FastBDT_number_individualPurityTransformation", 0);
        m_individualPurityTransformation.resize(numberOfIndividualPurityTransformation);
        for (unsigned int i = 0; i < numberOfIndividualPurityTransformation; ++i) {
          m_individualPurityTransformation[i] = pt.get<bool>(std::string("FastBDT_individualPurityTransformation") + std::to_string(i));
        }

      } else {
        m_flatnessLoss = -1.0;
        m_sPlot = false;
      }
#endif
    }

    void FastBDTOptions::save(boost::property_tree::ptree& pt) const
    {
#if FastBDT_VERSION_MAJOR >= 5
      pt.put("FastBDT_version", 2);
#else
      pt.put("FastBDT_version", 1);
#endif
      pt.put("FastBDT_nTrees", m_nTrees);
      pt.put("FastBDT_nCuts", m_nCuts);
      pt.put("FastBDT_nLevels", m_nLevels);
      pt.put("FastBDT_shrinkage", m_shrinkage);
      pt.put("FastBDT_randRatio", m_randRatio);
#if FastBDT_VERSION_MAJOR >= 5
      pt.put("FastBDT_flatnessLoss", m_flatnessLoss);
      pt.put("FastBDT_sPlot", m_sPlot);
      pt.put("FastBDT_number_individual_nCuts", m_individual_nCuts.size());
      for (unsigned int i = 0; i < m_individual_nCuts.size(); ++i) {
        pt.put(std::string("FastBDT_individual_nCuts") + std::to_string(i), m_individual_nCuts[i]);
      }
      pt.put("FastBDT_purityTransformation", m_purityTransformation);
      pt.put("FastBDT_number_individualPurityTransformation", m_individualPurityTransformation.size());
      for (unsigned int i = 0; i < m_individualPurityTransformation.size(); ++i) {
        pt.put(std::string("FastBDT_individualPurityTransformation") + std::to_string(i), m_individualPurityTransformation[i]);
      }
#endif
    }

    po::options_description FastBDTOptions::getDescription()
    {
      po::options_description description("FastBDT options");
      description.add_options()
      ("nTrees", po::value<unsigned int>(&m_nTrees), "Number of trees in the forest. Reasonable values are between 10 and 1000")
      ("nLevels", po::value<unsigned int>(&m_nLevels)->notifier(check_bounds<unsigned int>(0, 20, "nLevels")),
       "Depth d of trees. The last layer of the tree will contain 2^d bins. Maximum is 20. Resonable values are 2 and 6.")
      ("shrinkage", po::value<double>(&m_shrinkage)->notifier(check_bounds<double>(0.0, 1.0, "shrinkage")),
       "Shrinkage of the boosting algorithm. Reasonable values are between 0.01 and 1.0.")
      ("nCutLevels", po::value<unsigned int>(&m_nCuts)->notifier(check_bounds<unsigned int>(0, 20, "nCutLevels")),
       "Number of cut levels N per feature. 2^N Bins will be used per feature. Reasonable values are between 6 and 12.")
#if FastBDT_VERSION_MAJOR >= 5
      ("individualNCutLevels", po::value<std::vector<unsigned int>>(&m_individual_nCuts)->multitoken()->notifier(
         check_bounds_vector<unsigned int>(0, 20, "individualNCutLevels")),
       "Number of cut levels N per feature. 2^N Bins will be used per feature. Reasonable values are between 6 and 12. One value per feature (including spectators) should be provided, if parameter is not set the global value specified by nCutLevels is used for all features.")
      ("sPlot", po::value<bool>(&m_sPlot),
       "Since in sPlot each event enters twice, this option modifies the sampling algorithm so that the matching signal and background events are selected together.")
      ("flatnessLoss", po::value<double>(&m_flatnessLoss),
       "Activate Flatness Loss, all spectator variables are assumed to be variables in which the signal and background efficiency should be flat. negative values deactivates flatness loss.")
      ("purityTransformation", po::value<bool>(&m_purityTransformation),
       "Activates purity transformation on all features: Add the purity transformed of all features in addition to the training. This will double the number of features and slow down the inference considerably")
      ("individualPurityTransformation", po::value<std::vector<bool>>(&m_individualPurityTransformation)->multitoken(),
       "Activates purity transformation for each feature: Vector of boolean values which decide if the purity transformed of the feature should be added in addition to this training.")
#endif
      ("randRatio", po::value<double>(&m_randRatio)->notifier(check_bounds<double>(0.0, 1.0001, "randRatio")),
       "Fraction of the data sampled each training iteration. Reasonable values are between 0.1 and 1.0.");
      return description;
    }


    FastBDTTeacher::FastBDTTeacher(const GeneralOptions& general_options,
                                   const FastBDTOptions& specific_options) : Teacher(general_options),
      m_specific_options(specific_options) { }

    Weightfile FastBDTTeacher::train(Dataset& training_data) const
    {

      unsigned int numberOfFeatures = training_data.getNumberOfFeatures();
#if FastBDT_VERSION_MAJOR >= 4
      unsigned int numberOfSpectators = training_data.getNumberOfSpectators();
#else
      // Deactivate support for spectators below version 4!
      unsigned int numberOfSpectators = 0;
#endif

      // FastBDT Version 4 has a simplified interface with a sklearn style Classifier
#if FastBDT_VERSION_MAJOR >= 5
      if (m_specific_options.m_individual_nCuts.size() != 0
          and m_specific_options.m_individual_nCuts.size() != numberOfFeatures + numberOfSpectators) {
        B2ERROR("You provided individual nCut values for each feature and spectator, but the total number of provided cuts is not same as as the total number of features and spectators.");
      }

      std::vector<bool> individualPurityTransformation = m_specific_options.m_individualPurityTransformation;
      if (m_specific_options.m_purityTransformation) {
        if (individualPurityTransformation.size() == 0) {
          for (unsigned int i = 0; i < numberOfFeatures; ++i) {
            individualPurityTransformation.push_back(true);
          }
        }
      }

      std::vector<unsigned int> individual_nCuts = m_specific_options.m_individual_nCuts;
      if (individual_nCuts.size() == 0) {
        for (unsigned int i = 0; i < numberOfFeatures + numberOfSpectators; ++i) {
          individual_nCuts.push_back(m_specific_options.m_nCuts);
        }
      }

      FastBDT::Classifier classifier(m_specific_options.m_nTrees,  m_specific_options.m_nLevels, individual_nCuts,
                                     m_specific_options.m_shrinkage, m_specific_options.m_randRatio,
                                     m_specific_options.m_sPlot, m_specific_options.m_flatnessLoss, individualPurityTransformation,
                                     numberOfSpectators, true);

      std::vector<std::vector<float>> X(numberOfFeatures + numberOfSpectators);
      const auto& y = training_data.getSignals();
      if (not isValidSignal(y)) {
        B2FATAL("The training data is not valid. It only contains one class instead of two.");
      }
      const auto& w = training_data.getWeights();
      for (unsigned int i = 0; i < numberOfFeatures; ++i) {
        X[i] = training_data.getFeature(i);
      }
      for (unsigned int i = 0; i < numberOfSpectators; ++i) {
        X[i + numberOfFeatures] = training_data.getSpectator(i);
      }
      classifier.fit(X, y, w);
#else
      const auto& y = training_data.getSignals();
      if (not isValidSignal(y)) {
        B2FATAL("The training data is not valid. It only contains one class instead of two.");
      }
      std::vector<FastBDT::FeatureBinning<float>> featureBinnings;
      std::vector<unsigned int> nBinningLevels;
      for (unsigned int iFeature = 0; iFeature < numberOfFeatures; ++iFeature) {
        auto feature = training_data.getFeature(iFeature);

        unsigned int nCuts = m_specific_options.m_nCuts;
#if FastBDT_VERSION_MAJOR >= 3
        featureBinnings.push_back(FastBDT::FeatureBinning<float>(nCuts, feature));
#else
        featureBinnings.push_back(FastBDT::FeatureBinning<float>(nCuts, feature.begin(), feature.end()));
#endif
        nBinningLevels.push_back(nCuts);
      }

      for (unsigned int iSpectator = 0; iSpectator < numberOfSpectators; ++iSpectator) {
        auto feature = training_data.getSpectator(iSpectator);

        unsigned int nCuts = m_specific_options.m_nCuts;
#if FastBDT_VERSION_MAJOR >= 3
        featureBinnings.push_back(FastBDT::FeatureBinning<float>(nCuts, feature));
#else
        featureBinnings.push_back(FastBDT::FeatureBinning<float>(nCuts, feature.begin(), feature.end()));
#endif
        nBinningLevels.push_back(nCuts);
      }

      unsigned int numberOfEvents = training_data.getNumberOfEvents();

#if FastBDT_VERSION_MAJOR >= 4
      FastBDT::EventSample eventSample(numberOfEvents, numberOfFeatures, numberOfSpectators, nBinningLevels);
#else
      FastBDT::EventSample eventSample(numberOfEvents, numberOfFeatures, nBinningLevels);
#endif
      std::vector<unsigned int> bins(numberOfFeatures + numberOfSpectators);
      for (unsigned int iEvent = 0; iEvent < numberOfEvents; ++iEvent) {
        training_data.loadEvent(iEvent);
        for (unsigned int iFeature = 0; iFeature < numberOfFeatures + numberOfSpectators; ++iFeature) {
          bins[iFeature] = featureBinnings[iFeature].ValueToBin(training_data.m_input[iFeature]);
        }
        for (unsigned int iSpectator = 0; iSpectator < numberOfSpectators; ++iSpectator) {
          bins[iSpectator + numberOfFeatures] = featureBinnings[iSpectator + numberOfFeatures].ValueToBin(
                                                  training_data.m_spectators[iSpectator]);
        }
        eventSample.AddEvent(bins, training_data.m_weight, training_data.m_isSignal);
      }

      FastBDT::ForestBuilder dt(eventSample, m_specific_options.m_nTrees, m_specific_options.m_shrinkage, m_specific_options.m_randRatio,
                                m_specific_options.m_nLevels);
#if FastBDT_VERSION_MAJOR >= 3
      FastBDT::Forest<float> forest(dt.GetShrinkage(), dt.GetF0(), true);
#else
      FastBDT::Forest forest(dt.GetShrinkage(), dt.GetF0());
#endif
      for (auto t : dt.GetForest()) {
#if FastBDT_VERSION_MAJOR >= 3
        auto tree = FastBDT::removeFeatureBinningTransformationFromTree(t, featureBinnings);
        forest.AddTree(tree);
#else
        forest.AddTree(t);
#endif
      }

#endif


      Weightfile weightfile;
      std::string custom_weightfile = weightfile.generateFileName();
      std::fstream file(custom_weightfile, std::ios_base::out | std::ios_base::trunc);

#if FastBDT_VERSION_MAJOR >= 5
      file << classifier << std::endl;
#else
#if FastBDT_VERSION_MAJOR >= 3
      file << forest << std::endl;
#else
      file << featureBinnings << std::endl;
      file << forest << std::endl;
#endif
#endif
      file.close();

      weightfile.addOptions(m_general_options);
      weightfile.addOptions(m_specific_options);
      weightfile.addFile("FastBDT_Weightfile", custom_weightfile);
      weightfile.addSignalFraction(training_data.getSignalFraction());

      std::map<std::string, float> importance;
#if FastBDT_VERSION_MAJOR >= 5
      for (auto& pair : classifier.GetVariableRanking()) {
        importance[m_general_options.m_variables[pair.first]] = pair.second;
      }
#else
      for (auto& pair : forest.GetVariableRanking()) {
        importance[m_general_options.m_variables[pair.first]] = pair.second;
      }
#endif
      weightfile.addFeatureImportance(importance);

      return weightfile;

    }

    void FastBDTExpert::load(Weightfile& weightfile)
    {

      std::string custom_weightfile = weightfile.generateFileName();
      weightfile.getFile("FastBDT_Weightfile", custom_weightfile);
      std::fstream file(custom_weightfile, std::ios_base::in);

      int version = weightfile.getElement<int>("FastBDT_version", 0);
      B2DEBUG(100, "FastBDT Weightfile Version " << version);
      if (version < 2) {
#if FastBDT_VERSION_MAJOR >= 3
        std::stringstream s;
        {
          std::string t;
          std::fstream file2(custom_weightfile, std::ios_base::in);
          getline(file2, t);
          s << t;
        }
        int dummy;
        // Try to read to integers, if this is sucessfull we have a old weightfile with a Feature Binning before the Tree.
        if (!(s >> dummy >> dummy)) {
          B2DEBUG(100, "FastBDT: I read a new weightfile of FastBDT using the new FastBDT version 3. Everythings fine!");
          // New format since version 3
          m_expert_forest = FastBDT::readForestFromStream<float>(file);
        } else {
          B2INFO("FastBDT: I read an old weightfile of FastBDT using the new FastBDT version 3."
                 "I will convert your FastBDT on-the-fly to the new version."
                 "Retrain the classifier to get rid of this message");
          // Old format before version 3
          // We read in first the feature binnings and than rewrite the tree
          std::vector<FastBDT::FeatureBinning<float>> feature_binnings;
          file >> feature_binnings;
          double F0;
          file >> F0;
          double shrinkage;
          file >> shrinkage;
          // This parameter was not available in the old version
          bool transform2probability = true;
          FastBDT::Forest<unsigned int> temp_forest(shrinkage, F0, transform2probability);
          unsigned int size;
          file >> size;
          for (unsigned int i = 0; i < size; ++i) {
            temp_forest.AddTree(FastBDT::readTreeFromStream<unsigned int>(file));
          }

          FastBDT::Forest<float> cleaned_forest(temp_forest.GetShrinkage(), temp_forest.GetF0(), temp_forest.GetTransform2Probability());
          for (auto& tree : temp_forest.GetForest()) {
            cleaned_forest.AddTree(FastBDT::removeFeatureBinningTransformationFromTree(tree, feature_binnings));
          }
          m_expert_forest = cleaned_forest;
        }
#else
        B2INFO("FastBDT: I read an old weightfile of FastBDT using the old FastBDT version."
               "I try to fix the weightfile first to avoid problems due to NaN and inf values."
               "Consider to switch to the newer version of FastBDT (newer externals)");
        // Check for nan or inf in file and replace with 0
        std::stringstream s;
        std::string t;
        while (getline(file, t)) {
          size_t f = 0;

          while ((f = t.find("inf", f)) != std::string::npos) {
            t.replace(f, std::string("inf").length(), std::string("0.0"));
            f += std::string("0.0").length();
            B2WARNING("Found infinity in FastBDT weightfile, I replace it with 0 to prevent horrible crashes, this is fixed in the newer version");
          }
          f = 0;
          while ((f = t.find("nan", f)) != std::string::npos) {
            t.replace(f, std::string("nan").length(), std::string("0.0"));
            f += std::string("0.0").length();
            B2WARNING("Found nan in FastBDT weightfile, I replace it with 0 to prevent horrible crashes, this is fixed in the newer version");
          }
          s << t + '\n';
        }
        s >> m_expert_feature_binning;
        m_expert_forest = FastBDT::readForestFromStream(s);
#endif
      }
#if FastBDT_VERSION_MAJOR >= 5
      else {
        m_use_simplified_interface = true;
        m_classifier = FastBDT::Classifier(file);
      }
#else
      else {
        B2ERROR("Unknown Version 2 of Weightfile, please use a more recent FastBDT version");
      }
#endif
      file.close();

      weightfile.getOptions(m_specific_options);
    }

    std::vector<float> FastBDTExpert::apply(Dataset& test_data) const
    {

      std::vector<float> probabilities(test_data.getNumberOfEvents());
      for (unsigned int iEvent = 0; iEvent < test_data.getNumberOfEvents(); ++iEvent) {
        test_data.loadEvent(iEvent);
#if FastBDT_VERSION_MAJOR >= 3
#if FastBDT_VERSION_MAJOR >= 5
        if (m_use_simplified_interface)
          probabilities[iEvent] = m_classifier.predict(test_data.m_input);
        else
          probabilities[iEvent] = m_expert_forest.Analyse(test_data.m_input);
#else
        probabilities[iEvent] = m_expert_forest.Analyse(test_data.m_input);
#endif
#else
        std::vector<unsigned int> bins(m_expert_feature_binning.size());
        for (unsigned int iFeature = 0; iFeature < m_expert_feature_binning.size(); ++iFeature) {
          bins[iFeature] = m_expert_feature_binning[iFeature].ValueToBin(test_data.m_input[iFeature]);
        }
        probabilities[iEvent] = m_expert_forest.Analyse(bins);
#endif
      }

      return probabilities;

    }

  }
}
