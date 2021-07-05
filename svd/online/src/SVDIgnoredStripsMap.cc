/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/online/SVDIgnoredStripsMap.h>
#include <boost/property_tree/xml_parser.hpp>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

using namespace Belle2;
using namespace std;
using boost::property_tree::ptree;


SVDIgnoredStripsMap::SVDIgnoredStripsMap(const string& xmlFilename):
  m_Map(12), m_lastSensorID(0)
{
  // If the xmlFilename is empty, the user apparently doesn't want the map.
  // So keep low-profile, don't bother.
  if (xmlFilename == "") {
    B2INFO("No xml list of ignored strips specified.");
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
              "SVD ignored strips map cannot be initialized." << endl
             );
    return;
  }

  try {
    read_xml(xmlFullPath, propertyTree);
  } catch (std::exception const& ex) {
    B2WARNING("STD excpetion raised during xml parsing " << ex.what() << endl <<
              "SVD ignored strips map cannot be initialized." << endl);
    return;
  } catch (...) {
    B2WARNING("Unknown excpetion raised during xml parsing "
              "SVD ignored strips map cannot be initialized." << endl);
    return;
  }

  try {
    // traverse the xml tree: navigate through the daughters of <SVD>
    VxdID sensorID;
    for (ptree::value_type const& layer : propertyTree.get_child("SVD"))
      if (layer.first == "layer") {
        sensorID.setLayerNumber(static_cast<unsigned short>(layer.second.get<int>("<xmlattr>.n")));
        for (ptree::value_type const& ladder : layer.second)
          if (ladder.first == "ladder") {
            sensorID.setLadderNumber(static_cast<unsigned short>(ladder.second.get<int>("<xmlattr>.n")));
            for (ptree::value_type const& sensor : ladder.second)
              if (sensor.first == "sensor") {
                sensorID.setSensorNumber(static_cast<unsigned short>(sensor.second.get<int>("<xmlattr>.n")));
                for (ptree::value_type const& side : sensor.second)
                  if (side.first == "side") {
                    std::string tagSide = side.second.get<std::string>("<xmlattr>.side");
                    sensorID.setSegmentNumber((tagSide == "U" || tagSide == "u") ? 1 : 0);
                    // We have complete VxdID, now we read the list of strips.
                    IgnoredStripsSet strips;
                    for (ptree::value_type const& tag : side.second)
                      if (tag.first == "strip") {
                        strips.insert(tag.second.get<unsigned short>("<xmlattr>.stripNo"));
                      } else if (tag.first == "stripsFromTo") {
                        auto limits = tag.second;
                        unsigned short fromStrip = limits.get<unsigned short>("<xmlattr>.fromStrip");
                        unsigned short toStrip   = limits.get<unsigned short>("<xmlattr>.toStrip");
                        for (unsigned short iStrip = fromStrip; iStrip <= toStrip; iStrip++)
                          strips.insert(iStrip);
                      }
                    m_Map.insert(std::pair<unsigned short, IgnoredStripsSet>(sensorID.getID(), strips));
                  } // if side
              } // if sensor
          } // if ladder
      }  // if sensor
  } catch (...) {
    B2WARNING("Unknown excpetion raised during map initialization! "
              "SVD ignored strips map may be corrupted." << endl);
    return;
  }
}

const std::set<unsigned short>& SVDIgnoredStripsMap::getIgnoredStrips(VxdID id)
{
  if (id == m_lastSensorID)
    return m_lastIgnored;
  else {
    m_lastSensorID = id;
    auto mapIter = m_Map.find(id);
    if (mapIter != m_Map.end()) {
      m_lastIgnored = mapIter->second;
    } else {
      m_lastIgnored.clear();
    }
    return m_lastIgnored;
  }
}

bool SVDIgnoredStripsMap::stripOK(VxdID id, unsigned short strip)
{
  if (id != m_lastSensorID) {
    m_lastSensorID = id;
    auto mapIter = m_Map.find(id);
    if (mapIter != m_Map.end()) {
      m_lastIgnored = mapIter->second;
    } else {
      m_lastIgnored.clear();
      return true;
    }
  }
  return (m_lastIgnored.count(strip) == 0);
}




