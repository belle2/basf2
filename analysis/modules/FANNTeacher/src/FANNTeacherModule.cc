/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Fernando Abudinen                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/FANNTeacher/FANNTeacherModule.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/utility/WorkingDirectoryManager.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>

#include <memory>
#include <fstream>
#include <cmath>

#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TObjArray.h>
#include <TFormula.h>

namespace Belle2 {


  REG_MODULE(FANNTeacher)

  FANNTeacherModule::FANNTeacherModule() : Module(), m_sample_var(nullptr), m_maxSamples(0), m_nSamples(0)
  {
    setDescription("Trains FANN method with given particle lists as training samples. "
                   "The target variable has to be an integer valued variable which defines the clusters in the sample. "
                   "e.g. isSignal for signal and background cluster or abs_PDG to define different MC-PDGs as clusters. "
                   "The clusters are trained against each other. "
                   "See also https://belle2.cc.kek.jp/~twiki/bin/view/Software/FANN for detailed instructions.");

    setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

    std::vector<std::string> empty;
    addParam("listNames", m_listNames,
             "Particles from these ParticleLists are used as input. If no name is given the teacher is applied to every event once, and one can only use variables which accept nullptr as Particle*",
             empty);
    addParam("variables", m_variableNames, "Input variables used by the FANN method");

    addParam("prefix", m_prefix,
             "Prefix which is used by the FANNTeacherModule to store its MLP parameters file, the file where the training samples will be saved and the log file",
             std::string("MLP"));
    addParam("treeName", m_treeName, "Tree name used in output file, default is prefix + '_tree'", std::string(""));

    addParam("workingDirectory", m_workingDirectory,
             "Working directory in which the config file and the weight file directory is created", std::string("."));

    addParam("sample", m_sample,
             "Variable used for inverse sampling rates. Usually this is the same as the target", std::string(""));
    std::map<int, unsigned int> defaultInverseSamplingRates;
    addParam("inverseSamplingRates", m_inverseSamplingRates, "Map of class id and inverse sampling rate for this class.",
             defaultInverseSamplingRates);

    addParam("methods", m_methods,
             "Vector of Tuples with (Name, Type, Config) of the methods. Valid types are: BDT, KNN, Fisher, Plugin. For type 'Plugin', the plugin matching the Name attribute will be loaded (e.g. NeuroBayes). The Config is passed to the FANN Method and is documented in the FANN UserGuide.");

    addParam("target", m_targetName,
             "Target used by the Multi-Layer Perceptron, it does not need to be an integer-valued variable.", std::string(""));

    m_neuronTypes["FANN_LINEAR"] = FANN_LINEAR;
    m_neuronTypes["FANN_THRESHOLD"] =  FANN_THRESHOLD;
    m_neuronTypes["FANN_THRESHOLD_SYMMETRIC"] =  FANN_THRESHOLD_SYMMETRIC;
    m_neuronTypes["FANN_SIGMOID"] =  FANN_SIGMOID;
    m_neuronTypes["FANN_SIGMOID_STEPWISE"] =  FANN_SIGMOID_STEPWISE;
    m_neuronTypes["FANN_SIGMOID_SYMMETRIC"] =  FANN_SIGMOID_SYMMETRIC;
    m_neuronTypes["FANN_SIGMOID_SYMMETRIC_STEPWISE"] =  FANN_SIGMOID_SYMMETRIC_STEPWISE ;
    m_neuronTypes["FANN_GAUSSIAN"] =  FANN_GAUSSIAN;
    m_neuronTypes["FANN_GAUSSIAN_SYMMETRIC"] =  FANN_GAUSSIAN_SYMMETRIC;
    m_neuronTypes["FANN_ELLIOT"] =  FANN_ELLIOT;
    m_neuronTypes["FANN_ELLIOT_SYMMETRIC"] =  FANN_ELLIOT_SYMMETRIC;
    m_neuronTypes["FANN_LINEAR_PIECE"] =  FANN_LINEAR_PIECE;
    m_neuronTypes["FANN_LINEAR_PIECE_SYMMETRIC"] =  FANN_LINEAR_PIECE_SYMMETRIC;
    m_neuronTypes["FANN_SIN_SYMMETRIC"] =  FANN_SIN_SYMMETRIC;
    m_neuronTypes["FANN_COS_SYMMETRIC"] =  FANN_COS_SYMMETRIC;
    m_neuronTypes["FANN_SIN"] =  FANN_SIN;
    m_neuronTypes["FANN_COS"] =  FANN_COS;
    m_trainingMethods["FANN_TRAIN_INCREMENTAL"] = FANN_TRAIN_INCREMENTAL;
    m_trainingMethods["FANN_TRAIN_BATCH"]      = FANN_TRAIN_BATCH;
    m_trainingMethods["FANN_TRAIN_RPROP"]      = FANN_TRAIN_RPROP;
    m_trainingMethods["FANN_TRAIN_QUICKPROP"]  = FANN_TRAIN_QUICKPROP;
    m_trainingMethods["FANN_TRAIN_SARPROP"]    = FANN_TRAIN_SARPROP;
    m_trainingMethodsOPENMP["FANN_TRAIN_RPROP"] = parallel_fann::train_epoch_irpropm_parallel;
    m_trainingMethodsOPENMP["FANN_TRAIN_BATCH"]      = parallel_fann::train_epoch_batch_parallel;
    m_trainingMethodsOPENMP["FANN_TRAIN_QUICKPROP"] = parallel_fann::train_epoch_quickprop_parallel;
    m_trainingMethodsOPENMP["FANN_TRAIN_SARPROP"]  = parallel_fann::train_epoch_sarprop_parallel;

  }

