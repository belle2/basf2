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
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "Name of the directory where histograms will be placed in the ROOT file.",
           std::string("SVDDose"));
  addParam("offlineZSShaperDigits", m_SVDShaperDigitsName,
           "Name of the SVDShaperDigits to use for computing occupancy (with ZS5).",
           std::string("SVDShaperDigitsZS5"));
  addParam("BeamRevolutionCycle", m_revolutionTime,
           "Beam revolution cycle in musec", 5120 / 508.0);
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
  const double timeInCycle = timeSinceInj - (int)(timeSinceInj / m_revolutionTime) * m_revolutionTime;
  const bool isHER = theTTD->GetIsHER(0);

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
