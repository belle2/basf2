/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni,                        *
 *               Peter Kvasnicka, Tadeas Bilka                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/online/PXDIgnoredPixelsMap.h>
#include <boost/property_tree/xml_parser.hpp>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <vxd/geometry/GeoCache.h>
#include <pxd/geometry/SensorInfo.h>

using namespace Belle2;
using namespace std;
using boost::property_tree::ptree;


PXDIgnoredPixelsMap::PXDIgnoredPixelsMap(const string& xmlFilename):
  m_Map(0), m_MapSingles(0), m_lastSensorID(0), m_lastSensorVCells(0)
{
  // If the xmlFilename is empty, the user apparently doesn't want the map.
  // So keep low-profile, don't bother.
  if (xmlFilename == "") {
    B2DEBUG(10, "No xml list of ignored pixels specified.");
    return;
  }
  // Create an empty property tree object

  ptree propertyTree;

  // Load the XML file into the property tree. If reading fails
  // (cannot open file, parse error), an exception is thrown.
  string xmlFullPath = FileSystem::findFile(xmlFilename);

  if (! FileSystem::fileExists(xmlFullPath)) {
    B2WARNING("The xml filename: " << xmlFilename << endl <<
              "resolved to: " << xmlFullPath << endl <<
              "by FileSystem::findFile does not exist." << endl <<
              "PXD ignored pixels map cannot be initialized." << endl
             );
    return;
  }

  try {
    read_xml(xmlFullPath, propertyTree);
  } catch (std::exception const& ex) {
    B2WARNING("STD excpetion raised during xml parsing " << ex.what() << endl <<
              "PXD ignored pixels map cannot be initialized." << endl);
    return;
  } catch (...) {
    B2WARNING("Unknown excpetion raised during xml parsing "
              "PXD ignored pixels map cannot be initialized." << endl);
    return;
  }

  try {
    // traverse the xml tree: navigate through the daughters of <PXD>
    VxdID sensorID;
    for (ptree::value_type const& layer : propertyTree.get_child("PXD"))
      if (layer.first == "layer") {
        sensorID.setLayerNumber(static_cast<unsigned short>(layer.second.get<int>("<xmlattr>.n")));
        for (ptree::value_type const& ladder : layer.second)
          if (ladder.first == "ladder") {
            sensorID.setLadderNumber(static_cast<unsigned short>(ladder.second.get<int>("<xmlattr>.n")));
            for (ptree::value_type const& sensor : ladder.second)
              if (sensor.first == "sensor") {
                sensorID.setSensorNumber(static_cast<unsigned short>(sensor.second.get<int>("<xmlattr>.n")));
                PXDIgnoredPixelsMap::IgnoredPixelsRangeSet ranges;
                PXDIgnoredPixelsMap::IgnoredSinglePixelsSet singles;
                const VXD::SensorInfoBase& info = VXD::GeoCache::getInstance().get(sensorID);
                for (ptree::value_type const& tag : sensor.second) {
                  if (tag.first == "pixels") {
                    auto limits = tag.second;
                    // All possible attributes, default = -1 (att not present)
                    short uStart = limits.get<short>("<xmlattr>.uStart", -1);
                    short uEnd   = limits.get<short>("<xmlattr>.uEnd", -1);
                    short vStart = limits.get<short>("<xmlattr>.vStart", -1);
                    short vEnd   = limits.get<short>("<xmlattr>.vEnd", -1);

                    // Fill remaining range parameters
                    if (uStart != -1 && vStart != -1 && uEnd == -1 && vEnd == -1) {
                      // mask one pixel
                      uEnd = uStart;
                      vEnd = vStart;
                    } else if (uStart != -1 && vStart == -1 && uEnd == -1 && vEnd == -1) {
                      // mask column u ... all v rows
                      uEnd = uStart;
                      vStart = 0;
                      vEnd = info.getVCells() - 1;
                    } else if (uStart == -1 && vStart != -1 && uEnd == -1 && vEnd == -1) {
                      // mask row v ... all u columns
                      vEnd = vStart;
                      uStart = 0;
                      uEnd = info.getUCells() - 1;
                    } else if (uStart != -1 && vStart == -1 && uEnd != -1 && vEnd == -1) {
                      // columns from ... to ... (all rows there)
                      vStart = 0;
                      vEnd = info.getVCells() - 1;
                    } else if (uStart == -1 && vStart != -1 && uEnd == -1 && vEnd != -1) {
                      // rows from ... to ... (all columns there)
                      uStart = 0;
                      uEnd = info.getUCells() - 1;
                    } else if (uStart != -1 && vStart != -1 && uEnd != -1 && vEnd != -1) {
                      // already ok, rectangular area
                    } else {
                      // do not accept other combinations
                      continue;
                    }
                    // ensure positive and meaningfull values
                    unsigned short uS(uStart);
                    unsigned short vS(vStart);
                    unsigned short uE(uEnd);
                    unsigned short vE(vEnd);
                    if (uE >= info.getUCells()) uE = info.getUCells() - 1;
                    if (vE >= info.getVCells()) vE = info.getVCells() - 1;
                    if (vS > vE) vS = vE;
                    if (uS > uE) uS = uE;
                    // area of the masked range
                    unsigned int area = (uE - uS + 1) * (vE - vS + 1);
                    if (area == 1) {
                      // Single pixel masking:
                      // We store pixels by unique id in hash table
                      unsigned int uid = uStart * info.getVCells() + vStart;
                      // uid will be used to generate hash in unordered_set for quick access
                      singles.insert(uid);
                    } else {
                      // lambda function to decide if (u,v) is inside this range
                      PXDIgnoredPixelsMap::pixel_range_test_prototype range_mask =
                        [ = ](unsigned int u, unsigned int v) -> bool
                      { return (uS <= u && u <= uE && vS <= v && v <= vE); };
                      // area is used to sort ranges from largest to smallest
                      ranges.insert(std::make_pair(area, range_mask));
                    }
                  }
                }
                m_Map.insert(std::pair<unsigned short, PXDIgnoredPixelsMap::IgnoredPixelsRangeSet>(sensorID.getID(), ranges));
                m_MapSingles.insert(std::pair<unsigned short, PXDIgnoredPixelsMap::IgnoredSinglePixelsSet>(sensorID.getID(), singles));
              } // if sensor
          } // if ladder
      }  // if sensor
  } catch (...) {
    B2WARNING("Unknown exception raised during map initialization! "
              "PXD ignored pixels map may be corrupted." << endl);
    return;
  }
}

