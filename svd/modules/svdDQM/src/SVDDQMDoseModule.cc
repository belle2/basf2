#include <svd/modules/svdDQM/SVDDQMDoseModule.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include "TDirectory.h"
#include "TString.h"
#include <map>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

REG_MODULE(SVDDQMDose)

SVDDQMDoseModule::SVDDQMDoseModule() : HistoModule()
{
  setDescription("Monitor SVD dose TODO...");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("eventTypeFilter", m_eventFilter,
           "Types of events to include in the plots (see SVDDQMDoseModule::EEventType).", 7U);
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "Name of the directory where histograms will be placed in the ROOT file.",
           std::string("SVDDose"));
  addParam("offlineZSShaperDigits", m_SVDShaperDigitsName,
           "Name of the SVDShaperDigits to use for computing occupancy (with ZS5).",
           std::string("SVDShaperDigitsZS5"));
  addParam("noInjectionTimeout", m_noInjectionTime,
           "Time (microseconds) since last injection after which an event is considered \"No Injection\".",
           30e3);
  addParam("beamRevolutionCycle", m_revolutionTime,
           "Beam revolution cycle in microseconds.", 5120 / 508.0);
}

void SVDDQMDoseModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  TH1F h_occupancy(
    "SVDInstOccupancy_@layer_@ladder_@sensor_@side",
    "SVD Instantaneous Occupancy L@layer.@ladder.@sensor @side;Occupancy [%];Count / bin",
    20, 0, 7.8125);
  m_occupancy = new SVDHistograms<TH1F>(h_occupancy);

  TH2F h_nHitsVsTime(
    "SVDHitsVsInjTime2_@layer_@ladder_@sensor_@side",
    "SVD Hits L@layer.@ladder.@sensor @side;Time since last injection [#mus];Time in beam cycle [#mus];Hits / bin",
    500, 0, 30e3, 100, 0, m_revolutionTime);
  m_nHitsVsTime = new SVDHistograms<TH2F>(h_nHitsVsTime);

  h_nEvtsVsTime = new TH2F(
    "SVDEvtsVsInjTime2",
    "SVD Events;Time since last injection [#mus];Time in beam cycle [#mus];Events / bin",
    500, 0, 30e3, 100, 0, m_revolutionTime);

  m_groupOccupanciesU.reserve(c_sensorGroups.size()); // Allocate memory only once
  m_groupOccupanciesV.reserve(c_sensorGroups.size());
  TString name = "SVDInstOccupancy_", title = "SVD Instantaneous Occupancy ";
  TString axisTitle = ";Occupancy [%];Count / bin";
  for (const SensorGroup& group : c_sensorGroups) {
    m_groupOccupanciesU.push_back(
      new TH1F(name + group.nameSuffix + "P", title + group.titleSuffix + " P" + axisTitle,
               group.nBins, group.xMin, group.xMax));
    m_groupOccupanciesV.push_back(
      new TH1F(name + group.nameSuffix + "N", title + group.titleSuffix + " N" + axisTitle,
               group.nBins, group.xMin, group.xMax));
  }

  oldDir->cd();
}

void SVDDQMDoseModule::initialize()
{
  REG_HISTOGRAM

  // Parameters
  m_rawTTD.isOptional();
  m_digits.isOptional(m_SVDShaperDigitsName);

  // Total number of strips per group
  static bool nStripsComputed = false; // Compute only once
  if (nStripsComputed)
    return;
  nStripsComputed = true;
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  for (const auto& layer : geo.getLayers(VXD::SensorInfoBase::SVD)) {
    for (const auto& ladder : geo.getLadders(layer)) {
      for (const auto& sensor : geo.getSensors(ladder)) {
        const auto& sInfo = VXD::GeoCache::get(sensor);
        for (const SensorGroup& group : c_sensorGroups) {
          if (group.contains(sensor)) {
            group.nStripsU += sInfo.getUCells();
            group.nStripsV += sInfo.getVCells();
          }
        }
      }
    }
  }
}

void SVDDQMDoseModule::beginRun()
{
  m_occupancy->reset();
  m_nHitsVsTime->reset();
  h_nEvtsVsTime->Reset();
  for (const auto& histPtr : m_groupOccupanciesU)
    histPtr->Reset();
  for (const auto& histPtr : m_groupOccupanciesV)
    histPtr->Reset();
}

