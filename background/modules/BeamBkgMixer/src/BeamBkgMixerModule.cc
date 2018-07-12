/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgMixer/BeamBkgMixerModule.h>



// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// SimHits
#include <pxd/dataobjects/PXDSimHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <top/dataobjects/TOPSimHit.h>
#include <arich/dataobjects/ARICHSimHit.h>
#include <ecl/dataobjects/ECLHit.h>
#include <bklm/dataobjects/BKLMSimHit.h>
#include <eklm/dataobjects/EKLMSimHit.h>
#include <simulation/dataobjects/BeamBackHit.h>

// MetaData
#include <framework/dataobjects/EventMetaData.h>
#include <background/dataobjects/BackgroundMetaData.h>
#include <background/dataobjects/BackgroundInfo.h>

// Root
#include <TRandom3.h>

//std::find
#include <algorithm>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(BeamBkgMixer)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  BeamBkgMixerModule::BeamBkgMixerModule() : Module()
  {
    // set module description (e.g. insert text)
    setDescription("Beam background mixer at SimHit level that uses beam background"
                   " simulation output directly (collision files) and not ROF files. "
                   "Each background event is shifted in time randomly within "
                   "a time window specified with minTime and maxTime.");

    // Add parameters
    addParam("backgroundFiles", m_backgroundFiles,
             "List of background (collision) files (wildcards not allowed - "
             "use python glob.glob() to expand to list of files)");
    addParam("minTime", m_minTime,
             "Time window lower edge in nano seconds", -1000.0);
    addParam("maxTime", m_maxTime,
             "Time window upper edge in nano seconds", 800.0);
    addParam("overallScaleFactor", m_overallScaleFactor,
             "Overall factor to scale rates of backgrounds", 1.0);
    addParam("scaleFactors", m_scaleFactors,
             "Factors to scale rates of backgrounds. "
             "Possible tag names: " + m_bgTypes.getBGTypes(),
             m_scaleFactors);
    addParam("components", m_components,
             "Detector components to be included, empty list means all components",
             m_components);
    addParam("wrapAround", m_wrapAround,
             "if true wrap around events passing time window upper edge", true);
    addParam("minTimeECL", m_minTimeECL,
             "Time window lower edge for ECL in nano seconds", -17600.0);
    addParam("maxTimeECL", m_maxTimeECL,
             "Time window upper edge for ECL in nano seconds", 8500.0);
    addParam("minTimePXD", m_minTimePXD,
             "Time window lower edge for PXD in nano seconds", -10000.0);
    addParam("maxTimePXD", m_maxTimePXD,
             "Time window upper edge for PXD in nano seconds", 10000.0);

    addParam("beambackhits", m_BeamBackHits,
             "If true also add the BeamBackHits collection for the selected "
             "subdetectors to the output file", false);

    addParam("maxEdepECL", m_maxEdepECL,
             "maximal deposited energy of ECLHit to accept BG event for mixing"
             "(0 means accept all events)", 1.0);

    addParam("cacheSize", m_cacheSize,
             "file cache size in Mbytes. If negative, use root default", 0);
  }

  BeamBkgMixerModule::~BeamBkgMixerModule()
  {
  }

  void BeamBkgMixerModule::initialize()
  {

    // included components

    std::vector<std::string> components = m_components;
    m_PXD = isComponentIncluded(components, "PXD");
    m_SVD = isComponentIncluded(components, "SVD");
    m_CDC = isComponentIncluded(components, "CDC");
    m_TOP = isComponentIncluded(components, "TOP");
    m_ARICH = isComponentIncluded(components, "ARICH");
    m_ECL = isComponentIncluded(components, "ECL");
    m_BKLM = isComponentIncluded(components, "BKLM");
    m_EKLM = isComponentIncluded(components, "EKLM");

    // ignore these ones

    isComponentIncluded(components, "MagneticField2d");
    isComponentIncluded(components, "MagneticField3d");
    isComponentIncluded(components, "MagneticField");
    isComponentIncluded(components, "MagneticFieldConstant4LimitedRCDC");
    isComponentIncluded(components, "MagneticFieldConstant4LimitedRSVD");
    isComponentIncluded(components, "BeamPipe");
    isComponentIncluded(components, "Cryostat");
    isComponentIncluded(components, "FarBeamLine");
    isComponentIncluded(components, "HeavyMetalShield");
    isComponentIncluded(components, "COIL");
    isComponentIncluded(components, "STR");
    isComponentIncluded(components, "VXDService");

    if (!components.empty()) {
      std::string str;
      for (unsigned i = 0; i < components.size(); ++i) str = str + " " + components[i];
      B2WARNING("Unknown components:" << str);
    }

    // check files and append them to sample container

    for (auto file : m_backgroundFiles) {

      // wildcarding is not allowed anymore
      if (TString(file.c_str()).Contains("*")) {
        B2ERROR(file << ": wildcards are not allowed");
        continue;
      }

      // check the file existance
      TFile* f = TFile::Open(file.c_str(), "READ");
      if (!f) {
        B2ERROR(file << ": file not found");
        continue;
      }
      if (!f->IsOpen()) {
        B2ERROR(file << ": can't open file");
        continue;
      }
      f->Close();

      TChain persistent("persistent");
      int nFiles = persistent.Add(file.c_str());
      if (nFiles == 0) {
        B2ERROR(file << ": no such files");
        continue;
      }
      if (persistent.GetEntries() == 0) {
        B2ERROR(file << ": tree 'persistent' has no entries");
        continue;
      }

      TObject* bkgMetaData = 0; // Note: allocation left to root
      TBranch* branchBMD = persistent.GetBranch("BackgroundMetaData");
      if (!branchBMD) {
        B2ERROR(file << ": branch 'BackgroundMetaData' not found");
        continue;
      }
      branchBMD->SetAddress(&bkgMetaData);

      std::vector<SimHitBase::BG_TAG> tags;
      std::vector<std::string> types;
      std::vector<BackgroundMetaData::EFileType> fileTypes;
      double realTime = 0;
      for (unsigned k = 0; k < persistent.GetEntries(); k++) {
        persistent.GetEntry(k);
        BackgroundMetaData* bgMD = static_cast<BackgroundMetaData*>(bkgMetaData);
        tags.push_back(bgMD->getBackgroundTag());
        types.push_back(bgMD->getBackgroundType());
        fileTypes.push_back(bgMD->getFileType());
        realTime += bgMD->getRealTime();
      }
      if (realTime <= 0) {
        B2ERROR(file << ": invalid realTime: " << realTime);
        continue;
      }
      for (unsigned i = 1; i < tags.size(); ++i) {
        if (tags[i] != tags[0]) {
          B2ERROR(file << ": files with mixed background types not supported");
          continue;
        }
      }
      for (unsigned i = 1; i < fileTypes.size(); ++i) {
        if (fileTypes[i] != fileTypes[0]) {
          B2ERROR(file << ": files with mixed file types not supported");
          continue;
        }
      }

      appendSample(tags[0], types[0], file, realTime, fileTypes[0]);

    }


    // set scale factors

    for (auto scaleFactor : m_scaleFactors) {
      std::string type = std::get<0>(scaleFactor);
      if (m_bgTypes.getTag(type) == 0)
        B2ERROR("Unknown beam background type found in 'scaleFactors': " << type << "\n"
                "Possible are: " + m_bgTypes.getBGTypes());
      for (auto& bkg : m_backgrounds) {
        if (bkg.type.find(type) != std::string::npos)
          bkg.scaleFactor *= std::get<1>(scaleFactor);
      }
    }

    // open files for reading SimHits

    for (auto& bkg : m_backgrounds) {

      // define TChain for reading SimHits
      bkg.tree = new TChain("tree");
      for (unsigned i = 0; i < bkg.fileNames.size(); ++i) {
        bkg.numFiles += bkg.tree->Add(bkg.fileNames[i].c_str());
      }

      bkg.numEvents = bkg.tree->GetEntries();
      bkg.rate =  bkg.numEvents / bkg.realTime * bkg.scaleFactor;

      if (m_cacheSize >= 0) bkg.tree->SetCacheSize(m_cacheSize * 1024 * 1024);

      if (m_PXD and bkg.tree->GetBranch("PXDSimHits"))
        bkg.tree->SetBranchAddress("PXDSimHits", &m_simHits.PXD);
      if (m_SVD and bkg.tree->GetBranch("SVDSimHits"))
        bkg.tree->SetBranchAddress("SVDSimHits", &m_simHits.SVD);
      if (m_CDC and bkg.tree->GetBranch("CDCSimHits"))
        bkg.tree->SetBranchAddress("CDCSimHits", &m_simHits.CDC);
      if (m_TOP and bkg.tree->GetBranch("TOPSimHits"))
        bkg.tree->SetBranchAddress("TOPSimHits", &m_simHits.TOP);
      if (m_ARICH and bkg.tree->GetBranch("ARICHSimHits"))
        bkg.tree->SetBranchAddress("ARICHSimHits", &m_simHits.ARICH);
      if (m_ECL and bkg.tree->GetBranch("ECLHits"))
        bkg.tree->SetBranchAddress("ECLHits", &m_simHits.ECL);
      if (m_BKLM and bkg.tree->GetBranch("BKLMSimHits"))
        bkg.tree->SetBranchAddress("BKLMSimHits", &m_simHits.BKLM);
      if (m_EKLM and bkg.tree->GetBranch("EKLMSimHits"))
        bkg.tree->SetBranchAddress("EKLMSimHits", &m_simHits.EKLM);

      if (m_BeamBackHits and bkg.tree->GetBranch("BeamBackHits"))
        bkg.tree->SetBranchAddress("BeamBackHits", &m_simHits.BeamBackHits);

      // print INFO
      std::string unit(" ns");
      double realTime = bkg.realTime;
      if (realTime >= 1000.0) {realTime /= 1000.0; unit = " us";}
      if (realTime >= 1000.0) {realTime /= 1000.0; unit = " ms";}
      if (realTime >= 1000.0) {realTime /= 1000.0; unit = " s";}

      B2INFO("BeamBkgMixer: " << bkg.type <<
             " files=" << bkg.numFiles <<
             " events=" << bkg.numEvents <<
             " realTime=" << realTime << unit <<
             " scaleFactor=" << bkg.scaleFactor <<
             " rate=" << bkg.rate * 1000 << " MHz");
    }


    // SimHits registration

    StoreArray<PXDSimHit> pxdSimHits;
    if (m_PXD) pxdSimHits.registerInDataStore();

    StoreArray<SVDSimHit> svdSimHits;
    if (m_SVD) svdSimHits.registerInDataStore();

    StoreArray<CDCSimHit> cdcSimHits;
    if (m_CDC) cdcSimHits.registerInDataStore();

    StoreArray<TOPSimHit> topSimHits;
    if (m_TOP) topSimHits.registerInDataStore();

    StoreArray<ARICHSimHit> arichSimHits;
    if (m_ARICH) arichSimHits.registerInDataStore();

    StoreArray<ECLHit> eclHits;
    if (m_ECL) eclHits.registerInDataStore();

    StoreArray<BKLMSimHit> bklmSimHits;
    if (m_BKLM) bklmSimHits.registerInDataStore();

    StoreArray<EKLMSimHit> eklmSimHits;
    if (m_EKLM) eklmSimHits.registerInDataStore();

    StoreArray<BeamBackHit> beamBackHits;
    if (m_BeamBackHits) beamBackHits.registerInDataStore();


    // add BackgroundInfo to persistent tree

    StoreObjPtr<BackgroundInfo> bkgInfo("", DataStore::c_Persistent);
    bkgInfo.registerInDataStore();
    bkgInfo.create();
    bkgInfo->setMethod(BackgroundInfo::c_Mixing);
    bkgInfo->setComponents(m_components);
    bkgInfo->setMinTime(m_minTime);
    bkgInfo->setMaxTime(m_maxTime);
    bkgInfo->setMinTimeECL(m_minTimeECL);
    bkgInfo->setMaxTimeECL(m_maxTimeECL);
    bkgInfo->setMinTimePXD(m_minTimePXD);
    bkgInfo->setMaxTimePXD(m_maxTimePXD);
    bkgInfo->setWrapAround(m_wrapAround);
    bkgInfo->setMaxEdepECL(m_maxEdepECL);
    for (auto& bkg : m_backgrounds) {
      BackgroundInfo::BackgroundDescr descr;
      descr.tag = bkg.tag;
      descr.type = bkg.type;
      descr.fileType = bkg.fileType;
      descr.fileNames = bkg.fileNames;
      descr.realTime = bkg.realTime;
      descr.numEvents = bkg.numEvents;
      descr.scaleFactor = bkg.scaleFactor;
      descr.rate = bkg.rate;
      descr.reused = 0;
      bkgInfo->appendBackgroundDescr(descr);
    }

  }

  void BeamBkgMixerModule::beginRun()
  {
  }

  void BeamBkgMixerModule::event()
  {
    StoreArray<PXDSimHit> pxdSimHits;
    StoreArray<SVDSimHit> svdSimHits;
    StoreArray<CDCSimHit> cdcSimHits;
    StoreArray<TOPSimHit> topSimHits;
    StoreArray<ARICHSimHit> arichSimHits;
    StoreArray<ECLHit> eclHits;
    StoreArray<BKLMSimHit> bklmSimHits;
    StoreArray<EKLMSimHit> eklmSimHits;
    StoreArray<BeamBackHit> beamBackHits;
    StoreObjPtr<BackgroundInfo> bkgInfo("", DataStore::c_Persistent);

    for (auto& bkg : m_backgrounds) {

      if (bkg.fileType != BackgroundMetaData::c_Usual) continue;

      double mean = bkg.rate * (m_maxTime - m_minTime);
      int nev = gRandom->Poisson(mean);

      for (int iev = 0; iev < nev; iev++) {
        double timeShift = gRandom->Rndm() * (m_maxTime - m_minTime) + m_minTime;
        bkg.tree->GetEntry(bkg.eventCount);

        if (acceptEvent(m_simHits.ECL)) {
          addSimHits(pxdSimHits, m_simHits.PXD, timeShift, m_minTime, m_maxTime);
          addSimHits(svdSimHits, m_simHits.SVD, timeShift, m_minTime, m_maxTime);
          addSimHits(cdcSimHits, m_simHits.CDC, timeShift, m_minTime, m_maxTime);
          addSimHits(topSimHits, m_simHits.TOP, timeShift, m_minTime, m_maxTime);
          addSimHits(arichSimHits, m_simHits.ARICH, timeShift, m_minTime, m_maxTime);
          addSimHits(eclHits, m_simHits.ECL, timeShift, m_minTime, m_maxTime);
          addSimHits(bklmSimHits, m_simHits.BKLM, timeShift, m_minTime, m_maxTime);
          addSimHits(eklmSimHits, m_simHits.EKLM, timeShift, m_minTime, m_maxTime);
          addBeamBackHits(beamBackHits, m_simHits.BeamBackHits, timeShift,
                          m_minTime, m_maxTime);
        } else {
          iev--;
          std::string message = "BeamBkgMixer: event " + to_string(bkg.eventCount)
                                + " of " + bkg.type + " rejected due to large energy deposit in ECL";
          m_rejected[message] += 1;
          m_rejectedCount++;
          if (m_rejectedCount < 10) {
            B2INFO("BeamBkgMixer: event rejected due to large energy deposit in ECL");
          } else if (m_rejectedCount == 10) {
            B2INFO("BeamBkgMixer: event rejected due to large energy deposit in ECL "
                   << "(message will be suppressed now)");
          }
        }

        bkg.eventCount++;
        if (bkg.eventCount >= bkg.numEvents) {
          bkg.eventCount = 0;
          std::string message = "BeamBkgMixer: events of " + bkg.type + " will be re-used";
          m_reused[message] += 1;
          if (m_reused[message] == 1) B2INFO(message);
          bkgInfo->incrementReusedCounter(bkg.index);
        }
      }
    }


    for (auto& bkg : m_backgrounds) {

      if (bkg.fileType != BackgroundMetaData::c_ECL) continue;

      double mean = bkg.rate * (m_maxTimeECL - m_minTimeECL);
      int nev = gRandom->Poisson(mean);

      for (int iev = 0; iev < nev; iev++) {
        double timeShift = gRandom->Rndm() * (m_maxTimeECL - m_minTimeECL) + m_minTimeECL;
        if (timeShift > m_minTime and timeShift < m_maxTime) continue;
        bkg.tree->GetEntry(bkg.eventCount);

        if (acceptEvent(m_simHits.ECL)) {
          double minTime = m_minTimeECL;
          double maxTime = m_maxTimeECL;
          if (timeShift <= m_minTime) {
            maxTime = m_minTime;
          } else {
            minTime = m_maxTime;
          }
          addSimHits(eclHits, m_simHits.ECL, timeShift, minTime, maxTime);
        } else {
          iev--;
          std::string message = "BeamBkgMixer: event " + to_string(bkg.eventCount)
                                + " of " + bkg.type + " rejected due to large energy deposit in ECL";
          m_rejected[message] += 1;
          m_rejectedCount++;
          if (m_rejectedCount < 10) {
            B2INFO("BeamBkgMixer: event rejected due to large energy deposit in ECL");
          } else if (m_rejectedCount == 10) {
            B2INFO("BeamBkgMixer: event rejected due to large energy deposit in ECL "
                   << "(message will be suppressed now)");
          }
        }

        bkg.eventCount++;
        if (bkg.eventCount >= bkg.numEvents) {
          bkg.eventCount = 0;
          std::string message = "BeamBkgMixer: events of " + bkg.type + " will be re-used";
          m_reused[message] += 1;
          if (m_reused[message] == 1) B2INFO(message);
          bkgInfo->incrementReusedCounter(bkg.index);
        }
      }

    }


    for (auto& bkg : m_backgrounds) {

      if (bkg.fileType != BackgroundMetaData::c_PXD) continue;

      double mean = bkg.rate * (m_maxTimePXD - m_minTimePXD);
      int nev = gRandom->Poisson(mean);

      for (int iev = 0; iev < nev; iev++) {
        double timeShift = gRandom->Rndm() * (m_maxTimePXD - m_minTimePXD) + m_minTimePXD;
        if (timeShift > m_minTime and timeShift < m_maxTime) continue;
        bkg.tree->GetEntry(bkg.eventCount);

        double minTime = m_minTimePXD;
        double maxTime = m_maxTimePXD;
        if (timeShift <= m_minTime) {
          maxTime = m_minTime;
        } else {
          minTime = m_maxTime;
        }
        addSimHits(pxdSimHits, m_simHits.PXD, timeShift, minTime, maxTime);

        bkg.eventCount++;
        if (bkg.eventCount >= bkg.numEvents) {
          bkg.eventCount = 0;
          std::string message = "BeamBkgMixer: events of " + bkg.type + " will be re-used";
          m_reused[message] += 1;
          if (m_reused[message] == 1) B2INFO(message);
          bkgInfo->incrementReusedCounter(bkg.index);
        }
      }

    }

  }


  void BeamBkgMixerModule::endRun()
  {
  }

  void BeamBkgMixerModule::terminate()
  {

    B2INFO("BeamBkgMixer - reused samples:");
    for (const auto& message : m_reused) {
      B2INFO(message.first << "(occured " << message.second << " times)");
    }
    B2INFO("BeamBkgMixer - rejected events:");
    for (const auto& message : m_rejected) {
      B2INFO(message.first << "(occured " << message.second << " times)");
    }

    for (auto& bkg : m_backgrounds) {
      delete bkg.tree;
    }

  }


  bool BeamBkgMixerModule::isComponentIncluded(std::vector<std::string>& components,
                                               const std::string& component)
  {
    if (m_components.empty()) return true;
    auto iterator = std::find(components.begin(), components.end(), component);
    if (iterator != components.end()) {
      components.erase(iterator);
      return true;
    }
    return false;
  }


  void BeamBkgMixerModule::appendSample(SimHitBase::BG_TAG  tag,
                                        const std::string& type,
                                        const std::string& fileName,
                                        double realTime,
                                        BackgroundMetaData::EFileType fileType)
  {
    for (auto& bkg : m_backgrounds) {
      if (tag == bkg.tag and fileType == bkg.fileType) {
        bkg.fileNames.push_back(fileName);
        bkg.realTime += realTime;
        return;
      }
    }
    std::string ftype = type;
    if (fileType == BackgroundMetaData::c_ECL) ftype += "(ECL)";
    if (fileType == BackgroundMetaData::c_PXD) ftype += "(PXD)";
    unsigned index = m_backgrounds.size();
    m_backgrounds.push_back(BkgFiles(tag, ftype, fileName, realTime,
                                     m_overallScaleFactor, fileType, index));
  }


  bool BeamBkgMixerModule::acceptEvent(TClonesArray* cloneArrayECL)
  {
    if (!cloneArrayECL) return true;
    if (m_maxEdepECL == 0) return true;

    int numEntries = cloneArrayECL->GetEntriesFast();
    for (int i = 0; i < numEntries; i++) {
      ECLHit* simHit = static_cast<ECLHit*>(cloneArrayECL->AddrAt(i));
      if (simHit->getEnergyDep() > m_maxEdepECL) return false;
    }
    return true;
  }


} // end Belle2 namespace

