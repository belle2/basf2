/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Moritz Nadler                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/VXDTFHelperTools/RawSecMapMergerWithSpacePointsModule.h>
#include <tracking/dataobjects/VXDTFSecMap.h>
#include <tracking/dataobjects/SecMapVector.h> // needed for rootImport
#include <tracking/dataobjects/VXDTFRawSecMap.h> // needed for rootImport
#include <tracking/dataobjects/FullSecID.h>
#include <framework/gearbox/Const.h>
#include <tracking/dataobjects/FilterID.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/Stream.h> // to be able to stream TObjects into xml-coded files

#include <fstream>
#include <iostream>

// #include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <TVector3.h>
#include <TFile.h>
#include <TKey.h>
#include <TCollection.h>
#include <TTree.h>
#include <TBranch.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(RawSecMapMergerWithSpacePoints)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

RawSecMapMergerWithSpacePointsModule::RawSecMapMergerWithSpacePointsModule() : Module()
{
  //Set module properties
  setDescription("this module takes a root file containing a raw sectorMap created by the SecMapTrainerBaseModule and converts it to a sectormap which can be read by the VXDTF. Please check the parameters to be set...");
//   setPropertyFlags(c_ParallelProcessingCertified); /// WARNING this module should _not_ be used for parallel processing! Its task is to create the sector maps only once...

  /// setting standard values for steering parameters
  std::vector<double> defaultConfigU = {0., 0.5, 1.}; // sector sizes
  std::vector<double> defaultConfigV = {0., 0.33, 0.67, 1.};
  std::vector<double> originVec = {0., 0., 0.};
  std::vector<int> sampleThreshold = {1, 100};
  std::vector<double> smallSampleQuantiles = {0., 1.};
  std::vector<double> sampleQuantiles = {0.001, 0.999};
  std::vector<double> stretchFactor = {0.02, 0.}; // lower stretchFactor: change by 2%
  std::string rootFileName = "FilterCalculatorResults";


  addParam("importROOTorXML", m_PARAMimportROOTorXML,
           "if true, sectorMap is imported via rootFile, if false, it is imported via XML-file", bool(true));


  /// following parameters only needed if importing from xml file (-> importROOTorXML = false):
  addParam("detectorType", m_PARAMdetectorType,
           "only needed if importROOTorXML = false: defines which detector type has to be exported. Allowed values: 'VXD', 'PXD', 'SVD'",
           string("SVD"));
  addParam("sectorSetup", m_PARAMsectorSetup,
           "only needed if importROOTorXML = false: lets you chose the sectorSetup (compatibility of sensors, individual cutoffs,...) accepts 'std', 'low', 'high' and 'personal', please note that the chosen setup has to exist as a xml-file in ../tracking/data/friendList_XXX.xml. If you can not create your own xml files using e.g. the filterCalculatorModule, use params for  'tuneCutoffXXX' or 'setupWeigh' instead. multipass supported by setting setups in a row",
           string("std"));
  addParam("sectorConfigU", m_PARAMsectorConfigU,
           "only needed if importROOTorXML = false: allows defining the the config of the sectors in U direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0",
           defaultConfigU);
  addParam("sectorConfigV", m_PARAMsectorConfigV,
           "only needed if importROOTorXML = false: allows defining the the config of the sectors in V direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0",
           defaultConfigV);
  addParam("setOrigin", m_PARAMsetOrigin,
           "only needed if importROOTorXML = false: standard origin is (0,0,0). If you want to have the map calculated for another origin, set here(x,y,z)",
           originVec);
  addParam("magneticFieldStrength", m_PARAMmagneticFieldStrength,
           "only needed if importROOTorXML = false: set strength of magnetic field in Tesla, standard is 1.5T", double(1.5));

  addParam("additionalInfo", m_PARAMadditionalInfo,
           "only needed if importROOTorXML = false: this variable is reserved for extra info which shall be stored in the container, e.g. date of production or other useful info for the user(it shall be formatted before storing it), this info will be displayed by the VXDTF on Info-level",
           string(""));


  /// following parameters only needed if importing from root file (-> importROOTorXML = true):
  addParam("sampleThreshold", m_PARAMsampleThreshold,
           "only needed if importROOTorXML = true: exactly two entries allowed: first: minimal sample size for sector-combination, second: threshold for 'small samples' where behavior is less strict. If sampleSize is bigger than second, normal behavior is chosen",
           sampleThreshold);

  addParam("filterRareCombinations", m_PARMfilterRareCombinations,
           "only needed if importROOTorXML = true: use this member if you want to steer whether rare sector-friend-combinations shall be filtered or not. Set true if you want to filter these combinations or set false if otherwise. ",
           bool(false));

  addParam("rarenessFilter", m_PARAMrarenessFilter,
           "only needed if importROOTorXML = true: use this member if you want to steer whether rare sector-friend-combinations shall be filtered or not, here you can set the threshold for filter. 100% = 1. 1% = 0.01%. Example: if you choose 0.01, all friendsectors which occur less often than in 1% of all cases when main sector was used, are deleted in the friendship-relations",
           double(0.0));
  addParam("smallSampleQuantiles", m_PARAMsmallSampleQuantiles,
           "only needed if importROOTorXML = true: behiavior of small sample sizes, exactly two entries allowed: first: lower quantile, second: upper quantile. only values between 0-1 are allowed",
           smallSampleQuantiles);

  addParam("sampleQuantiles", m_PARAMsampleQuantiles,
           "only needed if importROOTorXML = true: behiavior of normal sample sizes, exactly two entries allowed: first: lower quantile, second: upper quantile. only values between 0-1 are allowed",
           sampleQuantiles);

  addParam("stretchFactor", m_PARAMstretchFactor,
           "only needed if importROOTorXML = true: exactly two entries allowed: first: stretchFactor for small sample size for sector-combination, second: stretchFactor for normal sample size for sector-combination: WARNING if you simply want to produce wider cutoffs in the VXDTF, please use the tuning parameters there! This parameter here is only if you know what you are doing, since it changes the values in the XML-file directly",
           stretchFactor);

  addParam("sortByDistance2origin", m_PARAMsortByDistance2origin,
           "only needed if importROOTorXML = true: if set to true, sectors are not treated by layerID but by distance to origin. This has an effect on filtering bad sector combinations. Recommendation: use false for normal Belle2-VXD-case and true for beam test situations where layerIDs are mixed up",
           false);

  addParam("rootFileName", m_PARAMrootFileName, "only needed if importROOTorXML = true: sets the root filename", rootFileName);

  addParam("printFinalMaps", m_PARAMprintFinalMaps,
           "only needed if importROOTorXML = true: if true, a complete list of sectors (B2INFO) and its friends (B2DEBUG-1) will be printed on screen",
           bool(true));
}