  void FANNTeacherModule::initialize()
  {

    Variable::Manager& manager = Variable::Manager::Instance();
    if (m_targetName == "") {
      B2FATAL("No target given: A target Variable is needed for Training an MLP");

    } else {
      m_target =  manager.getVariable(m_targetName);
      B2INFO("FANN Teacher is going to train a Multi-Layer Percetron at the termination.");
    }

    for (auto& name : m_listNames) {
      StoreObjPtr<ParticleList>::required(name);
    }

    if (m_treeName == "") {
      m_treeName = m_prefix + "_tree";
    }

    if (m_variableNames.size() != 0) {
      m_variables =  manager.getVariables(m_variableNames);
      if (m_variables.size() != m_variableNames.size()) {
        B2ERROR("One or more feature variables could not be loaded via the Variable::Manager. Check the names!");
      }
    } else {
      B2ERROR("No input variables given!");
    }

    Double_t nInput = m_variables.size();//Number of Neurons in the input layer.

    if (m_workingDirectory.back() != '/') {
      m_workingDirectory += '/';
    }
    m_logFilename = m_prefix + std::string("FANNlogFile");
    m_trainFilename = m_prefix + std::string("FANNdataFile.root");
    m_filename = m_prefix + std::string("FANNweightFile.root");
    m_trainArrayname = std::string("FANNtrainSamples");

    m_MLPs.clear();

//  Set Parameters for each given method and initialize MLPs.
    for (auto& tuple : m_methods) {

      // Default Values
      std::vector<unsigned short> nNodes;
      nNodes.push_back(m_variables.size());
      int nCycles = 10000; // Maximum number of training epochs.
      std::string neuronType = "FANN_SIGMOID_SYMMETRIC"; // Type of activation function.
      std::string trainingMethod = "FANN_TRAIN_RPROP"; // Type of activation function.
      float validationFraction =
        0.5; //Training is stopped if validation error is higher than checkInterval epochs ago, i.e. either the validation error is increasing or the gain is less than the fluctuations.
      int randomSeeds = 1; //Number of training runs with different random start weights.
      int testRate = 500;
      int nThreads = 1; //Number of threads for training
      bool epochMonitoring = true;
      std::string arrayname = std::get<0>(tuple);
      std::stringstream iMethod(std::get<2>(tuple));
      std::string parameter;

      while (std::getline(iMethod, parameter, ':')) {

        std::stringstream iParameter(parameter);
        std::string segments;
        std::vector<std::string> iParameterAndValue;
        while (std::getline(iParameter, segments, '=')) {
          iParameterAndValue.push_back(segments);
        }

        //If the parsed parameter string have the form variable=value each variable will be identified and the default value replaced
        if (iParameterAndValue.size() == 2) {
          if (iParameterAndValue[0] == "HiddenLayers") {
            std::stringstream iLayers(iParameterAndValue[1]);
            std::string layer;
            while (std::getline(iLayers, layer, ',')) {
              for (auto& character : layer) {
                if (character == 'N') character = 'x';
              }
              TFormula* iLayerSize = new TFormula("iLayerSize", layer.c_str());
              nNodes.push_back(iLayerSize->Eval(nInput));
            }
          } else if (iParameterAndValue[0] == "NCycles") {
            nCycles = atoi(iParameterAndValue[1].c_str());
            if (nCycles < 1) B2ERROR("Invalid number of cycles: NCycles =" << iParameterAndValue[1]);
          } else if (iParameterAndValue[0] == "NeuronType") {
            if (m_neuronTypes.find(iParameterAndValue[1]) == m_neuronTypes.end()) {
              B2ERROR("Wrong NeuronType: " << iParameterAndValue[1]);
            } else neuronType = iParameterAndValue[1];
          } else if (iParameterAndValue[0] == "TrainingMethod") {
            if (m_trainingMethods.find(iParameterAndValue[1]) == m_trainingMethods.end()) {
              B2ERROR("Wrong TrainingMethod: " << iParameterAndValue[1]);
            } else trainingMethod = iParameterAndValue[1];
          }   else if (iParameterAndValue[0] == "ValidationFraction") {
            validationFraction = atof(iParameterAndValue[1].c_str());
            if (validationFraction < 0.1
                || validationFraction > 0.8) B2ERROR("Invalid number of ValidationFraction: ValidationFraction =" << iParameterAndValue[1] <<
                                                       ". Please replace by a float number between 0.1 and 0.8");
          } else if (iParameterAndValue[0] == "RandomSeeds") {
            randomSeeds = atoi(iParameterAndValue[1].c_str());
            if (randomSeeds < 1) B2ERROR("Invalid number of RandomSeeds: RandomSeeds =" << iParameterAndValue[1]);
          } else if (iParameterAndValue[0] == "TestRate") {
            testRate = atoi(iParameterAndValue[1].c_str());
            if (testRate < 1) B2ERROR("Invalid number of TestRate: TestRate =" << iParameterAndValue[1]);
          } else if (iParameterAndValue[0] == "NThreads") {
            nThreads = atoi(iParameterAndValue[1].c_str());
            if (nThreads < 1) B2ERROR("Invalid number of threads: NThreads =" << iParameterAndValue[1]);
          } else if (iParameterAndValue[0] == "EpochMonitoring") {
            if (iParameterAndValue[1] == "True") epochMonitoring = true;
            else if (iParameterAndValue[1] == "False") epochMonitoring = false;
            else B2ERROR("EpochMonitoring must be True or False! EpochMonitoring =" << iParameterAndValue[1]);
          } else {
            B2ERROR(" Parameter" << iParameterAndValue[0] << "could not be identified. Please check the name!");
          }
        }
      }
      nNodes.push_back(1);
//          create new MLP and Datasets
//     loadTraindata(m_trainFilename)
      if (!load(m_filename, arrayname)) {
        m_MLPs.push_back(FANNMLP(arrayname, m_variableNames, m_targetName, nNodes, neuronType, trainingMethod, nCycles,
                                 validationFraction, randomSeeds, testRate, nThreads, epochMonitoring));
      } else B2ERROR("FANN Method " << arrayname << " already trained in File " << m_filename << ".")
      }
  }

