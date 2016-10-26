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

  ARICHDQMModule::ARICHDQMModule() : Module(), m_hHits(0),  m_hBits(0), m_chHist(0)
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

    f->Write();
  }


} // end Belle2 namespace

