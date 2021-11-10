#include <trg/cdc/dataobjects/CDCTriggerMLPData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <TObjArray.h>
#include <TFile.h>
#include <framework/database/DBObjPtr.h>
#include <fann.h>
#include <parallel_fann.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <trg/cdc/NeuroTrigger.h>
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/multi_array.hpp"
#include <cdc/geometry/CDCGeometryPar.h>

using namespace Belle2;
template<unsigned inodes, unsigned onodes>
bool loadData(const std::string& filename, std::vector<CDCTriggerMLPData>& dataset, unsigned maxdata)
{
  unsigned events1[5] = {0, 0, 0, 0, 0};
  std::ifstream gzipfile1(filename, std::ios_base::in | std::ios_base::binary);
  boost::iostreams::filtering_istream arrayStream1;
  arrayStream1.push(boost::iostreams::gzip_decompressor());
  arrayStream1.push(gzipfile1);
  CDCTriggerMLPData::NeuroSet<inodes, onodes> dataline1;
  if (gzipfile1.is_open()) {
    while (arrayStream1 >> dataline1) {
      events1[dataline1.expert]++;
    }

    gzipfile1.close();
  }
  unsigned events[5] = {0, 0, 0, 0, 0};
  std::ifstream gzipfile(filename, std::ios_base::in | std::ios_base::binary);
  boost::iostreams::filtering_istream arrayStream;
  arrayStream.push(boost::iostreams::gzip_decompressor());
  arrayStream.push(gzipfile);
  CDCTriggerMLPData::NeuroSet<inodes, onodes> dataline;
  int tev = (int) events1[dataline.expert] / maxdata;
  if (gzipfile.is_open()) {
    while (arrayStream >> dataline) {
      if (dataset[dataline.expert].nSamples() <= maxdata && events[dataline.expert] % tev == 0) {
        dataset[dataline.expert].addSample(dataline);
      }
      events[dataline.expert]++;
    }
    gzipfile.close();
    return true;
  } else {
    std::cout << "Could not open file: " << filename << std::endl;
    return false;
  }
  std::cout << "Loaded " << dataset.size() << " datasamples from " << filename << std::endl;
}
void train(NeuroTrigger& nt, unsigned iexp, unsigned maxEpochs, unsigned repeat, double wMax, unsigned checkInterval,
           unsigned allow_peaks, unsigned train_algo, float initWeightsMin, float initWeightsMax,
           unsigned threads, std::vector<CDCTriggerMLPData>& dtrain, std::vector<CDCTriggerMLPData>& dvalid,
           std::vector<CDCTriggerMLPData>& dtest)
{
  std::string logFilename = "logoutput";
  // initialize network
  unsigned nLayers = nt[iexp].nLayers();
  unsigned* nNodes = new unsigned[nLayers];
  for (unsigned il = 0; il < nLayers; ++il) {
    nNodes[il] = nt[iexp].nNodesLayer(il);
  }
  struct fann* ann = fann_create_standard_array(nLayers, nNodes);
  // initialize training and validation data
  //CDCTriggerMLPData currentData = m_trainSets[isector];
  // train set
  unsigned nTrain = dtrain[iexp].nSamples();
  unsigned nValid = dvalid[iexp].nSamples();
  unsigned nTest = dtest[iexp].nSamples();
  std::cout << "train: " << nTrain << " , valid: " << nValid << " , test: " << nTest << std::endl;
  struct fann_train_data* train_data =
    fann_create_train(nTrain, nNodes[0], nNodes[nLayers - 1]);
  for (unsigned i = 0; i < nTrain; ++i) {
    std::vector<float> input = dtrain[iexp].getInput(i);
    for (unsigned j = 0; j < input.size(); ++j) {
      train_data->input[i][j] = input[j];
    }
    std::vector<float> target = dtrain[iexp].getTarget(i);
    for (unsigned j = 0; j < target.size(); ++j) {
      train_data->output[i][j] = target[j];
    }
  }
  // validation set
  struct fann_train_data* valid_data =
    fann_create_train(nValid, nNodes[0], nNodes[nLayers - 1]);
  for (int i = 0; i < nValid; ++i) {
    std::vector<float> input = dvalid[iexp].getInput(i);
    for (unsigned j = 0; j < input.size(); ++j) {
      valid_data->input[i][j] = input[j];
    }
    std::vector<float> target = dvalid[iexp].getTarget(i);
    for (unsigned j = 0; j < target.size(); ++j) {
      valid_data->output[i][j] = target[j];
    }
  }
  // set network parameters
  fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
  fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);
  if (train_algo == 0) {
    fann_set_training_algorithm(ann, FANN_TRAIN_RPROP);
  } else if (train_algo == 1) {
    fann_set_training_algorithm(ann, FANN_TRAIN_QUICKPROP);
  }
  double bestRMS = 999.;
  // keep full train error curve for best run
  std::vector<double> bestTrainLog = {};
  std::vector<double> bestValidLog = {};
  std::vector<std::vector<double> > allTrainLogs(repeat);
  std::vector<std::vector<double> > allValidLogs(repeat);
  // keep train error of optimum for all runs
  std::vector<double> trainOptLog = {};
  std::vector<double> validOptLog = {};
  // repeat training several times with different random start weights
  for (int irun = 0; irun < repeat; ++irun) {
    double bestValid = 999.;
    std::vector<double> trainLog = {};
    std::vector<double> validLog = {};
    trainLog.assign(maxEpochs, 0.);
    validLog.assign(maxEpochs, 0.);
    int breakEpoch = 0;
    int bestEpoch = 0;
    std::vector<fann_type> bestWeights = {};
    bestWeights.assign(nt[iexp].nWeights(), 0.);
    fann_randomize_weights(ann, -0.1, 0.1);
    fann_randomize_weights(ann, initWeightsMin, initWeightsMax);
    // train and save the network
    // number of invalid stop conditions to allow for validation peaks
    unsigned peakex = 0;
    for (int epoch = 1; epoch <= maxEpochs; ++epoch) {
#ifdef HAS_OPENMP
      double mse = parallel_fann::train_epoch_irpropm_parallel(ann, train_data, threads);
#else
      double mse = fann_train_epoch(ann, train_data);
#endif
      trainLog[epoch - 1] = mse;
      // reduce weights that got too large
      for (unsigned iw = 0; iw < ann->total_connections; ++iw) {
        if (ann->weights[iw] > wMax)
          ann->weights[iw] = wMax;
        else if (ann->weights[iw] < -wMax)
          ann->weights[iw] = -wMax;
      }
      // evaluate validation set
      fann_reset_MSE(ann);
#ifdef HAS_OPENMP
      double valid_mse = parallel_fann::test_data_parallel(ann, valid_data, threads);
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
        bestEpoch = epoch;
      }
      // break when validation error increases

      if (epoch > checkInterval && valid_mse > validLog[epoch - checkInterval]) {
        if (allow_peaks > peakex) {
          peakex++;
        } else {
          B2INFO("Training run " << irun << " stopped in epoch " << epoch);
          B2INFO("Train error: " << mse << ", valid error: " << valid_mse <<
                 ", best valid: " << bestValid);
          breakEpoch = epoch;
          break;
        }
      } else {
        peakex = 0;
      }
      // print current status
      if (epoch == 1 || (epoch < 100 && epoch % 10 == 0) || epoch % 100 == 0) {
        std::cout << "Epoch " << epoch << ": Train error = " << mse <<
                  ", valid error = " << valid_mse << ", best valid = " << bestValid << std::endl;
      }
    }
    if (breakEpoch == 0) {
      std::cout << "Training run " << irun << " finished in epoch " << maxEpochs << std::endl;
      breakEpoch = maxEpochs;
    }
    trainOptLog.push_back(trainLog[bestEpoch - 1]);
    validOptLog.push_back(validLog[bestEpoch - 1]);
    // test trained network
    std::vector<float> oldWeights = nt[iexp].getWeights();
    nt[iexp].setWeights(bestWeights);
    std::vector<double> sumSqr;
    std::cout << "nsamples: " << dtest[iexp].nSamples() << std::endl;
    sumSqr.assign(nNodes[nLayers - 1], 0.);
    for (unsigned i = 0; i < dtest[iexp].nSamples(); ++i) {
      std::vector<float> output = nt.runMLP(iexp, dtest[iexp].getInput(i));
      std::vector<float> target = dtest[iexp].getTarget(i);
      for (unsigned iout = 0; iout < output.size(); ++iout) {
        float diff = output[iout] - nt[iexp].unscaleTarget(target)[iout];
        if (iout == 0) {
          //std::cout << "network: " << output[iout] << " , target: " << target[iout] << " , diff: " << diff << std::endl;
        }
        sumSqr[iout] += diff * diff;
      }
    }
    double sumSqrTotal = 0;
    std::cout << "sumsqr: " << sumSqr[0] << "zindex: " << sumSqr[nt[iexp].zIndex()] << std::endl;
    sumSqrTotal += sumSqr[nt[iexp].zIndex()];
    B2INFO("RMS z: " << sqrt(sumSqr[nt[iexp].zIndex()] / nTest) << "cm");
    sumSqr[nt[iexp].thetaIndex()] /= (Unit::deg * Unit::deg);
    sumSqrTotal += sumSqr[nt[iexp].thetaIndex()];
    B2INFO("RMS theta: " << sqrt(sumSqr[nt[iexp].thetaIndex()] / nTest) << "deg");
    double RMS = sqrt(sumSqrTotal / nTest / sumSqr.size());
    if (RMS < bestRMS) {
      bestRMS = RMS;
      bestTrainLog.assign(trainLog.begin(), trainLog.begin() + breakEpoch);
      bestValidLog.assign(validLog.begin(), validLog.begin() + breakEpoch);
    } else {
      nt[iexp].setWeights(oldWeights);
    }
    B2INFO("RMS on test samples: " << RMS << " (best: " << bestRMS << ")");
    allTrainLogs[irun].assign(trainLog.begin(), trainLog.begin() + breakEpoch);
    allValidLogs[irun].assign(validLog.begin(), validLog.begin() + breakEpoch);
  }
  // save training log
  // full error curve for best run
  std::ofstream logstreamb(logFilename + "_BestRun_" + std::to_string(iexp) + ".log");
  for (unsigned i = 0; i < bestTrainLog.size(); ++i) {
    logstreamb << bestTrainLog[i] << " " << bestValidLog[i] << std::endl;
  }
  logstreamb.close();
  for (int jrun = 0; jrun < repeat; jrun ++) {
    std::ofstream logstream(logFilename + "_Sector_" + std::to_string(iexp) + "_RunNr_" + std::to_string(jrun) + ".log");
    for (unsigned i = 0; i < allTrainLogs[jrun].size(); ++i) {
      logstream << allTrainLogs[jrun][i] << " " << allValidLogs[jrun][i] << std::endl;
    }
    logstream.close();
  }
  // training optimum for all runs
  std::ofstream logstreamOpt(logFilename + "_AllOptima_" + std::to_string(iexp) + ".log");
  for (unsigned i = 0; i < trainOptLog.size(); ++i) {
    logstreamOpt << trainOptLog[i] << " " << validOptLog[i] << std::endl;
  }
  logstreamOpt.close();

  // free memory
  fann_destroy_train(train_data);
  fann_destroy_train(valid_data);
  fann_destroy(ann);
  delete[] nNodes;
}

