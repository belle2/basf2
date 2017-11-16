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

// root
#include "TH1F.h"

using namespace std;
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
  addParam("outputDigitsName", m_OutputDigitsName, "name of BKLMDigit store array", string("BKLMDigits"));
}

BKLMDQMModule::~BKLMDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void BKLMDQMModule::defineHisto()
{
  m_LayerHits = new TH1F("layer hits", "layer hits",
                         40, 0, 15);
  m_CTime = new TH1F("CTime", "Lowest 16 bits of the B2TT CTime signal",
                     500, -50, 50);
  m_CTime->GetXaxis()->SetTitle("CTime");

  m_SimTime = new TH1F("SimTime", "MC simulation event hit time",
                       40, -10, 10);
  m_SimTime->GetXaxis()->SetTitle("time [ns]");

  m_Time = new TH1F("Time", "Reconstructed hit time relative to trigger",
                    40, -10, 10);
  m_Time->GetXaxis()->SetTitle("time [ns]");

  m_SimEDep = new TH1F("SimEDep", "MC simulation pulse height",
                       25, 0, 25);
  m_SimEDep->GetXaxis()->SetTitle("pulse height [MeV]");

  m_EDep = new TH1F("EDep", "Reconstructed pulse height",
                    25, 0, 25);
  m_EDep->GetXaxis()->SetTitle("pulse height [MeV]");

  m_SimNPixel = new TH1F("SimNPixel", "Simulated number of MPPC pixels",
                         500, 0, 500);
  m_SimNPixel->GetXaxis()->SetTitle("Simulated number of MPPC pixels");

  m_NPixel = new TH1F("NPixel", "Reconstructed number of MPPC pixels",
                      500, 0, 500);
  m_NPixel->GetXaxis()->SetTitle("Reconstructed number of MPPC pixels");

}


void BKLMDQMModule::initialize()
{
  REG_HISTOGRAM   // required to register histograms to HistoManager
  StoreArray<BKLMDigit> digits(m_OutputDigitsName);
  digits.isRequired();
  digits.registerInDataStore();
}

void BKLMDQMModule::beginRun()
{
}

void BKLMDQMModule::event()
{

  StoreArray<BKLMDigit> digits(m_OutputDigitsName);
  for (int i = 0; i < digits.getEntries(); i++) {
    BKLMDigit* bklmDigit = static_cast<BKLMDigit*>(digits[i]);
    m_LayerHits->Fill(bklmDigit->getModuleID());
    m_CTime->Fill(bklmDigit->getCTime());
    m_SimTime->Fill(bklmDigit->getSimTime());
    m_Time->Fill(bklmDigit->getTime());
    m_SimEDep->Fill(bklmDigit->getSimEDep());
    m_EDep->Fill(bklmDigit->getEDep());
    m_SimNPixel->Fill(bklmDigit->getSimNPixel());
    m_NPixel->Fill(bklmDigit->getNPixel());
  }
}

void BKLMDQMModule::endRun()
{
}

void BKLMDQMModule::terminate()
{
  delete m_LayerHits;
  delete m_CTime;
  delete m_SimTime;
  delete m_Time;
  delete m_SimEDep;
  delete m_EDep;
  delete m_SimNPixel;
  delete m_NPixel;
}
