/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <cdc/modules/cdcDQM/CDCDQMModule.h>

// CDC

// framework - DataStore
#include <framework/datastore/StoreArray.h>

// Dataobject classes
#include <framework/database/DBObjPtr.h>

#include <TF1.h>
#include <TVector3.h>

#include <fstream>
#include <math.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(CDCDQM);

  CDCDQMModule::CDCDQMModule() : HistoModule()
  {
    // set module description (e.g. insert text)
    setDescription("Make summary of data quality.");
    setPropertyFlags(c_ParallelProcessingCertified);
    addParam("cdcHitName", m_cdcHitName, "Name of the CDCHit List name..", string(""));
    addParam("cdcRawHitName", m_cdcRawHitName, "Name of the CDCRawHit List name..", string(""));

  }

  CDCDQMModule::~CDCDQMModule()
  {
  }

  void CDCDQMModule::defineHisto()
  {

    TDirectory* oldDir = gDirectory;
    TDirectory* dirCDCDQM = oldDir->mkdir("CDC");

    dirCDCDQM->cd();

    m_hNEvents = new TH1F("hNEvents", "hNEvents", 10, 0, 10);
    m_hNEvents->GetXaxis()->SetBinLabel(1, "number of events");
    m_hOcc = new TH1F("hOcc", "hOccupancy", 150, 0, 1.5);

    for (int i = 0; i < 300; ++i) {
      m_hADC[i] = new TH1F(Form("hADC%d", i), Form("hADC%d", i), 400, 0, 400);
      m_hTDC[i] = new TH1F(Form("hTDC%d", i), Form("hTDC%d", i), 1000, 4200, 5200);
      m_hTDCbig[i] = new TH1F(Form("hTDCbig%d", i), Form("hTDCbig%d", i), 200, 4200, 5200);
      m_hADCTDC[i] = new TH2F(Form("hADCTDC%d", i), Form("hADCTDC%d", i), 100, 0, 400, 200, 4200, 5200);
      m_hADCTOT[i] = new TH2F(Form("hADCTOT%d", i), Form("hADCTOT%d", i), 100, 0, 200, 15, 0, 15);
    }

    for (int i = 0; i < 56; ++i) {
      m_hHit[i] = new TH1F(Form("hHitL%d", i), Form("hHitL%d", i), m_nSenseWires[i], 0, m_nSenseWires[i]);
    }
    // m_hOcc->SetOption("LIVE");
    oldDir->cd();
  }

  void CDCDQMModule::initialize()
  {
    REG_HISTOGRAM

    m_cdcHits.isRequired(m_cdcHitName);
    m_cdcRawHits.isRequired(m_cdcRawHitName);
    m_trgSummary.isRequired();
  }

  void CDCDQMModule::beginRun()
  {

    m_hNEvents->Reset();
    for (int i = 0; i < 300; ++i) {
      m_hADC[i]->Reset();
      m_hTDC[i]->Reset();
      m_hTDCbig[i]->Reset();
      m_hADCTDC[i]->Reset();
      m_hADCTOT[i]->Reset();
    }
    for (int i = 0; i < 56; ++i) {
      m_hHit[i]->Reset();
    }
    m_hOcc->Reset();
  }

  void CDCDQMModule::event()
  {
    const int nWires = 14336;
    setReturnValue(1);
    if (!m_trgSummary.isValid() || (m_trgSummary->getTimType() == Belle2::TRGSummary::TTYP_RAND)) {
      setReturnValue(0);
      return;
    }

    m_nEvents += 1;
    m_hOcc->Fill(static_cast<float>(m_cdcHits.getEntries()) / nWires);
    for (const auto& raw : m_cdcRawHits) {
      int bid = raw.getBoardId();
      int adc = raw.getFADC();
      int tdc = raw.getTDC();
      int tot = raw.getTOT();
      m_hADC[bid]->Fill(adc);
      m_hTDC[bid]->Fill(tdc);
      m_hADCTDC[bid]->Fill(adc, tdc);
      if (adc > 50 && tot > 1) {
        m_hTDCbig[bid]->Fill(tdc);
        m_hADCTOT[bid]->Fill(adc, tot);
      }
    }
    for (const auto& hit : m_cdcHits) {
      int lay = hit.getICLayer();
      int wire = hit.getIWire();
      m_hHit[lay]->Fill(wire);
    }
  }

  void CDCDQMModule::endRun()
  {
    m_hNEvents->SetBinContent(1, m_nEvents);
  }

  void CDCDQMModule::terminate()
  {
  }
}
