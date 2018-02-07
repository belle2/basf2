/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/reconstruction/NNWaveFitter.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <framework/utilities/FileSystem.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace SVD;
using namespace Eigen;

/** Counting things in NN:
 *
 * Layers: input (hidden layers) output = n_hidden_lyaers + 2 =
 * m_nLayers 0,1,2,... m_nLayers - 1.
 * Layer sizes: input is 6, output is given by time binning in training,
 * so we know terms 0 and m_nLayers - 1, and we can even establish hidden
 * layer sizes beforehand from the repr of the classifier!
 * Weights are about connections between layers, so we have w01, w12,..
 * w(m_nLayers-2)(m_nLayers-1) so we denote w1, w2, ... w(m_nLayers - 1)
 * Intercepts are related  to neurons where connections converge and
 * activation is applied. So we have i1,...i(m_nLayers - 1).
 */

int NNWaveFitter::readNetworkData(const string& xmlData)
{
  using namespace boost::property_tree;
  using boost::property_tree::ptree;

  ptree propertyTree;

  try {
    B2DEBUG(400, "Reading xml");
    stringstream ss;
    ss << xmlData;
    read_xml(ss, propertyTree);
  } catch (const ptree_error& e) {
    B2ERROR("Failed to parse xml data: " << e.what());
  } catch (std::exception const& ex) {
    B2ERROR("STD excpetion " << ex.what() << " in parsing.");
    return -1;
  }
  // 1. Check that we have a classifier with relu activation.
  try {
    // 1a. Classifier
    string function_name = propertyTree.get<string>("PMML.NeuralNetwork.<xmlattr>.functionName");
    if (function_name != "classification") {
      B2ERROR("This is incorrect network, expected multiclass classifier.");
      return -1;
    }
    B2DEBUG(400, "Claasifier confirmed.");
    // 1b. Activation
    string activationFunction = propertyTree.get<string>("PMML.NeuralNetwork.<xmlattr>.activationFunction");
    if (activationFunction != "rectifier") {
      B2ERROR("Expected rectifier (relu) activation, found " << activationFunction << "instead.");
      return -1;
    }
    B2DEBUG(400, "Activation confirmed.");
    // 1c.Waveform type FIXME: no logic or checking here:
    string waveFormType = propertyTree.get<string>("PMML.MiningBuildTask.Training.Waveform");
    B2DEBUG(400, "Waveform set to " << waveFormType);
  } catch (const ptree_error& e) {
    B2ERROR("PropertyTree excpetion : " << e.what() << " in network check.");
    return -1;
  }

  // 2. Read network topology
  try {
    string pathString = "PMML.MiningBuildTask.NetworkParameters";
    map<size_t, size_t> m_layer_sizes_map;
    for (const auto& layer_tag : propertyTree.get_child(pathString)) {
      if (layer_tag.first == "<xmlattr>") continue;
      size_t layer_no = layer_tag.second.get<size_t>("<xmlattr>.number");
      size_t layer_size = layer_tag.second.get<size_t>("<xmlattr>.size");
      m_layer_sizes_map.insert(make_pair(layer_no, layer_size));
    }
    // Once we have layer sizes, we can set weight matrices and intercept
    // vectors to proper sizes.
    m_nLayers = m_layer_sizes_map.size();
    m_layerSizes.resize(m_nLayers);
    for (size_t iLayer = 0; iLayer < m_nLayers; ++iLayer)
      m_layerSizes[iLayer] = m_layer_sizes_map[iLayer];
    for (size_t iLayer = 1; iLayer < m_nLayers; ++iLayer) {
      m_networkCoefs.insert(make_pair(iLayer, make_pair(
                                        MatrixXd(m_layerSizes[iLayer], m_layerSizes[iLayer - 1]),
                                        VectorXd(m_layerSizes[iLayer])
                                      )
                                     ));
    }
    for (size_t iLayer = 0; iLayer < m_nLayers; ++iLayer)
      m_layerStates.push_back(VectorXd(m_layerSizes[iLayer]));
    B2DEBUG(400, "Network topology read.");
  } catch (const ptree_error& e) {
    B2ERROR("PropertyTree excpetion : " << e.what() << " when reading network topology.");
    return -1;
  }

  // Read training parameter bounds
  try {
    string pathString("PMML.MiningBuildTask.Training");
    for (const auto& param_tag : propertyTree.get_child(pathString)) {
      if (param_tag.first == "Parameter") {
        string valueString(param_tag.second.get<string>("<xmlattr>.value"));
        double low = param_tag.second.get<double>("<xmlattr>.low");
        double high = param_tag.second.get<double>("<xmlattr>.high");
        if (valueString == "amplitude") {
          m_amplitudeBounds.first =  low;
          m_amplitudeBounds.second = high;
        } else if (valueString == "t0") {
          m_timeShiftBounds.first = low * Unit::ns;
          m_timeShiftBounds.second = high * Unit::ns;
        } else if (valueString == "tau") {
          m_waveWidthBounds.first = low * Unit::ns;
          m_waveWidthBounds.second = high * Unit::ns;
        }
      }
    }
    // Set bounds in the tau encoder.
    // Tau bounds are raw values!
    m_tauCoder.setBounds(m_amplitudeBounds.first, m_amplitudeBounds.second,
                         m_waveWidthBounds.first, m_waveWidthBounds.second);
    B2DEBUG(400, "Read parameter bounds.");
  } catch (const ptree_error& e) {
    B2ERROR("PropertyTree excpetion: " << e.what() << " when reading parameter bounds.");
    return -1;
  }

  // Reading output binning.
  try {
    string pathString("PMML.DataDictionary.DataField");
    m_binCenters.resize(m_layerSizes[m_nLayers - 1]);
    m_bins.resize(m_layerSizes[m_nLayers - 1] + 1);
    for (const auto& value_tag : propertyTree.get_child(pathString)) {
      if (value_tag.first == "Value") {
        size_t i = value_tag.second.get<size_t>("<xmlattr>.value");
        if (i == 1) // numbering is from 1!
          m_bins[0] = value_tag.second.get<double>("<xmlattr>.lower") * Unit::ns;
        m_bins[i] = value_tag.second.get<double>("<xmlattr>.upper") * Unit::ns;
        m_binCenters[i - 1] = value_tag.second.get<double>("<xmlattr>.midpoint") * Unit::ns;
      }
    }
    m_fitTool = std::shared_ptr<NNWaveFitTool>(new NNWaveFitTool(m_bins, m_binCenters, WaveGenerator(m_wave)));
    B2DEBUG(400, "Outputs done.");
  } catch (const ptree_error& e) {
    B2ERROR("PropertyTree excpetion: " << e.what() << " when reading bin data.");
    return -1;
  }

  // Reading neurons.
  try {
    string pathString("PMML.NeuralNetwork");
    for (const auto& nl_tag : propertyTree.get_child(pathString)) {
      if (nl_tag.first != "NeuralLayer") continue;
      B2DEBUG(400, "Reading neural layers " << nl_tag.first << " " << nl_tag.second.size());
      for (const auto& neuron_tag : nl_tag.second) {
        if (neuron_tag.first != "Neuron") continue;
        double bias = neuron_tag.second.get<double>("<xmlattr>.bias");
        string sid = neuron_tag.second.get<string>("<xmlattr>.id");
        size_t layer = stoi(sid.substr(0, 1)); // !! Won't work with > 9 !!
        size_t pos = stoi(sid.substr(2, sid.size()));
        B2DEBUG(400, "Reading neurons " << layer << "/" << pos << " bias: " << bias);
        m_networkCoefs[layer].second(pos - 1) = bias;
        for (const auto& con_tag : neuron_tag.second) {
          if (con_tag.first != "Con") continue;
          double weight = con_tag.second.get<double>("<xmlattr>.weight");
          string sid2 = con_tag.second.get<string>("<xmlattr>.from");
          size_t pos2 = stoi(sid2.substr(sid2.find('/') + 1, sid2.size()));
          m_networkCoefs[layer].first(pos - 1, pos2 - 1) = weight;
          B2DEBUG(400, "Reading connections " << sid2 << " weight: " << weight);
        }
      }
    }
    B2DEBUG(400, "Neurons done.");
  } catch (const ptree_error& e) {
    B2ERROR("PropertyTree excpetion: " << e.what() << " when reading neurons.");
    return -1;
  }
  return 0;
}

