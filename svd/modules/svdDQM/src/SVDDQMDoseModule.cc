#include <svd/modules/svdDQM/SVDDQMDoseModule.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include "TDirectory.h"
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
    "SVD Instantaneous Occupancy L@layer.@ladder.@sensor @side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
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

  h_occupancyL3U[0] = new TH1F(
    "SVDInstOccupancy_L3P",
    "SVD Instantaneous Occupancy L3 (all) P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL3U[1] = new TH1F(
    "SVDInstOccupancy_L3X1P",
    "SVD Instantaneous Occupancy L3.X.1 P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL3U[2] = new TH1F(
    "SVDInstOccupancy_L3X2P",
    "SVD Instantaneous Occupancy L3.X.2 P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL4U[0] = new TH1F(
    "SVDInstOccupancy_L4P",
    "SVD Instantaneous Occupancy L4 (all) P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL4U[1] = new TH1F(
    "SVDInstOccupancy_L4X1P",
    "SVD Instantaneous Occupancy L4.X.1 P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL4U[2] = new TH1F(
    "SVDInstOccupancy_L4X2P",
    "SVD Instantaneous Occupancy L4.X.2 P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL4U[3] = new TH1F(
    "SVDInstOccupancy_L4X3P",
    "SVD Instantaneous Occupancy L4.X.3 P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL5U[0] = new TH1F(
    "SVDInstOccupancy_L5P",
    "SVD Instantaneous Occupancy L5 (all) P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL5U[1] = new TH1F(
    "SVDInstOccupancy_L5X1P",
    "SVD Instantaneous Occupancy L5.X.1 P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL5U[2] = new TH1F(
    "SVDInstOccupancy_L5X2P",
    "SVD Instantaneous Occupancy L5.X.2 P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL5U[3] = new TH1F(
    "SVDInstOccupancy_L5X3P",
    "SVD Instantaneous Occupancy L5.X.3 P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL5U[4] = new TH1F(
    "SVDInstOccupancy_L5X4P",
    "SVD Instantaneous Occupancy L5.X.4 P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL6U[0] = new TH1F(
    "SVDInstOccupancy_L6P",
    "SVD Instantaneous Occupancy L6 (all) P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL6U[1] = new TH1F(
    "SVDInstOccupancy_L6X1P",
    "SVD Instantaneous Occupancy L6.X.1 P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL6U[2] = new TH1F(
    "SVDInstOccupancy_L6X2P",
    "SVD Instantaneous Occupancy L6.X.2 P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL6U[3] = new TH1F(
    "SVDInstOccupancy_L6X3P",
    "SVD Instantaneous Occupancy L6.X.3 P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL6U[4] = new TH1F(
    "SVDInstOccupancy_L6X4P",
    "SVD Instantaneous Occupancy L6.X.4 P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL6U[5] = new TH1F(
    "SVDInstOccupancy_L6X5P",
    "SVD Instantaneous Occupancy L6.X.5 P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);

  h_occupancyL3V[0] = new TH1F(
    "SVDInstOccupancy_L3N",
    "SVD Instantaneous Occupancy L3 (all) N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL3V[1] = new TH1F(
    "SVDInstOccupancy_L3X1N",
    "SVD Instantaneous Occupancy L3.X.1 N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL3V[2] = new TH1F(
    "SVDInstOccupancy_L3X2N",
    "SVD Instantaneous Occupancy L3.X.2 N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL4V[0] = new TH1F(
    "SVDInstOccupancy_L4N",
    "SVD Instantaneous Occupancy L4 (all) N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL4V[1] = new TH1F(
    "SVDInstOccupancy_L4X1N",
    "SVD Instantaneous Occupancy L4.X.1 N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL4V[2] = new TH1F(
    "SVDInstOccupancy_L4X2N",
    "SVD Instantaneous Occupancy L4.X.2 N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL4V[3] = new TH1F(
    "SVDInstOccupancy_L4X3N",
    "SVD Instantaneous Occupancy L4.X.3 N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL5V[0] = new TH1F(
    "SVDInstOccupancy_L5N",
    "SVD Instantaneous Occupancy L5 (all) N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL5V[1] = new TH1F(
    "SVDInstOccupancy_L5X1N",
    "SVD Instantaneous Occupancy L5.X.1 N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL5V[2] = new TH1F(
    "SVDInstOccupancy_L5X2N",
    "SVD Instantaneous Occupancy L5.X.2 N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL5V[3] = new TH1F(
    "SVDInstOccupancy_L5X3N",
    "SVD Instantaneous Occupancy L5.X.3 N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL5V[4] = new TH1F(
    "SVDInstOccupancy_L5X4N",
    "SVD Instantaneous Occupancy L5.X.4 N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL6V[0] = new TH1F(
    "SVDInstOccupancy_L6N",
    "SVD Instantaneous Occupancy L6 (all) N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL6V[1] = new TH1F(
    "SVDInstOccupancy_L6X1N",
    "SVD Instantaneous Occupancy L6.X.1 N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL6V[2] = new TH1F(
    "SVDInstOccupancy_L6X2N",
    "SVD Instantaneous Occupancy L6.X.2 N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL6V[3] = new TH1F(
    "SVDInstOccupancy_L6X3N",
    "SVD Instantaneous Occupancy L6.X.3 N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL6V[4] = new TH1F(
    "SVDInstOccupancy_L6X4N",
    "SVD Instantaneous Occupancy L6.X.4 N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL6V[5] = new TH1F(
    "SVDInstOccupancy_L6X5N",
    "SVD Instantaneous Occupancy L6.X.5 N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);

  h_occupancyL3Umid = new TH1F(
    "SVDInstOccupancy_L3Pmid",
    "SVD Instantaneous Occupancy L3 mid plane P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL4Umid = new TH1F(
    "SVDInstOccupancy_L4Pmid",
    "SVD Instantaneous Occupancy L4 mid plane P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL5Umid = new TH1F(
    "SVDInstOccupancy_L5Pmid",
    "SVD Instantaneous Occupancy L5 mid plane P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL6Umid = new TH1F(
    "SVDInstOccupancy_L6Pmid",
    "SVD Instantaneous Occupancy L6 mid plane P-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL3Vmid = new TH1F(
    "SVDInstOccupancy_L3Nmid",
    "SVD Instantaneous Occupancy L3 mid plane N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL4Vmid = new TH1F(
    "SVDInstOccupancy_L4Nmid",
    "SVD Instantaneous Occupancy L4 mid plane N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL5Vmid = new TH1F(
    "SVDInstOccupancy_L5Nmid",
    "SVD Instantaneous Occupancy L5 mid plane N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_occupancyL6Vmid = new TH1F(
    "SVDInstOccupancy_L6Nmid",
    "SVD Instantaneous Occupancy L6 mid plane N-side;Occupancy [%];Count / (0.05%)",
    100, 0, 5);

  oldDir->cd();
}

void SVDDQMDoseModule::initialize()
{
  REG_HISTOGRAM

  m_rawTTD.isOptional();
  m_digits.isOptional(m_SVDShaperDigitsName);
}

void SVDDQMDoseModule::beginRun()
{
  m_occupancy->reset();
  m_nHitsVsTime->reset();
  h_nEvtsVsTime->Reset();
}

void SVDDQMDoseModule::event()
{
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

  // Count hits
  map<VxdID, int> hitsU, hitsV;
  for (const SVDShaperDigit& hit : m_digits) {
    m_nHitsVsTime->fill(hit.getSensorID(), hit.isUStrip(), timeSinceInj, timeInCycle);
    // This relies on the fact that map::operator[] initializes nonexistent
    // items to 0 instead of throwing out_of_range (that's what map::at does)
    if (hit.isUStrip())
      hitsU[hit.getSensorID()]++;
    else
      hitsV[hit.getSensorID()]++;
  }

  // Count events
  h_nEvtsVsTime->Fill(timeSinceInj, timeInCycle);

  // Compute instantaneous occupancy
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  for (const auto& layer : geo.getLayers(VXD::SensorInfoBase::SVD)) {
    // Total hits [0] in the layer and [i] in LL.X.i (see h_occupancyL*)
    int layerHitsU[layer.getLayerNumber()] = {0};
    int layerHitsV[layer.getLayerNumber()] = {0};
    for (const auto& ladder : geo.getLadders(layer)) {
      for (const auto& sensor : geo.getSensors(ladder)) {
        const auto& sInfo = VXD::GeoCache::get(sensor);
        int hU = hitsU[sensor];
        int hV = hitsV[sensor];
        m_occupancy->fill(sensor, true, hU * 100.0 / sInfo.getUCells());
        m_occupancy->fill(sensor, false, hV * 100.0 / sInfo.getVCells());
        layerHitsU[0] += hU;
        layerHitsV[0] += hV;
        layerHitsU[sensor.getSensorNumber()] += hU;
        layerHitsV[sensor.getSensorNumber()] += hV;
      }
    }
    switch (layer.getLayerNumber()) {
      case 3:
        h_occupancyL3U[0]->Fill(layerHitsU[0] * 100.0 / 14.0 / 768.0);
        h_occupancyL3V[0]->Fill(layerHitsV[0] * 100.0 / 14.0 / 768.0);
        for (int i = 1; i <= 2; i++) {
          h_occupancyL3U[i]->Fill(layerHitsU[i] * 100.0 / 7.0 / 768.0);
          h_occupancyL3V[i]->Fill(layerHitsV[i] * 100.0 / 7.0 / 768.0);
        }
        break;
      case 4:
        h_occupancyL4U[0]->Fill(layerHitsU[0] * 100.0 / 30.0 / 768.0);
        h_occupancyL4V[0]->Fill(layerHitsV[0] * 100.0 / 30.0 / 512.0);
        for (int i = 1; i <= 3; i++) {
          h_occupancyL4U[i]->Fill(layerHitsU[i] * 100.0 / 10.0 / 768.0);
          h_occupancyL4V[i]->Fill(layerHitsV[i] * 100.0 / 10.0 / 512.0);
        }
        break;
      case 5:
        h_occupancyL5U[0]->Fill(layerHitsU[0] * 100.0 / 48.0 / 768.0);
        h_occupancyL5V[0]->Fill(layerHitsV[0] * 100.0 / 48.0 / 512.0);
        for (int i = 1; i <= 4; i++) {
          h_occupancyL5U[i]->Fill(layerHitsU[i] * 100.0 / 12.0 / 768.0);
          h_occupancyL5V[i]->Fill(layerHitsV[i] * 100.0 / 12.0 / 512.0);
        }
        break;
      case 6:
        h_occupancyL6U[0]->Fill(layerHitsU[0] * 100.0 / 80.0 / 768.0);
        h_occupancyL6V[0]->Fill(layerHitsV[0] * 100.0 / 80.0 / 512.0);
        for (int i = 1; i <= 5; i++) {
          h_occupancyL6U[i]->Fill(layerHitsU[i] * 100.0 / 16.0 / 768.0);
          h_occupancyL6V[i]->Fill(layerHitsV[i] * 100.0 / 16.0 / 512.0);
        }
        break;
    }
  }

  // Mid plane occupancies
  int midHitsU = 0, midHitsV = 0;
  for (const VxdID& id : c_L3midSensors) {
    midHitsU += hitsU[id];
    midHitsV += hitsV[id];
  }
  h_occupancyL3Umid->Fill(midHitsU * 100.0 / 768.0 / c_L3midSensors.size());
  h_occupancyL3Vmid->Fill(midHitsV * 100.0 / 768.0 / c_L3midSensors.size());

  midHitsU = midHitsV = 0;
  for (const VxdID& id : c_L4midSensors) {
    midHitsU += hitsU[id];
    midHitsV += hitsV[id];
  }
  h_occupancyL4Umid->Fill(midHitsU * 100.0 / 768.0 / c_L4midSensors.size());
  h_occupancyL4Vmid->Fill(midHitsV * 100.0 / 512.0 / c_L4midSensors.size());

  midHitsU = midHitsV = 0;
  for (const VxdID& id : c_L5midSensors) {
    midHitsU += hitsU[id];
    midHitsV += hitsV[id];
  }
  h_occupancyL5Umid->Fill(midHitsU * 100.0 / 768.0 / c_L5midSensors.size());
  h_occupancyL5Vmid->Fill(midHitsV * 100.0 / 512.0 / c_L5midSensors.size());

  midHitsU = midHitsV = 0;
  for (const VxdID& id : c_L6midSensors) {
    midHitsU += hitsU[id];
    midHitsV += hitsV[id];
  }
  h_occupancyL6Umid->Fill(midHitsU * 100.0 / 768.0 / c_L6midSensors.size());
  h_occupancyL6Vmid->Fill(midHitsV * 100.0 / 512.0 / c_L6midSensors.size());
}
