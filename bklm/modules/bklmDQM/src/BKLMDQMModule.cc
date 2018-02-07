/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vipin Gaur, Prof. Leo Piilonen                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Module manager
#include <framework/core/HistoModule.h>

// Own include
#include <bklm/modules/bklmDQM/BKLMDQMModule.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobject classes
#include <bklm/dataobjects/BKLMDigit.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <mdst/dataobjects/Track.h>

// root
#include "TVector3.h"
#include "TDirectory.h"

// boost
#include <boost/format.hpp>
#include "TH1F.h"
#include <stdio.h>

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BKLMDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
BKLMDQMModule::BKLMDQMModule() : HistoModule()
{
  // set module description (e.g. insert text)
  setDescription("BKLM DQM histogrammer");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("outputDigitsName", m_outputDigitsName, "name of BKLMDigit store array", string("BKLMDigits"));

}

BKLMDQMModule::~BKLMDQMModule()
{
}

void BKLMDQMModule::defineHisto()
{

  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("BKLM")->cd();

  h_layerHits = new TH1F("layer hits", "layer hits",
                         40, 0, 15);
  h_ctime = new TH1F("ctime", "Lowest 16 bits of the B2TT CTime signal",
                     100, -2, 2);
  h_ctime->GetXaxis()->SetTitle("ctime");
  h_simtime = new TH1F("simtime", "MC simulation event hit time",
                       100, 0, 1000);
  h_simtime->GetXaxis()->SetTitle("time [ns]");
  h_time = new TH1F("time", "Reconstructed hit time relative to trigger",
                    100, 0, 1000);
  h_time->GetXaxis()->SetTitle("time [ns]");
  h_time->SetOption("LIVE");
  h_simEDep = new TH1F("simEDep", "MC simulation pulse height",
                       25, 0, 25);
  h_simEDep->GetXaxis()->SetTitle("pulse height [MeV]");
  h_eDep = new TH1F("eDep", "Reconstructed pulse height",
                    25, 0, 25);
  h_eDep->GetXaxis()->SetTitle("pulse height [MeV]");
  h_eDep->SetOption("LIVE");
  h_simNPixel = new TH1F("simNPixel", "Simulated number of MPPC pixels",
                         500, 0, 500);
  h_simNPixel->GetXaxis()->SetTitle("Simulated number of MPPC pixels");
  h_nPixel = new TH1F("nPixel", "Reconstructed number of MPPC pixels",
                      500, 0, 500);
  h_nPixel->GetXaxis()->SetTitle("Reconstructed number of MPPC pixels");
  h_moduleID = new TH1F("module ID", "detector-module identifier",
                        40, 0, 200000000);
  h_moduleID->GetXaxis()->SetTitle("detector-module identifier");
  h_zStrips = new TH1F("zStrips", "z-measuring strip numbers of the 2D hit",
                       100, 0, 500);
  h_zStrips->GetXaxis()->SetTitle("z-measuring strip numbers of the 2D hit");
  h_zStrips->SetOption("LIVE");
  h_phiStrip = new TH1F("phiStrip", "Phi strip number of muon hit",
                        50, -0.5, 49.5);
  h_phiStrip->GetXaxis()->SetTitle("Phi strip number of muon hit");
  h_phiStrip->SetOption("LIVE");
  h_sector = new TH1F("sector", "Sector number of muon hit",
                      10, -0.5, 9.5);
  h_sector->GetXaxis()->SetTitle("Sector number of muon hit");
  h_sector->SetOption("LIVE");
  h_layer = new TH1F("layer", "Layer number of muon hit",
                     16, -0.5, 15.5);
  h_layer->GetXaxis()->SetTitle("Layer number of muon hit");
  h_layer->SetOption("LIVE");
  h_rBKLMHit2ds = new TH1F("rBKLMHit2ds", "Distance from z axis in transverse plane of muon hit",
                           30, 200.0, 350.0);
  h_rBKLMHit2ds->GetXaxis()->SetTitle("Distance from z axis in transverse plane of muon hit");
  h_rBKLMHit2ds->SetOption("LIVE");
  h_zBKLMHit2ds = new TH1F("zBKLMHit2ds", "Axial position of muon hit",
                           100, -200.0, 300.0);
  h_zBKLMHit2ds->GetXaxis()->SetTitle("Axial position of muon hit");
  h_zBKLMHit2ds->SetOption("LIVE");
  h_yvsxBKLMHit2ds = new TH2F("yvsx", "Position projected into y-x plane of muon hit",
                              140, -350.0, 350.0, 140, -350.0, 350.0);
  h_yvsxBKLMHit2ds->GetXaxis()->SetTitle("x(cm)");
  h_yvsxBKLMHit2ds->GetYaxis()->SetTitle("y(cm)");
  h_yvsxBKLMHit2ds->SetOption("LIVE");
  h_xvszBKLMHit2ds = new TH2F("xvsz", "Position projected into x-z plane of muon hit",
                              140, -300.0, 400.0, 140, -350.0, 350.0);
  h_xvszBKLMHit2ds->GetXaxis()->SetTitle("z(cm)");
  h_xvszBKLMHit2ds->GetYaxis()->SetTitle("x(cm)");
  h_xvszBKLMHit2ds->SetOption("LIVE");
  h_yvszBKLMHit2ds = new TH2F("yvsz", "Position projected into y-z plane of muon hit",
                              140, -300.0, 400.0, 140, -350.0, 350.0);
  h_yvszBKLMHit2ds->GetXaxis()->SetTitle("z(cm)");
  h_yvszBKLMHit2ds->GetYaxis()->SetTitle("y(cm)");
  h_yvszBKLMHit2ds->SetOption("LIVE");

  oldDir->cd();

}


