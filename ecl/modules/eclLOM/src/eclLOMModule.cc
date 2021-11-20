/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <iostream>
#include <ecl/modules/eclLOM/eclLOMModule.h>
#include "trg/ecl/dataobjects/TRGECLWaveform.h"
//#include "trg/ecl/dataobjects/TRGECLDigi.h"
#include "mdst/dataobjects/MCParticle.h"

using namespace std;
using namespace Belle2;
using namespace ECL;

REG_MODULE(ECLLOM)


ECLLOMModule::ECLLOMModule()
{
  setDescription("module to emulate Luminosity Online Monitor");
  addParam("ThresholdFE", BhabhaHitThresholdFE, "", 3.0); // Threshold for Forward Endcap
  addParam("ThresholdBE", BhabhaHitThresholdBE, "", 1.0); // Threshold for Backward Endcap
  addParam("ThresholdBkg", BackgroundThreshold, "", 0.5); // Threshold when sector considered as lighted
  addParam("DiscrTime", DiscrTime, "", 1000.0); // Duration of "1" (positive) signal from discriminators, in ns
  addParam("includeInnerFE", includeFEInnerTC, "", false); // Flag to include/exclude Inner part of the Forward Endcap
  addParam("saveSignal", saveSignal, "", false); // Flag to store or not signals' waveforms
  addParam("testFileName", m_lomtestFilename, "", std::string("lomtest.root")); //output file
  m_EvtNum = 0;
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++) {
      CoincidenceCounterMatrix[i][j] = 0;
      SumCoincidenceCounterMatrix[i][j] = 0;
    }
  }
}

ECLLOMModule::~ECLLOMModule()
{

}

void ECLLOMModule::initialize()
{
  StoreArray<MCParticle>   MCParticles;
  StoreArray<TRGECLWaveform>  TrgEclWaveformArray;
  if (!(MCParticles.isRequired() && TrgEclWaveformArray.isRequired())) {
    //Fatal is not neccessary here as the storeArrays should just look
    //empty if not registered but let's make sure everything is present
    B2FATAL("Not all collections found, exiting processing");
  }

  testfile = new TFile(m_lomtestFilename.c_str(), "RECREATE");
  testtree = new TTree("lom_tree", "");

  testtree->Branch("ev", &m_EvtNum);
  testtree->Branch("BE_amp[16]", BE_Amplitude, "BE_amp[16]/D");
  testtree->Branch("FE_amp[16]", FE_Amplitude, "FE_amp[16]/D");
  testtree->Branch("FESum_MaxAmp", &FESum_MaxAmp);
  testtree->Branch("BESum_MaxAmp", &BESum_MaxAmp);
  testtree->Branch("FESum_MaxId", &FESum_MaxId);
  testtree->Branch("BESum_MaxId", &BESum_MaxId);
  testtree->Branch("BE_Pedal[16]", BE_Pedal, "BE_Pedal[16]/D");
  testtree->Branch("FE_Pedal[16]", FE_Pedal, "FE_Pedal[16]/D");

  testtree->Branch("Bhabha", &isBhabha);
  testtree->Branch("BhNum", &BhNum);

  testtree->Branch("mc_en[2]", mcen, "mc_en[2]/D");
  testtree->Branch("mc_th[2]", mcth, "mc_th[2]/D");
  testtree->Branch("mc_ph[2]", mcph, "mc_ph[2]/D");

  testtree->Branch("com_en[2]", com_en, "com_en[2]/D");
  testtree->Branch("com_th[2]", com_th, "com_th[2]/D");
  testtree->Branch("com_ph[2]", com_ph, "com_ph[2]/D");

  if (saveSignal) {
    testtree->Branch("BE_wf[16][64]", BE_Waveform_100ns, "BE_wf[16][64]/D");
    testtree->Branch("FE_wf[16][64]", FE_Waveform_100ns, "FE_wf[16][64]/D");
  }

  //additional histograms. Represent data over the whole dataset:
  h2Coin = new TH2D("Coins", "Coincidence Matrix", 16, 0, 16, 16, 0, 16);
  h2SumCoin = new TH2D("SumCoins", "Sum Coincidence Matrix", 16, 0, 16, 16, 0, 16);
  h2FEAmp = new TH2D("FE_AmpId", "", 16, 0, 16, 100, 0, 8);
  h2BEAmp = new TH2D("BE_AmpId", "", 16, 0, 16, 100, 0, 8);
  h1BEHits = new TH1D("BE_Fired", "", 16, 0, 16);
  h1FEHits = new TH1D("FE_Fired", "", 16, 0, 16);

  NSamples = 631;
}