  void FANNTeacherModule::event()
  {
    for (auto& listName : m_listNames) {
      StoreObjPtr<ParticleList> list(listName);
      // Calculate Signal Probability for Particles

      for (unsigned i = 0; i < list->getListSize(); ++i) {
        const Particle* particle = list->getParticle(i);
        addSampleToSet(particle);
      }
    }

    if (m_listNames.empty()) {
      addSampleToSet(nullptr);
    }
  }


  void FANNTeacherModule::terminate()
  {
    // do training for all sectors with sufficient training samples
    for (unsigned isector = 0; isector < m_MLPs.size(); ++isector) {
      float nTestAndValid = m_trainSet.nSamples() * m_MLPs[isector].getValidationFraction();
      int nTest = int(nTestAndValid * 0.5); // Number of events in the test sample.
      int nValid = int(nTestAndValid * 0.5);
      float nTrainMin = 10 * m_MLPs[isector].nWeights();
      if (m_trainSet.nSamples() < (nTrainMin + nValid + nTest)) {
        B2WARNING("Not enough training samples for sector " << isector << " (" << (nTrainMin + nValid + nTest)
                  << ") found, requested " << m_trainSet.nSamples() << " events.");
        continue;
      }

      m_MLPs[isector].setMinAndMaxInputSample(m_trainSet.getMinAndMaxInputSample());
      m_MLPs[isector].setMinAndMaxTargetSample(m_trainSet.getMinAndMaxTargetSample());

      for (unsigned var = 0; var < m_variables.size(); ++var) {
        if (m_MLPs[isector].getMinAndMaxInputSample()[0][var] == m_MLPs[isector].getMinAndMaxInputSample()[1][var]) {
          B2WARNING("The variable " << m_variableNames[var] << " is always equal to " << m_MLPs[isector].getMinAndMaxInputSample()[1][var] <<
                    ". It will be scaled to -1 but should be better removed.");
        }
      }

      if (m_MLPs[isector].getMinAndMaxTargetSample()[0][0] == m_MLPs[isector].getMinAndMaxTargetSample()[1][0]) {
        B2ERROR("The target variable " << m_targetName << " is always equals to " << m_MLPs[isector].getMinAndMaxTargetSample()[1][0] <<
                ". Nothing to be trained.");
      }

      train(isector);
      // save everything to file
      save(m_filename, m_MLPs[isector].getArrayName());
    }

    saveTraindata(m_trainFilename);
  }