bool NNWaveFitter::checkCoefficients(const std::string& dumpname, double tol)
{
  bool result = true;
  ifstream dump(dumpname);

  // Read the dumpfile line by line and compare coefficients.
  string line;
  getline(dump, line); // Header
  // The dimension of the matrix is
  // (m_layerSizes[iLayer], m_layerSizes[iLayer-1])
  // and the dimension of the intercepts is
  // m_layerSizes[iLayer]
  for (size_t iLayer = 1; iLayer < m_nLayers; ++iLayer) {
    getline(dump, line); // Layer
    getline(dump, line); // Matrix
    // the matrix is m_networkCoefs[iLayer].first, and has
    // m_layerSizes[iLayer] rows with m_layerSizes[iLayer-1] entries
    // in a row (columns)
    for (size_t iRow = 0; iRow < m_layerSizes[iLayer]; ++iRow) {
      getline(dump, line);
      istringstream iline(line);
      for (size_t iCol = 0; iCol < m_layerSizes[iLayer - 1]; ++iCol) {
        double value = 0;
        iline >> value;
        if (fabs(value - m_networkCoefs[iLayer].first(iRow, iCol)) > tol) {
          B2DEBUG(90, "Mismatch in weights in layer: " << iLayer
                  << " row: " << iRow << " col: " << iCol
                  << " C++: " << m_networkCoefs[iLayer].first(iRow, iCol)
                  << " Python: " << value
                  << " Diff: " << m_networkCoefs[iLayer].first(iRow, iCol) - value
                 );
          result = false;
        }
      }
    }

    getline(dump, line); // Intercepts
    // this is m_networkCoefs[iLayer].second, and has m_layerSizes[iLayer]
    // numbers in a single row.
    getline(dump, line);
    istringstream iline(line);
    for (size_t iRow = 0; iRow < m_layerSizes[iLayer]; ++iRow) {
      double value = 0;
      iline >> value;
      if (fabs(value - m_networkCoefs[iLayer].second(iRow)) > tol) {
        B2DEBUG(90, "Mismatch in intercepts in layer: " << iLayer
                << " row: " << iRow
                << " C++: " << m_networkCoefs[iLayer].second(iRow)
                << " Python: " << value
                << " Diff: " << m_networkCoefs[iLayer].second(iRow) - value
               );

        result = false;
      }
    }
  }
  return result;
}

