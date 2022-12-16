#include <trg/cdc/NeuroTriggerParameters.h>


#include <TFile.h>
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/multi_array.hpp"
#include <iostream>
#include <fstream>

using namespace Belle2;

NeuroTriggerParameters::NeuroTriggerParameters(std::string& filename)
{
  NeuroTriggerParameters::loadconfigtxt(filename);

}

void
NeuroTriggerParameters::loadconfigtxt(const std::string& filename)
{
  // now loading confile
  std::ifstream confile;
  try {
    confile.open(filename, std::ifstream::in);
  } catch (int e) {
    std::cout << "ERROR! While opening file: " << filename << "    Error code: " << e << std::endl;
    exit(EXIT_FAILURE);
  }
  std::string line_all;
  if (!confile.is_open()) {
    std::cout << "ERROR! While opening file: " << filename << std::endl;
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
    bool locked = false; //check, if "==" was used and the variable should be registered as locked
    //if (line.find('==') != std::string::npos) {
    //    locked = true;
    //}
    if (line.find('=') == std::string::npos) {
      continue;
    }
    line.erase(std::remove(line.begin(), line.end(), ' '), line.end()); // remove whitespaces in whole string
    par = line.substr(0, line.find('='));
    std::string l;
    l = line.substr(line.find("=") + 1, 1);
    if (l  == "=") {
      locked = true;
    }
    // now find the value that should be asigned to the "par"
    key = (locked) ? line.substr((line.find('=') + 2), line.length() - line.find('=') - 2) : line.substr((line.find('=') + 1),
          line.length() - line.find('=') - 1);
    if (par == "nInput")         {
      nInput = std::stoul(key);
      if (locked) {nInput.lock();}
    } else if (par == "nOutput")        {
      nOutput = std::stoul(key);
      if (locked) {nOutput.lock();}
    } else if (par == "relevantCut")        {
      relevantCut = std::stod(key);
      if (locked) {relevantCut.lock();}
    } else if (par == "nMLP")           {
      nMLP = std::stoul(key);
      if (locked) {nMLP.lock();}
    } else if (par == "targetZ")        {
      targetZ = std::stoi(key);
      if (locked) {targetZ.lock();}
    } else if (par == "targetTheta")    {
      targetTheta = std::stoi(key);
      if (locked) {targetTheta.lock();}
    } else if (par == "multiplyHidden")    {
      multiplyHidden = std::stoi(key);
      if (locked) {multiplyHidden.lock();}
    } else if (par == "rescaleTarget")    {
      rescaleTarget = std::stoi(key);
      if (locked) {rescaleTarget.lock();}
    } else if (par == "cutSum")    {
      cutSum = std::stoi(key);
      if (locked) {cutSum.lock();}
    } else if (par == "tMax")           {
      tMax = std::stoul(key);
      if (locked) {tMax.lock();}
    } else if (par == "ETOption")       {
      ETOption = std::stoul(key);
      if (locked) {ETOption.lock();}
    } else if (par == "IDRanges")    {
      IDRanges = read2dArray<float>(key, locked);
    } else if (par == "phiRangeUse")    {
      phiRangeUse = read2dArray<float>(key, locked);
    } else if (par == "thetaRangeUse")    {
      thetaRangeUse = read2dArray<float>(key, locked);
    } else if (par == "invptRangeUse")    {
      invptRangeUse = read2dArray<float>(key, locked);
    } else if (par == "phiRangeTrain")    {
      phiRangeTrain = read2dArray<float>(key, locked);
    } else if (par == "thetaRangeTrain")    {
      thetaRangeTrain = read2dArray<float>(key, locked);
    } else if (par == "invptRangeTrain")    {
      invptRangeTrain = read2dArray<float>(key, locked);
    } else if (par == "nHidden")    {
      nHidden = read2dArray<float>(key, locked);
    } else if (par == "maxHitsPerSL")    {
      maxHitsPerSL = read1dArray<unsigned short>(key, locked);
    } else if (par == "outputScale")    {
      outputScale = read2dArray<float>(key, locked);
    } else if (par == "SLpattern")    {
      SLpattern = read1dArray<unsigned long>(key, locked);
    } else if (par == "SLpatternMask")    {
      SLpatternMask = read1dArray<unsigned long>(key, locked);
    } else if (par == "precision")    {
      precision = read1dArray<unsigned>(key, locked);
    }
  }
}
template<typename X>
bool NeuroTriggerParameters::checkarr(std::vector<std::vector<NNTParam<X>>> vec)
{
  // check, if a vector is already set. this is done by just checking the first member of the vector.
  if (vec.size() < 1) {return false;}
  else if (vec[0].size() < 1) {return false;}
  else {return vec[0][0].isSet();}
}