  void FANNTeacherModule::train(unsigned isector)
  {
#ifdef HAS_OPENMP
    B2INFO("Training a Multi-Layer Perceptron for Particle List with OpenMP");
#else
    B2INFO("Training a Multi-Layer Perceptron for Particle List without OpenMP");
#endif

    // initialize network

    unsigned num_input = m_variables.size(); //Number of variables.
    unsigned nEvents = m_trainSet.nSamples(); //Number of events.

    unsigned nLayers = m_MLPs[isector].nLayers(); //The total number of layers including the input and the output layer.
    unsigned num_output = 1; //Number of outputs per MLP.

    float nTestAndValid = m_trainSet.nSamples() * m_MLPs[isector].getValidationFraction();
    int nTest = int(nTestAndValid * 0.5); // Number of events in the test sample.
    int nValid = int(nTestAndValid * 0.5);


    unsigned nTrain = nEvents - nValid - nTest;
    unsigned* nNodes = new unsigned[nLayers];

    for (unsigned il = 0; il < nLayers; ++il) {
      nNodes[il] = m_MLPs[isector].nNodesLayer(il);
    }

    unsigned nWeights = m_MLPs[isector].nWeights();

    struct fann* ann = fann_create_standard_array(nLayers, nNodes);
    // initialize training and validation data
    FANNMLPData currentData = m_trainSet;
    // train set

    struct fann_train_data* train_data =
      fann_create_train(nTrain, num_input, num_output);
    for (unsigned i = 0; i < nTrain; ++i) {
      std::vector<float> input = m_MLPs[isector].scaleInput(currentData.getInput(i)); //here
      for (unsigned j = 0; j < input.size(); ++j) {
        train_data->input[i][j] = input[j];
      }
      std::vector<float> target = m_MLPs[isector].scaleTarget(currentData.getTarget(i)); //here
      for (unsigned j = 0; j < target.size(); ++j) {
        train_data->output[i][j] = target[j];
      }
    }
    // validation set
    struct fann_train_data* valid_data =
      fann_create_train(nValid, num_input, num_output);
    for (unsigned i = nTrain; i < nTrain + nValid; ++i) {
      std::vector<float> input = m_MLPs[isector].scaleInput(currentData.getInput(i)); //here
      for (unsigned j = 0; j < input.size(); ++j) {
        valid_data->input[i - nTrain][j] = input[j];
      }
      std::vector<float> target = m_MLPs[isector].scaleTarget(currentData.getTarget(i)); //here
      for (unsigned j = 0; j < target.size(); ++j) {
        valid_data->output[i - nTrain][j] = target[j];
      }
    }
    // set network parameters
    fann_set_activation_function_hidden(ann, m_neuronTypes[m_MLPs[isector].getNeuronType()]);
    fann_set_activation_function_output(ann, m_neuronTypes[m_MLPs[isector].getNeuronType()]);
    double bestRMS = 999.;
    std::vector<double> bestTrainLog = {};
    std::vector<double> bestValidLog = {};
    // repeat training several times with different random start weights
    for (int irun = 0; irun < m_MLPs[isector].getRandomSeeds(); ++irun) {
      double bestValid = 999.;
      std::vector<double> trainLog = {};
      std::vector<double> validLog = {};
      trainLog.assign(m_MLPs[isector].getNCycles(), 0.);
      validLog.assign(m_MLPs[isector].getNCycles(), 0.);
      int breakEpoch = 0;
      std::vector<fann_type> bestWeights = {};
      bestWeights.assign(nWeights, 0.); //here
      fann_randomize_weights(ann, -0.1, 0.1);

      // train and save the network
      for (int epoch = 1; epoch <= m_MLPs[isector].getNCycles(); ++epoch) {
        double mse;
#ifdef HAS_OPENMP
        if (m_MLPs[isector].getTrainingMethod() != "FANN_TRAIN_INCREMENTAL") {
          mse = m_trainingMethodsOPENMP[m_MLPs[isector].getTrainingMethod()](ann, train_data, m_MLPs[isector].getNThreads());
        } else {mse = parallel_fann::train_epoch_incremental_mod(ann, train_data);}
#else
        fann_set_training_algorithm(ann, m_trainingMethods[m_MLPs[isector].getTrainingMethod()]);
        mse = fann_train_epoch(ann, train_data);
#endif
        trainLog[epoch - 1] = mse;
        // evaluate validation set
        fann_reset_MSE(ann);
#ifdef HAS_OPENMP
        double valid_mse = parallel_fann::test_data_parallel(ann, valid_data, m_MLPs[isector].getNThreads());
#else
        double valid_mse = fann_test_data(ann, valid_data);
#endif
        validLog[epoch - 1] = valid_mse;
        // keep weights for lowest validation error
        if (valid_mse < bestValid) {
          bestValid = valid_mse;
          for (unsigned iw = 0; iw < ann->total_connections; ++iw) {
            bestWeights[iw] = ann->weights[iw];
          }
        }
        // break when validation error increases
        if (epoch > m_MLPs[isector].getTestRate() && valid_mse > validLog[epoch - m_MLPs[isector].getTestRate()]) {
          B2INFO("Training stopped in epoch " << epoch);
          B2INFO("Train error: " << mse << ", valid error: " << valid_mse <<
                 ", best valid: " << bestValid);
          breakEpoch = epoch;
          break;
        }
        // print current status
        if (epoch == 1 || (epoch < 100 && epoch % 10 == 0) || epoch % 100 == 0) {
          B2INFO("Epoch " << epoch << ": Train error = " << mse <<
                 ", valid error = " << valid_mse << ", best valid = " << bestValid);
        }
      }
      // test trained network
      std::vector<float> oldWeights = m_MLPs[isector].getWeights();
      m_MLPs[isector].setWeights(bestWeights);
      std::vector<double> sumSqr;
      sumSqr.assign(num_output, 0.);
      for (unsigned i = nTrain + nValid; i < m_trainSet.nSamples(); ++i) {
        std::vector<float> output = m_MLPs[isector].runMLP(m_trainSet.getInput(i));
        std::vector<float> target = m_trainSet.getTarget(i);
        for (unsigned iout = 0; iout < output.size(); ++iout) {
          float diff = output[iout] - target[iout];
          sumSqr[iout] += diff * diff;
        }
      }
      double sumSqrTotal = 0;

      sumSqrTotal += sumSqr[0]; //here
      B2INFO("RMS Output: " << sqrt(sumSqr[0] / nTest));
      double RMS = sqrt(sumSqrTotal / nTest / sumSqr.size());
      B2INFO("RMS on test samples: " << RMS << " (best: " << bestRMS << ")");
      if (RMS < bestRMS) {
        bestRMS = RMS;
        bestTrainLog.assign(trainLog.begin(), trainLog.begin() + breakEpoch);
        bestValidLog.assign(validLog.begin(), validLog.begin() + breakEpoch);
      } else {
        m_MLPs[isector].setWeights(oldWeights); //here
      }
    }
    // save training log
    std::ofstream logstream(m_logFilename + ".log");//+ "_" + to_string(isector) + ".log");
    for (unsigned i = 0; i < bestTrainLog.size(); ++i) {
      logstream << bestTrainLog[i] << " " << bestValidLog[i] << std::endl;
    }
    logstream.close();
    // free memory
    fann_destroy_train(train_data);
    fann_destroy_train(valid_data);
    fann_destroy(ann);
    delete[] nNodes;
  }