RawSecMapMergerWithSpacePointsModule::~RawSecMapMergerWithSpacePointsModule()
{

}



void RawSecMapMergerWithSpacePointsModule::initialize()
{
  /// imported parameters check:

  if (m_PARAMimportROOTorXML == true) { ///import via root file
    if (int(m_PARAMsampleQuantiles.size()) != 2) { B2FATAL(" parameter sampleQuantiles is wrong, only exactly 2 entries allowed!");}
    if (int(m_PARAMsampleThreshold.size()) != 2) { B2FATAL(" parameter sampleThreshold is wrong, only exactly 2 entries allowed!");}
    if (int(m_PARAMsmallSampleQuantiles.size()) != 2) { B2FATAL(" parameter smallSampleQuantiles is wrong, only exactly 2 entries allowed!");}
    if (int(m_PARAMstretchFactor.size()) != 2) { B2FATAL(" parameter stretchFactor is wrong, only exactly 2 entries allowed!");}
    if (m_PARAMrarenessFilter < 0. or m_PARAMrarenessFilter > 1.) {
      B2WARNING("ExportSectorMapModule::initialize:  parameter rarenessFilter is set to " << m_PARAMrarenessFilter <<
                ", which is not within the allowed range of 0 < x < 1! Setting to 0...!"); m_PARAMrarenessFilter = 0;
    }
  } else { /// import via xml file
    if (int(m_PARAMsetOrigin.size()) != 3) {
      B2WARNING("RawSecMapMergerWithSpacePointsModule::initialize: origin is set wrong, please set only 3 values (x,y,z). Rejecting user defined value and reset to (0,0,0)!");
      m_PARAMsetOrigin.clear();
      m_PARAMsetOrigin = {0., 0., 0.};
    }
    B2INFO("RawSecMapMergerWithSpacePointsModule::initialize: origin is set to: (x,y,z) (" << m_PARAMsetOrigin[0] << "," <<
           m_PARAMsetOrigin[1] << ","
           << m_PARAMsetOrigin[2] << ", magnetic field set to " << m_PARAMmagneticFieldStrength << "T");
  }
}




