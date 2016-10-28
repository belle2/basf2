/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Module manager
#include <framework/core/HistoModule.h>

// Own include
#include <arich/modules/arichDQM/ARICHDQMModule.h>
#include <arich/dbobjects/ARICHGeometryConfig.h>
#include <arich/dbobjects/ARICHChannelMapping.h>
#include <arich/dbobjects/ARICHMergerMapping.h>
#include <arich/dbobjects/ARICHCopperMapping.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// Dataobject classes
#include <arich/dataobjects/ARICHDigit.h>
#include <arich/dataobjects/ARICHHit.h>
#include <TH1F.h>
#include <TH2Poly.h>
#include <TFile.h>
#include <framework/database/DBObjPtr.h>
// print bitset
#include <bitset>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ARICHDQM)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ARICHDQMModule::ARICHDQMModule() : HistoModule()
  {
    // set module description (e.g. insert text)
    setDescription("ARICH DQM histogrammer");
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("histogramDirectoryName", m_histogramDirectoryName,
             "histogram directory in ROOT file", string("ARICH"));

  }

  ARICHDQMModule::~ARICHDQMModule()
  {
  }

  void ARICHDQMModule::defineHisto()
  {
    // Create a separate histogram directory and cd into it.
    TDirectory* oldDir = gDirectory;
    oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();

    m_hBits = new TH1F("hBits", "comulative hit bitmap", 4, -0.5, 3.5);
    m_hHits = new TH1F("hHits", "# of hits/event", 864, -0.5, 863.5);
    m_hHitsHapd = new TH2F("hHitsChn", "# of hits/channel/(1000 events);module ID; asic ch #", 420, 0.5, 420.5, 144, -0.5, 143.5);
    m_hHitsHapd->SetOption("colz");
    m_hHitsMerger = new TH1F("hHitsMerg", "# of hits/merger/(1000 events);merger ID #", 72, 0.5, 72.5);
    m_hHitsCopper = new TH1F("hHitsCopp", "# of hits/copper/(1000 events);copper ID #", 18, 0.5, 18.5);

  }

  void ARICHDQMModule::initialize()
  {
    // Register histograms (calls back defineHisto)
    REG_HISTOGRAM;

    StoreArray<ARICHDigit> digits;
    digits.isRequired();

    StoreArray<ARICHHit> hits;
    hits.isRequired();

  }


  void ARICHDQMModule::beginRun()
  {
  }

  void ARICHDQMModule::event()
  {

    DBObjPtr<ARICHGeometryConfig> m_geoPar;
    DBObjPtr<ARICHChannelMapping> chMap;
    DBObjPtr<ARICHMergerMapping> mrgMap;
    DBObjPtr<ARICHCopperMapping> cprMap;

    StoreArray<ARICHDigit> digits;
    StoreArray<ARICHHit> hits;

    for (const auto& digit : digits) {

      uint8_t bits = digit.getBitmap();
      for (int i = 0; i < 8; i++) {
        if (bits & (1 << i)) m_hBits->Fill(i);
        //std::cout <<  std::bitset<8>(bits) << std::endl;
      }
      int asicCh = digit.getChannelID();
      int modID = digit.getModuleID();
      m_hHitsHapd->Fill(modID, asicCh);
      unsigned mrgID = mrgMap->getMergerID(modID);
      unsigned cprID = cprMap->getCopperID(mrgID);
      m_hHitsMerger->Fill(mrgID);
      m_hHitsCopper->Fill(cprID);
    }

    m_hHits->Fill(digits.getEntries());

  }


  void ARICHDQMModule::endRun()
  {
  }

  void ARICHDQMModule::terminate()
  {
  }


} // end Belle2 namespace