  void FANNTeacherModule::save(const std::string& filename, const std::string& arrayname)
  {
    WorkingDirectoryManager dummy(m_workingDirectory);
    B2INFO("Saving networks to file " << filename << ", array " << arrayname);
    TFile datafile(filename.c_str(), "UPDATE");
    TObjArray* MLPs = new TObjArray(m_MLPs.size());
    for (unsigned isector = 0; isector < m_MLPs.size(); ++isector) {
      MLPs->Add(&m_MLPs[isector]);
    }
    MLPs->Write(arrayname.c_str(), TObject::kSingleKey | TObject::kOverwrite);
    datafile.Close();
    MLPs->Clear();
    delete MLPs;
  }

  bool FANNTeacherModule::load(const std::string& filename, const std::string& arrayname)
  {
    TFile datafile(filename.c_str(), "READ");
    if (!datafile.IsOpen()) {
      B2WARNING("Could not open file " << filename);
      return false;
    }
    TObjArray* MLPs = (TObjArray*)datafile.Get(arrayname.c_str());
    if (!MLPs) {
      datafile.Close();
      B2WARNING("File " << filename << " does not contain key " << arrayname);
      return false;
    }
    m_MLPs.clear();
    for (int isector = 0; isector < MLPs->GetEntriesFast(); ++isector) {
      FANNMLP* expert = dynamic_cast<FANNMLP*>(MLPs->At(isector));
      if (expert) m_MLPs.push_back(*expert);
      else B2WARNING("Wrong type " << MLPs->At(isector)->ClassName() << ", ignoring this entry.");
    }
    MLPs->Clear();
    delete MLPs;
    datafile.Close();
    B2DEBUG(100, "loaded " << m_MLPs.size() << " networks");

    return true;
  }

