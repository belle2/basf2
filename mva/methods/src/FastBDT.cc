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

namespace Belle2 {
  namespace MVA {

    void FastBDTOptions::load(const boost::property_tree::ptree& pt)
    {
      int version = pt.get<int>("FastBDT_version");
      if (version != 1) {
        B2ERROR("Unkown weightfile version " << std::to_string(version));
        throw std::runtime_error("Unkown weightfile version " + std::to_string(version));
      }
      m_nTrees = pt.get<int>("FastBDT_nTrees");
      m_nCuts = pt.get<int>("FastBDT_nCuts");
      m_nLevels = pt.get<int>("FastBDT_nLevels");
      m_shrinkage = pt.get<double>("FastBDT_shrinkage");
      m_randRatio = pt.get<double>("FastBDT_randRatio");
    }

    void FastBDTOptions::save(boost::property_tree::ptree& pt) const
    {
      pt.put("FastBDT_version", 1);
      pt.put("FastBDT_nTrees", m_nTrees);
      pt.put("FastBDT_nCuts", m_nCuts);
      pt.put("FastBDT_nLevels", m_nLevels);
      pt.put("FastBDT_shrinkage", m_shrinkage);
      pt.put("FastBDT_randRatio", m_randRatio);
    }

    po::options_description FastBDTOptions::getDescription()
    {
      po::options_description description("FastBDT options");
      description.add_options()
      ("nTrees", po::value<unsigned int>(&m_nTrees), "Number of trees in the forest")
      ("nCutLevels", po::value<unsigned int>(&m_nCuts), "Number of cut levels per feature")
      ("nLevels", po::value<unsigned int>(&m_nLevels), "Depth of trees")
      ("shrinkage", po::value<double>(&m_shrinkage), "Shrinkage of the boosting algorithm")
      ("randRatio", po::value<double>(&m_randRatio), "Fraction of the data sampled each training iteration");
      return description;
    }


    FastBDTTeacher::FastBDTTeacher(const GeneralOptions& general_options,
                                   const FastBDTOptions& specific_options) : Teacher(general_options),
      m_specific_options(specific_options) { }

    Weightfile FastBDTTeacher::train(Dataset& training_data) const
    {

      unsigned int numberOfFeatures = training_data.getNumberOfFeatures();
      unsigned int numberOfEvents = training_data.getNumberOfEvents();

      std::vector<FastBDT::FeatureBinning<float>> featureBinnings;
      std::vector<unsigned int> nBinningLevels;
      for (unsigned int iFeature = 0; iFeature < numberOfFeatures; ++iFeature) {
        auto feature = training_data.getFeature(iFeature);
#if FastBDT_VERSION_MAJOR >= 3
        featureBinnings.push_back(FastBDT::FeatureBinning<float>(m_specific_options.m_nCuts, feature));
#else
        featureBinnings.push_back(FastBDT::FeatureBinning<float>(m_specific_options.m_nCuts, feature.begin(), feature.end()));
#endif
        nBinningLevels.push_back(m_specific_options.m_nCuts);
      }

      FastBDT::EventSample eventSample(numberOfEvents, numberOfFeatures, nBinningLevels);
      for (unsigned int iEvent = 0; iEvent < numberOfEvents; ++iEvent) {
        training_data.loadEvent(iEvent);
        std::vector<unsigned int> bins(numberOfFeatures);
        for (unsigned int iFeature = 0; iFeature < numberOfFeatures; ++iFeature) {
          bins[iFeature] = featureBinnings[iFeature].ValueToBin(training_data.m_input[iFeature]);
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

      Weightfile weightfile;
      std::string custom_weightfile = weightfile.getFileName();
      std::fstream file(custom_weightfile, std::ios_base::out | std::ios_base::trunc);
#if FastBDT_VERSION_MAJOR >= 3
      file << forest << std::endl;
#else
      file << featureBinnings << std::endl;
      file << forest << std::endl;
#endif
      file.close();

      weightfile.addOptions(m_general_options);
      weightfile.addOptions(m_specific_options);
      weightfile.addFile("FastBDT_Weightfile", custom_weightfile);
      weightfile.addSignalFraction(training_data.getSignalFraction());

      return weightfile;

    }

    void FastBDTExpert::load(Weightfile& weightfile)
    {

      std::string custom_weightfile = weightfile.getFileName();
      weightfile.getFile("FastBDT_Weightfile", custom_weightfile);
      std::fstream file(custom_weightfile, std::ios_base::in);
#if FastBDT_VERSION_MAJOR >= 3
      m_expert_forest = FastBDT::readForestFromStream<float>(file);
#else
      // Check for nan or inf in file and replace with 0
      std::stringstream s;
      std::string t;
      while (getline(file, t)) {
        size_t f = t.find("inf");
        if (f < std::string::npos) {
          t.replace(f, std::string("inf").length(), std::string("0.0"));
          B2WARNING("Found infinity in FastBDT weightfile, I replace it with 0 to prevent horrible crashes, this is fixed in the newer version");
        }
        f = t.find("nan");
        if (f < std::string::npos) {
          t.replace(f, std::string("nan").length(), std::string("0.0"));
          B2WARNING("Found nan in FastBDT weightfile, I replace it with 0 to prevent horrible crashes, this is fixed in the newer version");
        }
        s << t + '\n';
      }
      s >> m_expert_feature_binning;
      m_expert_forest = FastBDT::readForestFromStream(s);
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
        probabilities[iEvent] = m_expert_forest.Analyse(test_data.m_input);
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