template<typename X>
bool NeuroTriggerParameters::checkarr(std::vector<NNTParam<X>> vec)
{
  // check, if a vector is already set. this is done by just checking the first member of the vector.
  if (vec.size() < 1) {return false;}
  else {return vec[0].isSet();}
}

void NeuroTriggerParameters::saveconfigtxt(const std::string& filename)
{
  // save all the configuration otions in a file, which can later be used to initialize a new parameter set.
  // only paramters, that already have been set are saved. parameters which are locked, are written wit a "==".
  std::ofstream savestream;
  savestream.open(filename);
  savestream << "########################################################" << std::endl;
  savestream << "### Neurotrigger configuration file created by basf2 ###" << std::endl;
  savestream << "########################################################" << std::endl << std::endl;
  savestream << "# '=' means the parameter is set and can be changed," << std::endl;
  savestream << "# '==' means the parameter is locked and should not be changed." << std::endl << std::endl;

  savestream << "# number of input nodes " << std::endl;
  if (nInput.isSet()) {
    savestream << "nInput " << (nInput.isLocked() ? "== " : "= ") << nInput << std::endl;
    savestream  << std::endl;
  } else {savestream << "nInput = 27" << std::endl;}
  savestream << "# number of output nodes " << std::endl;
  if (nOutput.isSet()) {
    savestream << "nOutput " << (nOutput.isLocked() ? "== " : "= ") << nOutput << std::endl;
    savestream  << std::endl;
  } else {savestream << "nOutput = 2" << std::endl;}
  savestream << "# If true, relevantCut is applied to the sum over hit counters, " << std::endl;
  savestream << "#  otherwise directly on the hit counters." << std::endl;
  if (cutSum.isSet()) {
    savestream << "cutSum " << (cutSum.isLocked() ? "== " : "= ") << cutSum << std::endl;
    savestream  << std::endl;
  } else {savestream << "cutSum = 0" << std::endl;}
  savestream << "# only used in the idhist module. it defines the how much of the idrange is cut off after making the histogram " <<
             std::endl;
  if (relevantCut.isSet()) {
    savestream << "relevantCut " << (relevantCut.isLocked() ? "== " : "= ") << relevantCut << std::endl;
    savestream  << std::endl;
  } else {savestream << "relevantCut = 0.02" << std::endl;}
  savestream << "# flag to allow for target tracks lying out of the output range to be rescaled during training. " << std::endl;
  if (rescaleTarget.isSet()) {
    savestream << "rescaleTarget " << (rescaleTarget.isLocked() ? "== " : "= ") << rescaleTarget << std::endl;
    savestream  << std::endl;
  } else {savestream << "rescaleTarget = 0" << std::endl;}
  savestream << "# Number of networks. For network specific parameters you can give " << std::endl;
  savestream << "#  either a list with values for each network, or a single value that will be used" << std::endl;
  savestream << "#  for all. The ranges are also valid if nPhi * nPt * nTheta * nPattern = nMLPs. " << std::endl;
  if (nMLP.isSet()) {
    savestream << "nMLP " << (nMLP.isLocked() ? "== " : "= ") << nMLP << std::endl;
    savestream  << std::endl;
  } else {savestream << "nMLP = 5" << std::endl;}
  savestream << "# train z as output " << std::endl;
  if (targetZ.isSet()) {
    savestream << "targetZ " << (targetZ.isLocked() ? "== " : "= ") << targetZ << std::endl;
    savestream  << std::endl;
  } else {savestream << "targetZ = 1" << std::endl;}
  savestream << "# train theta as output " << std::endl;
  if (targetTheta.isSet()) {
    savestream << "targetTheta " << (targetTheta.isLocked() ? "== " : "= ") << targetTheta << std::endl;
    savestream  << std::endl;
  } else {savestream << "targetTheta = 1" << std::endl;}
  savestream << "# If true, multiply nHidden with number of input nodes. " << std::endl;
  if (multiplyHidden.isSet()) {
    savestream << "multiplyHidden " << (multiplyHidden.isLocked() ? "== " : "= ") << multiplyHidden << std::endl;
    savestream  << std::endl;
  } else {savestream << "multiplyHidden = 0" << std::endl;}
  savestream << "# Maximal drift time, identical for all networks. " << std::endl;
  if (tMax.isSet()) {
    savestream << "tMax " << (tMax.isLocked() ? "== " : "= ") << tMax << std::endl;
    savestream  << std::endl;
  } else {savestream << "tMax = 256" << std::endl;}
  savestream << "# Determine, how the event time should be obtained. The options are:" << std::endl;
  savestream << "# - fastestpriority" << std::endl;
  savestream << "# - fastest2d" << std::endl;
  savestream << "# - etf" << std::endl;
  savestream << "# - etf_or_fastestpriority" << std::endl;
  savestream << "# - etf_or_fastest2d" << std::endl;
  savestream << "# - etf_only" << std::endl;
  savestream << "# - etfcc" << std::endl;
  savestream << "# - etfhwin" << std::endl;
  savestream << "# - etfcc_or_fastestpriority" << std::endl;
  savestream << "# - min_etf_fastestpriority" << std::endl;
  savestream << "# - min_etfcc_fastestpriority" << std::endl;
  if (ETOption.isSet()) {
    savestream << "ETOption " << (ETOption.isLocked() ? "== " : "= ") << ETOption << std::endl;
    savestream  << std::endl;
  } else {savestream << "ETOption = 7" << std::endl;}
  savestream << "# Phi region for which MLP is used in degree for all networks. " << std::endl;
  if (checkarr(phiRangeUse)) {
    savestream << print2dArray<float>("phiRangeUse", phiRangeUse);
    savestream  << std::endl;
  } else {savestream << "phiRangeUse = [[0, 360]]" << std::endl;}
  savestream << "# relative ID range of the relevant wire IDs of the track segments " << std::endl;
  savestream << "#  that are taken into consideration when determining the best fitting track segments. " << std::endl;
  if (checkarr(IDRanges)) {
    savestream << print2dArray<float>("IDRanges", IDRanges);
    savestream  << std::endl;
  } else {
    savestream << "IDRanges = [[0,-1.5,1.5,-7.5,-0.5,-1.5,1.5,0.5,7.5,-1.5,1.5,-8.5,0.5,-2.5,1.5,-0.5,10.5,-3.5,2.5],";
    savestream <<             "[1,-1.5,1.5,-7.5,-0.5,-1.5,1.5,0.5,7.5,-1.5,1.5,-8.5,0.5,-2.5,1.5,-0.5,10.5,-3.5,2.5],";
    savestream <<             "[2,-1.5,1.5,-7.5,-0.5,-1.5,1.5,0.5,7.5,-1.5,1.5,-8.5,0.5,-2.5,1.5,-0.5,10.5,-3.5,2.5],";
    savestream <<             "[3,-1.5,1.5,-7.5,-0.5,-1.5,1.5,0.5,7.5,-1.5,1.5,-8.5,0.5,-2.5,1.5,-0.5,10.5,-3.5,2.5],";
    savestream <<             "[4,-1.5,1.5,-7.5,-0.5,-1.5,1.5,0.5,7.5,-1.5,1.5,-8.5,0.5,-2.5,1.5,-0.5,10.5,-3.5,2.5]]";
    savestream << std::endl;
  }
  savestream << "# Theta region for which MLP is used in degree for all networks. " << std::endl;
  if (checkarr(thetaRangeUse)) {
    savestream << print2dArray<float>("thetaRangeUse", thetaRangeUse);
    savestream  << std::endl;
  } else {savestream << "thetaRangeUse = [[10, 170]]" << std::endl;}
  savestream << "# Charge / Pt region for which MLP is used in 1/GeV for all networks. " << std::endl;
  if (checkarr(invptRangeUse)) {
    savestream << print2dArray<float>("invptRangeUse", invptRangeUse);
    savestream  << std::endl;
  } else {savestream << "invptRangeUse = [[-5, 5]]" << std::endl;}
  savestream << "# Phi region for which MLP is trained in degree for all networks. " << std::endl;
  savestream << "#   Can be larger than use range to avoid edge effects. " << std::endl;
  if (checkarr(phiRangeTrain)) {
    savestream << print2dArray<float>("phiRangeTrain", phiRangeTrain);
    savestream  << std::endl;
  } else {savestream << "phiRangeTrain = [[0, 360]]" << std::endl;}
  savestream << "# Theta region for which MLP is trained in degree for all networks. " << std::endl;
  savestream << "#   Can be larger than use range to avoid edge effects. " << std::endl;
  if (checkarr(thetaRangeTrain)) {
    savestream << print2dArray<float>("thetaRangeTrain", thetaRangeTrain);
    savestream  << std::endl;
  } else {savestream << "thetaRangeTrain = [[10, 170]]" << std::endl;}
  savestream << "# charge/Pt region for which MLP is trained in degree for all networks. " << std::endl;
  savestream << "#   Can be larger than use range to avoid edge effects. " << std::endl;
  if (checkarr(invptRangeTrain)) {
    savestream << print2dArray<float>("invptRangeTrain", invptRangeTrain);
    savestream  << std::endl;
  } else {savestream << "invptRangeTrain = [[-5, 5]]" << std::endl;}
  savestream << "# Number of nodes in each hidden layer for all networks" << std::endl;
  savestream << "#  or factor to multiply with number of inputs." << std::endl;
  savestream << "#  The number of layers is derived from the shape." << std::endl;
  if (checkarr(nHidden)) {
    savestream << print2dArray<float>("nHidden", nHidden);
    savestream  << std::endl;
  } else {savestream << "nHidden = [[81]]" << std::endl;}
  savestream << "# Maximum number of hits in a single super layer for all networks. " << std::endl;
  if (checkarr(maxHitsPerSL)) {
    savestream << print1dArray<unsigned short>("maxHitsPerSL", maxHitsPerSL);
    savestream  << std::endl;
  } else {savestream << "maxHitsPerSL = [1]" << std::endl;}
  savestream << "# Output scale for all networks. " << std::endl;
  if (checkarr(outputScale)) {
    savestream << print2dArray<float>("outputScale", outputScale);
    savestream  << std::endl;
  } else {savestream << "outputScale = [[-100, 100, 10, 170]]" << std::endl;}
  savestream << "# Super layer pattern for which MLP is trained for all networks." << std::endl;
  savestream << "#   Binary pattern of 9 * maxHitsPerSL bits (on/off for each hit)." << std::endl;
  savestream << "#   0 in bit <i>: hits from super layer <i> are not used." << std::endl;
  savestream << "#   1 in bit <i>: hits from super layer <i> are used." << std::endl;
  savestream << "#   SLpattern = 0: use any hits present, don't check the pattern. " << std::endl;
  if (checkarr(SLpattern)) {
    savestream << print1dArray<unsigned long>("SLpattern", SLpattern);
    savestream  << std::endl;
  } else {savestream << "SLpattern = [511, 383, 479, 503, 509]" << std::endl;}
  savestream << "# Super layer pattern mask for which MLP is trained for all networks." << std::endl;
  savestream << "#   Binary pattern of 9 * maxHitsPerSL bits (on/off for each hit)." << std::endl;
  savestream << "#   0 in bit <i>: super layer <i> may or may not have a hit." << std::endl;
  savestream << "#   1 in bit <i>: super layer <i>" << std::endl;
  savestream << "#                 - must have a hit if SLpattern bit <i> = 1" << std::endl;
  savestream << "#                 - must not have a hit if SLpattenr bit <i> = 0 " << std::endl;
  if (checkarr(SLpatternMask)) {
    savestream << print1dArray<unsigned long>("SLpatternMask", SLpatternMask);
    savestream  << std::endl;
  } else {savestream << "SLpatternMask = [170]" << std::endl;}
  savestream << "# precision used for the hardware simulation " << std::endl;
  if (checkarr(precision)) {
    savestream << print1dArray<unsigned>("precision", precision);
    savestream  << std::endl;
  } else {savestream << "precision = [12, 8, 8, 12, 10, 10]" << std::endl;}
  savestream.close();
}