void ECLLOMModule::beginRun()
{
}

void ECLLOMModule::event()
{
  clear_lom_data();
  get_MCparticles();
  get_waveforms();
  calculate_discr_output();
  calculate_amplitudes();
  // LOM logic
  for (int iSample = 300; iSample < 500; iSample++) { //300-500 window where BhaBha is expected
    isBhabhaPatternFE = calculate_FE_quality(iSample);
    isBhabhaPatternBE = calculate_BE_quality(iSample);
    calculate_coincidence(iSample);
    // generate bhabha signal
    for (int iFESector = 0; iFESector < 16; iFESector++) {
      // check opposite running sum:
      int iBESector = (iFESector + 8) % 16;
      if (SumCoincidenceMatrix[iFESector][iBESector] == 1 && isBhabhaPatternFE && isBhabhaPatternBE) {
        //coinsidence at first tick
        isBhabha = true;
        BhNum++;
      }
    }
  }
  testtree->Fill();
  m_EvtNum++;
}

void ECLLOMModule::endRun()
{
}

void ECLLOMModule::terminate()
{

  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++) {
      std::cout << CoincidenceCounterMatrix[i][j] << " ";
      h2Coin->SetBinContent(i + 1, j + 1, CoincidenceCounterMatrix[i][j]);
      h2SumCoin->SetBinContent(i + 1, j + 1, SumCoincidenceCounterMatrix[i][j]);
    }
    std::cout << std::endl;
  }
  testfile->Write();
  testfile->Close();
}


void ECLLOMModule::get_MCparticles()
{
  StoreArray<MCParticle> MCParticles;
  int nMCParticles = MCParticles.getEntries();
  if (nMCParticles >= 4) {
    for (int ind = 2; ind < 4; ind++) {
      mcen[ind - 2] = MCParticles[ind]->getEnergy();
      mcth[ind - 2] = MCParticles[ind]->getMomentum().Theta();
      mcph[ind - 2] = MCParticles[ind]->getMomentum().Phi();
    }

    TLorentzVector SummP(MCParticles[0]->get4Vector() + MCParticles[1]->get4Vector());
    TVector3 Boost_backV;
    Boost_backV = -SummP.BoostVector();
    TLorentzVector ComP[2];
    ComP[0] = MCParticles[2]->get4Vector();
    ComP[1] = MCParticles[3]->get4Vector();
    ComP[0].Boost(Boost_backV);
    ComP[1].Boost(Boost_backV);
    for (int ind = 0; ind < 2; ind++) {
      com_en[ind] = ComP[ind].E();
      com_th[ind] = ComP[ind].Vect().Theta();
      com_ph[ind] = ComP[ind].Vect().Phi();
    }
  }
}

void ECLLOMModule::get_waveforms()
{
  StoreArray<TRGECLWaveform> TrgEclWaveformArray;
  //int n_trg_digi = TrgEclDigiArray.getEntries();
  int n_trg_wf   = TrgEclWaveformArray.getEntries();
  // calculate signals of endcap sectors for LOM input
  // as sum of corresponding TC signals
  for (int i = 0; i < n_trg_wf; i++) {
    const TRGECLWaveform* TCWaveform = TrgEclWaveformArray[i];
    //int m_tcid = TCWaveform->getTCID();
    int tc_theta_id = TCWaveform->getThetaID(); //FE:1,2,3 BE:16,17  Checked for rel 4 02 08
    int tc_phi_id   = TCWaveform->getPhiID();   // 1 - 32
    double m_wf[64];
    TCWaveform->fillWaveform(m_wf);

    int iSectorIndex = (tc_phi_id - 1) / 2; // from 0 to 15
    if (tc_theta_id == 1 && !includeFEInnerTC) continue;

    for (int iSample = 0; iSample < 64; iSample++) {
      if (tc_theta_id <= 3) {   //Forward Endcap
        FE_Waveform_100ns[iSectorIndex][iSample] += m_wf[iSample];
      } else { // Backward Endcap
        if (tc_theta_id == 16 || tc_theta_id == 17) BE_Waveform_100ns[iSectorIndex][iSample] += m_wf[iSample];
      }
    }
  }
}

