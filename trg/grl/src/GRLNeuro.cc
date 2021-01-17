#include <framework/logging/Logger.h>
#include <trg/grl/GRLNeuro.h>
#include <trg/grl/dataobjects/GRLMLP.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dbobjects/CDCTriggerNeuroConfig.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <string>
#include <cmath>
#include <TFile.h>

using namespace Belle2;
using namespace CDC;
using namespace std;

void
GRLNeuro::initialize(const Parameters& p)
{
  // check parameters
  bool okay = true;
  // ensure that length of lists matches number of sectors
  if (p.nHidden.size() != 1 && p.nHidden.size() != p.nMLP) {
    B2ERROR("Number of nHidden lists should be 1 or " << p.nMLP);
    okay = false;
  }
  if (p.outputScale.size() != 1 && p.outputScale.size() != p.nMLP) {
    B2ERROR("Number of outputScale lists should be 1 or " << p.nMLP);
    okay = false;
  }
  // ensure that number of target nodes is valid
  unsigned short nTarget = int(p.targetresult);
  if (nTarget < 1) {
    B2ERROR("No outputs! Turn on targetresult.");
    okay = false;
  }
  for (unsigned iScale = 0; iScale < p.outputScale.size(); ++iScale) {
    if (p.outputScale[iScale].size() != 2 * nTarget) {
      B2ERROR("outputScale should be exactly " << 2 * nTarget << " values");
      okay = false;
    }
  }

  if (!okay) return;

  // initialize MLPs
  m_MLPs.clear();
  for (unsigned iMLP = 0; iMLP < p.nMLP; ++iMLP) {
    ////get indices for sector parameters
    //vector<unsigned> indices = getRangeIndices(p, iMLP);
    //get number of nodes for each layer
    //unsigned short nInput = 2*p.i_cdc_sector[iMLP] + 3*p.i_ecl_sector[iMLP];
    unsigned short nInput = p.i_cdc_sector[iMLP] + p.i_ecl_sector[iMLP];
    vector<float> nHidden = (p.nHidden.size() == 1) ? p.nHidden[0] : p.nHidden[iMLP];
    vector<unsigned short> nNodes = {nInput};
    for (unsigned iHid = 0; iHid < nHidden.size(); ++iHid) {
      if (p.multiplyHidden) {
        nNodes.push_back(nHidden[iHid] * nNodes[0]);
      } else {
        nNodes.push_back(nHidden[iHid]);
      }
    }
    nNodes.push_back(nTarget);
    unsigned short targetVars = int(p.targetresult);
    //get scaling values
    vector<float> outputScale = (p.outputScale.size() == 1) ? p.outputScale[0] : p.outputScale[iMLP];
    //create new MLP
    m_MLPs.push_back(GRLMLP(nNodes, targetVars, outputScale));
  }
  // load some values from the geometry that will be needed for the input
}

//vector<unsigned>
//GRLNeuro::getRangeIndices(const Parameters& p, unsigned isector)
//{
//  std::vector<unsigned> indices = {0, 0, 0, 0};
//  if (p.phiRange.size() * p.invptRange.size() * p.thetaRange.size() * p.SLpattern.size() == p.nMLP) {
//    indices[0] = isector % p.phiRange.size();
//    indices[1] = (isector / p.phiRange.size()) % p.invptRange.size();
//    indices[2] = (isector / (p.phiRange.size() * p.invptRange.size())) % p.thetaRange.size();
//    indices[3] = isector / (p.phiRange.size() * p.invptRange.size() * p.thetaRange.size());
//  } else {
//    indices[0] = (p.phiRange.size() == 1) ? 0 : isector;
//    indices[1] = (p.invptRange.size() == 1) ? 0 : isector;
//    indices[2] = (p.thetaRange.size() == 1) ? 0 : isector;
//    indices[3] = (p.SLpattern.size() == 1) ? 0 : isector;
//  }
//  return indices;
//}

//void
//GRLNeuro::initializeCollections(string hitCollectionName, string eventTimeName, std::string et_option)
//{
//  m_segmentHits.isRequired(hitCollectionName);
//  if (!((et_option == "fastestpriority") || (et_option == "zero") || (et_option == "fastest2d"))) {
//    m_eventTime.isRequired(eventTimeName);
//  }
//  m_hitCollectionName = hitCollectionName;
//}