template<typename X>
std::string NeuroTriggerParameters::print2dArray(const std::string& name, std::vector<std::vector<NNTParam<X>>> vecvec)
{
  // this is a class for piping the output of a 2d array to a string with brackets
  std::stringstream savestream;
  savestream << name << " " << (vecvec[0][0].isLocked() ? "== " : "= ") << "[";
  bool outcomma = false;
  for (auto x : vecvec) {
    if (outcomma) {savestream << ",";}
    outcomma = true;
    savestream << "[";
    bool incomma = false;
    for (auto y : x) {
      if (incomma) {savestream << ",";}
      incomma = true;
      savestream << y;
    }
    savestream << "]";
  }
  savestream << "]" << std::endl;
  return savestream.str();
}
template<typename X>
std::string NeuroTriggerParameters::print1dArray(const std::string& name, std::vector<NNTParam<X>> vecvec)
{
  // this is a class for piping the output of a 1d array to a string with brackets
  std::stringstream savestream;
  savestream << name << " " << (vecvec[0].isLocked() ? "== " : "= ") << "[";
  bool incomma = false;
  for (auto y : vecvec) {
    if (incomma) {savestream << ",";}
    incomma = true;
    savestream << y;
  }
  savestream << "]" << std::endl;
  return savestream.str();
}
template<typename X>
std::vector<std::vector<NNTParam<X>>> NeuroTriggerParameters::read2dArray(std::string keyx, bool locked)
{
  std::vector<std::vector<NNTParam<X>>> retarr;
  std::string key = keyx;
  // parse the brackets here to fill the vector: [[1,2], 3,4]]
  key = key.substr(key.find("[") + 1, std::string::npos); // without outer brackets: [1,2], [3,4]
  for (std::size_t ipos = 0; ipos != std::string::npos; ipos = key.find("[", ipos + 1)) {
    std::string pairstr = key.substr(ipos + 1, key.find("]", ipos + 1) - ipos - 1); // this shopuld be 1,2 now
    std::vector<NNTParam<X>> newpair;
    std::size_t jpos;
    for (jpos = 0; jpos != std::string::npos; jpos = pairstr.find(",", jpos)) {
      if (!(jpos == 0)) {jpos++;}
      newpair.push_back(NNTParam<X>(std::stof(pairstr.substr(jpos, pairstr.find(",") - jpos))));
      if (locked) {newpair.back().lock();}
    }
    retarr.push_back(newpair);
  }
  return retarr;
}

template<typename X>
std::vector<NNTParam<X>> NeuroTriggerParameters::read1dArray(std::string keyx, bool locked)
{
  std::string key = keyx;
  // parse the brackets here to fill the vector: [[1,2], 3,4]]
  std::string pairstr = key.substr(1, key.find("]", 1) - 1); // this shopuld be 1,2 now
  std::vector<NNTParam<X>> newpair;
  std::size_t jpos;
  for (jpos = 0; jpos != std::string::npos; jpos = pairstr.find(",", jpos)) {
    if (!(jpos == 0)) {jpos++;}
    newpair.push_back(NNTParam<X>(std::stof(pairstr.substr(jpos, pairstr.find(",") - jpos))));
    if (locked) {newpair.back().lock();}
  }
  return newpair;
}