void ECLLOMModule::calculate_discr_output()
{
  for (int iSector = 0; iSector < 16; iSector++) { // Calculating pedestals
    for (int iSample = 15; iSample < 36; iSample++) {
      BE_Pedal[iSector] += BE_Waveform_100ns[iSector][iSample] / 20;
      FE_Pedal[iSector] += FE_Waveform_100ns[iSector][iSample] / 20;
    }
  }
  double dAdT = 0; // convert 100 ns signal to 10 ns
  for (int iSector = 0; iSector < 16; iSector++) { // Linear interpolation from 100ns to 10ns
    for (int iSample = 0; iSample < 63; iSample++) {
      // forward
      dAdT = (FE_Waveform_100ns[iSector][iSample + 1] - FE_Waveform_100ns[iSector][iSample]) / 10.0;
      FE_Waveform_100ns[iSector][iSample] -= FE_Pedal[iSector]; //remove pedestals
      for (int j = 0; j < 10; j++) FE_Waveform_10ns[iSector][iSample * 10 + j] = FE_Waveform_100ns[iSector][iSample] + j * dAdT;
      FE_Waveform_10ns[iSector][630] = FE_Waveform_100ns[iSector][63] - FE_Pedal[iSector];
      //backward
      dAdT = (BE_Waveform_100ns[iSector][iSample + 1] - BE_Waveform_100ns[iSector][iSample]) / 10.0;
      BE_Waveform_100ns[iSector][iSample] -= BE_Pedal[iSector];
      for (int j = 0; j < 10; j++) BE_Waveform_10ns[iSector][iSample * 10 + j] = BE_Waveform_100ns[iSector][iSample] + j * dAdT;
      BE_Waveform_10ns[iSector][630] = BE_Waveform_100ns[iSector][63] - BE_Pedal[iSector];
    }
  }
  // calculate running sums for 10ns signal
  int TimeOfDiscr = int(DiscrTime / 10); //discriminator duration in samples
  for (int iSector = 0; iSector < 16; iSector++) {
    for (int iSample = 1; iSample < NSamples; iSample++) {
      int iNextSector = (iSector + 1) % 16;
      BESum_Waveform_10ns[iSector][iSample] = BE_Waveform_10ns[iSector][iSample] + BE_Waveform_10ns[iNextSector][iSample];
      FESum_Waveform_10ns[iSector][iSample] = FE_Waveform_10ns[iSector][iSample] + FE_Waveform_10ns[iNextSector][iSample];

      //filling Discriminators' signals
      if (FESum_Waveform_10ns[iSector][iSample] > BhabhaHitThresholdFE && FESum_Discr[iSector][iSample] == 0) {
        for (int j = iSample; j < iSample + TimeOfDiscr; j++) {
          if (j < NSamples) FESum_Discr[iSector][j] = 1;
        }
      }
      if (BESum_Waveform_10ns[iSector][iSample] > BhabhaHitThresholdBE && BESum_Discr[iSector][iSample] == 0) {
        for (int j = iSample; j < iSample + TimeOfDiscr; j++) {
          if (j < NSamples) BESum_Discr[iSector][j] = 1;
        }
      }
      if (FE_Waveform_10ns[iSector][iSample] > BackgroundThreshold && FEQual_Discr[iSector][iSample] == 0) {
        for (int j = iSample; j < iSample + TimeOfDiscr; j++) {
          if (j < NSamples) FEQual_Discr[iSector][j] = 1;
        }
      }
      if (BE_Waveform_10ns[iSector][iSample] > BackgroundThreshold && BEQual_Discr[iSector][iSample] == 0) {
        for (int j = iSample; j < iSample + TimeOfDiscr; j++) {
          if (j < NSamples) BEQual_Discr[iSector][j] = 1;
        }
      }
    }
  }
}


bool ECLLOMModule::calculate_BE_quality(int iSample)
{
  int nhit = 0;
  int First = 0;
  // calculate quality signal for backward endcap
  for (int iBESector = 0; iBESector < 16; iBESector++) {
    if (BEQual_Discr[iBESector][iSample]) {
      nhit++;
      if (nhit == 1) First = iBESector;
      if (nhit == 2 && !((iBESector + 1) % 16 == First || (First + 1) % 16 == iBESector)) return (false);
      if (nhit >= 3) return (false);
    }
  }
  return (true);
}

bool ECLLOMModule::calculate_FE_quality(int iSample)
{
  int nhit = 0;
  int First = 0;
  for (int iFESector = 0; iFESector < 16; iFESector++) {
    if (FEQual_Discr[iFESector][iSample]) {
      nhit++;
      if (nhit == 1) First = iFESector;
      if (nhit == 2 && !((iFESector + 1) % 16 == First || (First + 1) % 16 == iFESector)) return (false);
      if (nhit >= 3) return (false);
    }
  }
  return (true);
}

