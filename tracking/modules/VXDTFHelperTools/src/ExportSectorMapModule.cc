/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Moritz Nadler                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/VXDTFHelperTools/ExportSectorMapModule.h"
#include "tracking/dataobjects/VXDTFSecMap.h"
#include "tracking/vxdCaTracking/FullSecID.h"
#include <framework/gearbox/Const.h>
#include "tracking/vxdCaTracking/FilterID.h"
#include <framework/logging/Logger.h>
#include <framework/utilities/Stream.h> // to be able to stream TObjects into xml-coded files

#include <fstream>

#include <boost/foreach.hpp>
#include <boost/format.hpp>


using namespace std;
using namespace Belle2;
using namespace Tracking;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ExportSectorMap)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ExportSectorMapModule::ExportSectorMapModule() : Module()
{
  //Set module properties
  setDescription("imports xml-files of sectorMaps and exports RAM-friendly versions of it");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  addParam("detectorType", m_PARAMdetectorType, "defines which detector type has to be exported. Allowed values: 'VXD', 'PXD', 'SVD'", string("SVD"));
  addParam("sectorSetup", m_PARAMsectorSetup, "lets you chose the sectorSetup (compatibility of sensors, individual cutoffs,...) accepts 'std', 'low', 'high' and 'personal', please note that the chosen setup has to exist as a xml-file in ../tracking/data/friendList_XXX.xml. If you can not create your own xml files using e.g. the filterCalculatorModule, use params for  'tuneCutoffXXX' or 'setupWeigh' instead. multipass supported by setting setups in a row", string("std"));

}


ExportSectorMapModule::~ExportSectorMapModule()
{

}


void ExportSectorMapModule::initialize()
{
  boostClock::time_point beginEvent = boostClock::now();

  /// importing sectorMap including friend Information and friend specific cutoffs
  int totalFriendCounter = 0;

  GearDir belleDir("/Detector/Tracking/CATFParameters/");

  string chosenSetup = (boost::format("sectorList_%1%_%2%") % m_PARAMsectorSetup  % m_PARAMdetectorType).str();

  GearDir sectorList(belleDir, chosenSetup);

  B2INFO("importing sectors, using " << chosenSetup << " > " << sectorList.getNumberNodes("aSector") << " sectors found");

  if (sectorList.getNumberNodes("aSector") == 0) {
    B2FATAL("Failed to import sector map " << chosenSetup << "! No track finding possible. Please check ../tracking/data/VXDTFindex.xml whether your chosen sector maps are uncommented (and files linked there are not zipped) and recompile if you change entries...")
  }
  double cutoffMinValue, cutoffMaxValue;
  string aSectorName, aFriendName, aFilterName, min = "Min", max = "Max";

  pair<double, double> cutoff;
  BOOST_FOREACH(const GearDir & aSector, sectorList.getNodes("aSector")) {

    aSectorName = aSector.getString("secID");
    FullSecID secID = FullSecID(aSectorName); // same as aSectorName but info stored in an int

    GearDir friendList(aSector, "friendList/");

    B2DEBUG(1, " > > importing sector: " << aSectorName << " (named " << secID.getFullSecID() << " as an int) including " << friendList.getNumberNodes("aFriend") << " friends. ");
    totalFriendCounter += friendList.getNumberNodes("aFriend");

    SectorValue sectorValue;

    BOOST_FOREACH(const GearDir & aFriend, friendList.getNodes("aFriend")) {

      aFriendName = aFriend.getString("friendID");
      FullSecID friendID = FullSecID(aFriendName); // same as aFriendName but info stored in an int

      FriendValue friendValue;

      GearDir filterList(aFriend, "filterList/");

      BOOST_FOREACH(const GearDir & aFilter, filterList.getNodes("aFilter")) {

        aFilterName = aFilter.getString("filterID");
        GearDir cuts(aFilter, "cuts/");
        cutoffMinValue = 0., cutoffMaxValue = 0.;
        int filterID = FilterID().getFilterType(aFilterName);
        if (filterID == FilterID::numFilters) { B2FATAL("Filter in XML-File does not exist! check FilterID-class!")}
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

        } else if (aFilterName == FilterID::nameHelixFit) {
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

        } else if (aFilterName == FilterID::nameHelixHighOccupancyFit) {
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
        Cutoff cutoff = make_pair(filterID, cutoffValue);
        friendValue.push_back(cutoff);
        B2DEBUG(50, " > > importing filter: " << aFilterName << " (named " << filterID << " as an int and " << FilterID().getFilterString(filterID) << " as a recoded string) with min/max: " << cutoffMinValue << "/" << cutoffMaxValue << " filters. ");
      }
      Friend thisFriend = make_pair(friendID.getFullSecID(), friendValue);
      B2DEBUG(10, " > > importing friend: " << friendID.getFullSecString() << " (named " << friendID.getFullSecID() << " as an int) including " << friendValue.size() << " filters. ");
      sectorValue.push_back(thisFriend);
    }
    Sector thisSector = make_pair(secID.getFullSecID(), sectorValue);
    m_fullSectorMapCopy.push_back(thisSector);
  }


  string fileName = chosenSetup + string(".xml");
  std::ofstream file(fileName.c_str());
  VXDTFSecMap newSecMap = VXDTFSecMap();
  newSecMap.setSectorMap(m_fullSectorMapCopy);
  newSecMap.setMapName(chosenSetup);
  string  tagName = "<" + chosenSetup + ">\n";
  string  endTagName = "\n</" + chosenSetup + ">\n";
  file << tagName << Stream::escapeXML(Stream::serialize(&newSecMap)) << endTagName;

  boostClock::time_point stopTimer = boostClock::now();
  m_fillStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);
}


void ExportSectorMapModule::beginRun()
{
}


void ExportSectorMapModule::event()
{
}


void ExportSectorMapModule::endRun()
{
}


void ExportSectorMapModule::terminate()
{
  B2WARNING("importing the map took " << (m_fillStuff.count()) << " nanoseconds")
}


double ExportSectorMapModule::getXMLValue(GearDir& cuts, string& valueType, string& filterType)
{
  double aValue;
  try {
    aValue = cuts.getDouble(valueType);
  } catch (...) {
    B2WARNING("import of " << filterType << "-" << valueType << "-value failed! Setting to 0!!")
    aValue = 0;
  }
  return aValue;
}