void BKLMDQMModule::initialize()
{
  REG_HISTOGRAM   // required to register histograms to HistoManager
  StoreArray<BKLMDigit> digits(m_outputDigitsName);
  digits.isRequired();
  digits.registerInDataStore();
  StoreArray<BKLMHit2d> hits(m_outputHitsName);
  hits.registerInDataStore();
}
void BKLMDQMModule::beginRun()
{
}
void BKLMDQMModule::event()
{
  StoreArray<BKLMDigit> digits(m_outputDigitsName);
  int nent = digits.getEntries();
  for (int i = 0; i < nent; i++) {
    BKLMDigit* digit = static_cast<BKLMDigit*>(digits[i]);
    h_layerHits->Fill(digit->getModuleID());
    h_ctime->Fill(digit->getCTime());
    h_simtime->Fill(digit->getSimTime());
    h_time->Fill(digit->getTime());
    h_simEDep->Fill(digit->getSimEDep());
    h_eDep->Fill(digit->getEDep());
    h_simNPixel->Fill(digit->getSimNPixel());
    h_nPixel->Fill(digit->getNPixel());
  }
  StoreArray<BKLMHit2d> hits(m_outputHitsName);
  int nnent = hits.getEntries();
  for (int i = 0; i < nnent; i++) {
    BKLMHit2d* hit = static_cast<BKLMHit2d*>(hits[i]);
    h_moduleID->Fill(hit->getModuleID());
    h_zStrips->Fill(hit->getZStripAve());
    h_phiStrip->Fill(hit->getPhiStripAve());
    h_sector->Fill(hit->getSector());
    h_layer->Fill(hit->getLayer());
    h_rBKLMHit2ds->Fill(hit->getGlobalPosition().Perp());
    h_zBKLMHit2ds->Fill(hit->getGlobalPosition().Z());
    h_yvsxBKLMHit2ds->Fill(hit->getGlobalPosition().Y(), hit->getGlobalPosition().X());
    h_xvszBKLMHit2ds->Fill(hit->getGlobalPosition().X(), hit->getGlobalPosition().Z());
    h_yvszBKLMHit2ds->Fill(hit->getGlobalPosition().Y(), hit->getGlobalPosition().Z());
  }
}
void BKLMDQMModule::endRun()
{
}

void BKLMDQMModule::terminate()
{
}
