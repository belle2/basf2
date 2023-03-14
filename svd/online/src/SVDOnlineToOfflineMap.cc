/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "svd/online/SVDOnlineToOfflineMap.h"
#include <boost/property_tree/xml_parser.hpp>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <vxd/geometry/GeoCache.h>

using namespace Belle2;
using namespace std;
using boost::property_tree::ptree;


SVDOnlineToOfflineMap::SVDOnlineToOfflineMap(const string& xmlFilename)
  : m_MapUniqueName("")
  , m_currentChipInfo()
  , m_currentSensorInfo()
{

  // Create an empty property tree object

  ptree propertyTree;

  // Load the XML file into the property tree. If reading fails
  // (cannot open file, parse error), an exception is thrown.
  string xmlFullPath = FileSystem::findFile(xmlFilename);

  if (! FileSystem::fileExists(xmlFullPath)) {
    B2ERROR("The xml filename: " << xmlFilename << endl <<
            "resolved to: " << xmlFullPath << endl <<
            "by FileSystem::findFile does not exist." << endl <<
            "SVD online to offline map cannot be initialized." << endl <<
            "Be aware: no SVDShaperDigit will be produced by this module." << endl
           );
    return;
  }

  try {
    read_xml(xmlFullPath, propertyTree);
  } catch (std::exception const& ex) {
    B2ERROR("STD excpetion rised during xml parsing " << ex.what() << endl <<
            "SVD online to offline map cannot be initialized." << endl <<
            "Be aware: no SVDShaperDigits will be produced by this module." << endl);
    return;
  } catch (...) {
    B2ERROR("Unknown excpetion rised during xml parsing "
            "SVD online to offline map cannot be initialized." << endl <<
            "Be aware: no SVDShaperDigits will be produced by this module." << endl);
    return;
  }

  try {
    // traverse pt: let us navigate through the daughters of <SVD>
    for (ptree::value_type const& v : propertyTree.get_child("SVD")) {
      if (v.first == "unique") {
        m_MapUniqueName = v.second.get<string>("<xmlattr>.name");
        B2INFO("Loading the offline -> online SVD map named " << m_MapUniqueName);
      }
      // if the daughter is a <layer> then read it!
      if (v.first == "layer")
        ReadLayer(v.second.get<int>("<xmlattr>.n"), v.second);
    }
  } catch (...) {
    B2ERROR("Unknown excpetion rised during map initialization! "
            "SVD online to offline map corrupted." << endl <<
            "Be aware: the SVDShaperDigits will be unreliable." << endl);
    // To Do: rise an exception so that the calling module will skip the
    // SVDShaperDigits filling
    return;
  }
}

const SVDOnlineToOfflineMap::SensorInfo& SVDOnlineToOfflineMap::getSensorInfo(unsigned char FADC, unsigned char APV25)
{

  ChipID id(FADC, APV25);
  auto sensorIter = m_sensors.find(id);

  if (sensorIter == m_sensors.end()) {
    nBadMappingErrors++;

    if (!(nBadMappingErrors % m_errorRate)) B2ERROR("Combination not found in the SVD On-line to Off-line map:" << LogVar("FADC",
                                                      int(FADC)) << LogVar("APV", int(APV25)));

    m_currentSensorInfo.m_sensorID = 0;
    m_currentSensorInfo.m_channel0 = 0;
    m_currentSensorInfo.m_channel127 = 0;
    return m_currentSensorInfo;
  }
  m_currentSensorInfo = sensorIter->second;
  return m_currentSensorInfo;
}



const SVDOnlineToOfflineMap::ChipInfo& SVDOnlineToOfflineMap::getChipInfo(unsigned short layer,  unsigned short ladder,
    unsigned short dssd, bool side, unsigned short strip)
{
  SensorID id(layer, ladder, dssd, side);
  auto chipIter = m_chips.find(id);

  if (chipIter == m_chips.end())  B2WARNING(" The following combination: sensorID: " <<  layer << "." << ladder << "." << dssd <<
                                              ", isU=" << side << ", strip=" << strip <<
                                              " - is not found in the SVD Off-line to On-line map! The payload retrieved from database may be wrong! ");


  vector<ChipInfo> vecChipInfo = chipIter->second;

  ChipInfo info = {0, 0, 0, 0, 0};
  ChipInfo* pinfo = &info;

  for (std::vector<ChipInfo>::iterator it = vecChipInfo.begin() ; it != vecChipInfo.end(); ++it) {
    ChipInfo& chipInfo = *it;
    unsigned short channelFirst = min(chipInfo.stripFirst, chipInfo.stripLast);
    unsigned short channelLast = max(chipInfo.stripFirst, chipInfo.stripLast);

    if (strip >= channelFirst and strip <= channelLast) {
      pinfo = &chipInfo;
      pinfo->apvChannel = abs(strip - (pinfo->stripFirst));
    }
  }
  if (pinfo->fadc == 0) B2WARNING("The strip number " << strip << " is not found in the SVDOnlineToOfflineMap for sensor " << layer <<
                                    "." << ladder << "." << dssd << " on side " << (side ? "u" : "v") << "! Related APV chip is excluded in the hardware mapping.");

  m_currentChipInfo = *pinfo;
  return m_currentChipInfo;
}