void NNWaveFitter::setNetwrok(const string& xmlData)
{
  if ((xmlData != "") && (readNetworkData(xmlData) == 0)) {
    m_isValid = true;
  } else m_isValid = false;
  // We don't issue any additional warnings here.
}


NNWaveFitter::NNWaveFitter(string xmlData)
{
  m_wave = w_betaprime;
  if (xmlData != "") setNetwrok(xmlData);
}


std::shared_ptr<nnFitterBinData> NNWaveFitter::getFit(const apvSamples& samples, double tau)
{
  // Create the fit result object
  std::shared_ptr<nnFitterBinData> result(new nnFitterBinData);
  if (!m_isValid) return result;

  copy(samples.begin(), samples.end(), &m_layerStates[0](0));
  // Add tau in the last input cell
  m_layerStates[0][samples.size()] = m_tauCoder.encodeTau(tau);
  ostringstream os;
  os << "Fitting with tau = " << tau << " encoded to " << m_tauCoder.encodeTau(tau) << endl
     << "Samples: ";
  copy(samples.begin(), samples.end(), ostream_iterator<double>(os, " "));
  os << endl;
  m_tauCoder.print(os);
  B2DEBUG(100, os.str());
  // Calculate network response
  os.str() = "";
  os << "Layer states: " << endl;
  for (size_t iLayer = 1; iLayer < m_nLayers; ++iLayer) {
    m_layerStates[iLayer] = (
                              m_networkCoefs[iLayer].first * m_layerStates[iLayer - 1]
                              + m_networkCoefs[iLayer].second
                            ).unaryExpr(relu);
    os << m_layerStates[iLayer] << endl;
  }
  B2DEBUG(100, os.str());

  // Output transformation 1: softmax normalization

  m_layerStates[m_nLayers - 1] = softmax(m_layerStates[m_nLayers - 1]);

  result->resize(m_layerStates[m_nLayers - 1].size());
  for (size_t i = 0; i < result->size(); ++i)
    (*result)[i] = m_layerStates[m_nLayers - 1][i];

  os.str() = "";
  os << "Result: " << endl;
  copy(result->begin(), result->end(), ostream_iterator<double>(os, " "));
  os << endl;
  B2DEBUG(100, os.str());

  return result;
}

