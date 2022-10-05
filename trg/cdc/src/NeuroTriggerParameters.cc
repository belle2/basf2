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
    key = (locked) ? line.substr((line.find('=') + 2), line.length() - line.find('=') - 2) : line.substr((line.find('=') + 1),
          line.length() - line.find('=') - 1);
    if (par == "nInput")         {
      nInput = std::stoul(key);
      if (locked) {nInput.lock();}
    } else if (par == "nOutput")        {
      nOutput = std::stoul(key);
      if (locked) {nOutput.lock();}
    } else if (par == "nMLP")           {
      nMLP = std::stoul(key);
      if (locked) {nMLP.lock();}
    } else if (par == "targetZ")        {
      targetZ = std::stoi(key);
      if (locked) {targetZ.lock();}
    } else if (par == "targetTheta")    {
      targetTheta = std::stoi(key);
      if (locked) {targetTheta.lock();}
    } else if (par == "tMax")           {
      tMax = std::stoul(key);
      if (locked) {tMax.lock();}
    } else if (par == "ETOption")       {
      ETOption = std::stoul(key);
      if (locked) {ETOption.lock();}
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
    } else if (par == "maxHitsperSL")    {
      maxHitsperSL = read1dArray<unsigned short>(key, locked);
    } else if (par == "outputScale")    {
      outputScale = read1dArray<float>(key, locked);
    } else if (par == "SLPattern")    {
      SLPattern = read1dArray<unsigned long>(key, locked);
    } else if (par == "SLPatternMask")    {
      SLPatternMask = read1dArray<unsigned long>(key, locked);
    } else if (par == "precision")    {
      precision = read1dArray<unsigned>(key, locked);
    }
  }
}
template<typename X>
bool NeuroTriggerParameters::checkarr(std::vector<std::vector<NNTParam<X>>> vec)
{
  if (vec.size() < 1) {return false;}
  else if (vec[0].size() < 1) {return false;}
  else {return vec[0][0].isSet();}
}

template<typename X>
bool NeuroTriggerParameters::checkarr(std::vector<NNTParam<X>> vec)
{
  if (vec.size() < 1) {return false;}
  else {return vec[0].isSet();}
}

void NeuroTriggerParameters::saveconfigtxt(const std::string& filename)
{
  std::ofstream savestream;
  savestream.open(filename);
  savestream << "########################################################" << std::endl;
  savestream << "### Neurotrigger configuration file created by basf2 ###" << std::endl;
  savestream << "########################################################" << std::endl << std::endl;
  if (nOutput.isSet()) {savestream << "nOutput " << (nOutput.isLocked() ? "== " : "= ") << nOutput << std::endl;}
  if (nInput.isSet()) {savestream << "nInput " << (nInput.isLocked() ? "== " : "= ") << nInput << std::endl;}
  if (nMLP.isSet()) {savestream << "nMLP " << (nMLP.isLocked() ? "== " : "= ") << nMLP << std::endl;}
  if (targetZ.isSet()) {savestream << "targetZ " << (targetZ.isLocked() ? "== " : "= ") << targetZ << std::endl;}
  if (targetTheta.isSet()) {savestream << "targetTheta " << (targetTheta.isLocked() ? "== " : "= ") << targetTheta << std::endl;}
  if (tMax.isSet()) {savestream << "tMax " << (tMax.isLocked() ? "== " : "= ") << tMax << std::endl;}
  if (ETOption.isSet()) {savestream << "ETOption " << (ETOption.isLocked() ? "== " : "= ") << ETOption << std::endl;}

  if (checkarr(phiRangeUse)) {savestream << print2dArray<float>("phiRangeUse", phiRangeUse);}
  if (checkarr(thetaRangeUse)) {savestream << print2dArray<float>("thetaRangeUse", thetaRangeUse);}
  if (checkarr(invptRangeUse)) {savestream << print2dArray<float>("invptRangeUse", invptRangeUse);}
  if (checkarr(phiRangeTrain)) {savestream << print2dArray<float>("phiRangeTrain", phiRangeTrain);}
  if (checkarr(thetaRangeTrain)) {savestream << print2dArray<float>("thetaRangeTrain", thetaRangeTrain);}
  if (checkarr(invptRangeTrain)) {savestream << print2dArray<float>("invptRangeTrain", invptRangeTrain);}
  if (checkarr(nHidden)) {savestream << print2dArray<float>("nHidden", nHidden);}
  if (checkarr(maxHitsperSL)) {savestream << print1dArray<unsigned short>("maxHitsperSL", maxHitsperSL);}
  if (checkarr(outputScale)) {savestream << print1dArray<float>("outputScale", outputScale);}
  if (checkarr(SLPattern)) {savestream << print1dArray<unsigned long>("SLPattern", SLPattern);}
  if (checkarr(SLPatternMask)) {savestream << print1dArray<unsigned long>("SLPatternMask", SLPatternMask);}
  if (checkarr(precision)) {savestream << print1dArray<unsigned>("precision", precision);}
  savestream.close();
}

template<typename X>
std::string NeuroTriggerParameters::print2dArray(const std::string& name, std::vector<std::vector<NNTParam<X>>> vecvec)
{
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
    //newpair.push_back(NNTParam<X>(std::stof(pairstr.substr(jpos, pairstr.length()-jpos))));
    //if (locked) {newpair.back().lock();}
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