  void FANNTeacherModule::saveTraindata(const std::string& filename)
  {
    WorkingDirectoryManager dummy(m_workingDirectory);
    B2INFO("Saving traindata to file " << filename << ".");

    std::vector<float> input;
    input.resize(m_variableNames.size() + 1, 0);

    TFile* datafile = TFile::Open(filename.c_str(), "UPDATE");
    datafile->cd();

    // Search for an existing tree in the file
    TTree* tree = new TTree(m_treeName.c_str(), m_treeName.c_str());

    for (unsigned int i = 0; i < m_variableNames.size(); ++i)
      tree->Branch(Variable::makeROOTCompatible(m_variableNames[i]).c_str(), &input[i]);
    tree->Branch(Variable::makeROOTCompatible(m_targetName).c_str(), &input[m_variableNames.size() + 1]);

    for (unsigned int i = 0; i < m_variableNames.size(); ++i)
      tree -> SetBranchAddress(Variable::makeROOTCompatible(m_variableNames[i]).c_str(), &input[i]);
    tree -> SetBranchAddress(Variable::makeROOTCompatible(m_targetName).c_str(), &input[m_variableNames.size() + 1]);

    for (unsigned event = 0; event < m_trainSet.nSamples(); ++event) {
      // Fill the tree with the input variables
      for (unsigned int i = 0; i < m_variableNames.size(); ++i) {
        input[i] = m_trainSet.getInput(event)[i];
      }
      input[m_variableNames.size() + 1] = m_trainSet.getTarget(event)[0];
      tree->Fill();
    }
    tree->Write("", TObject::kOverwrite);

    const bool writeError = datafile->TestBit(TFile::kWriteError);

    if (writeError) {
      //m_file deleted first so we have a chance of closing it (though that will probably fail)
      delete datafile;
      B2FATAL("A write error occured while saving '" << filename << "', please check if enough disk space is available.");
    }

    datafile->Close();

  }