//template<unsigned inodes, unsigned onodes>
//std::vector<CDCTriggerMLPData::NeuroSet<inodes, onodes>> loadData(const std::string& filename)
//{
//  std::vector<CDCTriggerMLPData::NeuroSet<inodes, onodes>> retVal;
//  std::ifstream gzipfile(filename, std::ios_base::in | std::ios_base::binary);
//  boost::iostreams::filtering_istream arrayStream;
//  arrayStream.push(boost::iostreams::gzip_decompressor());
//  arrayStream.push(gzipfile);
//  CDCTriggerMLPData::NeuroSet<inodes, onodes> dataline;
//  std::string dline;
//  if (gzipfile.is_open()) {
//    while (arrayStream >> dataline) {
//      retVal.push_back(dataline);
//    }
//    gzipfile.close();
//  } else {std::cout << "Could not open file: " << filename << std::endl;}
//  return retVal;
//}

void debug(int dblevel, std::string msg, int verbosity)
{
  if (dblevel <= verbosity) {
    std::cout << "DEBUG: " << msg << std::endl;
  }
}
/**
 * small input parser class to get cmdline options
 */
class InputParser {
public:
  InputParser(int& argc, char** argv)
  {
    for (int i = 1; i < argc; ++i)
      this->tokens.push_back(std::string(argv[i]));
  }
  const std::string& getCmdOption(const std::string& option) const
  {
    std::vector<std::string>::const_iterator itr;
    itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != this->tokens.end() && ++itr != this->tokens.end()) {
      return *itr;
    }
    static const std::string empty_string("");
    return empty_string;
  }
  bool cmdOptionExists(const std::string& option) const
  {
    return std::find(this->tokens.begin(), this->tokens.end(), option)
           != this->tokens.end();
  }
