/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/VXDTFHelperTools/RawSecMapMergerModule.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(RawSecMapMerger)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

RawSecMapMergerModule::RawSecMapMergerModule() : Module()
{
  //Set module properties
  setDescription("this module takes a root file containing a raw sectorMap created by the SecMapTrainerBaseModule and converts it to a sectormap which can be read by the VXDTF. Please check the parameters to be set...");
//   setPropertyFlags(c_ParallelProcessingCertified); /// WARNING this module should _not_ be used for parallel processing! Its task is to create the sector maps only once...


  addParam("rootFileNames", m_PARAMrootFileNames,
           "List of files (wildcards not allowed - use python glob.glob() to expand to list of files)", {"lowTestRedesign_454970355.root"});

  addParam("mapNames", m_PARAMmapNames, "names of sectorMaps to be loaded. ", {""});
}





std::vector<std::string> RawSecMapMergerModule::getRootFiles(std::string mapName)
{
  B2INFO("RawSecMapMerger::getRootFiles(): loading mapName: " << mapName)

  vector<string> files4ThisMap;
  for (string& fileName : m_PARAMrootFileNames) {
    if (fileName.find(mapName) == string::npos) {
      B2DEBUG(1, "getRootFiles: fileName " << fileName << " was _not_ for map " << mapName)
      continue;
    }
    B2DEBUG(1, "getRootFiles: fileName " << fileName << " accepted for map " << mapName)
    files4ThisMap.push_back(fileName);
  }
  return move(files4ThisMap);
}





std::unique_ptr<TChain> RawSecMapMergerModule::createTreeChain(TrainerConfigData& configuration, std::string nHitString)
{
  B2INFO("RawSecMapMerger::createTreeChain(): loading mapName: " << configuration.secMapName << " with extension " << nHitString)
  unique_ptr<TChain> treeChain = unique_ptr<TChain>(new TChain((configuration.secMapName + nHitString).c_str()));

  vector<string> fileList = getRootFiles(configuration.secMapName);
  for (string& file : fileList) { treeChain->Add(file.c_str()); }

  return move(treeChain);
}





template<class ValueType> std::vector<BranchInterface<ValueType>> RawSecMapMergerModule::getBranches(
      std::unique_ptr<TChain>& chain,
      const std::vector<std::string>& branchNames)
{
  vector<BranchInterface<ValueType>> branches;
  B2INFO("RawSecMapMerger::getBranches(): loading branches: " << branchNames.size())
  unsigned nBranches = branchNames.size();

  branches.resize(nBranches, BranchInterface<ValueType>());
  for (unsigned fPos = 0; fPos < nBranches; fPos++) {
    branches[fPos].name = branchNames[fPos];
    chain->SetBranchAddress(
      branches[fPos].name.c_str(),
      &(branches[fPos].value),
      &(branches[fPos].branch));
  }
  B2INFO("RawSecMapMerger::getBranches():  done")
  return move(branches);
}





std::string RawSecMapMergerModule::prepareNHitSpecificStuff(
  unsigned nHits,
  TrainerConfigData& config,
  std::vector<std::string>& secBranchNames,
  std::vector<std::string>& filterBranchNames)
{
  if (nHits == 2) {
    secBranchNames = { "outerSecID", "innerSecID"};
    filterBranchNames = config.twoHitFilters;
    return "2Hit";
  }

  if (nHits == 3) {
    secBranchNames = { "outerSecID", "centerSecID", "innerSecID"};
    filterBranchNames = config.threeHitFilters;
    return "3Hit";
  }

  if (nHits == 4) {
    secBranchNames = { "outerSecID", "outerCenterSecID", "innerCenterSecID", "innerSecID"};
    filterBranchNames = config.fourHitFilters;
    return "4Hit";
  }

  B2ERROR("prepareNHitSpecificStuff: wrong chainLength!");
  return "";
}




