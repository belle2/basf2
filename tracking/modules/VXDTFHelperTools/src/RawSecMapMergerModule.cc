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
#include <tracking/trackFindingVXD/filterTools/SelectionVariableType.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingVXD/environment/VXDTFFiltersHelperFunctions.h>
#include <tracking/trackFindingVXD/filterTools/ObserverCheckMCPurity.h>

#include <vxd/geometry/GeoCache.h>

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

  addParam("mapNames", m_PARAMmapNames, "names of sectorMaps to be loaded.", {""});

  addParam("printFullGraphs", m_PARAMprintFullGraphs,
           "If true, the full trained graphs will be printed to screen. WARNING: produces a lot of output for full detector-cases!",
           bool(false));
}





std::vector<std::string> RawSecMapMergerModule::getRootFiles(std::string mapName)
{
  B2INFO("RawSecMapMerger::getRootFiles(): loading mapName: " << mapName);

  vector<string> files4ThisMap;
  for (string& fileName : m_PARAMrootFileNames) {
    if (fileName.find(mapName) == string::npos) {
      B2DEBUG(1, "getRootFiles: fileName " << fileName << " was _not_ accepted for map " << mapName);
      continue;
    }
    B2DEBUG(1, "getRootFiles: fileName " << fileName << " accepted for map " << mapName);
    files4ThisMap.push_back(fileName);
  }
  return files4ThisMap;
}





std::unique_ptr<TChain> RawSecMapMergerModule::createTreeChain(const SectorMapConfig& configuration, std::string nHitString)
{
  B2INFO("RawSecMapMerger::createTreeChain(): loading mapName: " << configuration.secMapName << " with extension " << nHitString);
  unique_ptr<TChain> treeChain = unique_ptr<TChain>(new TChain((configuration.secMapName + nHitString).c_str()));

  vector<string> fileList = getRootFiles(configuration.secMapName);
  for (string& file : fileList) { treeChain->Add(file.c_str()); }

  return treeChain;
}





template<class ValueType> std::vector<BranchInterface<ValueType>> RawSecMapMergerModule::getBranches(
      std::unique_ptr<TChain>& chain,
      const std::vector<std::string>& branchNames)
{
  vector<BranchInterface<ValueType>> branches;
  B2INFO("RawSecMapMerger::getBranches(): loading branches: " << branchNames.size());
  unsigned nBranches = branchNames.size();

  branches.resize(nBranches, BranchInterface<ValueType>());
  for (unsigned fPos = 0; fPos < nBranches; fPos++) {
    branches[fPos].name = branchNames[fPos];
    chain->SetBranchAddress(
      branches[fPos].name.c_str(),
      &(branches[fPos].value),
      &(branches[fPos].branch));
  }
  B2INFO("RawSecMapMerger::getBranches():  done");
  return branches;
}





