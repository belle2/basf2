#include <svd/modules/svdDQM/SVDDQMDoseModule.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/dataobjects/VxdID.h>
#include <svd/geometry/SensorInfo.h>
#include "TDirectory.h"

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;
using namespace Belle2::VXD;

REG_MODULE(SVDDQMDose)

SVDDQMDoseModule::SVDDQMDoseModule() : HistoModule()
{
  setDescription("Monitor SVD dose TODO...");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "Name of the directory where histograms will be placed in the ROOT file.",
           std::string("SVDDose"));
  addParam("ShaperDigits", m_SVDShaperDigitsName,
           "Name of the SVDShaperDigits to use for computing occupancy.",
           std::string(""));
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

  h_occupancy = new TH1F(
    "SVDInstOccupancy",
    "SVD Instantaneous Occupancy Distribution;Occupancy [%];Count / (0.05%)",
    100, 0, 5);
  h_nHitsVsTime = new TH2F(
    "SVDHitsVsInjTime2",
    "SVD Hits;Time since last injection [#mus];Time in beam cycle [#mus]",
    500, 0, 30e3, 100, 0, m_revolutionTime
  );
  h_nEvtsVsTime = new TH2F(
    "SVDEvtsVsInjTime2",
    "SVD Events;Time since last injection [#mus];Time in beam cycle [#mus]",
    500, 0, 30e3, 100, 0, m_revolutionTime
  );

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
  h_occupancy->Reset();
  h_nHitsVsTime->Reset();
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

  // Count hits
  unsigned int nHitsU = 0, nHitsV = 0;
  for (const SVDShaperDigit& hit : m_digits) {
    // TODO filtering on layer, ladder sensor? Dependin on parameters?
    if (hit.passesZS(1, 5)) {
      if (hit.isUStrip()) nHitsU++;
      // else nHitsV++;
    }
  }

  h_occupancy->Fill(nHitsU * 100.0 / 768.0 / 172.0); // TODO Use SensorInfo::getUCells()
  h_nHitsVsTime->Fill(timeSinceInj, timeInCycle, nHitsU);
  h_nEvtsVsTime->Fill(timeSinceInj, timeInCycle);
}