  bool FANNTeacherModule::loadTraindata(const std::string& filename)
  {
    TFile* datafile = TFile::Open(filename.c_str(), "READ");
    if (!datafile->IsOpen()) {
      B2WARNING("Could not open file " << filename);
      return false;
    }
    datafile->cd();

    std::vector<float> data;
    data.resize(m_variableNames.size() + 1, 0);
    TTree* tree = nullptr;

    datafile->GetObject(m_treeName.c_str(), tree);

    if (not tree) {
      B2INFO("Did not found a tree named " << m_treeName << " searching for another tree.");
      TIter next(datafile->GetListOfKeys());
      while (TObject* obj = next()) {
        if (std::string(obj->GetName()).find("_tree") != std::string::npos) {
          B2INFO("Found tree with name " << obj->GetName());
          datafile->GetObject(obj->GetName(), tree);
          break;
        }
      }
    }

    if (tree) {
      unsigned int nBranches = tree->GetNbranches();

      if ((m_variableNames.size() + 1) !=  nBranches)
        B2ERROR("Data File does not contain the number of needed variables, i.e. Branches.");

      for (unsigned int i = 0; i < m_variableNames.size(); ++i)
        tree -> SetBranchAddress(Variable::makeROOTCompatible(m_variableNames[i]).c_str(), &data[i]);
      tree -> SetBranchAddress(Variable::makeROOTCompatible(m_targetName).c_str(), &data[m_variableNames.size() + 1]);

      for (unsigned int event = 0; event < tree -> GetEntries(); ++event) {
        tree->GetEntry(event);

        std::vector<float> input;
        input.resize(m_variableNames.size(), 0);
        std::vector<float> targetraw;
        targetraw.resize(1, 0);

        for (unsigned int i = 0; i < m_variableNames.size(); ++i) {
          input[i] = data[i];
        }
        targetraw[0] = data[m_variableNames.size() + 1];

        m_trainSet.addSample(input, targetraw);
      }


    } else {
      B2WARNING("Couldn't find existing data, create new tree: Filename was " << filename << ", Treename was " <<
                m_treeName);
    }

    datafile-> Close();
    B2DEBUG(100, "loaded training set from file" << filename);
    return true;
  }

  std::vector<const Variable::Manager::Var*> FANNTeacherModule::getVariablesFromManager() const
  {
    Variable::Manager& manager = Variable::Manager::Instance();
    const auto& vars =  manager.getVariables(m_variableNames);
    if (vars.size() != m_variableNames.size()) {
      B2ERROR("One or more feature variables could not be loaded via the Variable::Manager. Check the names!");
    }
    return vars;
  }

  void FANNTeacherModule::addSampleToSet(const Particle* particle)
  {
    std::vector<float> input;
    input.resize(m_variableNames.size(), 0);
    std::vector<float> targetraw;
    targetraw.resize(1, 0);
    targetraw[0] = m_target ->function(particle);

    bool addToSet = true;
    if (!std::isfinite(targetraw[0])) {
      B2ERROR("Output of variable " << m_target->name << " is " << targetraw[0] << ", please fix it. Candidate will be skipped.");
      addToSet = false;
      return;
    }
    for (unsigned int i = 0; i < m_variables.size(); ++i) {
      if (m_variables[i] != nullptr)
        input[i] = m_variables[i]->function(particle);
      if (!std::isfinite(input[i])) {
        B2ERROR("Output of variable " << m_variables[i]->name << " is " << input[i] << ", please fix it. Candidate will be skipped.");
        addToSet = false;
        return;
      }
    }

    if (addToSet) {
      m_trainSet.addSample(input, targetraw);
    }
  }


} // Belle2 namespace