void RawSecMapMergerWithSpacePointsModule::terminate()
{
  // the whole job is done here to be able to create a full secMap in one steering file
  boostClock::time_point beginEvent = boostClock::now();

//   TVector3 origin(m_PARAMsetOrigin[0], m_PARAMsetOrigin[1], m_PARAMsetOrigin[2]);

  /// importing sectorMap including friend Information and friend specific cutoffs
  if (m_PARAMimportROOTorXML == false) {   // import via xml file

    pair<int, int> countedStuff = importXMLMap();             /// importXMLMap

    B2INFO("Imported via XML-file: number of Sector-Entries: " << m_fullSectorMapCopy.size() << ", of friend-entries: " <<
           countedStuff.first << ", of filter-entries: " << countedStuff.second << ", mField: " << m_PARAMmagneticFieldStrength <<
           "T, and additional Info:\n" << m_PARAMadditionalInfo);
  } else { // import via root file

    pair<int, int> countedStuff = importROOTMap();              /// importROOTMap

    for (auto& aMap : m_importedSectorMaps) {
      B2INFO("Imported via ROOT-file of a total of " << countedStuff.first << "/" << countedStuff.second <<
             " sectors/totalEntries, to XML converted number of Sector-Entries: " << aMap.size() << ", of friend-entries: " <<
             aMap.getNumOfFriends() << ", of final cutoffValues: " << aMap.getNumOfValues() << ", mField: " << aMap.getMagneticFieldStrength() <<
             "T, and additional Info:\n" << aMap.getAdditionalInfo());
    }
  }


  boostClock::time_point stopTimer = boostClock::now();
  m_fillStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);

  B2INFO("RawSecMapMergerWithSpacePointsModule::terminate: importing the map took " << (m_fillStuff.count()) << "/" << int(
           m_fillStuff.count() * 0.000001) << " nano-/milliseconds");
}



double RawSecMapMergerWithSpacePointsModule::getXMLValue(GearDir& cuts, string& valueType, string& filterType)
{
  double aValue;
  try {
    aValue = cuts.getDouble(valueType);
  } catch (...) {
    B2WARNING("import of " << filterType << "-" << valueType << "-value failed! Setting to 0!!");
    aValue = 0;
  }
  return aValue;
}



