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
             "list of background (collision) files: one file per background type");
    addParam("realTime", m_realTime,
             "real time in nano seconds that corresponds to background samle");
    addParam("minTime", m_minTime,
             "time window lower edge in nano seconds", -1000.0);
    addParam("maxTime", m_maxTime,
             "time window upper edge in nano seconds", 800.0);
    addParam("scaleFactors", m_scaleFactors, "factors to scale rates of backgrounds",
             m_scaleFactors);
  }

  BeamBkgMixerModule::~BeamBkgMixerModule()
  {
  }

  void BeamBkgMixerModule::initialize()
  {

    if (m_realTime <= 0) B2ERROR("invalid realTime: " << m_realTime);

    for (unsigned i = m_scaleFactors.size(); i < m_backgroundFiles.size(); ++i) {
      m_scaleFactors.push_back(1.0);
    }

    for (unsigned bkg = 0; bkg < m_backgroundFiles.size(); ++bkg) {
      B2INFO("opening file: " << m_backgroundFiles[bkg]);
      TFile* file = TFile::Open(m_backgroundFiles[bkg].c_str(), "READ");
      if (!file) {
        B2ERROR("Input file " << m_backgroundFiles[bkg] << " not found");
        continue;
      }
      if (!file->IsOpen()) {
        B2ERROR("Input file " << m_backgroundFiles[bkg] << " can't be open");
        continue;
      }
      m_files.push_back(file);
      TTree* tree = (TTree*)file->Get("tree");
      if (!tree) {
        B2WARNING(m_backgroundFiles[bkg] << " TTree 'tree' not found - skip this file");
        continue;
      }
      unsigned numEvents = tree->GetEntries();
      double rate =  numEvents / m_realTime * m_scaleFactors[bkg];
      m_trees.push_back(tree);
      m_numEvents.push_back(numEvents);
      m_eventCount.push_back(0);
      m_bkgRates.push_back(rate);

      B2INFO(" rate = " << rate * 1000 << " MHz");

      BkgHits hits; // Note: allocation of TClonesArray's left to root
      m_bkgSimHits.push_back(hits);

      unsigned i = m_bkgSimHits.size() - 1;
      tree->SetBranchAddress("PXDSimHits", &m_bkgSimHits[i].PXD);
      tree->SetBranchAddress("SVDSimHits", &m_bkgSimHits[i].SVD);
      tree->SetBranchAddress("CDCSimHits", &m_bkgSimHits[i].CDC);
      tree->SetBranchAddress("TOPSimHits", &m_bkgSimHits[i].TOP);
      tree->SetBranchAddress("ARICHSimHits", &m_bkgSimHits[i].ARICH);
      tree->SetBranchAddress("ECLSimHits", &m_bkgSimHits[i].ECL);
      tree->SetBranchAddress("BKLMSimHits", &m_bkgSimHits[i].BKLM);
      tree->SetBranchAddress("EKLMSimHits", &m_bkgSimHits[i].EKLM);
    }

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
          B2INFO("BeamBkgMixer: events re-used, bkg=" << bkg);
        }
      }
    }

  }


  void BeamBkgMixerModule::endRun()
  {
  }

  void BeamBkgMixerModule::terminate()
  {

    for (unsigned i = 0; i < m_files.size(); ++i) {
      if (m_files[i]) m_files[i]->Close();
    }

  }

  void BeamBkgMixerModule::printModuleParams() const
  {
  }


} // end Belle2 namespace

