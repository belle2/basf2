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

#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
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

// MetaData
#include <framework/dataobjects/EventMetaData.h>
#include <background/dataobjects/BackgroundMetaData.h>

// Root
#include <TRandom3.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(BeamBkgMixer)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  BeamBkgMixerModule::BeamBkgMixerModule() : Module(),
    m_PXD(false), m_SVD(false), m_CDC(false), m_TOP(false),
    m_ARICH(false), m_ECL(false), m_BKLM(false), m_EKLM(false)
  {
    // set module description (e.g. insert text)
    setDescription("Beam background mixer at SimHit level that uses beam background"
                   " simulation output directly (collision files) and not ROF files. "
                   "Each background event is shifted in time randomly within "
                   "a time window specified with minTime and maxTime.");
    // set property flags
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("backgroundFiles", m_backgroundFiles,
             "List of background (collision) files, "
             "wildcards possible (as in TChain)");
    addParam("minTime", m_minTime,
             "Time window lower edge in nano seconds", -1000.0);
    addParam("maxTime", m_maxTime,
             "Time window upper edge in nano seconds", 800.0);
    addParam("scaleFactors", m_scaleFactors,
             "Factors to scale rates of backgrounds. "
             "Possible tag names: " + m_bgTypes.getBGTypes(),
             m_scaleFactors);
    addParam("components", m_components,
             "Detector components to be included, empty list means all components",
             m_components);

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

      // if wildcarding is not used in the name ...
      if (!TString(file.c_str()).Contains("*")) {
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
      }

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
      persistent.SetBranchAddress("BackgroundMetaData", &bkgMetaData);
      if (bkgMetaData == 0) {
        B2ERROR(file << ": branch 'BackgroundMetaData' not found");
        continue;
      }
      std::vector<unsigned> tags;
      std::vector<std::string> types;
      double realTime = 0;
      for (unsigned k = 0; k < persistent.GetEntries(); k++) {
        persistent.GetEntry(k);
        BackgroundMetaData* bgMD = static_cast<BackgroundMetaData*>(bkgMetaData);
        tags.push_back(bgMD->getBackgroundTag());
        types.push_back(bgMD->getBackgroundType());
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

      appendSample(tags[0], types[0], file, realTime);

    }


    // set scale factors

    for (auto scaleFactor : m_scaleFactors) {
      std::string type = std::get<0>(scaleFactor);
      if (m_bgTypes.getTag(type) == 0)
        B2ERROR("Unknown beam background type found in 'scaleFactors': " << type << "\n"
                "Possible are: " + m_bgTypes.getBGTypes());
      for (auto & bkg : m_backgrounds) {
        if (type == bkg.type)
          bkg.scaleFactor =  std::get<1>(scaleFactor);
      }
    }

    // open files for reading SimHits

    for (auto & bkg : m_backgrounds) {

      // define TChain for reading SimHits
      bkg.tree = new TChain("tree");
      for (unsigned i = 0; i < bkg.fileNames.size(); ++i) {
        bkg.numFiles += bkg.tree->Add(bkg.fileNames[i].c_str());
      }

      bkg.numEvents = bkg.tree->GetEntries();
      bkg.rate =  bkg.numEvents / bkg.realTime * bkg.scaleFactor;

      if (m_PXD) bkg.tree->SetBranchAddress("PXDSimHits", &bkg.simHits.PXD);
      if (m_SVD) bkg.tree->SetBranchAddress("SVDSimHits", &bkg.simHits.SVD);
      if (m_CDC) bkg.tree->SetBranchAddress("CDCSimHits", &bkg.simHits.CDC);
      if (m_TOP) bkg.tree->SetBranchAddress("TOPSimHits", &bkg.simHits.TOP);
      if (m_ARICH) bkg.tree->SetBranchAddress("ARICHSimHits", &bkg.simHits.ARICH);
      if (m_ECL) bkg.tree->SetBranchAddress("ECLHits", &bkg.simHits.ECL);
      if (m_BKLM) bkg.tree->SetBranchAddress("BKLMSimHits", &bkg.simHits.BKLM);
      if (m_EKLM) bkg.tree->SetBranchAddress("EKLMSimHits", &bkg.simHits.EKLM);

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


    // SimHits
    StoreArray<PXDSimHit>::optional();
    StoreArray<SVDSimHit>::optional();
    StoreArray<CDCSimHit>::optional();
    StoreArray<TOPSimHit>::optional();
    StoreArray<ARICHSimHit>::optional();
    StoreArray<ECLHit>::optional();
    StoreArray<BKLMSimHit>::optional();
    StoreArray<EKLMSimHit>::optional();

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

    if (m_PXD && !pxdSimHits.isValid()) pxdSimHits.create();
    if (m_SVD && !svdSimHits.isValid()) svdSimHits.create();
    if (m_CDC && !cdcSimHits.isValid()) cdcSimHits.create();
    if (m_TOP && !topSimHits.isValid()) topSimHits.create();
    if (m_ARICH && !arichSimHits.isValid()) arichSimHits.create();
    if (m_ECL && !eclHits.isValid()) eclHits.create();
    if (m_BKLM && !bklmSimHits.isValid()) bklmSimHits.create();
    if (m_EKLM && !eklmSimHits.isValid()) eklmSimHits.create();

    for (auto & bkg : m_backgrounds) {
      double mean = bkg.rate * (m_maxTime - m_minTime);
      int nev = gRandom->Poisson(mean);

      for (int iev = 0; iev < nev; iev++) {
        double timeShift = gRandom->Rndm() * (m_maxTime - m_minTime) + m_minTime;
        bkg.tree->GetEntry(bkg.eventCount);

        addSimHits(pxdSimHits, bkg.simHits.PXD, timeShift);
        addSimHits(svdSimHits, bkg.simHits.SVD, timeShift);
        addSimHits(cdcSimHits, bkg.simHits.CDC, timeShift);
        addSimHits(topSimHits, bkg.simHits.TOP, timeShift);
        addSimHits(arichSimHits, bkg.simHits.ARICH, timeShift);
        addSimHits(eclHits, bkg.simHits.ECL, timeShift);
        addSimHits(bklmSimHits, bkg.simHits.BKLM, timeShift);
        addSimHits(eklmSimHits, bkg.simHits.EKLM, timeShift);

        bkg.eventCount++;
        if (bkg.eventCount >= bkg.numEvents) {
          bkg.eventCount = 0;
          B2INFO("BeamBkgMixer: events of " << bkg.type << " will be re-used");
        }
      }
    }

  }


  void BeamBkgMixerModule::endRun()
  {
  }

  void BeamBkgMixerModule::terminate()
  {

    for (auto & bkg : m_backgrounds) {
      delete bkg.tree;
    }

  }

  void BeamBkgMixerModule::printModuleParams() const
  {
  }


  bool BeamBkgMixerModule::isComponentIncluded(std::vector<std::string>& components,
                                               const std::string& component)
  {
    if (m_components.empty()) return true;

    for (unsigned i = 0; i < components.size(); ++i) {
      if (components[i] == component) {
        components.erase(components.begin() + i);
        --i;
        return true;
      }
    }
    return false;
  }


  void BeamBkgMixerModule::appendSample(unsigned tag,
                                        const std::string& type,
                                        const std::string& fileName,
                                        double realTime)
  {
    for (auto & bkg : m_backgrounds) {
      if (tag == bkg.tag) {
        bkg.fileNames.push_back(fileName);
        bkg.realTime += realTime;
        return;
      }
    }
    m_backgrounds.push_back(BkgFiles(tag, type, fileName, realTime));
  }



} // end Belle2 namespace