const std::set<PXDIgnoredPixelsMap::map_pixel> PXDIgnoredPixelsMap::getIgnoredPixels(VxdID id)
{
  // Merely for testing...
  // This function is quite ineffective, but it is not supposed to be run often
  // Also, it currently returns copy of the (possibly very big) set of masked pixels
  std::set<PXDIgnoredPixelsMap::map_pixel> pixels;
  const VXD::SensorInfoBase& info = VXD::GeoCache::getInstance().get(id);

  // This is quite slow solution but it merges duplicate maskings in the set
  for (int pixelU = 0; pixelU < info.getUCells(); pixelU++) {
    for (int pixelV = 0; pixelV < info.getVCells(); pixelV++) {
      PXDIgnoredPixelsMap::map_pixel px(pixelU, pixelV);
      if (!pixelOK(id, px))
        pixels.insert(px);
    }
  }
  return pixels;
}

bool PXDIgnoredPixelsMap::pixelOK(VxdID id, PXDIgnoredPixelsMap::map_pixel pixel)
{
  // If sensor id changed from last query, swich to temp maps
  // of the new sensor. Otherwise clear temp maps, as there is nothing to mask
  if (id != m_lastSensorID) {
    m_lastSensorID = id;
    m_lastSensorVCells =  VXD::GeoCache::getInstance().get(m_lastSensorID).getVCells();
    auto mapIter = m_Map.find(id);
    auto mapIterSingles = m_MapSingles.find(id);

    if (mapIter != m_Map.end())
      m_lastIgnored = mapIter->second;
    else
      m_lastIgnored.clear();

    if (mapIterSingles != m_MapSingles.end())
      m_lastIgnoredSingles = mapIterSingles->second;
    else
      m_lastIgnoredSingles.clear();

    if (mapIter == m_Map.end() && mapIterSingles == m_MapSingles.end()) {
      // Sensor has no masked ranges or single pixels in the maps => pixel ok
      return true;
    }
  }

  const unsigned int u = pixel.first;
  const unsigned int v = pixel.second;

  // loop over masked ranges for the sensor
  // (iter over multimap ordered by masked area)
  for (auto iter = m_lastIgnored.begin(); iter != m_lastIgnored.end(); ++iter) {
    // call the lambda function of masked range to check if pixel is inside
    if (iter->second(u, v))
      return false;
  }
  // Look if this is a single masked pixel
  // (lookup in hash table)
  if (m_lastIgnoredSingles.find(u * m_lastSensorVCells + v) != m_lastIgnoredSingles.end())
    return false;

  // Pixel not found in the mask => pixel OK
  return true;
}




