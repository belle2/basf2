#include <analysis/dataobjects/FANNMLPData.h>
#include <Eigen/Dense>

using namespace Belle2;

std::vector<std::vector<float>> FANNMLPData::getMinAndMaxInputSample()
{

  std::vector<std::vector<float>> minAndMaxInputSample;

  Eigen::MatrixXf inputMatrix(nSamples(), getInput(0).size());
  for (unsigned i = 0; i < nSamples(); ++i) {
    inputMatrix.row(i) = Eigen::VectorXf::Map(&getInput(i)[0], getInput(i).size());
  }

  std::vector<float> minInputSample;
  for (unsigned j = 0; j < getInput(0).size(); ++j) {
    minInputSample.push_back(inputMatrix.col(j).minCoeff());
  }
  minAndMaxInputSample.push_back(minInputSample);

  std::vector<float> maxInputSample;
  for (unsigned j = 0; j < getInput(0).size(); ++j) {
    maxInputSample.push_back(inputMatrix.col(j).maxCoeff());
  }
  minAndMaxInputSample.push_back(maxInputSample);

  return minAndMaxInputSample;

};


std::vector<std::vector<float>> FANNMLPData::getMinAndMaxTargetSample()
{

  std::vector<std::vector<float>> minAndMaxTargetSample;

  Eigen::MatrixXf inputMatrix(nSamples(), getTarget(0).size());
  for (unsigned i = 0; i < nSamples(); ++i) {
    inputMatrix.row(i) = Eigen::VectorXf::Map(&getTarget(i)[0], getTarget(i).size());
  }

  std::vector<float> minTargetSample;
  for (unsigned j = 0; j < getTarget(0).size(); ++j) {
    minTargetSample.push_back(inputMatrix.col(j).minCoeff());
  }
  minAndMaxTargetSample.push_back(minTargetSample);

  std::vector<float> maxTargetSample;
  for (unsigned j = 0; j < getTarget(0).size(); ++j) {
    maxTargetSample.push_back(inputMatrix.col(j).maxCoeff());
  }
  minAndMaxTargetSample.push_back(maxTargetSample);

  return minAndMaxTargetSample;

};
