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
#include <background/modules/BeamBkgHitRateMonitor/BKLMHitRateCounter.h>
#include <background/modules/BeamBkgHitRateMonitor/EKLMHitRateCounter.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>


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
             "trigger types for event selection (see TRGSummary.h for definitions)",
             m_trgTypes);
    addParam("topTimeOffset", m_topTimeOffset,
             "TOP: time offset of hits (to be subtracted) [ns]", 350.0);
    addParam("topTimeWindow", m_topTimeWindow,
             "TOP: time window in which to count hits [ns]", 100.0);

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
    m_rawTRGs.isRequired();
    m_trgSummary.isRequired();

    // create, set and append hit rate monitoring classes
    auto* pxd = new Background::PXDHitRateCounter();
    m_monitors.push_back(pxd);
    auto* svd = new Background::SVDHitRateCounter();
    m_monitors.push_back(svd);
    auto* cdc = new Background::CDCHitRateCounter();
    m_monitors.push_back(cdc);
    auto* top = new Background::TOPHitRateCounter(m_topTimeOffset, m_topTimeWindow);
    m_monitors.push_back(top);
    auto* arich = new Background::ARICHHitRateCounter();
    m_monitors.push_back(arich);
    auto* ecl = new Background::ECLHitRateCounter();
    m_monitors.push_back(ecl);
    auto* bklm = new Background::BKLMHitRateCounter();
    m_monitors.push_back(bklm);
    auto* eklm = new Background::EKLMHitRateCounter();
    m_monitors.push_back(eklm);

    // open output root file
    m_file = TFile::Open(m_outputFileName.c_str(), "RECREATE");
    if (not m_file) {
      B2FATAL("Cannot open output file '" << m_outputFileName << "' for writing");
    }

    // create tree
    m_tree = new TTree(m_treeName.c_str(), "hit rates of selected events");

    // set tree branches
    m_tree->Branch("run", &m_run, "run/I");
    m_tree->Branch("event", &m_event, "event/I");
    m_tree->Branch("trgType", &m_trgType, "trgType/I");
    m_tree->Branch("timeStamp", &m_timeStamp, "timeStamp/l");
    m_tree->Branch("utime", &m_utime, "utime/i");
    m_tree->Branch("ctime", &m_ctime, "ctime/i");
    m_tree->Branch("time", &m_time, "time/I");
    for (auto& monitor : m_monitors) {
      monitor->initialize(m_tree);
    }

    // other
    m_trgTypesCount.resize(m_trgTypes.size(), 0);

  }

  void BeamBkgHitRateMonitorModule::event()
  {
    // check if we can get unix time of the event
    if (m_rawTRGs.getEntries() == 0) {
      B2ERROR("RawTRGs are empty: cannot get unix time of the event - event skipped");
      return;
    }

    // get unix time of the event
    auto utime = m_rawTRGs[0]->GetTTUtime(0);
    if (m_utimeFirst == 0) {
      m_utimeFirst = utime;
      m_utimeMin = utime;
      m_utimeMax = utime;
    }
    m_utimeMin = std::min(m_utimeMin, utime);
    m_utimeMax = std::max(m_utimeMax, utime);

    // event selection
    if (not isEventSelected()) return;
    m_numEvents++;

    // clear rate monitors
    for (auto& monitor : m_monitors) {
      monitor->clear();
    }

    // fill ntuple
    m_run = m_eventMetaData->getRun();
    m_event = m_eventMetaData->getEvent();
    m_timeStamp = m_eventMetaData->getTime();
    m_utime = utime;
    m_ctime = m_rawTRGs[0]->GetTTCtime(0);
    m_time = m_utime - m_utimeFirst;
    for (auto& monitor : m_monitors) {
      monitor->accumulate();
      monitor->normalize();
    }
    m_tree->Fill();

  }

  void BeamBkgHitRateMonitorModule::terminate()
  {
    m_file->cd();
    m_file->Write();
    m_file->Close();

    // print a summary
    std::string trigs;
    for (unsigned i = 0; i < m_trgTypes.size(); i++) {
      trigs += "        trigger type " + std::to_string(m_trgTypes[i]) + ": " +
               std::to_string(m_trgTypesCount[i]) + " events\n";
    }
    B2RESULT(m_numEvents << " events selected for beam background hit rate monitoring.\n"
             << trigs
             << LogVar("first event utime ", m_utimeMin)
             << LogVar("start utime       ", m_utimeMin)
             << LogVar("stop utime        ", m_utimeMax)
             << LogVar("duration [seconds]", m_utimeMax - m_utimeMin)
            );
    B2RESULT("Output file: " << m_outputFileName);
  }

  bool BeamBkgHitRateMonitorModule::isEventSelected()
  {
    m_trgType = m_trgSummary->getTimType();
    unsigned i = 0;
    for (auto trgType : m_trgTypes) {
      if (m_trgType == trgType) {
        m_trgTypesCount[i]++;
        return true;
      }
      i++;
    }
    return false;
  }



} // end Belle2 namespace