std::string RawSecMapMergerModule::prepareNHitSpecificStuff(
  unsigned nHits,
  const SectorMapConfig& config,
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
          " branches");
  if (nEntries == 0) { B2WARNING("trainGraph: valid file but no data stored!"); return; }

  auto percentMark = nEntries / 10; auto progressCounter = 0;
  // log all sector-combinations and determine their absolute number of appearances:
  for (auto i = 0 ;  i <= nEntries; i++) {
    if (percentMark < 1 or (i % percentMark) == 0) {
      B2INFO("RawSecMapMerger::trainGraph(): with mark: " << percentMark << " and i=" << i << ", " << progressCounter <<
             "% related, mainGraph has got " << mainGraph.size() << " sectors...");
      progressCounter += 10;
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
         " branches");

  // creating main graph containing all subgraphs:
  vector<string> filterNames;
  for (auto& entry : filterBranches) { filterNames.push_back(entry.name); }
  SectorGraph<FilterType> mainGraph(filterNames);

  if (nEntries == 0) { B2WARNING("buildGraph: valid file but no data stored!"); return mainGraph; }
  auto percentMark = nEntries / 10;
  auto progressCounter = 0;

  // log all sector-combinations and determine their absolute number of appearances:
  for (auto i = 0 ;  i <= nEntries; i++) {
    if (percentMark < 1 or (i % percentMark) == 0) {
      B2INFO("RawSecMapMerger::buildGraph(): with mark: " << percentMark << " and i=" << i << ", " << progressCounter <<
             "% related, mainGraph has got " << mainGraph.size() << " sectors...");
      progressCounter += 10;
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

  B2INFO("RawSecMapMerger::buildGraph(): mainGraph finished - has now size: " << mainGraph.size());
  B2DEBUG(1, "fullGraph-Print: " << mainGraph.print());

  return mainGraph;
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
         " sector-/filter-branches");

  for (unsigned i = 0 ;  i < nEntries; i++) {
    if (percentMark > 1 and (i % percentMark) != 0) { continue; }
    progressCounter += 2;
    B2INFO("RawSecMapMerger::printData(): entry " << i << " of " << nEntries << ":");

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
    B2INFO(out << "\n");
  }
}





void RawSecMapMergerModule::printVXDTFFilters(const VXDTFFilters<SpacePoint>& filters,
                                              std::string configName, unsigned int nHitCombinations, bool print2File)
{
  SecMapHelper::printStaticSectorRelations<SpacePoint>(filters , configName , nHitCombinations, print2File);
}


///// removed and replaced by a version that gets all sensor ids from the geometry
///// TODO: remove
//std::vector<VxdID> RawSecMapMergerModule::getCompatibleVxdIDs(const SectorMapConfig& config)
//{
//
//  // TODO: remove that part and use the version in the bootstrap module
//
//  // TODO WARNING hardcoded values!
//  std::vector<unsigned> layers  = { 0, 1, 2, 3, 4, 5, 6};
//  std::vector<unsigned> ladders = { 0, 8, 12, 7, 10, 12, 16};
//  std::vector<unsigned> sensors = { 0, 2, 2, 2, 3, 4, 5};
//
//  std::vector<VxdID> vxdIDs;
//
//  for (unsigned layerID : config.allowedLayers) {
//    for (unsigned ladderID = 0; ladderID <= ladders.at(layerID); ladderID++) {
//      if (ladderID == 0 and layerID != 0) continue; // only virtual IP (layer 0) has ladder 0
//      for (unsigned sensorID = 0; sensorID <= sensors.at(layerID); sensorID++) {
//        if (sensorID == 0 and layerID != 0) continue; // only virtual IP (layer 0) has sensor 0
//        vxdIDs.push_back(VxdID(layerID, ladderID, sensorID));
//      }
//    }
//  }
//  return vxdIDs;
//}



template <class FilterType> unsigned RawSecMapMergerModule::addAllSectorsToSecMapThingy(const SectorMapConfig& config,
    SectorGraph<FilterType>& mainGraph, VXDTFFilters<SpacePoint>& segFilters)
{
  // get all VXD sensors in the geometry
  // WARNING: if a different geometry in the first training step was used this may lead to difficulties
  std::vector<VxdID> vxdIDs = VXD::GeoCache::getInstance().getListOfSensors();


  // collect all secIDs occured in training and use them to update the sectors in the SectorID in the VXDTFFilter
  // in particular the sublayerID which is determined from the graph
  for (VxdID sensor : vxdIDs) {

    std::vector< FullSecID> allTrainedSecIDsOfSensor = mainGraph.getAllFullSecIDsOfSensor(sensor);

    // this removes all FullSecIDs which occured more than once
    std::sort(allTrainedSecIDsOfSensor.begin(), allTrainedSecIDsOfSensor.end());
    allTrainedSecIDsOfSensor.erase(
      std::unique(
        allTrainedSecIDsOfSensor.begin(),
        allTrainedSecIDsOfSensor.end()),
      allTrainedSecIDsOfSensor.end());

    // lambda for finding the correct sector-position
    auto findSector = [](std::vector< FullSecID>& secIDs, int counter) {
      std::vector< FullSecID>::iterator iter = secIDs.begin();
      for (; iter != secIDs.end(); ++iter) {
        if (iter->getSecID() == counter) return iter;
      }
      return secIDs.end();
    };

    for (FullSecID sector : allTrainedSecIDsOfSensor) {
      // the search within that function will ignore the sublayerid
      segFilters.setSubLayerIDs(sector, sector.getSubLayerID());
    }

    B2DEBUG(1, "Sensor: " << sensor << " had " << allTrainedSecIDsOfSensor.size() << " trained IDs and ");
  } // end loop sensor of vxdIDs.


  // Thomas : removed that one as it is already added during bootstrapping
  // and add the virtual IP:
  //std::vector<double> uCuts4vIP = {}, vCuts4vIP = {};
  //allSecIDsOfThisSensor.clear();
  //allSecIDsOfThisSensor = {{0}};
  //segFilters.addSectorsOnSensor(uCuts4vIP, vCuts4vIP, allSecIDsOfThisSensor);

  return vxdIDs.size() + 1;
}





// TODO this is not yet capable of dealing with other than twoHitFilters. -> generalize!
template <class FilterType> void RawSecMapMergerModule::getSegmentFilters(
  const SectorMapConfig& config,
  SectorGraph<FilterType>& mainGraph,
  VXDTFFilters<SpacePoint>* xHitFilters,
  int nSecChainLength)
{

  // Thomas : possible bug, the sublayer id s have been updated only for the nSecChainLength==2 case
  /*
  if (xHitFilters->size() == 0) {
    unsigned nSectors = addAllSectorsToSecMapThingy(config, mainGraph, *xHitFilters);
    B2DEBUG(1, "RawSecMapMerger::getSegmentFilters: in addAllSectorsToSecMapThingy " << nSectors << " were added to secMap " <<
            config.secMapName);
  } else {
    B2DEBUG(1, "RawSecMapMerger::getSegmentFilters: in given xHitFilters-container has size of " << xHitFilters->size() <<
            " and therefore no further sectors have to be added.");
  }
  */
  // after rewriting this function only updates the sublayer ids of the already existing sectors
  // so it should only be executed once!!
  // TODO: remove the if by a better construction!! Also what happens if for the nSecChainLength>2 case the sublayerids need updates???

  // if( nSecChainLength == 2 ) addAllSectorsToSecMapThingy(config, mainGraph, *xHitFilters);

  B2DEBUG(1, "RawSecMapMerger::getSegmentFilters: secMap " << config.secMapName << " got the following sectors:\n" <<
          mainGraph.print());


  for (auto& subGraph : mainGraph) {

    if (nSecChainLength == 2) {
      add2HitFilters(*xHitFilters, subGraph.second, config);
    } else if (nSecChainLength == 3) {
      add3HitFilters(*xHitFilters, subGraph.second, config);
    } else if (nSecChainLength == 4) {
      add4HitFilters(*xHitFilters, subGraph.second, config);
    } else { B2FATAL("nSecChainLength " << nSecChainLength << " is not within allowed range [2;4]!"); }
  }
}





template <class FilterType> void RawSecMapMergerModule::add2HitFilters(VXDTFFilters<SpacePoint>&
    filterContainer, SubGraph<FilterType>& subGraph, const SectorMapConfig& config)
{
// //   // WARNING evil hack -> SelectionVariables themselves should be able to tell their own names!
// //   std::string named3D = "Distance3DSquared", namedXY = "Distance2DXYSquared", nameddZ = "Distance1DZ", namesRZ = "SlopeRZ",
// //               named3Dn = "Distance3DNormed";
  const auto& filterCutsMap = subGraph.getFinalQuantileValues();
  /// TODO tune cutoffs

  std::string filterVals;
  for (auto& filterName : config.twoHitFilters) {
    filterVals += filterName + ": "
                  + std::to_string(filterCutsMap.at(filterName).getMin())
                  + "/"
                  + std::to_string(filterCutsMap.at(filterName).getMax()) + ", ";
  }
  B2DEBUG(1, "SubGraph " << subGraph.getID().print() << " - filter:min/max: " << filterVals);

//   B2DEBUG(1, "SubGraph " << subGraph.getID().print() << " - min/max: " << named3D << ": " << filterCutsMap.at(named3D).getMin() << "/" << filterCutsMap.at(
//  named3D).getMax() << ", ")
  // // // //     B2DEBUG(1, namedXY << ": " << filterCutsMap.at(namedXY).getMin() << "/" << filterCutsMap.at(namedXY).getMax() << ", ")
  // // // //     B2DEBUG(1,  namesRZ << ": " << filterCutsMap.at(namesRZ).getMin() << "/" << filterCutsMap.at(namesRZ).getMax() << ", ")


  /// JKL Jan 2016 - minimal working example:
//   VXDTFFilters<SpacePoint>::twoHitFilter_t friendSectorsSegmentFilter =
//   ((filterCutsMap.at("Distance3DSquared").getMin() <= Distance3DSquared<SpacePoint>() <= filterCutsMap.at("Distance3DSquared").getMax()).observe(ObserverPrintResults()));


  /// JKL Feb 2016 - big working example:
//   VXDTFFilters<SpacePoint>::twoHitFilter_t friendSectorsSegmentFilter =
//     (
//       (filterCutsMap.at("Distance3DSquared").getMin() <= Distance3DSquared<SpacePoint>() <=
//        filterCutsMap.at("Distance3DSquared").getMax()).observe(VoidObserver()) &&
//       (filterCutsMap.at("Distance2DXYSquared").getMin() <= Distance2DXYSquared<SpacePoint>() <=
//        filterCutsMap.at("Distance2DXYSquared").getMax()).observe(VoidObserver()) &&
//       (filterCutsMap.at("Distance1DZ").getMin() <= Distance1DZ<SpacePoint>() <= filterCutsMap.at("Distance1DZ").getMax()).observe(
//         VoidObserver()) &&
//       (filterCutsMap.at("SlopeRZ").getMin() <= SlopeRZ<SpacePoint>() <= filterCutsMap.at("SlopeRZ").getMax()).observe(VoidObserver()) &&
//       (filterCutsMap.at("Distance3DNormed").getMin() <= Distance3DNormed<SpacePoint>() <=
//        filterCutsMap.at("Distance3DNormed").getMax()).observe(VoidObserver())
//     );
  VXDTFFilters<SpacePoint>::twoHitFilter_t friendSectorsSegmentFilter =
    (
      (
        (filterCutsMap.at("Distance3DSquared").getMin() <= Distance3DSquared<SpacePoint>() <=
         filterCutsMap.at("Distance3DSquared").getMax())  &&
        (filterCutsMap.at("Distance2DXYSquared").getMin() <= Distance2DXYSquared<SpacePoint>() <=
         filterCutsMap.at("Distance2DXYSquared").getMax()) &&
        (filterCutsMap.at("Distance1DZ").getMin() <= Distance1DZ<SpacePoint>() <= filterCutsMap.at("Distance1DZ").getMax()) &&
        (filterCutsMap.at("SlopeRZ").getMin() <= SlopeRZ<SpacePoint>() <= filterCutsMap.at("SlopeRZ").getMax()) &&
        (filterCutsMap.at("Distance3DNormed").getMin() <= Distance3DNormed<SpacePoint>() <=
         filterCutsMap.at("Distance3DNormed").getMax())
      )
    );



  // JKL Jan 2016 - standard version:
  // // // // // // //       (
  // // // // // // //         (
  // // // // // // //           filterCutsMap.at(named3D).getMin() <= Distance3DSquared<SpacePoint>() <= filterCutsMap.at(named3D).getMax()
  // // // // // // //         ).observe(Observer()).enable() &&
  // // // // // // //
  // // // // // // //         (
  // // // // // // //           filterCutsMap.at(namedXY).getMin() <= Distance2DXYSquared<SpacePoint>() <= filterCutsMap.at(namedXY).getMax()
  // // // // // // //         ).observe(Observer()).enable() &&
  // // // // // // //
  // // // // // // // //    (
  // // // // // // // //    filterCutsMap.at(nameddZ).getMin() <
  // // // // // // // //    Distance1DZ<SpacePoint>() <
  // // // // // // // //    filterCutsMap.at(nameddZ).getMax()
  // // // // // // // //    )/*.observe(Observer())*/.enable() &&
  // // // // // // //         ( /// WARNING HACK:
  // // // // // // //           /*filterCutsMap.at(named3D).getMin()*/std::numeric_limits< double >::min() <= Distance1DZ<SpacePoint>() <=
  // // // // // // //           std::numeric_limits< double >::max()/*filterCutsMap.at(named3D).getMax()*/
  // // // // // // //         )/*.observe(Observer())*/.enable() &&
  // // // // // // //
  // // // // // // //         (
  // // // // // // //           filterCutsMap.at(namesRZ).getMin() <= SlopeRZ<SpacePoint>() <= filterCutsMap.at(namesRZ).getMax()
  // // // // // // //         ).observe(Observer()).enable() &&
  // // // // // // //
  // // // // // // // //    (
  // // // // // // // //    Distance3DNormed<SpacePoint>() <
  // // // // // // // //    filterCutsMap.at(named3Dn).getMax()
  // // // // // // // //    ).enable()
  // // // // // // //         (Distance3DNormed<SpacePoint>() <= std::numeric_limits< double >::max()/*filterCutsMap.at(named3D).getMax()*/).enable()
  // // // // // // //       );

  auto secIDs = subGraph.getID().getFullSecIDs();

  // secIDs are sorted from outer to inner:
  B2DEBUG(1, "RawSecMapMerger::add2HitFilters: now adding FriendSectorFilter for secIDs (outer/inner): " << secIDs.at(
            0) << "/" << secIDs.at(1));
  if (filterContainer.addTwoHitFilter(secIDs.at(0), secIDs.at(1),
                                      friendSectorsSegmentFilter) == 0)
    B2WARNING("secMap: " << config.secMapName << "Problem adding the friendship relation from the inner sector:" <<
              secIDs.at(1) << " -> " << secIDs.at(0) << " outer sector");
}



template <class FilterType> void RawSecMapMergerModule::add3HitFilters(VXDTFFilters<SpacePoint>&
    filterContainer, SubGraph<FilterType>& subGraph, const SectorMapConfig&  config)
{
  const auto& filterCutsMap = subGraph.getFinalQuantileValues();
  /// TODO tune cutoffs

  std::string filterVals;
  for (auto& filterName : config.threeHitFilters) {
    filterVals += filterName + ": "
                  + std::to_string(filterCutsMap.at(filterName).getMin())
                  + "/"
                  + std::to_string(filterCutsMap.at(filterName).getMax()) + ", ";
  }
  B2DEBUG(1, "SubGraph " << subGraph.getID().print() << " - filter:min/max: " << filterVals);


  /// JKL Jan 2016 - minimal working example:
//   VXDTFFilters<SpacePoint>::threeHitFilter_t threeHitFilter =
//     (
//       (filterCutsMap.at("Angle3DSimple").getMin() <= Angle3DSimple<SpacePoint>() <= filterCutsMap.at("Angle3DSimple").getMax()).observe(Observer3HitPrintResults())
//       );


  /// JKL Feb 2016 - big working example:
  VXDTFFilters<SpacePoint>::threeHitFilter_t threeHitFilter =
    (
      (filterCutsMap.at("Angle3DSimple").getMin() <= Angle3DSimple<SpacePoint>() <= filterCutsMap.at("Angle3DSimple").getMax()) &&
      (filterCutsMap.at("CosAngleXY").getMin() <= CosAngleXY<SpacePoint>() <= filterCutsMap.at("CosAngleXY").getMax()) &&
      (filterCutsMap.at("AngleRZSimple").getMin() <= AngleRZSimple<SpacePoint>() <= filterCutsMap.at("AngleRZSimple").getMax()) &&
      (CircleDist2IP<SpacePoint>() <= filterCutsMap.at("CircleDist2IP").getMax()) &&
      (filterCutsMap.at("DeltaSlopeRZ").getMin() <= DeltaSlopeRZ<SpacePoint>()) <= filterCutsMap.at("DeltaSlopeRZ").getMax() &&
      (filterCutsMap.at("DeltaSlopeZoverS").getMin() <= DeltaSlopeZoverS<SpacePoint>() <=
       filterCutsMap.at("DeltaSlopeZoverS").getMax())  &&
      (filterCutsMap.at("DeltaSoverZ").getMin() <= DeltaSoverZ<SpacePoint>() <= filterCutsMap.at("DeltaSoverZ").getMax()) &&
      (filterCutsMap.at("HelixParameterFit").getMin() <= HelixParameterFit<SpacePoint>() <=
       filterCutsMap.at("HelixParameterFit").getMax()) &&
      (filterCutsMap.at("Pt").getMin() <= Pt<SpacePoint>() <= filterCutsMap.at("Pt").getMax()) &&
      (filterCutsMap.at("CircleRadius").getMin() <= CircleRadius<SpacePoint>() <= filterCutsMap.at("CircleRadius").getMax())

    ).observe(VoidObserver());


  auto secIDs = subGraph.getID().getFullSecIDs();

  // secIDs are sorted from outer to inner:
  B2DEBUG(1, "RawSecMapMerger::add3HitFilters: now adding FriendSectorFilter for secIDs (outer/center/inner): "
          << secIDs.at(0) << "/"
          << secIDs.at(1) << "/"
          << secIDs.at(2));
  if (filterContainer.addThreeHitFilter(secIDs.at(0), secIDs.at(1), secIDs.at(2),
                                        threeHitFilter) == 0)
    B2WARNING("secMap: " << config.secMapName << "Problem adding the friendship relation for the secIDs (outer/center/inner): "
              << secIDs.at(0) << "/"
              << secIDs.at(1) << "/"
              << secIDs.at(2));
}



template <class FilterType> void RawSecMapMergerModule::add4HitFilters(
  VXDTFFilters<SpacePoint>& /*filterContainer*/, SubGraph<FilterType>& /*subGraph*/,
  const SectorMapConfig&  /*config*/)
{

}








