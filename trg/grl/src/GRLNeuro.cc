/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

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
    //get indices for sector parameters
    unsigned short nInput = p.i_cdc_sector[iMLP] + p.i_ecl_sector[iMLP];
    vector<float> nhidden =  p.nHidden[iMLP];
    vector<unsigned short> nNodes = {nInput};
    for (unsigned iHid = 0; iHid < nhidden.size(); ++iHid) {
      if (p.multiplyHidden) {
        nNodes.push_back(nhidden[iHid] * nNodes[0]);
      } else {
        nNodes.push_back(nhidden[iHid]);
      }
    }
    nNodes.push_back(nTarget);
    unsigned short targetVars = int(p.targetresult);
    vector<float> outputScale = (p.outputScale.size() == 1) ? p.outputScale[0] : p.outputScale[iMLP];
    m_MLPs.push_back(GRLMLP(nNodes, targetVars, outputScale));
  }
}


float
GRLNeuro::relu(float x)
{
  if (x < 0) return 0;
  else return x;

}

float
GRLNeuro::runMLP(unsigned isector, const std::vector<float>& input)
{
  const GRLMLP& expert = m_MLPs[isector];
  vector<float> weights = expert.getWeights();
  vector<float> bias = expert.getBias();
  //vector<float> layerinput = {38.0,33.0,0.0,0.0,0.0,0.0,59.0,29.0,0.0,0.0,0.0,0.0,98.0,204.0,0.0,0.0,0.0,0.0,2.0};
  vector<float> layerinput = input;
  //for (int iin = 0 ; iin < input.size(); iin++) std::cout<< input[iin] << ",";
  vector<float> layeroutput2 = {};
  vector<float> layeroutput3 = {};
  vector<float> layeroutput4 = {};
  unsigned iw = 0;
  unsigned ib = 0;
  //loop over 1 -> 2 layer
  layeroutput2.clear();
  layeroutput2.assign(expert.getNumberOfNodesLayer(1), 0.);
  for (unsigned io = 0; io < layeroutput2.size(); ++io) {
    //loop over inputs
    for (unsigned ii = 0; ii < layerinput.size(); ++ii) {
      //std::cout<< " layerinput[" << ii <<"]:" << layerinput[ii]<< " weight: "<< weights[iw] << "muti: " <<layerinput[ii] * weights[iw] << endl;
      layeroutput2[io] += layerinput[ii] * weights[iw++];
    }
    //std::cout<< " bias: "<< bias[ib] << endl;
    layeroutput2[io] += bias[ib++];
    //apply activation function, ReLU and convert to float_to_fixed(x, 11, 0)
    layeroutput2[io] = relu(floor(layeroutput2[io]));
    //std::cout<< " layeroutput2["<< io <<"]:" << layeroutput2[io]  << endl;
  }
  //apply activation function, ReLU for hidden layer, and Sigmoid for output layer

  //loop over 2 -> 3 layer
  layeroutput3.clear();
  layeroutput3.assign(expert.getNumberOfNodesLayer(2), 0.);
  for (unsigned io = 0; io < layeroutput3.size(); ++io) {
    for (unsigned ii = 0; ii < layeroutput2.size(); ++ii) {
      //std::cout<< " layerinput["<< ii <<"]:" << layerinput[ii]<< " weight: "<< weights[iw] << "muti: " <<layerinput[ii] * weights[iw] << endl;
      layeroutput3[io] += layeroutput2[ii] * weights[iw++];
    }
    // std::cout<< " layeroutput["<< io <<"]:" << layeroutput[io] << " bias: "<< bias[ib] << endl;
    layeroutput3[io] += bias[ib++];
    //apply activation function, ReLU and convert to float_to_fixed(x, 7, 3)
    layeroutput3[io] = relu(float_to_fixed(layeroutput3[io], 7, 3)) ;
    //std::cout<< " layeroutput["<< io <<"]:" << layeroutput[io] << " il: "<< il << endl;
  }

  //loop over 3 -> 4 layer
  layeroutput4.clear();
  layeroutput4.assign(expert.getNumberOfNodesLayer(3), 0.);
  for (unsigned io = 0; io < layeroutput4.size(); ++io) {
    for (unsigned ii = 0; ii < layeroutput3.size(); ++ii) {
      //std::cout<< " layerinput["<< ii <<"]:" << layerinput[ii]<< " weight: "<< weights[iw] << "muti: " <<layerinput[ii] * weights[iw] << endl;
      layeroutput4[io] += layeroutput3[ii] * weights[iw++];
    }
    // std::cout<< " layeroutput["<< io <<"]:" << layeroutput[io] << " bias: "<< bias[ib] << endl;
    layeroutput4[io] += bias[ib++];
    //convert to float_to_fixed(x, 6, 2)
    layeroutput4[io] =  float_to_fixed(layeroutput4[io], 6, 2) ;
    //apply activation function, sigmiod and convert to float_to_fixed(x, 1, 6)
    layeroutput4[io] = mysigmiod(layeroutput4[io]);
    layeroutput4[io] = float_to_fixed(layeroutput4[io], 1, 6);
    //std::cout<< " layeroutput["<< io <<"]:" << layeroutput[io] << " il: "<< il << endl;
  }

  //std::cout<< "final value"<< layeroutput4[0] << std::endl;
  return layeroutput4[0];
}

