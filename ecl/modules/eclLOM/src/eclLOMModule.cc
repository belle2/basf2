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

#include <Math/Boost.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

REG_MODULE(ECLLOM);


ECLLOMModule::ECLLOMModule()
{
  setDescription("module to emulate Luminosity Online Monitor");
  addParam("thresholdFE", m_thresholdFE, "Threshold for Forward Endcap [GeV]", 3.0);
  addParam("thresholdBE", m_thresholdBE, "Threshold for Backward Endcap [GeV]", 1.0);
  addParam("thresholdBkg", m_thresholdBkg, "Threshold when sector considered as lighted [GeV]", 0.5);
  addParam("discrTime", m_discrTime, "Duration of '1' (positive) signal from discriminators [ns]", 1000.0);
  addParam("includeInnerFE", m_includeInnerFE, "Flag to include/exclude Inner part of the Forward Endcap", false);
  addParam("saveSignal", m_saveSignal, "Flag to store or not signals' waveforms", false);
  addParam("testFileName", m_testFileName, "output file", std::string("lomtest.root"));

  m_evtNum = 0;
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++) {
      m_CoincidenceCounterMatrix[i][j] = 0;
      m_SumCoincidenceCounterMatrix[i][j] = 0;
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

  m_testfile = new TFile(m_testFileName.c_str(), "RECREATE");
  m_testtree = new TTree("lom_tree", "");

  m_testtree->Branch("ev", &m_evtNum);
  m_testtree->Branch("BE_amp[16]", m_BE_Amplitude, "BE_amp[16]/D");
  m_testtree->Branch("FE_amp[16]", m_FE_Amplitude, "FE_amp[16]/D");
  m_testtree->Branch("FESum_MaxAmp", &m_FESum_MaxAmp);
  m_testtree->Branch("BESum_MaxAmp", &m_BESum_MaxAmp);
  m_testtree->Branch("FESum_MaxId", &m_FESum_MaxId);
  m_testtree->Branch("BESum_MaxId", &m_BESum_MaxId);
  m_testtree->Branch("BE_Pedal[16]", m_BE_Pedal, "BE_Pedal[16]/D");
  m_testtree->Branch("FE_Pedal[16]", m_FE_Pedal, "FE_Pedal[16]/D");

  m_testtree->Branch("Bhabha", &m_isBhabha);
  m_testtree->Branch("BhNum", &m_BhNum);

  m_testtree->Branch("mc_en[2]", m_mcen, "mc_en[2]/D");
  m_testtree->Branch("mc_th[2]", m_mcth, "mc_th[2]/D");
  m_testtree->Branch("mc_ph[2]", m_mcph, "mc_ph[2]/D");

  m_testtree->Branch("com_en[2]", m_com_en, "com_en[2]/D");
  m_testtree->Branch("com_th[2]", m_com_th, "com_th[2]/D");
  m_testtree->Branch("com_ph[2]", m_com_ph, "com_ph[2]/D");

  if (m_saveSignal) {
    m_testtree->Branch("BE_wf[16][64]", m_BE_Waveform_100ns, "BE_wf[16][64]/D");
    m_testtree->Branch("FE_wf[16][64]", m_FE_Waveform_100ns, "FE_wf[16][64]/D");
  }

  //additional histograms. Represent data over the whole dataset:
  m_h2Coin = new TH2D("Coins", "Coincidence Matrix", 16, 0, 16, 16, 0, 16);
  m_h2SumCoin = new TH2D("SumCoins", "Sum Coincidence Matrix", 16, 0, 16, 16, 0, 16);
  m_h2FEAmp = new TH2D("FE_AmpId", "", 16, 0, 16, 100, 0, 8);
  m_h2BEAmp = new TH2D("BE_AmpId", "", 16, 0, 16, 100, 0, 8);
  m_h1BEHits = new TH1D("BE_Fired", "", 16, 0, 16);
  m_h1FEHits = new TH1D("FE_Fired", "", 16, 0, 16);

  m_NSamples = 631;
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
    m_isBhabhaPatternFE = calculate_FE_quality(iSample);
    m_isBhabhaPatternBE = calculate_BE_quality(iSample);
    calculate_coincidence(iSample);
    // generate bhabha signal
    for (int iFESector = 0; iFESector < 16; iFESector++) {
      // check opposite running sum:
      int iBESector = (iFESector + 8) % 16;
      if (m_SumCoincidenceMatrix[iFESector][iBESector] == 1 && m_isBhabhaPatternFE && m_isBhabhaPatternBE) {
        //coinsidence at first tick
        m_isBhabha = true;
        m_BhNum++;
      }
    }
  }
  m_testtree->Fill();
  m_evtNum++;
}

