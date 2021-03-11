/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgHitRateMonitor/BeamBkgHitRateMonitorModule.h>
#include <background/modules/BeamBkgHitRateMonitor/PXDHitRateCounter.h>
#include <background/modules/BeamBkgHitRateMonitor/SVDHitRateCounter.h>
#include <background/modules/BeamBkgHitRateMonitor/CDCHitRateCounter.h>
#include <background/modules/BeamBkgHitRateMonitor/TOPHitRateCounter.h>
#include <background/modules/BeamBkgHitRateMonitor/ARICHHitRateCounter.h>
#include <background/modules/BeamBkgHitRateMonitor/ECLHitRateCounter.h>
#include <background/modules/BeamBkgHitRateMonitor/KLMHitRateCounter.h>

// framework aux
#include <framework/logging/Logger.h>

#include <framework/io/RootIOUtilities.h>
#include <framework/core/RandomNumbers.h>
#include <framework/core/Environment.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/database/Database.h>
#include <framework/utilities/EnvironmentVariables.h>

#include <boost/python.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(BeamBkgHitRateMonitor)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  BeamBkgHitRateMonitorModule::BeamBkgHitRateMonitorModule() : Module()

  {
    // set module description
    setDescription("A module for off-line monitoring of beam background hit rates.");

    /* No multiprocessing allowed!
     * setPropertyFlags(c_ParallelProcessingCertified);
     */

    // Add parameters
    addParam("outputFileName", m_outputFileName, "output file name",
             string("beamBkgHitRates.root"));
    addParam("treeName", m_treeName, "output tree name",
             string("tree"));
    m_trgTypes.push_back(TRGSummary::TTYP_DPHY);
    m_trgTypes.push_back(TRGSummary::TTYP_RAND);
    addParam("trgTypes", m_trgTypes,
             "trigger types for event selection (see TRGSummary.h for definitions). "
             "Empty list means all trigger types.",
             m_trgTypes);
    addParam("writeEmptyTimeStamps", m_writeEmptyTimeStamps,
             "if true, write to ntuple also empty time stamps", false);
    addParam("topTimeOffset", m_topTimeOffset,
             "TOP: time offset of hits (to be subtracted) [ns]", 25.0);
    addParam("topTimeWindow", m_topTimeWindow,
             "TOP: time window in which to count hits [ns]", 100.0);
    addParam("svdShaperDigitsName", m_svdShaperDigitsName,
             "SVDShaperDigits collection name", string(""));
    addParam("svdThrCharge", m_svdThrCharge,
             "Energy cur on SVD Cluster charge in electrons", 15000.);
    addParam("svdIgnoreHotStripsPayload", m_svdIgnoreHotStripsPayload,
             "If true, also SVD hot strips are counted as active", false);
    addParam("svdIgnoreMaskedStripsPayload", m_svdIgnoreMaskedStripsPayload,
             "If true, also SVD FADC-masked strips are counted as active", false);
    addParam("additionalDataDescription", m_additionalDataDescription,
             "Additional dictionary of "
             "name->value pairs to be added to the file metadata to describe the data",
             m_additionalDataDescription);
    addParam("cdcTimeWindowLowerEdgeSmallCell",  m_cdcTimeWindowLowerEdgeSmallCell,
             "CDC: lower edge of the time window for small cells [tdc count = ns]",
             4550);
    addParam("cdcTimeWindowUpperEdgeSmallCell",  m_cdcTimeWindowUpperEdgeSmallCell,
             "CDC: upper edge of the time window for small cells [tdc count = ns]",
             5050);
    addParam("cdcTimeWindowLowerEdgeNormalCell", m_cdcTimeWindowLowerEdgeNormalCell,
             "CDC: lower edge of the time window for normal cells [tdc count = ns]",
             4200);
    addParam("cdcTimeWindowUpperEdgeNormalCell", m_cdcTimeWindowUpperEdgeNormalCell,
             "CDC: upper edge of the time window for normal cells [tdc count = ns]",
             5050);
    addParam("cdcEnableBadWireTreatment", m_cdcEnableBadWireTreatment,
             "CDC: flag to enable the bad wire treatment", true);
    addParam("cdcEnableBackgroundHitFilter", m_cdcEnableBackgroundHitFilter,
             "CDC: flag to enable the CDC background hit (crosstakl, noise) filter", true);
    addParam("cdcEnableMarkBackgroundHit", m_cdcEnableMarkBackgroundHit,
             "CDC: flag to enable to mark background flag on CDCHit (set 0x100 bit for CDCHit::m_status).", false);

  }

  BeamBkgHitRateMonitorModule::~BeamBkgHitRateMonitorModule()
  {
    for (auto& monitor : m_monitors) {
      if (monitor) delete monitor;
    }
  }

  void BeamBkgHitRateMonitorModule::initialize()
  {
    // collections registration
    m_eventMetaData.isRequired();
    if (m_trgTypes.empty()) {
      m_trgSummary.isOptional(); // enables to run the module when TRGSummary is absent
    } else {
      m_trgSummary.isRequired();
    }
    m_fileMetaData.isOptional(); // enables to run the module with simulation

    // create, set and append hit rate monitoring classes
    auto* pxd = new Background::PXDHitRateCounter();
    m_monitors.push_back(pxd);
    auto* svd = new Background::SVDHitRateCounter(m_svdShaperDigitsName, m_svdThrCharge,
                                                  m_svdIgnoreHotStripsPayload,
                                                  m_svdIgnoreMaskedStripsPayload);
    m_monitors.push_back(svd);
    auto* cdc = new Background::CDCHitRateCounter(m_cdcTimeWindowLowerEdgeSmallCell,  m_cdcTimeWindowUpperEdgeSmallCell,
                                                  m_cdcTimeWindowLowerEdgeNormalCell, m_cdcTimeWindowUpperEdgeNormalCell,
                                                  m_cdcEnableBadWireTreatment, m_cdcEnableBackgroundHitFilter,
                                                  m_cdcEnableMarkBackgroundHit);
    m_monitors.push_back(cdc);
    auto* top = new Background::TOPHitRateCounter(m_topTimeOffset, m_topTimeWindow);
    m_monitors.push_back(top);
    auto* arich = new Background::ARICHHitRateCounter();
    m_monitors.push_back(arich);
    auto* ecl = new Background::ECLHitRateCounter();
    m_monitors.push_back(ecl);
    auto* klm = new Background::KLMHitRateCounter();
    m_monitors.push_back(klm);

    // open output root file
    m_file = TFile::Open(m_outputFileName.c_str(), "RECREATE");
    if (not m_file) {
      B2FATAL("Cannot open output file '" << m_outputFileName << "' for writing");
    }

    // create tree
    m_tree = new TTree(m_treeName.c_str(), "hit rates of selected events");

    // create persistent tree to store fileMetaData
    m_persistent = new TTree("persistent", "persistent data");
    m_persistent->Branch("FileMetaData", &m_outputFileMetaData);

    // set tree branches
    m_tree->Branch("run", &m_run, "run/I");
    m_tree->Branch("numEvents", &m_numEvents, "numEvents/I");
    m_tree->Branch("timeStamp", &m_timeStamp, "timeStamp/i");
    m_tree->Branch("time", &m_time, "time/I");
    for (auto& monitor : m_monitors) {
      monitor->initialize(m_tree);
    }

    // control histograms
    m_trgAll = new TH1F("trgAll", "trigger types of all events", 16, -0.5, 15.5);
    m_trgAll->SetXTitle("type of trigger timing source");
    m_trgSel = new TH1F("trgSel", "trigger types of selected events", 16, -0.5, 15.5);
    m_trgSel->SetXTitle("type of trigger timing source");

  }

  void BeamBkgHitRateMonitorModule::beginRun()
  {
    // clear buffers
    for (auto& monitor : m_monitors) {
      monitor->clear();
    }
    m_eventCounts.clear();

    // clear counters
    m_numEventsSelected = 0;
    m_trgTypesCount.clear();

    // set run number
    m_run = m_eventMetaData->getRun();

    // set unix time of the first event in the run
    unsigned utime = m_eventMetaData->getTime() / 1000000000;
    m_utimeFirst = utime;
    m_utimeMin = utime;
    m_utimeMax = utime + 1;

  }

  void BeamBkgHitRateMonitorModule::event()
  {
    // get unix time of the event
    unsigned utime = m_eventMetaData->getTime() / 1000000000;
    m_utimeMin = std::min(m_utimeMin, utime);
    m_utimeMax = std::max(m_utimeMax, utime + 1);

    // collect file meta data
    collectFileMetaData();

    // event selection
    if (not isEventSelected()) return;
    m_numEventsSelected++;

    // accumulate
    for (auto& monitor : m_monitors) {
      monitor->accumulate(utime);
    }
    m_eventCounts[utime] += 1;

  }

  void BeamBkgHitRateMonitorModule::endRun()
  {
    // fill ntuple
    for (unsigned utime = m_utimeMin; utime < m_utimeMax; utime++) {
      if (not m_writeEmptyTimeStamps) {
        if (m_eventCounts.find(utime) == m_eventCounts.end()) continue;
      }
      m_numEvents = m_eventCounts[utime];
      m_timeStamp = utime;
      m_time = utime - m_utimeMin;
      for (auto& monitor : m_monitors) {
        monitor->normalize(utime);
      }
      m_tree->Fill();
    }

    // count selected events in all runs
    m_allEventsSelected += m_numEventsSelected;

    // print a summary for this run
    std::string trigs;
    for (const auto& trgType : m_trgTypesCount) {
      trigs += "        trigger type " + std::to_string(trgType.first) + ": " +
               std::to_string(trgType.second) + " events\n";
    }
    B2INFO("Run " << m_run << ": " << m_numEventsSelected
           << " events selected for beam background hit rate monitoring.\n"
           << trigs
           << LogVar("first event utime ", m_utimeMin)
           << LogVar("start utime       ", m_utimeMin)
           << LogVar("stop utime        ", m_utimeMax)
           << LogVar("duration [seconds]", m_utimeMax - m_utimeMin)
          );
  }

  void BeamBkgHitRateMonitorModule::terminate()
  {
    setFileMetaData();
    m_persistent->Fill();

    // write to file and close
    m_file->cd();
    m_file->Write();
    m_file->Close();

    B2INFO("Output file: " << m_outputFileName);
  }

  bool BeamBkgHitRateMonitorModule::isEventSelected()
  {
    auto trgType = TRGSummary::TTYP_NONE;
    if (m_trgSummary.isValid()) trgType = m_trgSummary->getTimType();
    m_trgAll->Fill(trgType);

    if (m_trgTypes.empty()) {
      m_trgTypesCount[trgType] += 1;
      m_trgSel->Fill(trgType);
      return true;
    }
    for (auto type : m_trgTypes) {
      if (trgType == type) {
        m_trgTypesCount[trgType] += 1;
        m_trgSel->Fill(trgType);
        return true;
      }
    }
    return false;
  }


  void BeamBkgHitRateMonitorModule::collectFileMetaData()
  {
    // add file name to the list
    if (m_fileMetaData.isValid()) {
      std::string lfn = m_fileMetaData->getLfn();
      if (not lfn.empty() and (m_parentLfns.empty() or (m_parentLfns.back() != lfn))) {
        m_parentLfns.push_back(lfn);
      }
    }

    // low and high experiment, run and event numbers
    unsigned long experiment =  m_eventMetaData->getExperiment();
    unsigned long run =  m_eventMetaData->getRun();
    unsigned long event = m_eventMetaData->getEvent();
    if (m_experimentLow > m_experimentHigh) { //starting condition
      m_experimentLow = m_experimentHigh = experiment;
      m_runLow = m_runHigh = run;
      m_eventLow = m_eventHigh = event;
    } else {
      if ((experiment < m_experimentLow) or ((experiment == m_experimentLow) and ((run < m_runLow) or ((run == m_runLow)
                                             and (event < m_eventLow))))) {
        m_experimentLow = experiment;
        m_runLow = run;
        m_eventLow = event;
      }
      if ((experiment > m_experimentHigh) or ((experiment == m_experimentHigh) and ((run > m_runHigh) or ((run == m_runHigh)
                                              and (event > m_eventHigh))))) {
        m_experimentHigh = experiment;
        m_runHigh = run;
        m_eventHigh = event;
      }
    }

  }


  void BeamBkgHitRateMonitorModule::setFileMetaData()
  {

    if (m_fileMetaData.isValid() and not m_fileMetaData->isMC()) {
      m_outputFileMetaData.declareRealData();
    }

    m_outputFileMetaData.setNEvents(m_allEventsSelected);

    if (m_experimentLow > m_experimentHigh) {
      // starting condition so apparently no events at all
      m_outputFileMetaData.setLow(-1, -1, 0);
      m_outputFileMetaData.setHigh(-1, -1, 0);
    } else {
      m_outputFileMetaData.setLow(m_experimentLow, m_runLow, m_eventLow);
      m_outputFileMetaData.setHigh(m_experimentHigh, m_runHigh, m_eventHigh);
    }

    m_outputFileMetaData.setParents(m_parentLfns);
    RootIOUtilities::setCreationData(m_outputFileMetaData);
    m_outputFileMetaData.setRandomSeed(RandomNumbers::getSeed());
    m_outputFileMetaData.setSteering(Environment::Instance().getSteering());
    auto mcEvents = Environment::Instance().getNumberOfMCEvents();
    m_outputFileMetaData.setMcEvents(mcEvents);
    m_outputFileMetaData.setDatabaseGlobalTag(Database::Instance().getGlobalTags());

    for (const auto& item : m_additionalDataDescription) {
      m_outputFileMetaData.setDataDescription(item.first, item.second);
    }

    std::string lfn = m_file->GetName();
    lfn = boost::filesystem::absolute(lfn, boost::filesystem::initial_path()).string();
    std::string format = EnvironmentVariables::get("BELLE2_LFN_FORMATSTRING", "");
    if (!format.empty()) {
      auto format_filename = boost::python::import("B2Tools.format").attr("format_filename");
      lfn = boost::python::extract<std::string>(format_filename(format, m_outputFileName, m_outputFileMetaData.getJsonStr()));
    }
    m_outputFileMetaData.setLfn(lfn);

  }


} // end Belle2 namespace