private:
  std::vector <std::string> tokens;
};


int main(int argc, char** argv)
{
  InputParser input(argc, argv);
  if (input.cmdOptionExists("-h") || input.cmdOptionExists("--help") || input.cmdOptionExists("?")) {
    std::cout << "TBD" << std::endl;
    return 0;
  }
  const std::string& trainfilename = input.getCmdOption("-trainfile");
  if (trainfilename.empty()) {
    std::cout << "Error! No trainfile given!" << std::endl;
  }
  const std::string& validfilename = input.getCmdOption("-validfile");
  if (validfilename.empty()) {
    std::cout << "Error! No validfile given!" << std::endl;
  }
  const std::string& testfilename = input.getCmdOption("-testfile");
  if (testfilename.empty()) {
    std::cout << "Error! No testfile given!" << std::endl;
  }
  const std::string& idhistfile = input.getCmdOption("-idhistfile");
  if (idhistfile.empty()) {
    std::cout << "Error! No idistfile given!" << std::endl;
    return -1;
  }
  const std::string& traindirname = input.getCmdOption("-traindir");
  if (traindirname.empty()) {
    std::cout << "Error! No traindir given!" << std::endl;
  }
  const std::string& testdirname = input.getCmdOption("-testdir");
  if (testdirname.empty()) {
    std::cout << "Error! No testdir given!" << std::endl;
  }
  const std::string& validdirname = input.getCmdOption("-validdir");
  if (validdirname.empty()) {
    std::cout << "Error! No validdir given!" << std::endl;
  }
  const std::string& confilename = input.getCmdOption("-conf");
  if (confilename.empty()) {
    std::cout << "Error! No confile given!" << std::endl;
    return -1;
  }

  //set network structure
  std::vector<CDCTriggerMLPData> traindata;
  std::vector<CDCTriggerMLPData> validdata;
  std::vector<CDCTriggerMLPData> testdata;
  CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();

  //set training parameters
  unsigned nLayers;
  unsigned nInputNodes;
  unsigned nOutputNodes;
  unsigned nHiddenNodes;
  double wMax;
  unsigned nTrain;
  unsigned nValid;
  unsigned nTest;
  unsigned checkInterval;
  unsigned maxEpochs;
  unsigned repeatTrain;
  float thetaTrainMin;
  float thetaTrainMax;
  float phiTrainMin;
  float phiTrainMax;
  float invPtTrainMin;
  float invPtTrainMax;
  bool rescaleTarget;
  unsigned nThreads;
  std::string et_option;
  int SLPatternMask;
  int SLPattern0;
  int SLPattern1;
  int SLPattern2;
  int SLPattern3;
  int SLPattern4;
  float outZMin;
  float outZMax;
  float outThetaMin;
  float outThetaMax;
  float invPtMin;
  float invPtMax;
  float phiMin;
  float phiMax;
  float thetaMin;
  float thetaMax;
  unsigned allow_peaks = 0;
  unsigned trainAlgo = 0;
  float initWeightsMin;
  float initWeightsMax;

  // now loading confile
  std::ifstream confile;
  try {
    confile.open(confilename, std::ifstream::in);
  } catch (int e) {
    std::cout << "ERROR! While opening file: " << confilename << "    Error code: " << e << std::endl;
    exit(EXIT_FAILURE);
  }
  std::string line_all;
  if (!confile.is_open()) {
    std::cout << "ERROR! While opening file: " << confilename << std::endl;
    exit(EXIT_FAILURE);
  }
  while (std::getline(confile, line_all)) {
    // remove comments
    std::size_t hashtag = line_all.find('#');
    std::string line = line_all.substr(0, hashtag);
    std::string par;
    std::string key;
    std::string skip;
    if (line.length() < 3) {
      continue;
      // check, if line wasnt a pure comment line
    }
    if (line.find('=') == std::string::npos) {
      continue;
    }
    par = line.substr(0, line.find('='));
    par.erase(std::remove(par.begin(), par.end(), ' '), par.end()); // remove whitespaces in whole string

    if (par == "nLayers") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      nLayers = std::stoul(key);
    }
    if (par == "nInputNodes") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      nInputNodes = std::stoul(key);

    }
    if (par == "nOutputNodes") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      nOutputNodes = std::stoul(key);

    }
    if (par == "nHiddenNodes") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      nHiddenNodes = std::stoul(key);

    }
    if (par == "wMax") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      wMax = std::stod(key);

    }
    if (par == "nTrain") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      nTrain = std::stoul(key);

    }
    if (par == "nValid") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      nValid = std::stoul(key);

    }
    if (par == "nTest") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      nTest = std::stoul(key);

    }
    if (par == "checkInterval") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      checkInterval = std::stoul(key);

    }
    if (par == "maxEpochs") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      maxEpochs = std::stoul(key);

    }
    if (par == "repeatTrain") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      repeatTrain = std::stoul(key);

    }
    if (par == "thetaTrainMin") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      thetaTrainMin = std::stof(key);

    }
    if (par == "thetaTrainMax") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      thetaTrainMax = std::stof(key);

    }
    if (par == "outZMin") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      outZMin = std::stof(key);

    }
    if (par == "outZMax") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      outZMax = std::stof(key);

    }
    if (par == "outThetaMin") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      outThetaMin = std::stof(key);

    }
    if (par == "outThetaMax") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      outThetaMax = std::stof(key);

    }
    if (par == "invPtTrainMin") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      invPtTrainMin = std::stof(key);

    }
    if (par == "invPtTrainMax") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      invPtTrainMax = std::stof(key);

    }
    if (par == "phiTrainMin") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      phiTrainMin = std::stof(key);

    }
    if (par == "phiTrainMax") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      phiTrainMax = std::stof(key);

    }
    if (par == "rescaleTarget") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      rescaleTarget = std::stoi(key);

    }
    if (par == "nThreads") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      nThreads = std::stoul(key);

    }
    if (par == "et_option") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      et_option = key;

    }
    if (par == "SLPatternMask") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      SLPatternMask = std::stoi(key);

    }
    if (par == "SLPattern0") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      SLPattern0 = std::stoi(key);

    }
    if (par == "SLPattern1") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      SLPattern1 = std::stoi(key);

    }
    if (par == "SLPattern2") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      SLPattern2 = std::stoi(key);

    }
    if (par == "SLPattern3") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      SLPattern3 = std::stoi(key);

    }
    if (par == "SLPattern4") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      SLPattern4 = std::stoi(key);

    }
    if (par == "invPtMin") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      invPtMin = std::stof(key);

    }
    if (par == "invPtMax") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      invPtMax = std::stof(key);

    }
    if (par == "phiMin") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      phiMin = std::stof(key);

    }
    if (par == "phiMax") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      phiMax = std::stof(key);

    }
    if (par == "thetaMin") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      thetaMin = std::stof(key);

    }
    if (par == "thetaMax") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      thetaMax = std::stof(key);

    }
    if (par == "allowPeakForEpochs") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      allow_peaks = std::stoul(key);

    }
    if (par == "trainingAlgorithm") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      trainAlgo = std::stoul(key);

    }
    if (par == "initWeightsMin") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      initWeightsMin = std::stof(key);

    }
    if (par == "initWeightsMax") {
      key = line.substr((line.find('"') + 1), (line.find('"', line.find('"') + 1) - 1 - line.find('"')));
      initWeightsMax = std::stof(key);

    }
  }




  unsigned nTotalWeights = (nInputNodes + 1) * nHiddenNodes + (nHiddenNodes + 1) * nOutputNodes; //+1: constant node
  NeuroTrigger::Parameters nntparams;
  NeuroTrigger nnt;
  // set params here
  nntparams.thetaRangeTrain = {{thetaTrainMin, thetaTrainMax}};
  nntparams.nMLP = 5;
  nntparams.outputScale = {{outZMin, outZMax, outThetaMin, outThetaMax}};
  nntparams.targetTheta = true;
  nntparams.targetZ = true;
  nntparams.multiplyHidden = false;
  nntparams.phiRange = {{phiMin, phiMax}};
  nntparams.invptRange =  {{invPtMin, invPtMax}};
  nntparams.thetaRange = {{thetaMin, thetaMax}};
  nntparams.phiRangeTrain = {{phiTrainMin, phiTrainMax}};
  nntparams.thetaRangeTrain = {{thetaTrainMin, thetaTrainMax}};
  nntparams.invptRangeTrain = {{invPtTrainMin, invPtTrainMax}};
  nntparams.maxHitsPerSL = {1};
  nntparams.SLpattern = {SLPattern0, SLPattern1, SLPattern2, SLPattern3, SLPattern4};
  nntparams.SLpatternMask = {SLPatternMask};
  nntparams.tMax = 256;
  nntparams.et_option = et_option;


  nnt.initialize(nntparams);
  nnt.loadIDHist(idhistfile);
  std::cout << "ready with loading idhist" << std::endl;
  int wiresinlayer[9] = {160, 160, 192, 224, 256, 288, 320, 352, 384};
  for (unsigned iMLP = 0; iMLP < nnt.nSectors(); ++iMLP) {
    traindata.push_back(CDCTriggerMLPData());
    validdata.push_back(CDCTriggerMLPData());
    testdata.push_back(CDCTriggerMLPData());
    int layerId = 3;
    for (int iSL = 0; iSL < 9; ++iSL) {
      std::cout << "wires in layer " << iSL << ": " << cdc.nWiresInLayer(layerId) << std::endl;
      std::cout << "wires in layer " << iSL << ": " << wiresinlayer[iSL] << std::endl;
      traindata[iMLP].addCounters(wiresinlayer[iSL]);
      validdata[iMLP].addCounters(wiresinlayer[iSL]);
      testdata[iMLP].addCounters(wiresinlayer[iSL]);
      layerId += (iSL > 0 ? 6 : 7);
    }
  }
  std::cout << "initialized datasets" << std::endl;



  //initialize network
  //struct fann* ann = fann_create_standard(nLayers, nInputNodes, nHiddenNodes, nOutputNodes);
  unsigned arr[3];
  arr[0] = nInputNodes;
  arr[1] = nHiddenNodes;
  arr[2] = nOutputNodes;
  struct fann* ann = fann_create_standard_array(nLayers, arr);

  //intialize trainset (CDCTriggerNeuroTrainerModule.cc: line 666)
