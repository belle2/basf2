/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <cdc/modules/cdcDQM/CDCDQMModule.h>

// CDC

// Dataobject classes
#include <framework/database/DBObjPtr.h>

#include <TF1.h>
#include <TVector3.h>
#include <TDirectory.h>

#include <fstream>
#include <math.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(CDCDQM)

CDCDQMModule::CDCDQMModule() : HistoModule()
{
  // set module description (e.g. insert text)
  setDescription("Make summary of data quality.");
  addParam("MinHits", m_minHits, "Include only events with more than MinHits hits in ARICH", 0);
  setPropertyFlags(c_ParallelProcessingCertified);
}

CDCDQMModule::~CDCDQMModule()
{
}

void CDCDQMModule::defineHisto()
{

  TDirectory* oldDir = gDirectory;

  oldDir->mkdir("CDC");
  oldDir->cd("CDC");
  m_hNEvents = new TH1F("hNEvents", "hNEvents", 10, 0, 10);
  m_hNEvents->GetXaxis()->SetBinLabel(1, "number of events");
  m_hOcc = new TH1F("hOcc", "hOccupancy", 150, 0, 1.5);
  m_hADC = new TH2F("hADC", "hADC", 300, 0, 300, 1000, 0, 1000);
  m_hADCTOTCut = new TH2F("hADCTOTCut", "hADCTOTCut", 300, 0, 300, 1000, 0, 1000);
  m_hTDC = new TH2F("hTDC", "hTDC", 300, 0, 300, 1000, 4200, 5200);
  m_hHit = new TH2F("hHit", "hHit", 56, 0, 56, 400, 0, 400);

  oldDir->cd();
}

void CDCDQMModule::initialize()
{
  REG_HISTOGRAM
  m_cdcHits.isOptional();
  m_cdcRawHits.isOptional();
  m_trgSummary.isOptional();
}

void CDCDQMModule::beginRun()
{

  m_hNEvents->Reset();
  m_hADC->Reset();
  m_hADCTOTCut->Reset();
  m_hTDC->Reset();
  m_hHit->Reset();
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

  if (m_cdcHits.getEntries() < m_minHits) {

    setReturnValue(0); return;
  }
  m_nEvents += 1;
  m_hOcc->Fill(static_cast<float>(m_cdcHits.getEntries()) / nWires);
  for (const auto& raw : m_cdcRawHits) {
    int bid = raw.getBoardId();
    int adc = raw.getFADC();
    int tdc = raw.getTDC();
    int tot = raw.getTOT();
    m_hADC->Fill(bid, adc);
    if (tot > 4) {
      m_hADCTOTCut->Fill(bid, adc);
    }

    if (adc > 50 && tot > 1) {
      m_hTDC->Fill(bid, tdc);
    }
  }
  for (const auto& hit : m_cdcHits) {
    int lay = hit.getICLayer();
    int wire = hit.getIWire();
    m_hHit->Fill(lay, wire);
  }
}

void CDCDQMModule::endRun()
{
  m_hNEvents->SetBinContent(1, m_nEvents);
}

void CDCDQMModule::terminate()
{
}
