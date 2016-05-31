/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/methods/NeuroBayes.h>

#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace MVA {

    void NeuroBayesOptions::load(const boost::property_tree::ptree& pt)
    {
      int version = pt.get<int>("NeuroBayes_version");
      if (version != 1) {
        B2ERROR("Unkown weightfile version " << std::to_string(version));
        throw std::runtime_error("Unkown weightfile version " + std::to_string(version));
      }
      m_lossFunction = pt.get<std::string>("NeuroBayes_lossFunction");
      m_regularization = pt.get<std::string>("NeuroBayes_regularization");
      m_preprocessing = pt.get<int>("NeuroBayes_preprocessing");
      m_pruning = pt.get<int>("NeuroBayes_pruning");
      m_shapeTreat = pt.get<std::string>("NeuroBayes_shapeTreat");
      m_momentum = pt.get<float>("NeuroBayes_momentum");
      m_weightUpdate = pt.get<int>("NeuroBayes_weightUpdate");
      m_trainTestRatio = pt.get<float>("NeuroBayes_trainTestRatio");
      m_nIterations = pt.get<int>("NeuroBayes_nIterations");
      m_learningSpeed = pt.get<float>("NeuroBayes_learningSpeed");
      m_limitLearningSpeed = pt.get<float>("NeuroBayes_limitLearningSpeed");
      m_method = pt.get<std::string>("NeuroBayes_method");
      m_weightfile = pt.get<std::string>("NeuroBayes_weightfile");
    }

    void NeuroBayesOptions::save(boost::property_tree::ptree& pt) const
    {
      pt.put("NeuroBayes_version", 1);
      pt.put("NeuroBayes_lossFunction", m_lossFunction);
      pt.put("NeuroBayes_regularization", m_regularization);
      pt.put("NeuroBayes_preprocessing", m_preprocessing);
      pt.put("NeuroBayes_pruning", m_pruning);
      pt.put("NeuroBayes_shapeTreat", m_shapeTreat);
      pt.put("NeuroBayes_momentum", m_momentum);
      pt.put("NeuroBayes_weightUpdate", m_weightUpdate);
      pt.put("NeuroBayes_trainTestRatio", m_trainTestRatio);
      pt.put("NeuroBayes_nIterations", m_nIterations);
      pt.put("NeuroBayes_learningSpeed", m_learningSpeed);
      pt.put("NeuroBayes_limitLearningSpeed", m_limitLearningSpeed);
      pt.put("NeuroBayes_method", m_method);
      pt.put("NeuroBayes_weightfile", m_weightfile);
    }

    po::options_description NeuroBayesOptions::getDescription()
    {
      po::options_description description("NeuroBayes options");
      description.add_options()
      ("loss_function", po::value<std::string>(&m_lossFunction),
       "Type of loss-function: Possible choices are ENTROPY (default), QUADRATIC and COMBINED.")
      ("regularization", po::value<std::string>(&m_regularization),
       "Type of regularisation: Possible choices are: OFF , REG (default), ARD , ASR, ALL.")
      ("preprocessing", po::value<int>(&m_preprocessing),
       "Type of preprocessing: Refer to NeuroBayes-Howto A.1 for Details.")
      ("pruning", po::value<int>(&m_pruning), "NOT IMPLEMENTED")
      ("neurobayes_shapeTreat", po::value<std::string>(&m_shapeTreat),
       "Shape treatment: Possible choices are OFF, INCL, MARGINAL, DIAG and TOL.")
      ("momentum", po::value<float>(&m_momentum), "Optionally, a momentum can be specified for the training.")
      ("weightUpdate", po::value<int>(&m_weightUpdate),
       "Weight update: Normally, the weights are updated every 200 events.")
      ("trainTestRatio", po::value<float>(&m_trainTestRatio), "NOT IMPLEMENTED")
      ("nIterations", po::value<int>(&m_nIterations),
       "Number of training iterations: This parameter defines the number of training iterations.")
      ("learningSpeed", po::value<float>(&m_learningSpeed),
       "Increase learning speed by a multiplicative factor")
      ("limitLearningSpeed", po::value<float>(&m_limitLearningSpeed),
       "Limit learning speed: The maximal learning speed may be limited")
      ("neurobayes_method", po::value<std::string>(&m_method),
       "Training Method: It is possible to use the BFGS algorithm")
      ("preproFlagList", po::value<std::vector<int>>(&m_preproFlagList)->multitoken(),
       "Set individual preprocessing flags e.g. 12 12 12 12. See NeuroBayes-HowTo for infos")
      ("neurobayes_weightfile", po::value<std::string>(&m_weightfile),
       "Name of the weightfile produced temporarily by NeuroBayes");
      return description;
    }



    NeuroBayesTeacher::NeuroBayesTeacher(const GeneralOptions& general_options,
                                         const NeuroBayesOptions& specific_options) : Teacher(general_options), m_specific_options(specific_options) { }

    Weightfile NeuroBayesTeacher::train(Dataset& training_data) const
    {

      unsigned int numberOfFeatures = training_data.getNumberOfFeatures();
      unsigned int numberOfEvents = training_data.getNumberOfEvents();

      ::NeuroBayesTeacher* nb = ::NeuroBayesTeacher::Instance();
      nb->NB_DEF(true);

      nb->SetOutputFile(m_specific_options.m_weightfile.c_str());
      nb->NB_DEF_NODE1(numberOfFeatures + 1);
      nb->NB_DEF_NODE2(numberOfFeatures + 2);
      nb->NB_DEF_NODE3(1);
      nb->NB_DEF_TASK("CLA");

      // random number seed initialisation,
      // i has to be an odd number, the third argument is a debugging flag
      int i = 4701;
      int j = 21;
      nb->NB_RANVIN(i, j, 2);

      for (unsigned int iEvent = 0; iEvent < numberOfEvents; iEvent++) {
        training_data.loadEvent(iEvent);
        double type = training_data.m_isSignal ? 1.0 : 0.0;

        nb->SetWeight(training_data.m_weight);
        nb->SetTarget(type);
        nb->SetNextInput(numberOfFeatures, training_data.m_input.data());
      }

      nb->NB_DEF_PRE(m_specific_options.m_preprocessing);
      nb->NB_DEF_REG(m_specific_options.m_regularization.c_str());
      nb->NB_DEF_LOSS(m_specific_options.m_lossFunction.c_str());
      nb->NB_DEF_SHAPE(m_specific_options.m_shapeTreat.c_str());

      nb->NB_DEF_EPOCH(m_specific_options.m_weightUpdate);
      nb->NB_DEF_MOM(m_specific_options.m_momentum);

      nb->NB_DEF_SPEED(m_specific_options.m_learningSpeed);
      nb->NB_DEF_MAXLEARN(m_specific_options.m_limitLearningSpeed);

      nb->NB_DEF_ITER(m_specific_options.m_nIterations);
      nb->NB_DEF_METHOD(m_specific_options.m_method.c_str());
      //nb->NB_DEF_INITIALPRUNE(NeuroBayes_options.pruning);
      //nb->NB_DEF_RTRAIN(NeuroBayes_options.trainTestRatio);

      for (unsigned int iFeature = 0; iFeature < numberOfFeatures; ++iFeature) {
        unsigned int flag = 34;
        if (iFeature < m_specific_options.m_preproFlagList.size())
          flag = m_specific_options.m_preproFlagList[iFeature];
        nb->SetIndividualPreproFlag(iFeature, flag);
      }

      nb->TrainNet();

      Weightfile weightfile;
      weightfile.addOptions(m_general_options);
      weightfile.addOptions(m_specific_options);
      weightfile.addFile("NeuroBayes_Weightfile", m_specific_options.m_weightfile);
      weightfile.addSignalFraction(training_data.getSignalFraction());
      // Individual preprocessing flags are not saved at the moment, but they are saved in the expertise

      return weightfile;
    }

    void NeuroBayesExpert::load(Weightfile& weightfile)
    {


      std::string custom_weightfile = weightfile.getFileName();
      weightfile.getFile("NeuroBayes_Weightfile", custom_weightfile);

      m_expert = std::unique_ptr<::Expert>(new ::Expert(custom_weightfile));
      GeneralOptions general_options;
      weightfile.getOptions(general_options);
      weightfile.getOptions(m_specific_options);
    }

    std::vector<float> NeuroBayesExpert::apply(Dataset& test_data) const
    {
      std::vector<double> input_cache(test_data.getNumberOfFeatures(), 0.0);
      std::vector<float> probabilities(test_data.getNumberOfEvents());
      for (unsigned int iEvent = 0; iEvent < test_data.getNumberOfEvents(); ++iEvent) {
        test_data.loadEvent(iEvent);
        for (unsigned int i = 0; i < input_cache.size(); ++i)
          input_cache[i] = test_data.m_input[i];
        probabilities[iEvent] = m_expert->nb_expert(input_cache.data());
      }
      return probabilities;

    }

  }
}