//std::vector<CDCTriggerMLPData::NeuroSet<27, 2>> traindata = loadData<27, 2>(trainfilename);
  std::string traindir;
  std::string validdir;
  std::string testdir;
  traindir = traindirname; //"./traindir";
  validdir = validdirname; //"./validdir";
  testdir = testdirname; //"./testdir";
  unsigned count = 0;
  if (traindir.length() > 0) {
    for (const auto& entry : std::filesystem::directory_iterator(traindir)) {
      count++;
      std::cout << "loading file Nr. " << count << " :  " << entry.path() << std::endl;
      loadData<27, 2>(entry.path(), traindata, nTrain);
    }
  } else {
    std::cout << "loading single file :  " << trainfilename << std::endl;
    loadData<27, 2>(trainfilename, traindata, nTrain);
  }

  if (validdir.length() > 0) {
    for (const auto& entry : std::filesystem::directory_iterator(validdir)) {
      count++;
      std::cout << "loading file Nr. " << count << " :  " << entry.path() << std::endl;
      loadData<27, 2>(entry.path(), validdata, nValid);
    }
  } else {
    std::cout << "loading single file :  " << validfilename << std::endl;
    loadData<27, 2>(validfilename, validdata, nValid);
  }
  if (testdir.length() > 0) {
    for (const auto& entry : std::filesystem::directory_iterator(testdir)) {
      count++;
      std::cout << "loading file Nr. " << count << " :  " << entry.path() << std::endl;
      loadData<27, 2>(entry.path(), testdata, nTest);
    }
  } else {
    std::cout << "loading single file :  " << testfilename << std::endl;
    loadData<27, 2>(testfilename, testdata, nTest);
  }



  // loadData<27, 2>(trainfilename, traindata);
  // loadData<27, 2>(validfilename, validdata);
  // loadData<27, 2>(testfilename, testdata);

  for (unsigned i = 0; i < nnt.nSectors(); ++i) {
    //train(CDCTriggerMLPData& nt, unsigned iexp, unsigned maxEpochs, unsigned repeat, double wMax, unsigned checkInterval, unsigned threads, unsigned repeatTrain, std::vector<CDCTriggerMLPData>& dtrain, std::vector<CDCTriggerMLPData>& dvalid, std::vector<CDCTriggerMLPData>& dtest)
    train(nnt, i, maxEpochs, repeatTrain, wMax, checkInterval, allow_peaks, trainAlgo, initWeightsMin, initWeightsMax, nThreads,
          traindata, validdata, testdata);
  }
  nnt.save(confilename.substr(0, confilename.length() - 5) + ".root", "MLPs");
  return 0;


}
