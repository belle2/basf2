/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/reconstruction/StripCalibrationMap.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <svd/online/SVDOnlineToOfflineMap.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/utilities/FileSystem.h>
#include <sstream>
#include <algorithm>
#include <iterator>

using namespace Belle2;
using namespace Belle2::SVD;

//-----------------------------------------------------------------------------
namespace {
  template<typename T>
  void readArray(const GearDir& motherTag, std::string tagname,
                 std::array<T, 128>& values)
  {
    std::istringstream iss(motherTag.getString(tagname));
    std::copy(std::istream_iterator<T>(iss), std::istream_iterator<T>(),
              values.begin());
  }
} // anonymous namespace

//-----------------------------------------------------------------------------
bool StripCalibrationMap::readMapFromXml(const std::string& ooMapName, const std::string& calFileName)
{
  bool result = true;
  // Online-to-Offline map
  SVDOnlineToOfflineMap ooMap(ooMapName);

  // Name of the input config file
  Gearbox& reader = Gearbox::getInstance();
  std::size_t lastSlash = calFileName.rfind('/');
  std::string backendName = std::string("file:").append(calFileName.substr(0, lastSlash));

  reader.setBackends({backendName});
  reader.close();
  reader.open(calFileName.substr(lastSlash + 1));
  try {
    GearDir cfg("/cfg_document/calibration_run/");
    m_calLevel = cfg.getInt("@level", 0);
  } catch (gearbox::PathEmptyError e) {
    B2FATAL(e.what());
    result = false;
  } catch (gearbox::ConversionError e) {
    B2FATAL(e.what());
    result = false;
  }
  // OK, so we can read, let us store what we find piece by piece.
  std::array<decltype(StripData::m_noise), 128> noiseArray;
  std::array<decltype(StripData::m_goodStrip), 128> maskArray;
  std::array<decltype(StripData::m_calPeak), 128> peakArray;
  std::array<decltype(StripData::m_calWidth), 128> peakWidthArray;
  std::array<decltype(StripData::m_calTimeDelay), 128> peakTimeArray;

  try {
    GearDir backend_layout("cfg_document/back_end_layout/");
    for (const GearDir& fadc_tag : backend_layout.getNodes("fadc")) {
      int fadc_no = std::stoi(fadc_tag.getString("@id", "-1"), nullptr, 0);
      B2DEBUG(300, "FADC: " << fadc_no);
      for (const GearDir& adc_tag : fadc_tag.getNodes("adc")) {
        int adc_no = std::stoi(adc_tag.getString("@id", "-1"), nullptr, 0);
        B2DEBUG(300, "\tADC: " << adc_no);
        for (const GearDir& apv25_tag : adc_tag.getNodes("apv25")) {
          int apv25_no = std::stoi(apv25_tag.getString("@id", "-1"), nullptr, 0);
          B2DEBUG(300, "\t\tAPV25: " << apv25_no);
          int chip_no = 6 * adc_no + apv25_no;
          auto stripInfo = ooMap.getSensorInfo(
                             static_cast<unsigned char>(fadc_no),
                             static_cast<unsigned char>(chip_no)
                           );
          B2DEBUG(300, "\t\t\tVxdID: " << stripInfo.m_sensorID);
          B2DEBUG(300, "\t\t\tside: " << stripInfo.m_uSide);
          B2DEBUG(300, "\t\t\trows: " << stripInfo.m_channel0 << " to " <<
                  stripInfo.m_channel127);
          // Determine how we count strips
          int countIncrement = 1;
          int firstStrip = stripInfo.m_channel0;
          int lastStrip = stripInfo.m_channel127;
          if (stripInfo.m_channel0 > stripInfo.m_channel127) {
            countIncrement = -1;
            firstStrip = stripInfo.m_channel127;
            lastStrip = stripInfo.m_channel0;
          }
          // Get the noises array
          readArray(apv25_tag, "noises", noiseArray);
          // Get the masks array
          readArray(apv25_tag, "good_strips", maskArray);
          // Get the peaks array
          readArray(apv25_tag, "cal_peaks", peakArray);
          // Get peak widths
          readArray(apv25_tag, "cal_width", peakWidthArray);
          // Get peak times
          readArray(apv25_tag, "cal_peak_time", peakTimeArray);
          // Now pack all to the map
          B2DEBUG(300, "Saving...");
          for (size_t i = 0, strip = firstStrip; i < 128;
               strip += countIncrement, ++i) {
            // gain is electrons/ADU, so m_calLevel * 1000 / peak. We se
            // gain to 1.0 for nonsensical peak measurements.
            double gain = (peakArray[i] > 1.0) ? 22500 / peakArray[i] : 1.0;
            // noise: we convert form ADU to e-, so gain*noise
            double noise = gain * noiseArray[i];
            // peakWidth, peakTime: In hardware tests, time is measured in
            // units of 1/8 of APV25 cycle, i.e. 31.44/8 = 3.93 ns.
            // Hao's width is FWHM. Beta-prime FWHM is 0.50305 x width.
            // So the correction coefficient is 3.93/0.50305 ns = 7.81234
            // Peak time: we convert to time shift, for beta-prime it is
            // peak time - 0.25 * width
            double width = 7.81234 * peakWidthArray[i];
            double timeShift = 3.93 * peakTimeArray[i] - 0.25 * width;
            StripData package(maskArray[i], noise, gain, width, timeShift);
            B2DEBUG(300, "Saving: " << package.m_goodStrip << " " <<
                    package.m_noise << " " << package.m_calPeak << " " <<
                    package.m_calWidth << " " << package.m_calTimeDelay);
            unsigned int uID = getUniqueChannelID(stripInfo.m_sensorID,
                                                  stripInfo.m_uSide, strip);
            m_stripData.insert(std::make_pair(uID, package));
          } // for strips
          B2DEBUG(300, "Done.");
        } // for apv25 tags
      } // for adc tags
    } // for fadc tags
  } catch (gearbox::PathEmptyError e) {
    B2DEBUG(300, e.what());
    result = false;
  } catch (gearbox::ConversionError e) {
    B2DEBUG(300, e.what());
    result = false;
  }
  reader.close();
  B2DEBUG(300, "Document closed.");
  return result;
}

