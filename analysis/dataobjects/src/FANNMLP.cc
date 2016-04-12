#include <analysis/dataobjects/FANNMLP.h>
#include <cmath>

using namespace Belle2;

FANNMLP::FANNMLP():
  arrayName(), variableNames(), targetName(), nNodes(), neuronType(),
  trainingMethod(),
  nCycles(), validationFraction(),  randomSeeds(),
  testRate(), nThreads(), epochMonitoring()
{
  weights.assign(nWeightsCal(), 0.);
}

FANNMLP::FANNMLP(std::string arrayName,
                 std::vector<std::string> variableNames,
                 std::string targetName,
                 std::vector<unsigned short> nNodes,
                 std::string neuronType,
                 std::string trainingMethod,
                 int nCycles,
                 float validationFraction,
                 int randomSeeds,
                 int testRate,
                 int nThreads,
                 bool epochMonitoring):
  arrayName(arrayName), variableNames(variableNames), targetName(targetName), nNodes(nNodes), neuronType(neuronType),
  trainingMethod(trainingMethod),
  nCycles(nCycles), validationFraction(validationFraction),  randomSeeds(randomSeeds),
  testRate(testRate), nThreads(nThreads), epochMonitoring(epochMonitoring)
{
  weights.assign(nWeightsCal(), 0.);
}

unsigned
FANNMLP::nWeightsCal() const
{
  unsigned nWeights = 0;
  if (nLayers() > 1) {
    nWeights = (nNodes[0] + 1) * nNodes[1];
    for (unsigned il = 1; il < nLayers() - 1; ++il) {
      nWeights += (nNodes[il] + 1) * nNodes[il + 1];
    }
  }
  return nWeights;
}

std::vector<float>
FANNMLP::scaleInput(std::vector<float> input) const
{
  std::vector<float> scaled;
  std::vector<std::vector<float>> scalingfactors = getMinAndMaxInputSample();
  scaled.assign(input.size(), 0.);
  for (unsigned i = 0; i < input.size(); ++i) {
    if (scalingfactors[1][i] != scalingfactors[0][i]) {
      if (input[i] < scalingfactors[0][i]) scaled[i] = 1;
      else if (input[i] > scalingfactors[1][i]) scaled[i] = -1;
      else scaled[i] = -1 * (2. * (input[i] - scalingfactors[0][i]) / (scalingfactors[1][i] - scalingfactors[0][i]) - 1.);
    } else {
      scaled[i] = -1;
    }
  }
  return scaled;
}

std::vector<float>
FANNMLP::unscaleInput(std::vector<float> input) const
{
  std::vector<float> unscaled;
  std::vector<std::vector<float>> descalingfactors = getMinAndMaxInputSample();
  unscaled.assign(input.size(), 0.);
  for (unsigned i = 0; i < input.size(); ++i) {
    if (descalingfactors[1][i] != descalingfactors[0][i]) {
      unscaled[i] = (-1 * input[i] + 1.) * (descalingfactors[1][i] - descalingfactors[0][i]) / 2. + descalingfactors[0][i];
    } else {
      unscaled[i] = descalingfactors[0][i];
    }
  }
  return unscaled;
}

std::vector<float>
FANNMLP::scaleTarget(std::vector<float> target) const
{
  std::vector<float> scaled;
  std::vector<std::vector<float>> scalingfactors = getMinAndMaxTargetSample();
  scaled.assign(target.size(), 0.);
  for (unsigned i = 0; i < target.size(); ++i) {
    scaled[i] = 2. * (target[i] - scalingfactors[0][i]) / (scalingfactors[1][i] - scalingfactors[0][i]) - 1.;
  }
  return scaled;
}

std::vector<float>
FANNMLP::unscaleTarget(std::vector<float> target) const
{
  std::vector<float> unscaled;
  std::vector<std::vector<float>> descalingfactors = getMinAndMaxTargetSample();
  unscaled.assign(target.size(), 0.);
  for (unsigned i = 0; i < target.size(); ++i) {
    unscaled[i] = (target[i] + 1.) * (descalingfactors[1][i] - descalingfactors[0][i]) / 2. + descalingfactors[0][i];
  }
  return unscaled;
}

std::vector<float> FANNMLP::runMLP(std::vector<float> input)
{
  std::vector<float> layerinput = scaleInput(input);
  std::vector<float> layeroutput = {};
  unsigned iw = 0;
  for (unsigned il = 1; il < nLayers(); ++il) {
    //add bias input
    layerinput.push_back(1.);
    //prepare output
    layeroutput.clear();
    layeroutput.assign(nNodesLayer(il), 0.);
    //loop over outputs
    for (unsigned io = 0; io < layeroutput.size(); ++io) {
      //loop over inputs
      for (unsigned ii = 0; ii < layerinput.size(); ++ii) {
        layeroutput[io] += layerinput[ii] * weights[iw++];
      }
      //apply activation function
      layeroutput[io] = tanh(layeroutput[io] / 2.);
    }
    //output is new input
    layerinput = layeroutput;
  }
  return unscaleTarget(layeroutput);
}
