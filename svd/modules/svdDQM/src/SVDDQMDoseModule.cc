/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdDQM/SVDDQMDoseModule.h>
// Includes for the forward declarations
#include <rawdata/dataobjects/RawFTSW.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <vxd/dataobjects/VxdID.h>
// #include <svd/calibration/SVDFADCMaskedStrips.h>
// Other includes
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <TDirectory.h>
#include <TString.h>
#include <TMath.h>
#include <cmath>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

REG_MODULE(SVDDQMDose);

SVDDQMDoseModule::SVDDQMDoseModule() : HistoModule()
{
  setDescription("The SVD dose-monitoring DQM module. Fills histograms of the SVD's instantaneous occupancy and "
                 "of SVD occupancy vs time since last injection and time in beam revolution cycle.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("eventTypeFilter", m_eventFilter,
           "Types of events to include in the plots (1 = less than noInjectionTimeout after HER injection, "
           "2 = less than noInjectionTimeout after LER injection, 4 = more than noInjectionTimeout after any "
           "injection; bitwise or combinations are possible; see SVDDQMDoseModule::EEventType).", 7U);
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "Name of the directory where histograms will be placed in the ROOT file.",
           std::string("SVDDose"));
  addParam("offlineZSShaperDigits", m_SVDShaperDigitsName,
           "Name of the SVDShaperDigits to use for computing occupancy (default is SVDShaperDigitsZS5).",
           std::string("SVDShaperDigitsZS5"));
  addParam("noInjectionTimeout", m_noInjectionTime,
           "Time (microseconds) since last injection after which an event is considered \"No Injection\". "
           "Also the limit for the x axis of the 2D histograms.",
           30e3);
  m_trgTypes.push_back(TRGSummary::TTYP_POIS);
  addParam("trgTypes", m_trgTypes,
           "Trigger types for event selection. Empty to select everything. "
           "Default is only Poisson w/o inj. veto.",
           m_trgTypes);
}

void SVDDQMDoseModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  // Round noInjectionTimeout to the nearest multiple of the revolution cycle
  m_noInjectionTime = round(m_noInjectionTime / c_revolutionTime) * c_revolutionTime;

  h_nEvtsVsTime = new TH2F(
    "SVDEvtsVsTime",
    "SVD Events;Time since last injection [#mus];Time in beam cycle [#mus];Events / bin",
    500, 0, m_noInjectionTime, 100, 0, c_revolutionTime);

  m_groupOccupanciesU.reserve(c_sensorGroups.size()); // Allocate memory only once
  TString name = "SVDInstOccu_";
  TString title = "SVD Instantaneous Occupancy ";
  TString axisTitle = ";Occupancy [%];Count / bin";
  for (const SensorGroup& group : c_sensorGroups) {
    m_groupOccupanciesU.push_back(
      new TH1F(name + group.nameSuffix + "U",
               title + group.titleSuffix + " U-side" + axisTitle,
               group.nBins, group.xMin, group.xMax));
  }

  m_groupNHitsU.reserve(c_sensorGroups.size()); // Allocate memory only once
  name = "SVDHitsVsTime_";
  title = "SVD Hits ";
  axisTitle = ";Time since last injection [#mus];Time in beam cycle[#mus];Hits / bin";
  for (const SensorGroup& group : c_sensorGroups) {
    m_groupNHitsU.push_back(
      new TH2F(name + group.nameSuffix + "U",
               title + group.titleSuffix + " U-side" + axisTitle,
               500, 0, m_noInjectionTime, 100, 0, c_revolutionTime));
  }

  // Nbins for 1D histos such that bin width = beam_revolution_time / 2
  int nb1 = TMath::Nint(m_noInjectionTime * 2.0 / c_revolutionTime);
  h_nEvtsVsTime1 = new TH1F(
    "SVDEvtsVsTime1", "SVD Events;Time since last injection [#mus];Events / bin",
    nb1, 0, m_noInjectionTime);

  m_groupNHits1U.reserve(c_sensorGroups.size()); // Allocate memory only once
  name = "SVDHitsVsTime1_";
  title = "SVD Hits ";
  axisTitle = ";Time since last injection [#mus];Hits / bin";
  for (const SensorGroup& group : c_sensorGroups) {
    m_groupNHits1U.push_back(
      new TH1F(name + group.nameSuffix + "U",
               title + group.titleSuffix + " U-side" + axisTitle,
               nb1, 0, m_noInjectionTime));
  }

  // Include directory name in title: this histogram's canvas is made automatically,
  // so no analysis modules changes its title to show the event selection used.
  title = "SVDBunchNumVSNStrips - ";
  title += m_histogramDirectoryName;
  title += ";Bunch No.;Number of fired strips;Events / bin";
  h_bunchNumVsNHits = new TH2F("SVDBunchNumVSNStrips", title, 1280, 0, 1280, 10, 0, 10000);

  oldDir->cd();
}