bool GRLNeuro::load(unsigned isector, const string& weightfilename, const string& biasfilename)
{
  if (weightfilename.size() < 1) {
    B2ERROR("Could not load Neurotrigger weights from database!");
    return false;
  } else if (biasfilename.size() < 1) {
    B2ERROR("Could not load Neurotrigger bias from database!");
    return false;
  } else {
    std::ifstream wfile(weightfilename);
    if (!wfile.is_open()) {
      B2WARNING("Could not open file " << weightfilename);
      return false;
    } else {
      std::ifstream bfile(biasfilename);
      if (!bfile.is_open()) {
        B2WARNING("Could not open file " << biasfilename);
        return false;
      } else {
        GRLMLP& expert = m_MLPs[isector];
        std::vector<float> warray;
        std::vector<float> barray;
        warray.clear();
        barray.clear();

        float element;
        while (wfile >> element) {
          warray.push_back(element);
        }
        while (bfile >> element) {
          barray.push_back(element);
        }

        if (warray.size() != expert.nWeightsCal()) {
          B2ERROR("Number of weights is not equal to registered architecture!");
          return false;
        } else expert.setWeights(warray);
        if (barray.size() != expert.nBiasCal()) {
          B2ERROR("Number of bias is not equal to registered architecture!");
          return false;
        }
        //change the precision based in FPGA (hls4ml)
        for (int it = 0; it < warray.size(); it++) {
          if (it < 380)      warray[it] = float_to_fixed(warray[it], 2, 13);
          else if (it < 780) warray[it] = float_to_fixed(warray[it], 1, 13);
          else            warray[it] = float_to_fixed(warray[it], 2, 11);
        }

        for (int it = 0; it < barray.size(); it++) {
          if (it < 20)      barray[it] = float_to_fixed(barray[it], 4, 2);
          else if (it < 40) barray[it] = float_to_fixed(barray[it], 4, 3);
          else           barray[it] = float_to_fixed(barray[it], 0, 2);
        }

        expert.setWeights(warray);
        expert.setBias(barray);
        return true;
      }
    }
  }
}

float GRLNeuro::float_to_fixed(float num, int m, int n)
{
  //Get integer and decimal digits
  int integer_part = num;
  float fractional_part = num - integer_part;

  //floor to the nearest decimal digit
  fractional_part =  floor(fractional_part * (1 << n)) * (1.0 / (1 << n)) ;

  //Remove the overflow of integer bits
  //int final_integer = 0;
  //if( m >0 ){
  // final_integer = (integer_part & ((1 << m) -1)) ;
  //}

  float final_value = integer_part + fractional_part;
  return final_value;
}

