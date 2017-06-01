/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitry Matvienko                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <ecl/modules/eclDQM/eclDQM.h>
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <ecl/dataobjects/ECLDigit.h>
#include "TH1F.h"
#include <stdio.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLDQM);


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLDQMModule::ECLDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("ECL DQM");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing

}

ECLDQMModule::~ECLDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void ECLDQMModule::defineHisto()
{
  h_cid = new TH1F("cid", "Crystal ID", 8736, 1, 8737);
  h_cid->GetXaxis()->SetTitle("Cell ID");

  h_cid_Thr2MeV = new TH1F("cid_Thr2MeV", "Crystal ID with Thr = 2 MeV", 8736, 1, 8737);
  h_cid_Thr2MeV->GetXaxis()->SetTitle("Cell ID");

  h_cid_Thr5MeV = new TH1F("cid_Thr5MeV", "Crystal ID with Thr = 5 MeV", 8736, 1, 8737);
  h_cid_Thr5MeV->GetXaxis()->SetTitle("Cell ID");

  h_cid_Thr10MeV = new TH1F("cid_Thr10MeV", "Crystal ID with Thr  = 10 MeV", 8736, 1, 8737);
  h_cid_Thr10MeV->GetXaxis()->SetTitle("Cell ID");

  h_edep = new TH1F("edep", "Energy deposition in event", 300, 1000, 31000);
  h_edep->GetXaxis()->SetTitle("ADC counts");

  h_time_barrel_Thr5MeV = new TH1F("time_barrel_Thr5MeV", "Reconstructed time for ECL barrel with Thr = 5 MeV", 206, time_min,
                                   time_max);
  h_time_barrel_Thr5MeV->GetXaxis()->SetTitle("time [ns]");

  h_time_endcaps_Thr5MeV = new TH1F("time_endcaps_Thr5MeV", "Reconstructed time for ECL endcaps with Thr = 5 MeV", 206, time_min,
                                    time_max);
  h_time_endcaps_Thr5MeV->GetXaxis()->SetTitle("time [ns]");

  h_time_barrel_Thr10MeV = new TH1F("time_barrel_Thr10MeV", "Reconstructed time for ECL barrel with Thr = 10 MeV", 206, time_min,
                                    time_max);
  h_time_barrel_Thr10MeV->GetXaxis()->SetTitle("time [ns]");

  h_time_endcaps_Thr10MeV = new TH1F("time_endcaps_Thr10MeV", "Reconstructed time for ECL endcaps with Thr = 10 MeV", 206, time_min,
                                     time_max);
  h_time_endcaps_Thr10MeV->GetXaxis()->SetTitle("time [ns]");

  h_time_barrel_Thr50MeV = new TH1F("time_barrel_Thr50MeV", "Reconstructed time for ECL barrel with Thr = 50 MeV", 206, time_min,
                                    time_max);
  h_time_barrel_Thr50MeV->GetXaxis()->SetTitle("time [ns]");

  h_time_endcaps_Thr50MeV = new TH1F("time_endcaps_Thr50MeV", "Reconstructed time for ECL endcaps with Thr = 50 MeV", 206, time_min,
                                     time_max);
  h_time_endcaps_Thr50MeV->GetXaxis()->SetTitle("time [ns]");

  h_quality = new TH1F("quality", "Fit quality flag (0-good, 1- large amplitude, 2 - bad chi2)", 4, -1, 3);
  h_quality->GetXaxis()->SetTitle("Flag number");

  h_cid->SetLineColor(kPink + 6);
  h_cid->SetFillColor(kPink + 6);

  h_cid_Thr2MeV->SetLineColor(kPink + 6);
  h_cid_Thr2MeV->SetFillColor(kPink + 6);

  h_cid_Thr5MeV->SetLineColor(kPink + 6);
  h_cid_Thr5MeV->SetFillColor(kPink + 6);

  h_cid_Thr10MeV->SetLineColor(kPink + 6);
  h_cid_Thr10MeV->SetFillColor(kPink + 6);

  h_quality->SetLineColor(kPink + 6);
  h_quality->SetFillColor(kPink + 6);

}

void ECLDQMModule::initialize()
{
  REG_HISTOGRAM;   // required to register histograms to HistoManager

}

void ECLDQMModule::beginRun()
{
}


void ECLDQMModule::event()
{
  StoreArray<ECLDigit> ECLDigitData;
  int necl = ECLDigitData.getEntries();

  int ecletot = 0;

  for (int idig = 0; idig < necl; idig++) {
    auto eclhit = ECLDigitData[idig];

    int cid = eclhit->getCellId();
    int energy = eclhit->getAmp();
    int timing = eclhit->getTimeFit();
    int quality = eclhit->getQuality();

    ecletot += energy;

    h_cid->Fill(cid);   // get crystall CId
//  1 ADC = 0.05 MeV
    if (energy > 40)  h_cid_Thr2MeV->Fill(cid);
    if (energy > 100) h_cid_Thr5MeV->Fill(cid);
    if (energy > 200) h_cid_Thr10MeV->Fill(cid);


    if (energy > 100) {

      if (cid < 7777 && cid > 1152) h_time_barrel_Thr5MeV->Fill(float(timing) / 2.032); // get reconstructed time for Thr = 5 MeV in [ns]
      else h_time_endcaps_Thr5MeV->Fill(float(timing) / 2.032);

    }

    if (energy > 200)  {

      if (cid < 7777
          && cid > 1152) h_time_barrel_Thr10MeV->Fill(float(timing) / 2.032); // get reconstructed time for Thr = 10 MeV in [ns]
      else h_time_endcaps_Thr10MeV->Fill(float(timing) / 2.032);

    }

    if (energy > 1000) {

      if (cid < 7777
          && cid > 1152) h_time_barrel_Thr50MeV->Fill(float(timing) / 2.032); // get reconstructed time for Thr = 50 MeV in [ns]
      else h_time_endcaps_Thr50MeV->Fill(float(timing) / 2.032);

    }


    h_quality->Fill(quality);

  }

  h_edep->Fill(ecletot); //get energy deposition in event

}


void ECLDQMModule::endRun()
{
}


void ECLDQMModule::terminate()
{
}