template <class FilterType> void RawSecMapMergerModule::trainGraph(
  SectorGraph<FilterType>& mainGraph,
  std::unique_ptr<TChain>& chain,
  std::vector<BranchInterface<unsigned>>& sectorBranches,
  std::vector<BranchInterface<double>>& filterBranches)
{
  auto nEntries = chain->GetEntries();
  B2DEBUG(10, "RawSecMapMerger::trainGraph():  start of " << nEntries << " entries in tree and " << sectorBranches.size() <<
          " branches")
  if (nEntries == 0) { B2WARNING("trainGraph: valid file but no data stored!"); return; }

  auto percentMark = nEntries / 10; auto progressCounter = 0;
  // log all sector-combinations and determine their absolute number of appearances:
  for (auto i = 0 ;  i <= nEntries; i++) {
    if ((i % percentMark) == 0) {
      progressCounter += 10;
      B2INFO("RawSecMapMerger::trainGraph(): with mark: " << percentMark << " and i=" << i << ", " << progressCounter <<
             "% related, mainGraph has got " << mainGraph.size() << " sectors...")
    }
    auto thisEntry = chain->LoadTree(i);

    auto ids = getSecIDs(sectorBranches, thisEntry);
    auto currentID = SubGraphID(ids);

    auto pos = mainGraph.find(currentID);

    if (pos == mainGraph.end()) { B2WARNING("trainGraph: could not find subgraph " << currentID.print() << " - skipping entry..."); continue; }

    for (auto& filter : filterBranches) {
      filter.update(thisEntry);
      pos->second.addValue(FilterType(filter.name), filter.value);
    }
  } // entry-loop-end
}





template <class FilterType> SectorGraph<FilterType> RawSecMapMergerModule::buildGraph(
  std::unique_ptr<TChain>& chain,
  std::vector<BranchInterface<unsigned>>& sectorBranches,
  std::vector<BranchInterface<double>>& filterBranches)
{
  auto nEntries = chain->GetEntries();
  B2INFO("RawSecMapMerger::buildGraph():  start of " << nEntries << " entries in tree and " << sectorBranches.size() <<
         " branches")

  // creating main graph containing all subgraphs:
  vector<string> filterNames;
  for (auto& entry : filterBranches) { filterNames.push_back(entry.name); }
  SectorGraph<FilterType> mainGraph(filterNames);

  if (nEntries == 0) { B2WARNING("buildGraph: valid file but no data stored!"); return move(mainGraph); }
  auto percentMark = nEntries / 10;
  auto progressCounter = 0;

  // log all sector-combinations and determine their absolute number of appearances:
  for (auto i = 0 ;  i <= nEntries; i++) {
    if ((i % percentMark) == 0) {
      progressCounter += 10;
      B2INFO("RawSecMapMerger::buildGraph(): with mark: " << percentMark << " and i=" << i << ", " << progressCounter <<
             "% related, mainGraph has got " << mainGraph.size() << " sectors...")
    }
    auto thisEntry = chain->LoadTree(i);

    auto ids = getSecIDs(sectorBranches, thisEntry);
    auto currentID = SubGraphID(ids);
    B2DEBUG(10, "buildGraph-SubGraphID-print: id: " << currentID.print());

    auto pos = mainGraph.find(currentID);

    if (pos == mainGraph.end()) { pos = mainGraph.add(currentID); }

    if (pos == mainGraph.end()) { B2WARNING("could not find nor add subgraph - skipping entry..."); continue; }

    pos->second.wasFound();

    for (auto& filter : filterBranches) {
      filter.update(thisEntry);
      pos->second.checkAndReplaceIfMinMax(FilterType(filter.name), filter.value);
    }
  } // entry-loop-end

  B2INFO("RawSecMapMerger::buildGraph(): mainGraph finished - has now size: " << mainGraph.size())
  B2DEBUG(1, "fullGraph-Print: " << mainGraph.print());

  return move(mainGraph);
}





void RawSecMapMergerModule::printData(
  std::unique_ptr<TChain>& chain,
  std::vector<BranchInterface<unsigned>>& sectorBranches,
  std::vector<BranchInterface<double>>& filterBranches)
{
  // prepare everything:
  unsigned nEntries = chain->GetEntries();
  unsigned percentMark = 1;
  if (nEntries > 100) { percentMark = nEntries / 50; }
  unsigned progressCounter = 0;

  B2INFO("RawSecMapMerger::printData():  start of " << nEntries <<
         " entries in tree and " << sectorBranches.size() <<
         "/" << filterBranches.size() <<
         " sector-/filter-branches")

  for (unsigned i = 0 ;  i < nEntries; i++) {
    if ((i % percentMark) != 0) { continue; }
    progressCounter += 2;
    B2INFO("RawSecMapMerger::printData(): entry " << i << " of " << nEntries << ":")

    auto thisEntry = chain->LoadTree(i);

    string out;
    for (unsigned i = 0 ; i < sectorBranches.size(); i++) {
      sectorBranches[i].branch->GetEntry(thisEntry);
      out += sectorBranches[i].name + ": " + FullSecID(sectorBranches[i].value).getFullSecString() + ". ";
    }
    out += "\n";

    for (unsigned i = 0 ; i < filterBranches.size(); i++) {
      filterBranches[i].branch->GetEntry(thisEntry);
      out += filterBranches[i].name + ": " + to_string(filterBranches[i].value) + ". ";
    }
    B2INFO(out << "\n")
  }
}