std::pair<int, int> RawSecMapMergerWithSpacePointsModule::importXMLMap()
{
  VXDTFSecMap newSecMap = VXDTFSecMap();
  TVector3 origin(m_PARAMsetOrigin[0], m_PARAMsetOrigin[1], m_PARAMsetOrigin[2]);

  string chosenSetup = (boost::format("sectorList_%1%_%2%") % m_PARAMsectorSetup  % m_PARAMdetectorType).str();
  string fileName = chosenSetup + string(".xml");

  GearDir belleDir("/Detector/Tracking/CATFParameters/");

  GearDir sectorList(belleDir, chosenSetup);

  B2INFO("importing sectors, using " << chosenSetup << " > " << sectorList.getNumberNodes("aSector") << " sectors found");

  if (sectorList.getNumberNodes("aSector") == 0) {
    B2FATAL("Failed to import sector map " << chosenSetup <<
            "! No track finding possible. Please check ../tracking/data/VXDTFindex.xml whether your chosen sector maps are uncommented (and files linked there are not zipped) and recompile if you change entries...");
  }

  double cutoffMinValue, cutoffMaxValue;
  int countFriends = 0, countFilters = 0;
  string aSectorName, aFriendName, aFilterName, min = "Min", max = "Max";

  for (const GearDir& aSector : sectorList.getNodes("aSector")) {

    aSectorName = aSector.getString("secID");
    FullSecID secID = FullSecID(aSectorName); // same as aSectorName but info stored in an int

    GearDir friendList(aSector, "friendList/");

    B2DEBUG(5, " > > importing sector: " << aSectorName << " (named " << secID.getFullSecID() << " as an int) including " <<
            friendList.getNumberNodes("aFriend") << " friends. ");

    SectorValue sectorValue;

    for (const GearDir& aFriend : friendList.getNodes("aFriend")) {

      aFriendName = aFriend.getString("friendID");
      FullSecID friendID = FullSecID(aFriendName); // same as aFriendName but info stored in an int

      FriendValue friendValue;

      GearDir filterList(aFriend, "filterList/");

      for (const GearDir& aFilter : filterList.getNodes("aFilter")) {

        aFilterName = aFilter.getString("filterID");
        GearDir cuts(aFilter, "cuts/");
        cutoffMinValue = 0., cutoffMaxValue = 0.;
        int filterID = FilterID().getFilterType(aFilterName);
        if (filterID == FilterID::numFilters) { B2FATAL("Filter in XML-File does not exist! check FilterID-class!");}
        // now, for each filter will be checked, whether it shall be stored or not and whether the cutoffs shall be modified:
        if (aFilterName == FilterID::nameDistance3D) {   // first: activateDistance3D, second: tuneDistance3D
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameDistanceXY) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameDistanceZ) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameSlopeRZ) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameNormedDistance3D) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameAngles3D) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameAnglesXY) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameAnglesRZ) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::namePT) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameHelixParameterFit) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameDeltaSOverZ) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameDeltaSlopeZOverS) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameDistance2IP) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameDeltaSlopeRZ) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameDeltapT) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameDeltaDistance2IP) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameAnglesHighOccupancy3D) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameAnglesHighOccupancyXY) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameAnglesHighOccupancyRZ) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::namePTHighOccupancy) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameHelixParameterHighOccupancyFit) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameDistanceHighOccupancy2IP) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameDeltaSlopeHighOccupancyRZ) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameDeltapTHighOccupancy) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);

        } else if (aFilterName == FilterID::nameDeltaDistanceHighOccupancy2IP) {
          cutoffMinValue = getXMLValue(cuts, min, aFilterName);
          cutoffMaxValue = getXMLValue(cuts, max, aFilterName);
        }

        CutoffValue cutoffValue = make_pair(cutoffMinValue, cutoffMaxValue);
        countFilters++;
        Cutoff cutoff = make_pair(filterID, cutoffValue);
        friendValue.push_back(cutoff);
        B2DEBUG(50, " > > importing filter: " << aFilterName << " (named " << filterID << " as an int and " << FilterID().getFilterString(
                  filterID) << " as a recoded string) with min/max: " << cutoffMinValue << "/" << cutoffMaxValue << " filters. ");
      }
      Friend thisFriend = make_pair(friendID.getFullSecID(), friendValue);
      countFriends++;
      B2DEBUG(10, " > > importing friend: " << friendID.getFullSecString() << " (named " << friendID.getFullSecID() <<
              " as an int) including " << friendValue.size() << " filters. ");
      sectorValue.push_back(thisFriend);
    }
    Sector thisSector = make_pair(secID.getFullSecID(), sectorValue);
    m_fullSectorMapCopy.push_back(thisSector);
  }


  std::ofstream file(fileName.c_str());

  newSecMap.setSectorMap(m_fullSectorMapCopy);
  newSecMap.setMapName(chosenSetup);
  newSecMap.setSectorConfigU(m_PARAMsectorConfigU);
  newSecMap.setSectorConfigV(m_PARAMsectorConfigV);
  newSecMap.setOrigin(origin);
  newSecMap.setDetectorType(m_PARAMdetectorType);
  newSecMap.setMagneticFieldStrength(m_PARAMmagneticFieldStrength);
  newSecMap.setAdditionalInfo(m_PARAMadditionalInfo);
  string  tagName = "<" + chosenSetup + ">\n";
  string  endTagName = "\n</" + chosenSetup + ">\n";
  file << tagName << Stream::escapeXML(Stream::serializeAndEncode(&newSecMap)) << endTagName;

  return (make_pair(countFriends, countFilters));
}



