/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/online/SVDStripNoiseMap.h>
#include <framework/logging/Logger.h>

#include <cstdlib>
#include <fstream>

using namespace Belle2;
using namespace std;

SVDStripNoiseMap::SVDStripNoiseMap(SVDOnlineToOfflineMap* onl2offl_map_ptr):
  m_onl2offl_map_ptr(onl2offl_map_ptr)
{

  memset(m_pedestalMap , 0, sizeof(m_pedestalMap));
  memset(m_noiseMap    , 0, sizeof(m_noiseMap));
  memset(m_thresholdMap, 0, sizeof(m_thresholdMap));
  memset(m_goodStripMap, 0, sizeof(m_goodStripMap));

}

SVDStripNoiseMap::SVDStripNoiseMap(SVDOnlineToOfflineMap* onl2offl_map_ptr,
                                   const string& noisefilename):
  m_onl2offl_map_ptr(onl2offl_map_ptr)
{
  this->initializeMap(noisefilename);
}

int SVDStripNoiseMap::initializeMap(const string& noisefilename)
{

  B2INFO("initializeMap()");

  if (noisefilename == "") {
    B2INFO("CAUTION:: Pedestal and noise values are not changed, because noise file is not specified.");
    return 0;
  }

  if (m_onl2offl_map_ptr == nullptr) {
    B2ERROR("SVDOnlineToOfflineMap is not assigned correctly.");
    return -1;
  }

  memset(m_pedestalMap , 0, sizeof(m_pedestalMap));
  memset(m_noiseMap    , 0, sizeof(m_noiseMap));
  memset(m_thresholdMap, 0, sizeof(m_thresholdMap));
  memset(m_goodStripMap, 0, sizeof(m_goodStripMap));

  ifstream noisefile(noisefilename.c_str());
  if (!noisefile.is_open()) {
    B2ERROR("Cannot open noise file: " << noisefilename);
    return -1;
  }

  int   module, apv, strip, fadc, fadc_ch, good;
  float pedestal, gaus_noise, noise, rms, cmc;

  short file_section = 0;
  string line;
  while (getline(noisefile, line)) {

    if (line.substr(0, 1) == "#") continue;

    size_t space_pos;
    while ((space_pos = line.find(" ")) != string::npos) {
      line.erase(space_pos, 1);
    }

    if (line.empty()) continue;

    //B2INFO(line);

    if (line.substr(0, 1) == "[" && line.substr(line.size() - 1, 1) == "]") {
      if (line == "[noi]") file_section = 1;
      else if (line == "[fit]") file_section = 2;
      else                   file_section = 3;
    }//if(line.substr(0,1)=="["&&line.substr(line.size()-1,1)=="]"){
    else if (file_section == 1) {
      sscanf(line.c_str(), "str=%d,%d,%d,%d,%d,%d,%f,%f,%f,%f,%f",
             &module, &apv, &strip, &fadc, &fadc_ch, &good,
             &pedestal, &gaus_noise, &noise, &rms, &cmc);

      //**************************************************
      //*** This fadc_id definition is workround solution
      //*** for DESY beam data analysis.
      //*** It must be removed in future. (Katsuro)
      //**************************************************
      short fadc_id = 0;
      if (fadc == 0) fadc_id = 1;   //0x01 (p-side)
      else if (fadc == 1) fadc_id = 129; //0x81 (n-side)
      else             fadc_id = 1; //0x01
      //**************************************************

      //*** module value starts from 1 ***//
      unsigned short apv_base = 0;
      switch (module) {
        case  1 : apv_base = 0;  break;
        case  2 : apv_base = 6;  break;
        case  3 : apv_base = 24; break;
        case  4 : apv_base = 30; break;
        case  8 : apv_base = 0;  break;
        case  9 : apv_base = 6;  break;
        case 10 : apv_base = 24; break;
        case 11 : apv_base = 30; break;
        default : apv_base = 0;  break;
      }
      unsigned short apv_id   = apv_base + apv;
      VxdID vxd_id =
        (m_onl2offl_map_ptr->getSensorInfo(fadc_id, apv_id)).m_sensorID;
      //bool is_u = (((fadc_id>>7)&0x1)==0x0) ? true : false;
      bool is_u =
        (m_onl2offl_map_ptr->getSensorInfo(fadc_id, apv_id)).m_uSide;
      short svd_sensor_id = SVDPar::getSVDSensorID(vxd_id, is_u);
      if (svd_sensor_id < 0) {
        B2ERROR("Invalid SVDSensorID: " << svd_sensor_id << " (FADC ID: " << fadc_id << ", APV: " << apv << ")");
        return -1;
      }

      B2INFO("Sensor ID: " << svd_sensor_id << " Module: " << module << " VxdID: " << vxd_id.getID() << " (FADC ID: " << fadc_id <<
             ", APV: " << apv << ", STRIP: " << strip << ") " << ((good == 0) ? "BAD" : "   ") << " ped: " << pedestal << ", noi: " << noise);

      m_pedestalMap [svd_sensor_id][strip] = pedestal;
      m_noiseMap    [svd_sensor_id][strip] = gaus_noise;
      m_thresholdMap[svd_sensor_id][strip] = 5 * gaus_noise;
      m_goodStripMap[svd_sensor_id][strip] = (good == 0) ? false : true;

    }//else if(file_section==1){

  }//while(getline(noisefile,line)){

  return 0;
}

float SVDStripNoiseMap::getPedestal(VxdID id, bool is_u, short strip)
{

  if (strip < 0 || SVDPar::maxStrip <= strip) {
    B2ERROR("Invalid strip number: " << strip);
    return -9999.0;
  }

  short sensor_id = SVDPar::getSVDSensorID(id, is_u);
  if (sensor_id < 0 || SVDPar::nSensorID <= sensor_id) {
    B2ERROR("Invalid SVDSensorID: " << sensor_id);
    return -9999.0;
  }

  return m_pedestalMap[sensor_id][strip];
}

float SVDStripNoiseMap::getNoise(VxdID id, bool is_u, short strip)
{

  if (strip < 0 || SVDPar::maxStrip <= strip) {
    B2ERROR("Invalid strip number: " << strip);
    return -9999.0;
  }

  short sensor_id = SVDPar::getSVDSensorID(id, is_u);
  if (sensor_id < 0 || SVDPar::nSensorID <= sensor_id) {
    B2ERROR("Invalid SVDSensorID: " << sensor_id);
    return -9999.0;
  }

  return m_noiseMap[sensor_id][strip];
}

float SVDStripNoiseMap::getThreshold(VxdID id, bool is_u, short strip)
{

  if (strip < 0 || SVDPar::maxStrip <= strip) {
    B2ERROR("Invalid strip number: " << strip);
    return -9999.0;
  }

  short sensor_id = SVDPar::getSVDSensorID(id, is_u);
  if (sensor_id < 0 || SVDPar::nSensorID <= sensor_id) {
    B2ERROR("Invalid SVDSensorID: " << sensor_id);
    return -9999.0;
  }

  return m_thresholdMap[sensor_id][strip];
}

bool SVDStripNoiseMap::isGood(VxdID id, bool is_u, short strip)
{

  if (strip < 0 || SVDPar::maxStrip <= strip) {
    B2ERROR("Invalid strip number: " << strip);
    return false;
  }

  short sensor_id = SVDPar::getSVDSensorID(id, is_u);
  if (sensor_id < 0 || SVDPar::nSensorID <= sensor_id) {
    B2ERROR("Invalid SVDSensorID: " << sensor_id);
    return false;
  }

  return m_goodStripMap[sensor_id][strip];
}
