#include <svd/modules/svdDQM/SVDDQMDoseModule.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/dataobjects/VxdID.h>
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
    for (const auto& ladder : geo.getLadders(layer)) {
      for (const auto& sensor : geo.getSensors(ladder)) {
        // const auto& sInfo = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensor));
        const auto& sInfo = VXD::GeoCache::get(sensor);
        m_occupancy->fill(sensor, true, hitsU[sensor] * 100.0 / sInfo.getUCells());
        m_occupancy->fill(sensor, false, hitsV[sensor] * 100.0 / sInfo.getVCells());
      }
    }
  }
}