void ECLLOMModule::calculate_coincidence(int iSample)
{
  for (int iFESector = 0; iFESector < 16; iFESector++) {
    for (int iBESector = 0; iBESector < 16; iBESector++) {

      if (FE_Waveform_10ns[iFESector][iSample] > BhabhaHitThresholdFE && BE_Waveform_10ns[iBESector][iSample] > BhabhaHitThresholdBE) {
        if (CoincidenceMatrix[iFESector][iBESector] == 0) CoincidenceCounterMatrix[iFESector][iBESector]++;
        CoincidenceMatrix[iFESector][iBESector]++;
      } else {
        CoincidenceMatrix[iFESector][iBESector] = 0;
      }

      if (FESum_Discr[iFESector][iSample] && BESum_Discr[iBESector][iSample]) {
        if (SumCoincidenceMatrix[iFESector][iBESector] == 0) SumCoincidenceCounterMatrix[iFESector][iBESector]++;
        SumCoincidenceMatrix[iFESector][iBESector]++;
      }
      SumCoincidenceMatrix[iFESector][iBESector] = 0;
    }
  }
}


void ECLLOMModule::clear_lom_data()
{
  for (int isector = 0; isector < 16; isector++) {
    for (int iSample = 0; iSample < 64; iSample++) {
      BE_Waveform_100ns[isector][iSample] = 0;
      FE_Waveform_100ns[isector][iSample] = 0;
    }
    for (int iSample = 0; iSample < NSamples; iSample++) {
      BE_Waveform_10ns[isector][iSample] = 0;
      FE_Waveform_10ns[isector][iSample] = 0;
      BESum_Waveform_10ns[isector][iSample] = 0;
      FESum_Waveform_10ns[isector][iSample] = 0;
      FESum_Discr[isector][iSample] = 0;
      BESum_Discr[isector][iSample] = 0;
      FEQual_Discr[isector][iSample] = 0;
      BEQual_Discr[isector][iSample] = 0;
    }
    BE_Pedal[isector] = 0;
    FE_Pedal[isector] = 0;
    BE_Amplitude[isector] = 0;
    FE_Amplitude[isector] = 0;
    BESum_Amplitude[isector] = 0;
    FESum_Amplitude[isector] = 0;
    for (int jsector = 0; jsector < 16; jsector++) {
      CoincidenceMatrix[isector][jsector] = 0;
      SumCoincidenceMatrix[isector][jsector] = 0;
    }
  }
  isBhabha = 0;
  BhNum = 0;
  FESum_MaxAmp = 0;
  BESum_MaxAmp = 0;
  FESum_MaxId = -1;
  BESum_MaxId = -1;
}

void ECLLOMModule::calculate_amplitudes()
{
  for (int iSample = 0; iSample < NSamples; iSample++) {
    for (int isector = 0; isector < 16; isector++) {
      if (FE_Waveform_10ns[isector][iSample] > FE_Amplitude[isector]) FE_Amplitude[isector] = FE_Waveform_10ns[isector][iSample];
      if (FESum_Waveform_10ns[isector][iSample] > FESum_Amplitude[isector]) FESum_Amplitude[isector] =
          FESum_Waveform_10ns[isector][iSample];
      if (BE_Waveform_10ns[isector][iSample] > BE_Amplitude[isector]) BE_Amplitude[isector] = BE_Waveform_10ns[isector][iSample];
      if (BESum_Waveform_10ns[isector][iSample] > BESum_Amplitude[isector]) BESum_Amplitude[isector] =
          BESum_Waveform_10ns[isector][iSample];
    }
  }
  for (int i = 0; i < 16; i++) {
    if (FESum_Amplitude[i] > FESum_MaxAmp) {
      FESum_MaxAmp  = FESum_Amplitude[i];
      FESum_MaxId = i;
    }
    if (BESum_Amplitude[i] > BESum_MaxAmp) {
      BESum_MaxAmp = BESum_Amplitude[i];
      BESum_MaxId  = i;
    }
    if (FE_Amplitude[i] > 0.5) h2FEAmp->Fill(i, FE_Amplitude[i]);
    if (BE_Amplitude[i] > 0.5) h2BEAmp->Fill(i, BE_Amplitude[i]);

    if (FE_Amplitude[i] > BhabhaHitThresholdFE) h1FEHits->Fill(i);
    if (BE_Amplitude[i] > BhabhaHitThresholdBE) h1BEHits->Fill(i);
  }
}