float GRLNeuro::mysigmiod(float num)
{
  float sigmoid_table[1024] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.000976563, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00195313, 0.00292969, 0.00292969, 0.00292969, 0.00292969, 0.00292969, 0.00292969, 0.00292969, 0.00292969, 0.00292969, 0.00292969, 0.00292969, 0.00292969, 0.00292969, 0.00292969, 0.00292969, 0.00292969, 0.00292969, 0.00292969, 0.00292969, 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00488281, 0.00488281, 0.00488281, 0.00488281, 0.00488281, 0.00488281, 0.00488281, 0.00488281, 0.00488281, 0.00488281, 0.00488281, 0.00488281, 0.00585938, 0.00585938, 0.00585938, 0.00585938, 0.00585938, 0.00585938, 0.00585938, 0.00585938, 0.00585938, 0.00585938, 0.00683594, 0.00683594, 0.00683594, 0.00683594, 0.00683594, 0.00683594, 0.00683594, 0.00683594, 0.0078125, 0.0078125, 0.0078125, 0.0078125, 0.0078125, 0.0078125, 0.0078125, 0.0078125, 0.00878906, 0.00878906, 0.00878906, 0.00878906, 0.00878906, 0.00878906, 0.00878906, 0.00976563, 0.00976563, 0.00976563, 0.00976563, 0.00976563, 0.00976563, 0.0107422, 0.0107422, 0.0107422, 0.0107422, 0.0107422, 0.0107422, 0.0117188, 0.0117188, 0.0117188, 0.0117188, 0.0117188, 0.0126953, 0.0126953, 0.0126953, 0.0126953, 0.0126953, 0.0136719, 0.0136719, 0.0136719, 0.0136719, 0.0146484, 0.0146484, 0.0146484, 0.0146484, 0.015625, 0.015625, 0.015625, 0.015625, 0.0166016, 0.0166016, 0.0166016, 0.0166016, 0.0175781, 0.0175781, 0.0175781, 0.0175781, 0.0185547, 0.0185547, 0.0185547, 0.0195313, 0.0195313, 0.0195313, 0.0205078, 0.0205078, 0.0205078, 0.0214844, 0.0214844, 0.0214844, 0.0224609, 0.0224609, 0.0224609, 0.0234375, 0.0234375, 0.0244141, 0.0244141, 0.0244141, 0.0253906, 0.0253906, 0.0253906, 0.0263672, 0.0263672, 0.0273438, 0.0273438, 0.0283203, 0.0283203, 0.0292969, 0.0292969, 0.0292969, 0.0302734, 0.0302734, 0.03125, 0.03125, 0.0322266, 0.0322266, 0.0332031, 0.0332031, 0.0341797, 0.0341797, 0.0351563, 0.0361328, 0.0361328, 0.0371094, 0.0371094, 0.0380859, 0.0380859, 0.0390625, 0.0400391, 0.0400391, 0.0410156, 0.0419922, 0.0419922, 0.0429688, 0.0439453, 0.0439453, 0.0449219, 0.0458984, 0.0458984, 0.046875, 0.0478516, 0.0488281, 0.0488281, 0.0498047, 0.0507813, 0.0517578, 0.0517578, 0.0527344, 0.0537109, 0.0546875, 0.0556641, 0.0566406, 0.0566406, 0.0576172, 0.0585938, 0.0595703, 0.0605469, 0.0615234, 0.0625, 0.0634766, 0.0644531, 0.0654297, 0.0664063, 0.0673828, 0.0683594, 0.0693359, 0.0703125, 0.0712891, 0.0722656, 0.0732422, 0.0742188, 0.0751953, 0.0761719, 0.0771484, 0.0791016, 0.0800781, 0.0810547, 0.0820313, 0.0830078, 0.0849609, 0.0859375, 0.0869141, 0.0878906, 0.0898438, 0.0908203, 0.0917969, 0.09375, 0.0947266, 0.0966797, 0.0976563, 0.0986328, 0.100586, 0.101563, 0.103516, 0.104492, 0.106445, 0.107422, 0.109375, 0.110352, 0.112305, 0.114258, 0.115234, 0.117188, 0.119141, 0.120117, 0.12207, 0.124023, 0.125, 0.126953, 0.128906, 0.130859, 0.132813, 0.134766, 0.135742, 0.137695, 0.139648, 0.141602, 0.143555, 0.145508, 0.147461, 0.149414, 0.151367, 0.15332, 0.155273, 0.157227, 0.160156, 0.162109, 0.164063, 0.166016, 0.167969, 0.170898, 0.172852, 0.174805, 0.177734, 0.179688, 0.181641, 0.18457, 0.186523, 0.189453, 0.191406, 0.194336, 0.196289, 0.199219, 0.201172, 0.204102, 0.206055, 0.208984, 0.211914, 0.213867, 0.216797, 0.219727, 0.222656, 0.224609, 0.227539, 0.230469, 0.233398, 0.236328, 0.239258, 0.242188, 0.244141, 0.24707, 0.25, 0.25293, 0.255859, 0.259766, 0.262695, 0.265625, 0.268555, 0.271484, 0.274414, 0.277344, 0.28125, 0.28418, 0.287109, 0.290039, 0.293945, 0.296875, 0.299805, 0.303711, 0.306641, 0.310547, 0.313477, 0.317383, 0.320313, 0.324219, 0.327148, 0.331055, 0.333984, 0.337891, 0.34082, 0.344727, 0.348633, 0.351563, 0.355469, 0.358398, 0.362305, 0.366211, 0.370117, 0.373047, 0.376953, 0.380859, 0.384766, 0.387695, 0.391602, 0.395508, 0.399414, 0.40332, 0.407227, 0.410156, 0.414063, 0.417969, 0.421875, 0.425781, 0.429688, 0.433594, 0.4375, 0.441406, 0.445313, 0.449219, 0.453125, 0.457031, 0.460938, 0.464844, 0.46875, 0.472656, 0.476563, 0.480469, 0.484375, 0.488281, 0.492188, 0.496094, 0.5, 0.50293, 0.506836, 0.510742, 0.514648, 0.518555, 0.522461, 0.526367, 0.530273, 0.53418, 0.538086, 0.541992, 0.545898, 0.549805, 0.553711, 0.557617, 0.561523, 0.56543, 0.569336, 0.573242, 0.577148, 0.581055, 0.584961, 0.588867, 0.591797, 0.595703, 0.599609, 0.603516, 0.607422, 0.611328, 0.614258, 0.618164, 0.62207, 0.625977, 0.628906, 0.632813, 0.636719, 0.640625, 0.643555, 0.647461, 0.650391, 0.654297, 0.658203, 0.661133, 0.665039, 0.667969, 0.671875, 0.674805, 0.678711, 0.681641, 0.685547, 0.688477, 0.692383, 0.695313, 0.699219, 0.702148, 0.705078, 0.708984, 0.711914, 0.714844, 0.717773, 0.72168, 0.724609, 0.727539, 0.730469, 0.733398, 0.736328, 0.739258, 0.743164, 0.746094, 0.749023, 0.751953, 0.754883, 0.756836, 0.759766, 0.762695, 0.765625, 0.768555, 0.771484, 0.774414, 0.776367, 0.779297, 0.782227, 0.785156, 0.787109, 0.790039, 0.792969, 0.794922, 0.797852, 0.799805, 0.802734, 0.804688, 0.807617, 0.80957, 0.8125, 0.814453, 0.817383, 0.819336, 0.821289, 0.824219, 0.826172, 0.828125, 0.831055, 0.833008, 0.834961, 0.836914, 0.838867, 0.841797, 0.84375, 0.845703, 0.847656, 0.849609, 0.851563, 0.853516, 0.855469, 0.857422, 0.859375, 0.861328, 0.863281, 0.864258, 0.866211, 0.868164, 0.870117, 0.87207, 0.874023, 0.875, 0.876953, 0.878906, 0.879883, 0.881836, 0.883789, 0.884766, 0.886719, 0.888672, 0.889648, 0.891602, 0.892578, 0.894531, 0.895508, 0.897461, 0.898438, 0.900391, 0.901367, 0.902344, 0.904297, 0.905273, 0.907227, 0.908203, 0.90918, 0.911133, 0.912109, 0.913086, 0.914063, 0.916016, 0.916992, 0.917969, 0.918945, 0.919922, 0.921875, 0.922852, 0.923828, 0.924805, 0.925781, 0.926758, 0.927734, 0.928711, 0.929688, 0.930664, 0.931641, 0.932617, 0.933594, 0.93457, 0.935547, 0.936523, 0.9375, 0.938477, 0.939453, 0.94043, 0.941406, 0.942383, 0.942383, 0.943359, 0.944336, 0.945313, 0.946289, 0.947266, 0.947266, 0.948242, 0.949219, 0.950195, 0.950195, 0.951172, 0.952148, 0.953125, 0.953125, 0.954102, 0.955078, 0.955078, 0.956055, 0.957031, 0.957031, 0.958008, 0.958984, 0.958984, 0.959961, 0.960938, 0.960938, 0.961914, 0.961914, 0.962891, 0.962891, 0.963867, 0.964844, 0.964844, 0.96582, 0.96582, 0.966797, 0.966797, 0.967773, 0.967773, 0.96875, 0.96875, 0.969727, 0.969727, 0.969727, 0.970703, 0.970703, 0.97168, 0.97168, 0.972656, 0.972656, 0.973633, 0.973633, 0.973633, 0.974609, 0.974609, 0.974609, 0.975586, 0.975586, 0.976563, 0.976563, 0.976563, 0.977539, 0.977539, 0.977539, 0.978516, 0.978516, 0.978516, 0.979492, 0.979492, 0.979492, 0.980469, 0.980469, 0.980469, 0.981445, 0.981445, 0.981445, 0.981445, 0.982422, 0.982422, 0.982422, 0.982422, 0.983398, 0.983398, 0.983398, 0.983398, 0.984375, 0.984375, 0.984375, 0.984375, 0.985352, 0.985352, 0.985352, 0.985352, 0.986328, 0.986328, 0.986328, 0.986328, 0.986328, 0.987305, 0.987305, 0.987305, 0.987305, 0.987305, 0.988281, 0.988281, 0.988281, 0.988281, 0.988281, 0.988281, 0.989258, 0.989258, 0.989258, 0.989258, 0.989258, 0.989258, 0.990234, 0.990234, 0.990234, 0.990234, 0.990234, 0.990234, 0.990234, 0.991211, 0.991211, 0.991211, 0.991211, 0.991211, 0.991211, 0.991211, 0.991211, 0.992188, 0.992188, 0.992188, 0.992188, 0.992188, 0.992188, 0.992188, 0.992188, 0.993164, 0.993164, 0.993164, 0.993164, 0.993164, 0.993164, 0.993164, 0.993164, 0.993164, 0.993164, 0.994141, 0.994141, 0.994141, 0.994141, 0.994141, 0.994141, 0.994141, 0.994141, 0.994141, 0.994141, 0.994141, 0.994141, 0.995117, 0.995117, 0.995117, 0.995117, 0.995117, 0.995117, 0.995117, 0.995117, 0.995117, 0.995117, 0.995117, 0.995117, 0.995117, 0.995117, 0.996094, 0.996094, 0.996094, 0.996094, 0.996094, 0.996094, 0.996094, 0.996094, 0.996094, 0.996094, 0.996094, 0.996094, 0.996094, 0.996094, 0.996094, 0.996094, 0.996094, 0.996094, 0.996094, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.99707, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.998047, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023, 0.999023};

  int data_round = num * 1024 / 16;
  int index = data_round + 8 * 1024 / 16;
  if (index < 0)   index = 0;
  if (index > 1023) index = 1023;
  return sigmoid_table[index];

}
