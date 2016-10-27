/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <arich/modules/arichDQM/ARICHDQMModule.h>
#include <arich/dbobjects/ARICHGeometryConfig.h>
#include <arich/dbobjects/ARICHChannelMapping.h>
#include <arich/dbobjects/ARICHMergerMapping.h>
#include <arich/dbobjects/ARICHCopperMapping.h>

// channel histogram
#include <arich/utility/ARICHChannelHist.h>


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

  ARICHDQMModule::ARICHDQMModule() : Module(), m_hHits(0),  m_hBits(0), m_hHitsHapd(0), m_hHitsMerger(0), m_hHitsCopper(0),
    m_chHist(0)
  {
    // set module description (e.g. insert text)
    setDescription("Fills ARICHHits collection from ARICHDigits");
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("outputFileName", m_fname, "output file name", string("ARICHHists.root"));
  }

  ARICHDQMModule::~ARICHDQMModule()
  {
  }

  void ARICHDQMModule::initialize()
  {

    StoreArray<ARICHDigit> digits;
    digits.isRequired();

    StoreArray<ARICHHit> hits;
    hits.isRequired();

    m_hBits = new TH1F("hBits", "comulative hit bitmap", 4, -0.5, 3.5);
    m_hHits = new TH1F("hHits", "# of hits/event", 864, -0.5, 863.5);
    m_hHitsHapd = new TH2F("hHitsChn", "# of hits/channel/(1000 events);module ID; asic ch #", 420, 0.5, 420.5, 144, -0.5, 143.5);
    m_hHitsMerger = new TH1F("hHitsMerg", "# of hits/merger/(1000 events);merger ID #", 72, 0.5, 72.5);
    m_hHitsCopper = new TH1F("hHitsCopp", "# of hits/copper/(1000 events);copper ID #", 18, 0.5, 18.5);
    m_chHist = new ARICHChannelHist("hits", "ARICH channel hits");

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
      m_chHist->fillBin(modID, asicCh);
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

    TFile* f = new TFile(m_fname.data(), "RECREATE");
    m_hHits->Write();
    m_hBits->Write();
    m_chHist->Write();
    m_hHitsHapd->SetOption("colz");
    double scale = 1000. / double(m_hHits->GetEntries());
    m_hHitsHapd->Scale(scale);
    m_hHitsHapd->Write();
    m_hHitsMerger->Write();
    m_hHitsMerger->Scale(scale);
    m_hHitsCopper->Write();
    m_hHitsCopper->Scale(scale);
    f->Write();
  }


} // end Belle2 namespace

