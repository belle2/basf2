/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <top/modules/TOPDoublePulseGenerator/TOPDoublePulseGeneratorModule.h>
#include <top/geometry/TOPGeometryPar.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

// datastore classes
#include <framework/dataobjects/EventMetaData.h>
#include <top/dataobjects/TOPDigit.h>

// root
#include <TRandom.h>
#include <TFile.h>
#include <TH1F.h>


using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPDoublePulseGenerator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPDoublePulseGeneratorModule::TOPDoublePulseGeneratorModule() : Module()

  {
    // set module description
    setDescription("Generator of calibration double pulses");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("moduleIDs", m_moduleIDs,
             "list of slots for which to generate cal pulse, empty list means all slots.",
             m_moduleIDs);
    m_asicChannels.push_back(0);
    addParam("asicChannels", m_asicChannels,
             "ASIC calibration channels (0 - 7), empty list means all channels.",
             m_asicChannels);
    addParam("timeDifference", m_timeDifference,
             "time difference between first and second pulse [ns].", 21.87);
    addParam("timeResolution", m_timeResolution,
             "sigma of time difference [ns].", 40.0e-3);
    addParam("sampleTimeIntervals", m_sampleTimeIntervals,
             "vector of 256 sample time intervals to construct sample times. "
             "If empty, equidistant intervals will be used.", m_sampleTimeIntervals);
    addParam("useDatabase", m_useDatabase,
             "if true, use sample times from database instead of sampleTimeIntervals.",
             false);
    addParam("storageWindows", m_storageWindows,
             "number of storage windows (old FW used 64 out of 512)", (unsigned) 512);
    addParam("outputFileName", m_outputFileName,
             "if given, sample times will be saved as root histograms in this file",
             std::string(""));

  }

  TOPDoublePulseGeneratorModule::~TOPDoublePulseGeneratorModule()
  {
    if (m_timebase) delete m_timebase;
  }

  void TOPDoublePulseGeneratorModule::initialize()
  {
    // Output

    StoreArray<TOPDigit> digits;
    digits.registerInDataStore();

    // prepare vectors to loop on

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();

    if (m_moduleIDs.empty()) {
      for (const auto& module : geo->getModules()) {
        m_moduleIDs.push_back(module.getModuleID());
      }
    } else {
      for (auto moduleID : m_moduleIDs) {
        if (!geo->isModuleIDValid(moduleID))
          B2ERROR("Invalid module ID found in input list: " << moduleID);
      }
    }

    if (m_asicChannels.empty()) {
      for (unsigned ch = 0; ch < 8; ch++) m_asicChannels.push_back(ch);
    } else {
      for (unsigned ch : m_asicChannels) {
        if (ch > 7)
          B2ERROR("Invalid ASIC channel found in input list: " << ch);
      }
    }

    // set sample times

    double syncTimeBase = geo->getNominalTDC().getSyncTimeBase();
    m_sampleTimes.setTimeAxis(syncTimeBase); // equidistant
    m_sampleDivisions = (0x1 << geo->getNominalTDC().getSubBits());

    if (m_useDatabase) {
      m_timebase = new DBObjPtr<TOPCalTimebase>;
    } else if (m_sampleTimeIntervals.empty()) {
      B2INFO("TOPDoublePulseGenerator: using equidistant sample times");
    } else if (m_sampleTimeIntervals.size() == 256) {
      std::vector<double> timeAxis;
      timeAxis.push_back(0);
      for (auto dt : m_sampleTimeIntervals) timeAxis.push_back(dt + timeAxis.back());
      double rescale = 2 * syncTimeBase / timeAxis.back();
      for (auto& t : timeAxis) t *= rescale;
      m_sampleTimes.setTimeAxis(timeAxis, syncTimeBase); // given by steering
      B2INFO("TOPDoublePulseGenerator: using sample times from steering");
    } else {
      B2ERROR("sampleTimeIntervals: size must be 256 or empty");
    }

    if (!m_outputFileName.empty()) storeSampleTimes(m_outputFileName);

  }


  void TOPDoublePulseGeneratorModule::beginRun()
  {
    StoreObjPtr<EventMetaData> evtMetaData;
    if (m_useDatabase) {
      if (!(*m_timebase).isValid()) {
        B2FATAL("Sample time calibration requested but not available for run "
                << evtMetaData->getRun()
                << " of experiment " << evtMetaData->getExperiment());
      }
    }

  }


  void TOPDoublePulseGeneratorModule::event()
  {

    StoreArray<TOPDigit> digits;

    const auto& chMapper = TOPGeometryPar::Instance()->getChannelMapper();
    const auto& feMapper = TOPGeometryPar::Instance()->getFrontEndMapper();

    double timeError = m_timeResolution / sqrt(2.0); // for single pulse

    for (auto moduleID : m_moduleIDs) {
      for (unsigned asic = 0; asic < 64; asic++) {
        for (auto asicChannel : m_asicChannels) {
          unsigned channel = asic * 8 + asicChannel;
          auto pixelID = chMapper.getPixelID(channel);

          // sample times: from steering or database
          const TOPSampleTimes* sampleTimes = &m_sampleTimes;
          if (m_useDatabase) {
            unsigned scrodID = 0;
            int bs = asic / 16;
            const auto* feMap = feMapper.getMap(moduleID, bs);
            if (feMap) scrodID = feMap->getScrodID();
            sampleTimes = (*m_timebase)->getSampleTimes(scrodID, channel % 128);
            if (!sampleTimes->isCalibrated()) {
              B2WARNING("No sample time calibration available for SCROD " << scrodID
                        << " channel " << channel % 128 << " - equidistant will be used");
            }
          }

          // first calpulse
          double time = gRandom->Rndm() * sampleTimes->getTimeRange();
          unsigned window = gRandom->Rndm() * m_storageWindows;
          double sample = sampleTimes->getSample(window, time);
          auto* digit = digits.appendNew(moduleID, pixelID, sample);
          digit->setFirstWindow(window);
          digit->setTime(time);
          digit->setTimeError(timeError);
          digit->setChannel(channel);
          digit->setHitQuality(TOPDigit::c_CalPulse);

          // second calpulse
          time += gRandom->Gaus(m_timeDifference, m_timeResolution);
          sample = sampleTimes->getSample(window, time);
          digit = digits.appendNew(moduleID, pixelID, sample);
          digit->setFirstWindow(window);
          digit->setTime(time);
          digit->setTimeError(timeError);
          digit->setChannel(channel);
          digit->setHitQuality(TOPDigit::c_CalPulse);
        }
      }
    }

  }


  void TOPDoublePulseGeneratorModule::endRun()
  {
  }

  void TOPDoublePulseGeneratorModule::terminate()
  {
  }


  void TOPDoublePulseGeneratorModule::storeSampleTimes(std::string fileName)
  {
    if (fileName.empty()) return;

    TFile* fout = TFile::Open(fileName.c_str(), "recreate");
    if (!fout) {
      B2ERROR("Can't open the output file " << fileName);
      return;
    }

    const auto& feMapper = TOPGeometryPar::Instance()->getFrontEndMapper();

    TH1F scrods("scrodID", "scrod ID's mapped to slots/boardstacks", 64, -0.5, 63.5);
    scrods.SetXTitle("(slot - 1) * 4 + boardstack");
    scrods.SetYTitle("scrod ID");

    for (auto moduleID : m_moduleIDs) {
      for (int bs = 0; bs < 4; bs++) {
        const auto* feMap = feMapper.getMap(moduleID, bs);
        if (!feMap) {
          B2ERROR("No front-end mapping available for slot " << moduleID
                  << " boardstack " << bs);
          continue;
        }
        unsigned scrodID = feMap->getScrodID();
        std::string subdir = "scrod" + std::to_string(scrodID);
        int i = (moduleID - 1) * 4 + bs;
        scrods.SetBinContent(i + 1, scrodID);
        fout->mkdir(subdir.c_str());
      }
    }
    scrods.Write();

    for (auto moduleID : m_moduleIDs) {
      for (unsigned asic = 0; asic < 64; asic++) {
        int bs = asic / 16;
        const auto* feMap = feMapper.getMap(moduleID, bs);
        if (!feMap) continue;
        unsigned scrodID = feMap->getScrodID();
        std::string subdir = "scrod" + std::to_string(scrodID);
        fout->cd(subdir.c_str());
        for (auto asicChannel : m_asicChannels) {
          unsigned channel = (asic * 8 + asicChannel) % 128;
          const TOPSampleTimes* sampleTimes = &m_sampleTimes;
          if (m_useDatabase) {
            sampleTimes = (*m_timebase)->getSampleTimes(scrodID, channel);
          }
          string forWhat = "scrod " + to_string(scrodID) +
                           " channel " + to_string(channel) +
                           " (slot" + to_string(moduleID) + ", as" + to_string(asic) +
                           ", ch" + to_string(asicChannel) + ")";
          auto timeAxis = sampleTimes->getTimeAxis();
          saveAsHistogram(timeAxis, "sampleTimes_ch" + to_string(channel),
                          "Generator input: sample times for " + forWhat,
                          "sample number", "t [ns]");
          std::vector<double> dt;
          for (unsigned i = 1; i < timeAxis.size(); i++) {
            dt.push_back(timeAxis[i] - timeAxis[i - 1]);
          }
          saveAsHistogram(dt, "dt_ch" + to_string(channel),
                          "Generator input: sample time bins for " + forWhat,
                          "sample number", "#Delta t [ns]");
        }
      }
    }

    fout->Close();

  }


  void TOPDoublePulseGeneratorModule::saveAsHistogram(const std::vector<double>& vec,
                                                      const std::string& name,
                                                      const std::string& title,
                                                      const std::string& xTitle,
                                                      const std::string& yTitle) const
  {
    if (vec.empty()) return;

    TH1F h(name.c_str(), title.c_str(), vec.size(), 0, vec.size());
    h.SetXTitle(xTitle.c_str());
    h.SetYTitle(yTitle.c_str());
    if (name.find("Fit") != string::npos) h.SetLineColor(2);

    for (unsigned i = 0; i < vec.size(); i++) h.SetBinContent(i + 1, vec[i]);

    h.Write();
  }

} // end Belle2 namespace