void SVDDQMDoseModule::event()
{
  // Allocate only once, especially good for the vectors and the maps
  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  static map<VxdID, int> hitsU;
  static map<VxdID, int> hitsV;
  static vector<int> groupHitsU(c_sensorGroups.size(), 0);
  static vector<int> groupHitsV(c_sensorGroups.size(), 0);

  if (!m_rawTTD.isValid()) {
    B2WARNING("Missing RawFTSW, SVDDQMDose is skipped.");
    return;
  }
  if (!m_digits.isValid()) {
    B2WARNING("Missing SVDShaperDigit " << m_SVDShaperDigitsName
              << ", SVDDQMDose is skipped.");
    return;
  }

  if (m_rawTTD.getEntries() == 0)
    return;
  RawFTSW* theTTD = m_rawTTD[0];
  // 127 MHz is the (inexactly rounded) clock of the ticks
  const double timeSinceInj = theTTD->GetTimeSinceLastInjection(0) / 127.0;
  const bool isHER = theTTD->GetIsHER(0);
  const EEventType eventType = timeSinceInj > m_noInjectionTime ? c_NoInjection : (isHER ? c_HERInjection : c_LERInjection);
  if (((unsigned int)eventType & m_eventFilter) == 0U)
    return;
  const double timeInCycle = timeSinceInj - (int)(timeSinceInj / m_revolutionTime) * m_revolutionTime;

  // Reset counters
  for (auto& [sensor, count] : hitsU) count = 0;
  for (auto& [sensor, count] : hitsV) count = 0;
  for (int& count : groupHitsU) count = 0;
  for (int& count : groupHitsV) count = 0;

  // Count hits
  for (const SVDShaperDigit& hit : m_digits) {
    m_nHitsVsTime->fill(hit.getSensorID(), hit.isUStrip(), timeSinceInj, timeInCycle);
    const VxdID& sensorID = hit.getSensorID();
    if (hit.isUStrip()) {
      // This relies on the fact that std::map::operator[] adds an item and
      // sets it to 0 when it doesn't exist
      hitsU[sensorID]++;
      for (unsigned int i = 0; i < c_sensorGroups.size(); i++)
        if (c_sensorGroups[i].contains(sensorID))
          groupHitsU[i]++;
    } else {
      hitsV[sensorID]++;
      for (unsigned int i = 0; i < c_sensorGroups.size(); i++)
        if (c_sensorGroups[i].contains(sensorID))
          groupHitsV[i]++;
    }
  }

  // Count events
  h_nEvtsVsTime->Fill(timeSinceInj, timeInCycle);

  // Compute instantaneous occupancy
  for (const auto& layer : geo.getLayers(VXD::SensorInfoBase::SVD)) {
    for (const auto& ladder : geo.getLadders(layer)) {
      for (const auto& sensor : geo.getSensors(ladder)) {
        const auto& sInfo = VXD::GeoCache::get(sensor);
        m_occupancy->fill(sensor, true, hitsU[sensor] * 100.0 / sInfo.getUCells());
        m_occupancy->fill(sensor, false, hitsV[sensor] * 100.0 / sInfo.getVCells());
      }
    }
  }
  for (unsigned int i = 0; i < c_sensorGroups.size(); i++) {
    m_groupOccupanciesU[i]->Fill(groupHitsU[i] * 100.0 / c_sensorGroups[i].nStripsU);
    m_groupOccupanciesV[i]->Fill(groupHitsV[i] * 100.0 / c_sensorGroups[i].nStripsV);
  }
}

const std::vector<SVDDQMDoseModule::SensorGroup> SVDDQMDoseModule::c_sensorGroups = {
  {"L3XX", "L3 all", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 3; }},
  {"L3X1", "L3.X.1", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 3 && s.getSensorNumber() == 1; }},
  {"L3X2", "L3.X.2", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 3 && s.getSensorNumber() == 2; }},
  {"L4XX", "L4 all", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 4; }},
  {"L4X1", "L4.X.1", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 4 && s.getSensorNumber() == 1; }},
  {"L4X2", "L4.X.2", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 4 && s.getSensorNumber() == 2; }},
  {"L4X3", "L4.X.3", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 4 && s.getSensorNumber() == 3; }},
  {"L5XX", "L5 all", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 5; }},
  {"L5X1", "L5.X.1", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 5 && s.getSensorNumber() == 1; }},
  {"L5X2", "L5.X.2", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 5 && s.getSensorNumber() == 2; }},
  {"L5X3", "L5.X.3", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 5 && s.getSensorNumber() == 3; }},
  {"L5X4", "L5.X.4", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 5 && s.getSensorNumber() == 4; }},
  {"L6XX", "L6 all", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 6; }},
  {"L6X1", "L6.X.1", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 6 && s.getSensorNumber() == 1; }},
  {"L6X2", "L6.X.2", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 6 && s.getSensorNumber() == 2; }},
  {"L6X3", "L6.X.3", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 6 && s.getSensorNumber() == 3; }},
  {"L6X4", "L6.X.4", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 6 && s.getSensorNumber() == 4; }},
  {"L6X5", "L6.X.5", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 6 && s.getSensorNumber() == 5; }},
  {"L3mid", "L3 mid plane (L3.1.X and L3.2.X)", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 3 && s.getLadderNumber() < 3; }},
  {"L4mid", "L4 mid plane (L4.6.1 and L4.6.2)", 30, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 4 && s.getLadderNumber() == 6 && s.getSensorNumber() < 3; }},
  {"L5mid", "L5 mid plane (L5.8.1 and L5.8.2)", 30, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 5 && s.getLadderNumber() == 8 && s.getSensorNumber() < 3; }},
  {"L6mid", "L6 mid plane (L6.10.1 and L6.10.2)", 30, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 6 && s.getLadderNumber() == 10 && s.getSensorNumber() < 3; }}
};