bool SVDOnlineToOfflineMap::isAPVinMap(unsigned short layer,  unsigned short ladder,
                                       unsigned short dssd, bool side, unsigned short strip)
{
  SensorID id(layer, ladder, dssd, side);
  auto chipIter = m_chips.find(id);

  if (chipIter == m_chips.end()) return false;

  vector<ChipInfo> vecChipInfo = chipIter->second;

  ChipInfo info = {0, 0, 0, 0, 0};
  ChipInfo* pinfo = &info;

  for (std::vector<ChipInfo>::iterator it = vecChipInfo.begin() ; it != vecChipInfo.end(); ++it) {
    ChipInfo& chipInfo = *it;
    unsigned short channelFirst = min(chipInfo.stripFirst, chipInfo.stripLast);
    unsigned short channelLast = max(chipInfo.stripFirst, chipInfo.stripLast);

    if (strip >= channelFirst and strip <= channelLast) {
      pinfo = &chipInfo;
      pinfo->apvChannel = abs(strip - (pinfo->stripFirst));
    }
  }
  if (pinfo->fadc == 0) return false;

  return true;
}

bool SVDOnlineToOfflineMap::isAPVinMap(VxdID sensorID, bool side, unsigned short strip)
{
  return isAPVinMap(sensorID.getLayerNumber(), sensorID.getLadderNumber(), sensorID.getSensorNumber(), side, strip);
}


SVDShaperDigit* SVDOnlineToOfflineMap::NewShaperDigit(unsigned char FADC,
                                                      unsigned char APV25, unsigned char channel, short samples[6], float time)
{
  // Issue a warning, we'll be sending out a null pointer.
  if (channel > 127) {
    B2WARNING(" channel out of range (0-127):" << LogVar("channel", int(channel)));
    return nullptr;
  }
  const SensorInfo& info = getSensorInfo(FADC, APV25);
  short strip = getStripNumber(channel, info);

  SVDShaperDigit::APVRawSamples rawSamples;
  copy(samples, samples + SVDShaperDigit::c_nAPVSamples, rawSamples.begin());

  // create SVDShaperDigit only for existing sensor
  if (info.m_sensorID) {
    return new SVDShaperDigit(info.m_sensorID, info.m_uSide, strip, rawSamples, time);
  } else {
    return nullptr;
  }
}


void
SVDOnlineToOfflineMap::ReadLayer(int nlayer, ptree const& xml_layer)
{
  // traverse xml_layer: let us navigate through the daughters of <layer>
  for (ptree::value_type const& v : xml_layer) {
    // if the daughter is a <ladder> then read it!
    if (v.first == "ladder") {
      ReadLadder(nlayer, v.second.get<int>("<xmlattr>.n"), v.second);
    }
  }
}

void
SVDOnlineToOfflineMap::ReadLadder(int nlayer, int nladder, ptree const& xml_ladder)
{
  // traverse xml_ladder: let us navigate through the daughters of <ladder>
  for (ptree::value_type const& v : xml_ladder) {
    // if the daughter is a <sensor> then read it!
    if (v.first == "sensor") {
      ReadSensor(nlayer, nladder, v.second.get<int>("<xmlattr>.n"), v.second);
    }
  }
}

