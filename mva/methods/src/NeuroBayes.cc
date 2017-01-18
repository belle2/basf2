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

#include <assert.h>


#ifdef DEACTIVATE_NEUROBAYES
NeuroBayesTeacher* NeuroBayesTeacher::instance;

NeuroBayesTeacher* NeuroBayesTeacher::Instance(dsa::ec_t**, int , dsa::log_func_t*,
                                               void*, dsa::delete_enclosed_func_t*)
{
  printf("ERROR: NeuroBayesTeacher not available.");
  return 0;
}

void NeuroBayesTeacher::NB_DEF(bool) {assert(0);}
void NeuroBayesTeacher::NB_DEF_TASK(const char*) {assert(0);}
void NeuroBayesTeacher::NB_DEF_TASK(std::string&) {assert(0);}
void NeuroBayesTeacher::NB_DEF_DEBUG(int) {assert(0);}
void NeuroBayesTeacher::NB_DEF_PRE(int) {assert(0);}
void NeuroBayesTeacher::NB_DEF_INITIALPRUNE(int) {assert(0);}
void NeuroBayesTeacher::NB_DEF_NODE1(int) {assert(0);}
void NeuroBayesTeacher::NB_DEF_NODE2(int) {assert(0);}
void NeuroBayesTeacher::NB_DEF_NODE3(int) {assert(0);}
void NeuroBayesTeacher::NB_DEF_REG(const char*) {assert(0);}
void NeuroBayesTeacher::NB_DEF_LEARNDIAG(int) {assert(0);}
void NeuroBayesTeacher::NB_DEF_LOSS(const char*) {assert(0);}
void NeuroBayesTeacher::NB_DEF_SHAPE(const char*) {assert(0);}
void NeuroBayesTeacher::NB_DEF_METHOD(const char*) {assert(0);}
void NeuroBayesTeacher::NB_DEF_MOM(float) {assert(0);}
void NeuroBayesTeacher::NB_DEF_EPOCH(int) {assert(0);}
void NeuroBayesTeacher::NB_DEF_ITER(int) {assert(0);}
void NeuroBayesTeacher::NB_DEF_RTRAIN(float) {assert(0);}
void NeuroBayesTeacher::NB_DEF_SPEED(float) {assert(0);}
void NeuroBayesTeacher::NB_DEF_MAXLEARN(float) {assert(0);}
void NeuroBayesTeacher::NB_DEF_RELIMPORTANCE(float) {assert(0);}
void NeuroBayesTeacher::NB_DEF_SURRO(float) {assert(0);}
void NeuroBayesTeacher::NB_DEF_PRUNEMIN(float) {assert(0);}
void NeuroBayesTeacher::NB_DEF_PRUNEMAX(float) {assert(0);}
void NeuroBayesTeacher::NB_DEF_PRUNERESULT(float) {assert(0);}
void NeuroBayesTeacher::NB_DEF_QUANTILE(float) {assert(0);}
void NeuroBayesTeacher::NB_RANVIN(int   ,
                                  int   ,
                                  int) {assert(0);}
void NeuroBayesTeacher::NB_DEF_LOSSWGT(float) {assert(0);}
void NeuroBayesTeacher::NB_DEF_TDELTA(float) {assert(0);}
void NeuroBayesTeacher::NB_DEF_WEIGHT_MODE(int) {assert(0);}
void NeuroBayesTeacher::NB_DEF_SPLOT_MODE(int) {assert(0);}
void NeuroBayesTeacher::NB_TABDEF1(float*, float*,
                                   int , float*, int ,
                                   dsa::common_t*) {assert(0);}
void NeuroBayesTeacher::SetIndividualPreproFlag(int , int , const char*) {assert(0);}
void NeuroBayesTeacher::SetIndividualPreproParameter(int , int ,
                                                     float) {assert(0);}
void NeuroBayesTeacher::SetTarget(float) {assert(0);}
void NeuroBayesTeacher::SetWeight(float , float) {assert(0);}
void NeuroBayesTeacher::SetNextInput(int , float*) {assert(0);}
void NeuroBayesTeacher::SetNEvents(int) {assert(0);}
void NeuroBayesTeacher::TrainNet(bool) {assert(0);}
void NeuroBayesTeacher::SetOutputFile(const char*) {assert(0);}
void NeuroBayesTeacher::SetHistosFile(const char*) {assert(0);}
void NeuroBayesTeacher::SetCArrayFile(const char*) {assert(0);}
float* NeuroBayesTeacher::nb_get_expertise() {assert(0); return NULL; }
void NeuroBayesTeacher::nb_infoout(float*, float*, int*, int*,
                                   float*, float*, float*, float*,
                                   int*, int*) {assert(0);}
void NeuroBayesTeacher::nb_correl_signi(const char [],
                                        const char []) {assert(0);}
void NeuroBayesTeacher::nb_correl_signi(char**, const char [],
                                        const char []) {assert(0);}
char** NeuroBayesTeacher::nb_get_varnames(int*) {assert(0); return NULL;}
int* NeuroBayesTeacher::nb_get_individual_prepro_flags(int*) {assert(0); return NULL;}
void NeuroBayesTeacher::SayHello() {assert(0);}
NeuroBayesTeacher::~NeuroBayesTeacher() {assert(0);}

#endif

namespace Belle2 {
  namespace MVA {

    bool IsNeuroBayesAvailable()
    {
#ifdef NEUROBAYES_VERSION
      return (nb_init_common(0, NULL, NULL, NULL, 0) != NULL and ::NeuroBayesTeacher::Instance() != 0);
#else
      return (::NeuroBayesTeacher::Instance() != 0);
#endif
    }

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

      // Since we don't know the exact interface of the function nb_infoout,
      // we give for each of the pointers an array of the size of the number of variables
      // and hope that this is enough to avoid a heap overflow!
      std::vector<float> weightsum(numberOfFeatures, 0.0);
      std::vector<float> total(numberOfFeatures, 0.0);
      std::vector<int> keep(numberOfFeatures, 0);
      std::vector<int> rank(numberOfFeatures, 0);
      std::vector<float> single(numberOfFeatures, 0.0);
      std::vector<float> added(numberOfFeatures, 0.0);
      std::vector<float> global(numberOfFeatures, 0.0);
      std::vector<float> loss(numberOfFeatures, 0.0);
      std::vector<int> nvar(numberOfFeatures, 0);
      std::vector<int> index(numberOfFeatures, 0);
      nb->nb_infoout(&weightsum[0], &total[0], &keep[0], &rank[0], &single[0],
                     &added[0], &global[0], &loss[0], &nvar[0], &index[0]);

      std::map<std::string, float> feature_importances;
      for (unsigned int iFeature = 0; iFeature < numberOfFeatures; ++iFeature) {
        feature_importances[m_general_options.m_variables[iFeature]] = loss[iFeature];
      }

      Weightfile weightfile;
      weightfile.addOptions(m_general_options);
      weightfile.addOptions(m_specific_options);
      weightfile.addFile("NeuroBayes_Weightfile", m_specific_options.m_weightfile);
      weightfile.addSignalFraction(training_data.getSignalFraction());
      weightfile.addFeatureImportance(feature_importances);
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

