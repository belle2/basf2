/* Nanae Taniguchi 2015.05.21 */

#include <cdc/modules/cdcDQM/cdcDQM7.h>

#include <framework/core/HistoModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>

#include <framework/datastore/StoreArray.h>

#include <cdc/dataobjects/CDCHit.h>

#include "TH1F.h"

#include <stdio.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(cdcDQM7)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

cdcDQM7Module::cdcDQM7Module() : HistoModule()
{
  //Set module properties
  setDescription("Exercise1 module");
  //  setDescription(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
}

cdcDQM7Module::~cdcDQM7Module()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

TH1F* h_tdc; // TDC hit
TH1F* h_adc; // FADC sum
TH1F* h_layer; // ilayer hit
TH1F* h_nhits; // cell hits in the first ilayer

void cdcDQM7Module::defineHisto()
{
  h_tdc = new TH1F("TDC", "TDC", 100, 7400, 8400);
  h_adc = new TH1F("ADC", "ADC", 50, 0, 500);
  h_layer = new TH1F("iLayer", "iLayer", 6, 0, 6);
  h_nhits = new TH1F("nhits", "nhits", 16, 0, 16);

  h_layer->SetMinimum(0);
  h_nhits->SetMinimum(0);
  h_tdc->SetFillColor(kPink + 7);
  h_adc->SetFillColor(kOrange + 6);
  h_layer->SetFillColor(kSpring + 7);
  h_nhits->SetFillColor(kAzure + 6);
}

void cdcDQM7Module::initialize()
{
  REG_HISTOGRAM   // required to register histograms to HistoManager

}

void cdcDQM7Module::beginRun()
{
}


void cdcDQM7Module::event()
{
  StoreArray<CDCHit> cdcHits;
  int nent = cdcHits.getEntries();

  for (int i = 0; i < nent; i++) {
    CDCHit* cdchit = static_cast<CDCHit*>(cdcHits[i]);

    h_tdc->Fill(cdchit->getTDCCount());
    h_adc->Fill(cdchit->getADCCount());
    h_layer->Fill(cdchit->getILayer());

    if (cdchit->getILayer() == 0) {
      h_nhits->Fill(cdchit->getIWire());
    }
  }

}


void cdcDQM7Module::endRun()
{
}


void cdcDQM7Module::terminate()
{
}