void
SVDOnlineToOfflineMap::ReadSensor(int nlayer, int nladder, int nsensor, ptree const& xml_sensor)
{
  // traverse xml_sensor: let us navigate through the daughters of <sensor>
  for (ptree::value_type const& v : xml_sensor) {
    // if the daughter is one side <> then read it!
    if (v.first == "side") {
      std::string tagSide = v.second.get<std::string>("<xmlattr>.side");

      bool isOnSideU = (tagSide == "U" || tagSide == "u");
      bool isOnSideV = (tagSide == "V" || tagSide == "v");

      if ((! isOnSideU) && (! isOnSideV)) {
        B2ERROR("Side '" << tagSide << "' on layer " << nlayer
                << " ladder " << nladder << " sensor " << nsensor
                << " is neither 'U' nor 'V'");
      }

      ReadSensorSide(nlayer, nladder, nsensor, isOnSideU, v.second);
    }
  }

}

void
SVDOnlineToOfflineMap::ReadSensorSide(int nlayer, int nladder, int nsensor, bool isU,
                                      ptree const& xml_side)
{

  // traverse xml_sensor: let us navigate through the daughters of <side>


  vector<ChipInfo> vecInfo;   // for packer
  SensorID sid(nlayer, nladder, nsensor, isU);

  for (ptree::value_type const& v : xml_side) {
    // if the daughter is a <chip>

    if (v.first == "chip") {
      auto tags = v.second;
      unsigned char  chipN = tags.get<unsigned char>("<xmlattr>.n");
      unsigned char  FADCn = tags.get<unsigned char>("<xmlattr>.FADCn");

      //storing info on FADC numbers and related APV chips
      FADCnumbers.insert(FADCn);
      APVforFADCmap.insert(std::pair<unsigned char, unsigned char>(FADCn, chipN));

      ChipID cid(FADCn, chipN);

      auto sensorIter = m_sensors.find(cid);

      if (sensorIter != m_sensors.end()) {
        B2WARNING("Repeated insertion for FADC " << FADCn << " and APV "
                  << chipN << ", layer/ladder/sensor " << nlayer << "/" << nladder
                  << "/" << nsensor << ", side " << (isU ? "u" : "v"));
      }
      unsigned short stripNumberCh0  = tags.get<unsigned short>("<xmlattr>.strip_number_of_ch0");
      unsigned short stripNumberCh127 = tags.get<unsigned short>("<xmlattr>.strip_number_of_ch127");


      SensorInfo sinfo;
      sinfo.m_sensorID = VxdID(nlayer, nladder, nsensor);
      sinfo.m_uSide = isU;
      sinfo.m_parallel = (stripNumberCh127 > stripNumberCh0);
      sinfo.m_channel0 = stripNumberCh0;
      sinfo.m_channel127 = stripNumberCh127;

      m_sensors[cid] = sinfo;

      // for packer
      ChipInfo cinfo;
      cinfo.fadc = FADCn;
      cinfo.apv  = chipN;
      cinfo.stripFirst = stripNumberCh0;
      cinfo.stripLast = stripNumberCh127;
      cinfo.apvChannel = 0;

      vecInfo.push_back(cinfo);

    } //chip

  } // for daughters

  m_chips[sid] = vecInfo;  // for packer
}

void SVDOnlineToOfflineMap::prepFADCmaps(FADCmap& map1, FADCmap& map2)
{
  unsigned short it = 0;

  for (auto ifadc = FADCnumbers.begin(); ifadc != FADCnumbers.end(); ++ifadc) {
    map2[it] = *ifadc;
    map1[*ifadc] = it++;
  }
}

void SVDOnlineToOfflineMap::prepareListOfMissingAPVs()
{


  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

  for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD))
    for (auto ladder : geoCache.getLadders(layer))
      for (Belle2::VxdID sensor :  geoCache.getSensors(ladder))
        for (int view = 0; view < 2; view++) {

          int nAPVs = 6;
          if (layer.getLayerNumber() != 3 && view == 0)
            nAPVs = 4;

          //loop on all APVs of the side
          for (int apv = 0; apv < nAPVs; apv++) {
            B2DEBUG(29, "checking " << sensor.getLayerNumber() << "." << sensor.getLadderNumber() << "." << sensor.getSensorNumber() <<
                    ", view = " << view << ", apv = " << apv);
            if (! isAPVinMap(sensor, view, apv * 128 + 63.5)) {
              missingAPV tmp_missingAPV;
              tmp_missingAPV.m_sensorID = sensor;
              tmp_missingAPV.m_isUSide = view;
              tmp_missingAPV.m_halfStrip = apv * 128 + 63.5;

              m_missingAPVs.push_back(tmp_missingAPV);
              B2DEBUG(29, "FOUND MISSING APV: " << sensor << ", " << view << ", " << apv);
            }

          }

        }

}