void SVDDQMDoseModule::initialize()
{
  REG_HISTOGRAM

  // Parameters
  m_rawTTD.isOptional();
  m_digits.isOptional(m_SVDShaperDigitsName);
  m_trgSummary.isOptional();

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
            // TODO exclude strips that are masked on FADC? It shouldn't matter much...
            group.nStripsU += sInfo.getUCells();
          }
        }
      }
    }
  }
}

void SVDDQMDoseModule::beginRun()
{
  h_nEvtsVsTime->Reset();
  h_nEvtsVsTime1->Reset();
  for (const auto& histPtr : m_groupOccupanciesU)
    histPtr->Reset();
  for (const auto& histPtr : m_groupNHitsU)
    histPtr->Reset();
  for (const auto& histPtr : m_groupNHits1U)
    histPtr->Reset();
  h_bunchNumVsNHits->Reset();
}

void SVDDQMDoseModule::event()
{
  // Allocate only once, especially good for the vectors
  // static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  static vector<int> groupHitsU(c_sensorGroups.size(), 0);

  if (m_trgTypes.size()) { // If not trg types are given, take everything
    if (!m_trgSummary.isValid()) {
      B2WARNING("Missing TRGSummary, SVDDQMDose is skipped.");
      return;
    }
    auto ttyp = m_trgSummary->getTimType();
    bool discardEvent = true;
    for (const auto& ttyp2 : m_trgTypes) {
      if (ttyp == ttyp2) {
        discardEvent = false;
        break;
      }
    }
    if (discardEvent)
      return;
  }

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
  const double timeSinceInj = theTTD->GetTimeSinceLastInjection(0) / c_globalClock;
  const bool isHER = theTTD->GetIsHER(0);
  const EEventType eventType = timeSinceInj > m_noInjectionTime ? c_NoInjection : (isHER ? c_HERInjection : c_LERInjection);
  if (((unsigned int)eventType & m_eventFilter) == 0U)
    return;
  const double timeInCycle = timeSinceInj - (int)(timeSinceInj / c_revolutionTime) * c_revolutionTime;

  // Reset counters
  for (int& count : groupHitsU) count = 0;

  // Count hits
  for (const SVDShaperDigit& hit : m_digits) {
    const VxdID& sensorID = hit.getSensorID();
    if (hit.isUStrip()) {
      for (unsigned int i = 0; i < c_sensorGroups.size(); i++) {
        if (c_sensorGroups[i].contains(sensorID)) {
          groupHitsU[i]++; // For instantaneous occupancy
          m_groupNHitsU[i]->Fill(timeSinceInj, timeInCycle);
          m_groupNHits1U[i]->Fill(timeSinceInj);
        }
      }
    }
  }

  // Count events
  h_nEvtsVsTime->Fill(timeSinceInj, timeInCycle);
  h_nEvtsVsTime1->Fill(timeSinceInj);

  // Compute instantaneous occupancy
  for (unsigned int i = 0; i < c_sensorGroups.size(); i++)
    m_groupOccupanciesU[i]->Fill(groupHitsU[i] * 100.0 / c_sensorGroups[i].nStripsU);

  // Bunch num vs fired strips
  h_bunchNumVsNHits->Fill(theTTD->GetBunchNumber(0), m_digits.getEntries());
}

const std::vector<SVDDQMDoseModule::SensorGroup> SVDDQMDoseModule::c_sensorGroups = {
  {"L3XX", "L3", c_defaultNBins, c_defaultOccuMin, c_defaultOccuMax, [](const VxdID & s) { return s.getLayerNumber() == 3; }},
  {"L4XX", "L4", c_defaultNBins, c_defaultOccuMin, c_defaultOccuMax, [](const VxdID & s) { return s.getLayerNumber() == 4; }},
  {"L5XX", "L5", c_defaultNBins, c_defaultOccuMin, c_defaultOccuMax, [](const VxdID & s) { return s.getLayerNumber() == 5; }},
  {"L6XX", "L6", c_defaultNBins, c_defaultOccuMin, c_defaultOccuMax, [](const VxdID & s) { return s.getLayerNumber() == 6; }},
  {"L31X", "L3.1", c_defaultNBins, c_defaultOccuMin, c_defaultOccuMax, [](const VxdID & s) { return s.getLayerNumber() == 3 && s.getLadderNumber() == 1; }},
  {"L32X", "L3.2", c_defaultNBins, c_defaultOccuMin, c_defaultOccuMax, [](const VxdID & s) { return s.getLayerNumber() == 3 && s.getLadderNumber() == 2; }}
};