vector<float>
GRLNeuro::runMLP(unsigned isector, const vector<float>& input)
{
  const GRLMLP& expert = m_MLPs[isector];
  vector<float> weights = expert.getWeights();
  vector<float> layerinput = input;
  vector<float> layeroutput = {};
  unsigned iw = 0;
  for (unsigned il = 1; il < expert.nLayers(); ++il) {
    //add bias input
    layerinput.push_back(1.);
    //prepare output
    layeroutput.clear();
    layeroutput.assign(expert.nNodesLayer(il), 0.);
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
  return layeroutput;
}

vector<float>
GRLNeuro::runMLPFix(unsigned isector, const vector<float>& input)
{
  unsigned precisionInput = m_precision[3];
  unsigned precisionWeights = m_precision[4];
  unsigned precisionLUT = m_precision[5];
  unsigned precisionTanh = m_precision[3];
  unsigned dp = precisionInput + precisionWeights - precisionLUT;

  const GRLMLP& expert = m_MLPs[isector];
  // transform inputs to fixed point (cut off to input precision)
  vector<long> inputFix(input.size(), 0);
  for (unsigned ii = 0; ii < input.size(); ++ii) {
    inputFix[ii] = long(input[ii] * (1 << precisionInput));
  }
  // transform weights to fixed point (round to weight precision)
  vector<float> weights = expert.getWeights();
  vector<long> weightsFix(weights.size(), 0);
  for (unsigned iw = 0; iw < weights.size(); ++iw) {
    weightsFix[iw] = long(round(weights[iw] * (1 << precisionWeights)));
  }
  // maximum input value for the tanh LUT
  unsigned xMax = unsigned(ceil(atanh(1. - 1. / (1 << (precisionTanh + 1))) *
                                (1 << (precisionLUT + 1))));

  // run MLP
  vector<long> layerinput = inputFix;
  vector<long> layeroutput = {};
  unsigned iw = 0;
  for (unsigned il = 1; il < expert.nLayers(); ++il) {
    // add bias input
    layerinput.push_back(1 << precisionInput);
    // prepare output
    layeroutput.clear();
    layeroutput.assign(expert.nNodesLayer(il), 0);
    // loop over outputs
    for (unsigned io = 0; io < layeroutput.size(); ++io) {
      // loop over inputs
      for (unsigned ii = 0; ii < layerinput.size(); ++ii) {
        layeroutput[io] += layerinput[ii] * weightsFix[iw++];
      }
      // apply activation function -> LUT, calculated on the fly here
      unsigned long bin = abs(layeroutput[io]) >> dp;
      // correction to get symmetrical rounding errors
      float x = (bin + 0.5 - 1. / (1 << (dp + 1))) / (1 << precisionLUT);
      long tanhLUT = (bin < xMax) ? long(round(tanh(x / 2.) * (1 << precisionTanh))) : (1 << precisionTanh);
      layeroutput[io] = (layeroutput[io] < 0) ? -tanhLUT : tanhLUT;
    }
    // output is new input
    layerinput = layeroutput;
  }

  // transform output back to float before unscaling
  vector<float> output(layeroutput.size(), 0.);
  for (unsigned io = 0; io < output.size(); ++io) {
    output[io] = layeroutput[io] / float(1 << precisionTanh);
  }
  return output;
}

void
GRLNeuro::save(const string& filename, const string& arrayname)
{
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

//bool
//GRLNeuro::load(const string& filename, const string& arrayname)
//{
//  if (filename.size() < 1) {
//    m_MLPs.clear();
//    m_MLPs = m_cdctriggerneuroconfig->getMLPs();
//    if (m_MLPs.size() == 0) {
//      B2ERROR("Could not load Neurotrigger weights from database!");
//      return false;
//    }
//    B2DEBUG(2, "Loaded Neurotrigger MLP weights from database: " +  m_cdctriggerneuroconfig->getNNName());
//    B2DEBUG(100, "loaded " << m_MLPs.size() << " networks from database");
//    // load some values from the geometry that will be needed for the input
//    setConstants();
//    return true;
//  } else {
//    TFile datafile(filename.c_str(), "READ");
//    if (!datafile.IsOpen()) {
//      B2WARNING("Could not open file " << filename);
//      return false;
//    }
//
//    TObjArray* MLPs = (TObjArray*)datafile.Get(arrayname.c_str());
//    if (!MLPs) {
//      datafile.Close();
//      B2WARNING("File " << filename << " does not contain key " << arrayname);
//      return false;
//    }
//    m_MLPs.clear();
//    for (int isector = 0; isector < MLPs->GetEntriesFast(); ++isector) {
//      GRLMLP* expert = dynamic_cast<GRLMLP*>(MLPs->At(isector));
//      if (expert) m_MLPs.push_back(*expert);
//      else B2WARNING("Wrong type " << MLPs->At(isector)->ClassName() << ", ignoring this entry.");
//    }
//    MLPs->Clear();
//    delete MLPs;
//    datafile.Close();
//    B2DEBUG(100, "loaded " << m_MLPs.size() << " networks");
//
//    // load some values from the geometry that will be needed for the input
//    setConstants();
//
//    return true;
//  }
//}