void ECLLOMModule::endRun()
{
}

void ECLLOMModule::terminate()
{

  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++) {
      std::cout << m_CoincidenceCounterMatrix[i][j] << " ";
      m_h2Coin->SetBinContent(i + 1, j + 1, m_CoincidenceCounterMatrix[i][j]);
      m_h2SumCoin->SetBinContent(i + 1, j + 1, m_SumCoincidenceCounterMatrix[i][j]);
    }
    std::cout << std::endl;
  }
  m_testfile->Write();
  m_testfile->Close();
}


void ECLLOMModule::get_MCparticles()
{
  StoreArray<MCParticle> MCParticles;
  int nMCParticles = MCParticles.getEntries();
  if (nMCParticles >= 4) {
    for (int ind = 2; ind < 4; ind++) {
      m_mcen[ind - 2] = MCParticles[ind]->getEnergy();
      m_mcth[ind - 2] = MCParticles[ind]->getMomentum().Theta();
      m_mcph[ind - 2] = MCParticles[ind]->getMomentum().Phi();
    }

    ROOT::Math::PxPyPzEVector SummP(MCParticles[0]->get4Vector() + MCParticles[1]->get4Vector());
    ROOT::Math::XYZVector Boost_backV = SummP.BoostToCM();
    ROOT::Math::PxPyPzEVector ComP[2];
    ComP[0] = MCParticles[2]->get4Vector();
    ComP[1] = MCParticles[3]->get4Vector();
    ComP[0] = ROOT::Math::Boost(Boost_backV) * ComP[0];
    ComP[1] = ROOT::Math::Boost(Boost_backV) * ComP[1];
    for (int ind = 0; ind < 2; ind++) {
      m_com_en[ind] = ComP[ind].E();
      m_com_th[ind] = ComP[ind].Theta();
      m_com_ph[ind] = ComP[ind].Phi();
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
    if (tc_theta_id == 1 && !m_includeInnerFE) continue;

    for (int iSample = 0; iSample < 64; iSample++) {
      if (tc_theta_id <= 3) {   //Forward Endcap
        m_FE_Waveform_100ns[iSectorIndex][iSample] += m_wf[iSample];
      } else { // Backward Endcap
        if (tc_theta_id == 16 || tc_theta_id == 17) m_BE_Waveform_100ns[iSectorIndex][iSample] += m_wf[iSample];
      }
    }
  }
}

void ECLLOMModule::calculate_discr_output()
{
  for (int iSector = 0; iSector < 16; iSector++) { // Calculating pedestals
    for (int iSample = 15; iSample < 36; iSample++) {
      m_BE_Pedal[iSector] += m_BE_Waveform_100ns[iSector][iSample] / 20;
      m_FE_Pedal[iSector] += m_FE_Waveform_100ns[iSector][iSample] / 20;
    }
  }
  double dAdT; // convert 100 ns signal to 10 ns
  for (int iSector = 0; iSector < 16; iSector++) { // Linear interpolation from 100ns to 10ns
    for (int iSample = 0; iSample < 63; iSample++) {
      // forward
      dAdT = (m_FE_Waveform_100ns[iSector][iSample + 1] - m_FE_Waveform_100ns[iSector][iSample]) / 10.0;
      m_FE_Waveform_100ns[iSector][iSample] -= m_FE_Pedal[iSector]; //remove pedestals
      for (int j = 0; j < 10; j++) m_FE_Waveform_10ns[iSector][iSample * 10 + j] = m_FE_Waveform_100ns[iSector][iSample] + j * dAdT;
      m_FE_Waveform_10ns[iSector][630] = m_FE_Waveform_100ns[iSector][63] - m_FE_Pedal[iSector];
      //backward
      dAdT = (m_BE_Waveform_100ns[iSector][iSample + 1] - m_BE_Waveform_100ns[iSector][iSample]) / 10.0;
      m_BE_Waveform_100ns[iSector][iSample] -= m_BE_Pedal[iSector];
      for (int j = 0; j < 10; j++) m_BE_Waveform_10ns[iSector][iSample * 10 + j] = m_BE_Waveform_100ns[iSector][iSample] + j * dAdT;
      m_BE_Waveform_10ns[iSector][630] = m_BE_Waveform_100ns[iSector][63] - m_BE_Pedal[iSector];
    }
  }
  // calculate running sums for 10ns signal
  int TimeOfDiscr = int(m_discrTime / 10); //discriminator duration in samples
  for (int iSector = 0; iSector < 16; iSector++) {
    for (int iSample = 1; iSample < m_NSamples; iSample++) {
      int iNextSector = (iSector + 1) % 16;
      m_BESum_Waveform_10ns[iSector][iSample] = m_BE_Waveform_10ns[iSector][iSample] + m_BE_Waveform_10ns[iNextSector][iSample];
      m_FESum_Waveform_10ns[iSector][iSample] = m_FE_Waveform_10ns[iSector][iSample] + m_FE_Waveform_10ns[iNextSector][iSample];

      //filling Discriminators' signals
      if (m_FESum_Waveform_10ns[iSector][iSample] > m_thresholdFE && m_FESum_Discr[iSector][iSample] == 0) {
        for (int j = iSample; j < iSample + TimeOfDiscr; j++) {
          if (j < m_NSamples) m_FESum_Discr[iSector][j] = 1;
        }
      }
      if (m_BESum_Waveform_10ns[iSector][iSample] > m_thresholdBE && m_BESum_Discr[iSector][iSample] == 0) {
        for (int j = iSample; j < iSample + TimeOfDiscr; j++) {
          if (j < m_NSamples) m_BESum_Discr[iSector][j] = 1;
        }
      }
      if (m_FE_Waveform_10ns[iSector][iSample] > m_thresholdBkg && m_FEQual_Discr[iSector][iSample] == 0) {
        for (int j = iSample; j < iSample + TimeOfDiscr; j++) {
          if (j < m_NSamples) m_FEQual_Discr[iSector][j] = 1;
        }
      }
      if (m_BE_Waveform_10ns[iSector][iSample] > m_thresholdBkg && m_BEQual_Discr[iSector][iSample] == 0) {
        for (int j = iSample; j < iSample + TimeOfDiscr; j++) {
          if (j < m_NSamples) m_BEQual_Discr[iSector][j] = 1;
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
    if (m_BEQual_Discr[iBESector][iSample]) {
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
    if (m_FEQual_Discr[iFESector][iSample]) {
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

      if (m_FE_Waveform_10ns[iFESector][iSample] > m_thresholdFE && m_BE_Waveform_10ns[iBESector][iSample] > m_thresholdBE) {
        if (m_CoincidenceMatrix[iFESector][iBESector] == 0) m_CoincidenceCounterMatrix[iFESector][iBESector]++;
        m_CoincidenceMatrix[iFESector][iBESector]++;
      } else {
        m_CoincidenceMatrix[iFESector][iBESector] = 0;
      }

      if (m_FESum_Discr[iFESector][iSample] && m_BESum_Discr[iBESector][iSample]) {
        if (m_SumCoincidenceMatrix[iFESector][iBESector] == 0) m_SumCoincidenceCounterMatrix[iFESector][iBESector]++;
        m_SumCoincidenceMatrix[iFESector][iBESector]++;
      }
      m_SumCoincidenceMatrix[iFESector][iBESector] = 0;
    }
  }
}


void ECLLOMModule::clear_lom_data()
{
  for (int isector = 0; isector < 16; isector++) {
    for (int iSample = 0; iSample < 64; iSample++) {
      m_BE_Waveform_100ns[isector][iSample] = 0;
      m_FE_Waveform_100ns[isector][iSample] = 0;
    }
    for (int iSample = 0; iSample < m_NSamples; iSample++) {
      m_BE_Waveform_10ns[isector][iSample] = 0;
      m_FE_Waveform_10ns[isector][iSample] = 0;
      m_BESum_Waveform_10ns[isector][iSample] = 0;
      m_FESum_Waveform_10ns[isector][iSample] = 0;
      m_FESum_Discr[isector][iSample] = 0;
      m_BESum_Discr[isector][iSample] = 0;
      m_FEQual_Discr[isector][iSample] = 0;
      m_BEQual_Discr[isector][iSample] = 0;
    }
    m_BE_Pedal[isector] = 0;
    m_FE_Pedal[isector] = 0;
    m_BE_Amplitude[isector] = 0;
    m_FE_Amplitude[isector] = 0;
    m_BESum_Amplitude[isector] = 0;
    m_FESum_Amplitude[isector] = 0;
    for (int jsector = 0; jsector < 16; jsector++) {
      m_CoincidenceMatrix[isector][jsector] = 0;
      m_SumCoincidenceMatrix[isector][jsector] = 0;
    }
  }
  m_isBhabha = 0;
  m_BhNum = 0;
  m_FESum_MaxAmp = 0;
  m_BESum_MaxAmp = 0;
  m_FESum_MaxId = -1;
  m_BESum_MaxId = -1;
}

void ECLLOMModule::calculate_amplitudes()
{
  for (int iSample = 0; iSample < m_NSamples; iSample++) {
    for (int isector = 0; isector < 16; isector++) {
      if (m_FE_Waveform_10ns[isector][iSample] > m_FE_Amplitude[isector]) m_FE_Amplitude[isector] = m_FE_Waveform_10ns[isector][iSample];
      if (m_FESum_Waveform_10ns[isector][iSample] > m_FESum_Amplitude[isector]) m_FESum_Amplitude[isector] =
          m_FESum_Waveform_10ns[isector][iSample];
      if (m_BE_Waveform_10ns[isector][iSample] > m_BE_Amplitude[isector]) m_BE_Amplitude[isector] = m_BE_Waveform_10ns[isector][iSample];
      if (m_BESum_Waveform_10ns[isector][iSample] > m_BESum_Amplitude[isector]) m_BESum_Amplitude[isector] =
          m_BESum_Waveform_10ns[isector][iSample];
    }
  }
  for (int i = 0; i < 16; i++) {
    if (m_FESum_Amplitude[i] > m_FESum_MaxAmp) {
      m_FESum_MaxAmp  = m_FESum_Amplitude[i];
      m_FESum_MaxId = i;
    }
    if (m_BESum_Amplitude[i] > m_BESum_MaxAmp) {
      m_BESum_MaxAmp = m_BESum_Amplitude[i];
      m_BESum_MaxId  = i;
    }
    if (m_FE_Amplitude[i] > 0.5) m_h2FEAmp->Fill(i, m_FE_Amplitude[i]);
    if (m_BE_Amplitude[i] > 0.5) m_h2BEAmp->Fill(i, m_BE_Amplitude[i]);

    if (m_FE_Amplitude[i] > m_thresholdFE) m_h1FEHits->Fill(i);
    if (m_BE_Amplitude[i] > m_thresholdBE) m_h1BEHits->Fill(i);
  }
}
