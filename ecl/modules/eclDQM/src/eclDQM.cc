/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 *  ECL Data Quality Monitor                                              *
 *                                                                        *
 *  This module provides histograms for ECL Data Quality Monitoring       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitry Matvienko (d.v.matvienko@inp.nsk.su)              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//THIS MODULE
#include <ecl/modules/eclDQM/eclDQM.h>

//FRAMEWORK
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

//ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/utility/eclChannelMapper.h>

//ROOT
#include "TH1F.h"
#include <TDirectory.h>

//NAMESPACE(S)
using namespace Belle2;
using namespace ECL;
using namespace std;

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
  setDescription("ECL Data Quality Monitor");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "histogram directory in ROOT file", string("ECL"));

}

ECLDQMModule::~ECLDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void ECLDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;

  // Create a separate histogram directory and cd into it.

  TDirectory* dirDAQ = NULL;
  dirDAQ = oldDir->mkdir(m_histogramDirectoryName.c_str());
  dirDAQ->cd();

  h_cid = new TH1F("cid", "Crystal ID", 8736, 1, 8737);
  h_cid->GetXaxis()->SetTitle("Cell ID");

  h_cid_Thr2MeV = new TH1F("cid_Thr2MeV", "Crystal ID (Thr = 2 MeV)", 8736, 1, 8737);
  h_cid_Thr2MeV->GetXaxis()->SetTitle("Cell ID");

  h_cid_Thr5MeV = new TH1F("cid_Thr5MeV", "Crystal ID (Thr = 5 MeV)", 8736, 1, 8737);
  h_cid_Thr5MeV->GetXaxis()->SetTitle("Cell ID");

  h_cid_Thr10MeV = new TH1F("cid_Thr10MeV", "Crystal ID (Thr  = 10 MeV)", 8736, 1, 8737);
  h_cid_Thr10MeV->GetXaxis()->SetTitle("Cell ID");

  h_edep = new TH1F("edep", "Energy deposition in event", 500, 0, 5);
  h_edep->GetXaxis()->SetTitle("energy, [GeV]");

  h_time_barrel_Thr5MeV = new TH1F("time_barrel_Thr5MeV", "Reconstructed time for ECL barrel with Thr = 5 MeV", 206, -1030,
                                   1030);
  h_time_barrel_Thr5MeV->GetXaxis()->SetTitle("time [ns]");

  h_time_endcaps_Thr5MeV = new TH1F("time_endcaps_Thr5MeV", "Reconstructed time for ECL endcaps with Thr = 5 MeV", 206, -1030,
                                    1030);
  h_time_endcaps_Thr5MeV->GetXaxis()->SetTitle("time [ns]");

  h_time_barrel_Thr10MeV = new TH1F("time_barrel_Thr10MeV", "Reconstructed time for ECL barrel with Thr = 10 MeV", 206, -1030,
                                    1030);
  h_time_barrel_Thr10MeV->GetXaxis()->SetTitle("time [ns]");

  h_time_endcaps_Thr10MeV = new TH1F("time_endcaps_Thr10MeV", "Reconstructed time for ECL endcaps with Thr = 10 MeV", 206, -1030,
                                     1030);
  h_time_endcaps_Thr10MeV->GetXaxis()->SetTitle("time [ns]");

  h_time_barrel_Thr50MeV = new TH1F("time_barrel_Thr50MeV", "Reconstructed time for ECL barrel with Thr = 50 MeV", 206, -1030,
                                    1030);
  h_time_barrel_Thr50MeV->GetXaxis()->SetTitle("time [ns]");

  h_time_endcaps_Thr50MeV = new TH1F("time_endcaps_Thr50MeV", "Reconstructed time for ECL endcaps with Thr = 50 MeV", 206, -1030,
                                     1030);
  h_time_endcaps_Thr50MeV->GetXaxis()->SetTitle("time [ns]");

  h_quality = new TH1F("quality", "Fit quality flag (0-good, 1- large amplitude, 2 - bad chi2)", 4, -1, 3);
  h_quality->GetXaxis()->SetTitle("Flag number");

  oldDir->cd();
}

void ECLDQMModule::initialize()
{
  REG_HISTOGRAM;   // required to register histograms to HistoManager

  StoreArray<ECLDigit> ECLDigits;
  ECLDigits.isRequired();

  StoreArray<ECLCalDigit> ECLCalDigits;
  ECLCalDigits.isRequired();


}

void ECLDQMModule::beginRun()
{
}


void ECLDQMModule::event()
{
  StoreArray<ECLDigit> ECLDigits;
  StoreArray<ECLCalDigit> ECLCalDigits;

  for (auto& aECLDigit : ECLDigits) {

    h_quality->Fill(aECLDigit.getQuality());

  }


  double ecletot = 0;

  for (auto& aECLCalDigit : ECLCalDigits) {

    int cid        = aECLCalDigit.getCellId();
    double energy  = aECLCalDigit.getEnergy();
    double timing  = aECLCalDigit.getTime();

    h_cid->Fill(cid);   // get crystall CId

    if (energy > 0.002)  h_cid_Thr2MeV->Fill(cid);
    if (energy > 0.005)  h_cid_Thr5MeV->Fill(cid);
    if (energy > 0.010)  h_cid_Thr10MeV->Fill(cid);

    if (energy > 0.005) {

      if (cid > ECL_FWD_CHANNELS
          && cid < ECL_FWD_CHANNELS + ECL_BARREL_CHANNELS) h_time_barrel_Thr5MeV->Fill(
              timing); // get reconstructed time for Thr = 5 MeV in [ns]
      else h_time_endcaps_Thr5MeV->Fill(timing);

    }

    if (energy > 0.010)  {

      if (cid > ECL_FWD_CHANNELS
          && cid < ECL_FWD_CHANNELS + ECL_BARREL_CHANNELS) h_time_barrel_Thr10MeV->Fill(
              timing); // get reconstructed time for Thr = 10 MeV in [ns]
      else h_time_endcaps_Thr10MeV->Fill(timing);

    }

    if (energy > 0.050) {

      if (cid > ECL_FWD_CHANNELS
          && cid < ECL_FWD_CHANNELS + ECL_BARREL_CHANNELS) h_time_barrel_Thr50MeV->Fill(
              timing); // get reconstructed time for Thr = 50 MeV in [ns]
      else h_time_endcaps_Thr50MeV->Fill(timing);

    }

    ecletot += energy;

  }

  h_edep->Fill(ecletot); //get energy deposition in event in [GeV]


}


void ECLDQMModule::endRun()
{
}


void ECLDQMModule::terminate()
{
}