//-----------------------------------------------------------------------------
void StripCalibrationMap::constructDefaultMap()
{
  // Get list of sensors from the GeoCache
  const VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  const std::vector<VxdID> sensors = geo.getListOfSensors();
  if (sensors.size() == 0)
    B2FATAL("No GeoCache data, the code will not work!");

  // Clear the map
  m_stripData.clear();

  for (auto id : sensors) {
    unsigned int layer = id.getLayerNumber();
    if ((layer < 3) || (layer > 6)) continue; // not a SVD sensor in Belle II or TB
    const SensorInfo& info = dynamic_cast<const SVD::SensorInfo&>(geo.getSensorInfo(id));
    if (info.getType() != VXD::SensorInfoBase::SVD) {
      // This is a problem: We have non-SVD sensors where only SVD have to be. Report!
      B2FATAL("SVD sensors expected in layers 3 to 6. This code will not work!");
    }
    // u side
    StripData uStripData(true, info.getElectronicNoiseU(), info.getAduEquivalentU(), 270.0, 5.4);
    for (unsigned int uStrip = 0; uStrip < info.getUCells(); ++uStrip) {
      unsigned int uID = getUniqueChannelID(id, true, uStrip);
      m_stripData.insert(std::make_pair(uID, uStripData));
    }
    // v side
    StripData vStripData(true, info.getElectronicNoiseV(), info.getAduEquivalentV(), 270.0, -0.0);
    for (unsigned int vStrip = 0; vStrip < info.getVCells(); ++vStrip) {
      unsigned int uID = getUniqueChannelID(id, false, vStrip);
      m_stripData.insert(std::make_pair(uID, vStripData));
    }
  } // for sensors
}

//-----------------------------------------------------------------------------
std::string StripCalibrationMap::toString()
{
  std::ostringstream os;
  for (auto mapItem : m_stripData) {
    os << mapItem.first << ": " << mapItem.second.m_goodStrip << " "
       << mapItem.second.m_noise << " "
       << mapItem.second.m_calPeak << " "
       << mapItem.second.m_calWidth << " "
       << mapItem.second.m_calTimeDelay << std::endl;
  }
  return os.str();
}
