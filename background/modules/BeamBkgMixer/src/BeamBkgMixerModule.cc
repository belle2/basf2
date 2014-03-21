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
#include <ecl/dataobjects/ECLSimHit.h>
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

  BeamBkgMixerModule::BeamBkgMixerModule() : Module()

  {
    // set module description (e.g. insert text)
    setDescription("Beam background mixer at SimHit level that uses beam background simulation output directly (collision files) and not ROF files. Each background event is shifted in time randomly within a time window given by minTime and maxTime.");
    setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

    // Add parameters
    addParam("backgroundFiles", m_backgroundFiles,
             "List of background (collision) files: one file per background type, wildcards possible (as in TChain)");
    addParam("minTime", m_minTime,
             "Time window lower edge in nano seconds", -1000.0);
    addParam("maxTime", m_maxTime,
             "Time window upper edge in nano seconds", 800.0);
    addParam("scaleFactors", m_scaleFactors, "Factors to scale rates of backgrounds",
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

    // complete the list of scale factors with the default value
    for (unsigned i = m_scaleFactors.size(); i < m_backgroundFiles.size(); ++i) {
      m_scaleFactors.push_back(1.0);
    }

    // included components
    std::vector<std::string> components = m_components;
    bool PXD = isComponentIncluded(components, "PXD");
    bool SVD = isComponentIncluded(components, "SVD");
    bool CDC = isComponentIncluded(components, "CDC");
    bool TOP = isComponentIncluded(components, "TOP");
    bool ARICH = isComponentIncluded(components, "ARICH");
    bool ECL = isComponentIncluded(components, "ECL");
    bool BKLM = isComponentIncluded(components, "BKLM");
    bool EKLM = isComponentIncluded(components, "EKLM");

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

    // open files
    for (unsigned bkg = 0; bkg < m_backgroundFiles.size(); ++bkg) {

      B2INFO("opening file: " << m_backgroundFiles[bkg]);

      // check the file if wildcarding is not used in the name
      if (!TString(m_backgroundFiles[bkg].c_str()).Contains("*")) {
        TFile* file = TFile::Open(m_backgroundFiles[bkg].c_str(), "READ");
        if (!file) {
          B2ERROR(m_backgroundFiles[bkg] << ": file not found");
          continue;
        }
        if (!file->IsOpen()) {
          B2ERROR(m_backgroundFiles[bkg] << ": can't open file");
          continue;
        }
        file->Close();
      }

      // get realTime from a persistent tree
      TChain persistent("persistent");
      int nFiles = persistent.Add(m_backgroundFiles[bkg].c_str());
      if (nFiles == 0) {
        B2ERROR(m_backgroundFiles[bkg] << ": no such files");
        continue;
      }
      persistent.GetEntries();
      TObject* bkgMetaData = 0; // Note: allocation left to root
      persistent.SetBranchAddress("BackgroundMetaData", &bkgMetaData);
      if (bkgMetaData == 0) {
        B2ERROR(m_backgroundFiles[bkg] << ": branch 'BackgroundMetaData' not found");
        continue;
      }
      float realTime = 0;
      std::vector<unsigned> tags;
      for (unsigned k = 0; k < persistent.GetEntries(); k++) {
        persistent.GetEntry(k);
        realTime += ((BackgroundMetaData*)bkgMetaData)->getRealTime();
        tags.push_back(((BackgroundMetaData*)bkgMetaData)->getBackgroundTag());
      }
      if (realTime <= 0) {
        B2ERROR(m_backgroundFiles[bkg] << ": invalid realTime: " << realTime);
        continue;
      }
      for (unsigned i = 1; i < tags.size(); ++i) {
        if (tags[i] != tags[0]) {
          B2ERROR(m_backgroundFiles[bkg] <<
                  ": files with mixed background types not supported");
          continue;
        }
      }
      std::string bkgType = ((BackgroundMetaData*)bkgMetaData)->getBackgroundType();

      // define TChain for reading SimHits
      TChain* tree = new TChain("tree");
      tree->Add(m_backgroundFiles[bkg].c_str());
      unsigned numEvents = tree->GetEntries();
      double rate =  numEvents / realTime * m_scaleFactors[bkg];

      m_trees.push_back(tree);
      m_numEvents.push_back(numEvents);
      m_eventCount.push_back(0);
      m_bkgRates.push_back(rate);
      m_bkgTypes.push_back(bkgType);

      BkgHits hits; // Note: allocation of TClonesArray's left to root
      m_bkgSimHits.push_back(hits);

      unsigned i = m_bkgSimHits.size() - 1;
      if (PXD) tree->SetBranchAddress("PXDSimHits", &m_bkgSimHits[i].PXD);
      if (SVD) tree->SetBranchAddress("SVDSimHits", &m_bkgSimHits[i].SVD);
      if (CDC) tree->SetBranchAddress("CDCSimHits", &m_bkgSimHits[i].CDC);
      if (TOP) tree->SetBranchAddress("TOPSimHits", &m_bkgSimHits[i].TOP);
      if (ARICH) tree->SetBranchAddress("ARICHSimHits", &m_bkgSimHits[i].ARICH);
      if (ECL) tree->SetBranchAddress("ECLSimHits", &m_bkgSimHits[i].ECL);
      if (BKLM) tree->SetBranchAddress("BKLMSimHits", &m_bkgSimHits[i].BKLM);
      if (EKLM) tree->SetBranchAddress("EKLMSimHits", &m_bkgSimHits[i].EKLM);

      // print INFO
      std::string unit(" ns");
      if (realTime >= 1000.0) {realTime /= 1000.0; unit = " us";}
      if (realTime >= 1000.0) {realTime /= 1000.0; unit = " ms";}
      if (realTime >= 1000.0) {realTime /= 1000.0; unit = " s";}

      B2INFO("--> type: " << bkgType <<
             " (tag=" << tags[0] <<
             ") events: " << numEvents <<
             " realTime: " << realTime << unit <<
             " rate = " << rate * 1000 << " MHz");
    }

    // SimHits
    StoreArray<PXDSimHit>::optional();
    StoreArray<SVDSimHit>::optional();
    StoreArray<CDCSimHit>::optional();
    StoreArray<TOPSimHit>::optional();
    StoreArray<ARICHSimHit>::optional();
    StoreArray<ECLSimHit>::optional();
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
    StoreArray<ECLSimHit> eclSimHits;
    StoreArray<BKLMSimHit> bklmSimHits;
    StoreArray<EKLMSimHit> eklmSimHits;

    unsigned numBkg = m_trees.size();
    for (unsigned bkg = 0; bkg < numBkg; ++bkg) {
      double mean = m_bkgRates[bkg] * (m_maxTime - m_minTime);
      int nev = gRandom->Poisson(mean);

      for (int iev = 0; iev < nev; iev++) {
        double timeShift = gRandom->Rndm() * (m_maxTime - m_minTime) + m_minTime;
        m_trees[bkg]->GetEntry(m_eventCount[bkg]);

        addSimHits(pxdSimHits, m_bkgSimHits[bkg].PXD, timeShift);
        addSimHits(svdSimHits, m_bkgSimHits[bkg].SVD, timeShift);
        addSimHits(cdcSimHits, m_bkgSimHits[bkg].CDC, timeShift);
        addSimHits(topSimHits, m_bkgSimHits[bkg].TOP, timeShift);
        addSimHits(arichSimHits, m_bkgSimHits[bkg].ARICH, timeShift);
        addSimHits(eclSimHits, m_bkgSimHits[bkg].ECL, timeShift);
        addSimHits(bklmSimHits, m_bkgSimHits[bkg].BKLM, timeShift);
        addSimHits(eklmSimHits, m_bkgSimHits[bkg].EKLM, timeShift);

        m_eventCount[bkg]++;
        if (m_eventCount[bkg] >= m_numEvents[bkg]) {
          m_eventCount[bkg] = 0;
          B2INFO("BeamBkgMixer: events of " << m_bkgTypes[bkg] <<
                 " re-used (file#" << bkg << ")");
        }
      }
    }

  }


  void BeamBkgMixerModule::endRun()
  {
  }

  void BeamBkgMixerModule::terminate()
  {

    for (unsigned i = 0; i < m_trees.size(); ++i) {
      delete m_trees[i];
    }


  }

  void BeamBkgMixerModule::printModuleParams() const
  {
  }


  bool BeamBkgMixerModule::isComponentIncluded(std::vector<std::string>& components,
                                               const std::string component)
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



} // end Belle2 namespace