std::pair<int, int> RawSecMapMergerWithSpacePointsModule::importROOTMap()
{
  int countSectors = 0,
      countExternalMaps = 0,
      countTotalValues =
        0;// values are all the values measured for each cutoff-type which are used for calculating the cutoffs afterwards

  string fileName =  m_PARAMrootFileName + "SecMap.root";
  B2DEBUG(1, "importROOTMap: reading file " << fileName);
  SecMapVector importedMaps; // stores all different maps which were imported (similar maps are combined)


  /// reopen file, read entries and check them -> result is imported vector of Maps containing raw data for each sector-combi and filterType
  TFile importedRootFile(fileName.c_str());
  if (importedRootFile.IsZombie()) { B2ERROR("file could not be reopened!"); }
  else {
    int countNumberOfComparisons = 0, countMatches = 0, countMaps = 0, countSecMapVectors = 0;
    SecMapVector* retrievedVector; // pointer to current vector of secMaps in the root file

    TIter next(importedRootFile.GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)next())) {

      try {
        retrievedVector = static_cast<SecMapVector*>
                          (key->ReadObj()); // not very performant, but here, the performance is not an issue, especially since there is only a relatively small number of vectors imported - compared the size of their objects
      } catch (exception& e) {
        B2WARNING("Key was not a SecMapVector, therefore error message: " << e.what() << "\n Skipping this key...");
        continue;
      }

      countSecMapVectors++;
      B2INFO(" current secMapVector has " << retrievedVector->size() << " secMaps stored, this vector is number " << countSecMapVectors <<
             "!");
      unsigned nExternalMaps = retrievedVector->size();
      countExternalMaps += nExternalMaps;
//      std::vector< Belle2::SecMapVector::MapPack>
      for (SecMapVector::MapPack& tempSecMap : retrievedVector->getFullVector()) {  // full VXDTFRawSecMap including name
        countMaps++;
        B2INFO("opening new map " << tempSecMap.second.getMapName() <<  " (number " << countMaps << " so far), with " <<
               tempSecMap.second.getNumOfSectors() << " sectors, " << tempSecMap.second.getNumOfFriends() << " friends and " <<
               tempSecMap.second.getNumOfValues() << " total values" << endl;);


        /// merging intermediate maps to one map of each type:
        bool partnerMapFound = false;
        unsigned nInternalMaps = importedMaps.size();
        for (SecMapVector::MapPack& anotherMap : importedMaps.getFullVector()) {  // loop over already existing maps
          ++countNumberOfComparisons;
          B2INFO("Iteration " << countNumberOfComparisons << " (of " << nExternalMaps * nInternalMaps << "), current retrieved map: " <<
                 tempSecMap.first << ", current imported map: " << anotherMap.first << " taken from SecMapVector with size of " << nInternalMaps);
          B2DEBUG(5, "tempSecMap.second, anotherMap.second:\ndetectorType: " << tempSecMap.second.getDetectorType() << ", " <<
                  anotherMap.second.getDetectorType() << ",  size: " << tempSecMap.second.size() << ", " << anotherMap.second.size() << ",  MapName: "
                  << tempSecMap.second.getMapName() << ", " << anotherMap.second.getMapName() << ",  MagneticField: " <<
                  tempSecMap.second.getMagneticFieldStrength() << ", " << anotherMap.second.getMagneticFieldStrength() << ",  origin: " <<
                  tempSecMap.second.getOrigin().Mag() << ", " << anotherMap.second.getOrigin().Mag());

          if (tempSecMap.second == anotherMap.second) {  // compares VXDTFRawSecMaps
            countMatches++;
            B2INFO("Partner found for " << anotherMap.first << " and will now be merged " << tempSecMap.first << ", there were " << countMatches
                   << " matches for maps so far");            anotherMap.second.addSectorMap(
                     tempSecMap.second.getSectorMap()); // INFO: removes entries from imported map and adds it to existing one ( more precisely: if sector-friend-combination already exists and current filterType too, then all values of added map will be moved to the existing one. If the sector, or the sector-friend-combination or the filterType in that combi does not exist yet, then the data gets copied, not moved)
            anotherMap.second.addDistances(tempSecMap.second.getDistances()); // after adding sectors, the sector-distaces are added separately
            B2INFO("Map: " << anotherMap.first << " has been merged with " << tempSecMap.first);
            partnerMapFound = true;
            break;
          }
        }
        if (partnerMapFound == false) {
          importedMaps.push_back(tempSecMap);
          B2INFO("Map: " << tempSecMap.first << " has no compatible pendant imported yet, storing directly ");
        }

        B2INFO("ExportSectorMapModule::importROOTMap: finishing import of new map " << tempSecMap.second.getMapName());
        B2DEBUG(5, ", with " << tempSecMap.second.getNumOfSectors() << " sectors, " << tempSecMap.second.getNumOfFriends() <<
                " friends and " << tempSecMap.second.getNumOfValues() << " total values" << endl;);
      }
    }
    importedRootFile.Close();

    // some checks:
    stringstream importedMapsCounted;
    for (SecMapVector::MapPack& aMap : importedMaps.getFullVector()) {
      importedMapsCounted << "Imported map: " << aMap.second.getMapName() <<  ", with " <<
                          aMap.second.getNumOfSectors() << " sectors, " << aMap.second.getNumOfFriends() << " friends and " << aMap.second.getNumOfValues() <<
                          " total values" << endl;
//      ++countInternalMaps;
      countSectors += aMap.second.getNumOfSectors();
      countTotalValues += aMap.second.getNumOfValues();
    }

    B2INFO("counted manually afterwards: " << endl << importedMapsCounted.str() << endl);
    B2INFO("got " << countExternalMaps << "/" << importedMaps.size() << " external/imported maps inhabiting " << countTotalValues <<
           " total values");
  } // end root-check


  // convert to final map(s) and export:
  for (SecMapVector::MapPack& aRawMapPack : importedMaps.getFullVector()) {
    // feed rawMaps with some extra info first (needed for rawMap-import):
    aRawMapPack.second.setCutoffQuantiles(make_pair(m_PARAMsampleQuantiles.at(0), m_PARAMsampleQuantiles.at(1)));
    aRawMapPack.second.setSmallCutoffQuantiles(make_pair(m_PARAMsmallSampleQuantiles.at(0), m_PARAMsmallSampleQuantiles.at(1)));
    aRawMapPack.second.setSampleThreshold(m_PARAMsampleThreshold.at(1));
    aRawMapPack.second.setMinSampleThreshold(m_PARAMsampleThreshold.at(0));
    aRawMapPack.second.setSmallStretchFactor(m_PARAMstretchFactor.at(0));
    aRawMapPack.second.setStretchFactor(m_PARAMstretchFactor.at(1));
    aRawMapPack.second.setFilterByDistance2Origin(m_PARAMsortByDistance2origin);
    aRawMapPack.second.setRareSectorCombinations(m_PARMfilterRareCombinations, m_PARAMrarenessFilter);
    aRawMapPack.second.repairSecMap();

    VXDTFSecMap newMap;

    newMap.importRawSectorMap(aRawMapPack.second); // calculates cutoffs and copies all the rest into newMap

    m_importedSectorMaps.push_back(newMap);


    string xmlFileName = newMap.getMapName() + string(".xml");
    std::ofstream file(xmlFileName.c_str());

    string tagName = "<" + newMap.getMapName() + ">\n";
    string endTagName = "\n</" + newMap.getMapName() + ">\n";

    file << tagName << Stream::escapeXML(Stream::serializeAndEncode(&newMap)) << endTagName;
//     file << tagName << Stream::escapeXML(Stream::serializeXML(&newMap)) << endTagName;

    unsigned sectorCtr = 0, friendCtr = 0, cutoffTypesCtr = 0;

    for (const VXDTFSecMapTypedef::Sector& sectorEntry : newMap.getSectorMap()) {  // looping through sectors
      const VXDTFSecMapTypedef::SectorValue& currentFriends = sectorEntry.second;

      if (m_PARAMprintFinalMaps == true) {B2DEBUG(1, "Opening sector " << FullSecID(sectorEntry.first) << " which has got " << currentFriends.size() << " friends");}

      for (const VXDTFSecMapTypedef::Friend& friendEntry : currentFriends) {  // looping through friends
        unsigned nTypes = friendEntry.second.size();

        if (m_PARAMprintFinalMaps == true) {B2DEBUG(2, " > Opening sectorFriend " << FullSecID(friendEntry.first) << " having " << nTypes << " cutoffTypes.");}
        cutoffTypesCtr += nTypes;
        ++friendCtr;
      }
      ++sectorCtr;
    }
    B2INFO("printFinalMaps: secMap " << newMap.getMapName() << ": manually counted a total of " << sectorCtr << "/" << friendCtr << "/"
           << cutoffTypesCtr << " setors/friends/cutoffs in sectorMap");
  }

  return (make_pair(countSectors, countTotalValues));
}
