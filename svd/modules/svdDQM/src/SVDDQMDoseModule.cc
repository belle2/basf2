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
  // TODO parameters ...
}

void SVDDQMDoseModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  // TODO histograms ...

  oldDir->cd();
}

void SVDDQMDoseModule::initialize()
{
  REG_HISTOGRAM

  m_rawTTD.isOptional();
  m_digits.isOptional(m_SVDShaperDigitsName);
  // TODO parameters ...
}

void SVDDQMDoseModule::beginRun()
{
  // TODO reset histograms ...

  // Force re-checking masked strips
  m_nActiveStripsU = 0;
  m_nActiveStripsV = 0;
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

  // Count masked strips (if not already done)
  // TODO Move this part to beginRun? (and also initialize?)
  if (m_nActiveStripsU == 0) {
    if (!m_maskedStrips.isValid()) {
      B2WARNING("Missing SVDFADCMaskedStrips, SVDDQMDose is skipped.");
      return;
    }
    GeoCache& geo = GeoCache::getInstance();
    // TODO more filtering on layer, ladder, sensor? Depending on parameters?
    for (const VxdID& layerID : geo.getLayers(VXD::SensorInfoBase::SVD)) {
      for (const VxdID& ladderID : geo.getLadders(layerID)) {
        for (const VxdID& sensorID : geo.getSensors(ladderID)) {
          const SVD::SensorInfo& sInfo = dynamic_cast<const SVD::SensorInfo&>(GeoCache::get(sensorID));
          for (unsigned short strip = 0; strip < sInfo.getUCells(); strip++)
            if (!m_maskedStrips.isMasked(sensorID, true, strip))
              m_nActiveStripsU++;
          for (unsigned short strip = 0; strip < sInfo.getVCells(); strip++)
            if (!m_maskedStrips.isMasked(sensorID, false, strip))
              m_nActiveStripsV++;
        }
      }
    }
  }

  if (m_rawTTD.getEntries() == 0)
    return;
  RawFTSW* theTTD = m_rawTTD[0];

  // Count hits
  unsigned int nHitsU = 0, nHitsV = 0;
  for (const SVDShaperDigit& hit : m_digits) {
    // TODO filtering on layer, ladder sensor? Dependin on parameters?
    if (hit.passesZS(1 /* TODO is it 1? */, 5)) {
      if (hit.isUStrip()) nHitsU++;
      else nHitsV++;
    }
  }

  // TODO fill histograms ...
}
