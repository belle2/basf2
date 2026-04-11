/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/modules/PhysicsObjectsMiraBelle/PhysicsObjectsMiraBelleTauModule.h>
#include <analysis/dataobjects/ParticleList.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <TDirectory.h>
#include <TMath.h>

#include <cmath>
#include <map>

using namespace Belle2;

REG_MODULE(PhysicsObjectsMiraBelleTau);

PhysicsObjectsMiraBelleTauModule::PhysicsObjectsMiraBelleTauModule() : HistoModule()
{
  setDescription("Monitor Physics Objects Quality");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("TriggerIdentifierTau2trk", m_triggerIdentifierTau2trk,
           "Trigger identifier string used to select 1x1 tau mode for the histograms",
           std::string("software_trigger_cut&skim&accept_tau_2trk"));
  addParam("TriggerIdentifierTauNtrk", m_triggerIdentifierTauNtrk,
           "Trigger identifier string used to select 1x3 tau mode for the histograms",
           std::string("software_trigger_cut&skim&accept_tau_Ntrk"));
  addParam("tautau1x1PListName", m_tautau1x1PListName, "Name of the Z0 list reconstructed from 1x1 topology of tau decays",
           std::string("Z0:physMiraBelleTau1x1"));
  addParam("tautau1x3PListName", m_tautau1x3PListName, "Name of the Z0 list reconstructed from 1x3 topology of tau decays",
           std::string("Z0:physMiraBelleTau1x3"));

}

void PhysicsObjectsMiraBelleTauModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("PhysicsObjectsMiraBelleTau");
  oldDir->cd("PhysicsObjectsMiraBelleTau");

  // Create histograms for efficiency of ECL bits
  m_h_L1ECL1x1 = new TH1F("hist_L1ECL1x1", "hist_L1ECL1x1", 30, 0, 30);
  m_h_L1ECL1x1->GetXaxis()->SetBinLabel(1, "Reference(fff||ffo)");
  m_h_L1ECL1x3 = new TH1F("hist_L1ECL1x3", "hist_L1ECL1x3", 30, 0, 30);
  m_h_L1ECL1x3->GetXaxis()->SetBinLabel(1, "Reference(fff||ffo)");

  int bin = 2;

  // ECL bits
  for (const auto& bit : m_ECLTrgBit) {
    m_h_L1ECL1x1->GetXaxis()->SetBinLabel(bin, bit.c_str());
    m_h_L1ECL1x3->GetXaxis()->SetBinLabel(bin, bit.c_str());
    ++bin;
  }

  // Create histograms for efficiency of CDC bits
  m_h_L1CDC1x1 = new TH1F("hist_L1CDC1x1", "hist_L1CDC1x1", 30, 0, 30);
  m_h_L1CDC1x1->GetXaxis()->SetBinLabel(1, "Reference(hie||c4||eclmumu)");
  m_h_L1CDC1x3 = new TH1F("hist_L1CDC1x3", "hist_L1CDC1x3", 30, 0, 30);
  m_h_L1CDC1x3->GetXaxis()->SetBinLabel(1, "Reference(hie||c4||eclmumu)");

  bin = 2;

  // CDC bits
  for (const auto& bit : m_CDCTrgBit) {
    m_h_L1CDC1x1->GetXaxis()->SetBinLabel(bin, bit.c_str());
    m_h_L1CDC1x3->GetXaxis()->SetBinLabel(bin, bit.c_str());
    ++bin;
  }

  // Create histograms for efficiency of CDC-KLM bits
  m_h_L1CDCKLM1x1 = new TH1F("hist_L1CDCKLM1x1", "hist_L1CDCKLM1x1", 30, 0, 30);
  m_h_L1CDCKLM1x1->GetXaxis()->SetBinLabel(1, "Reference(hie||c4||eclmumu)");
  m_h_L1CDCKLM1x3 = new TH1F("hist_L1CDCKLM1x3", "hist_L1CDCKLM1x3", 30, 0, 30);
  m_h_L1CDCKLM1x3->GetXaxis()->SetBinLabel(1, "Reference(hie||c4||eclmumu)");

  bin = 2;

  // CDC-KLM bits
  for (const auto& bit : m_CDCKLMTrgBit) {
    m_h_L1CDCKLM1x1->GetXaxis()->SetBinLabel(bin, bit.c_str());
    m_h_L1CDCKLM1x3->GetXaxis()->SetBinLabel(bin, bit.c_str());
    ++bin;
  }


  oldDir->cd();
}


void PhysicsObjectsMiraBelleTauModule::initialize()
{
  REG_HISTOGRAM

  StoreObjPtr<SoftwareTriggerResult> result;
  result.isOptional();
}

void PhysicsObjectsMiraBelleTauModule::beginRun()
{
  m_h_L1ECL1x1->Reset();
  m_h_L1ECL1x3->Reset();
  m_h_L1CDC1x1->Reset();
  m_h_L1CDC1x3->Reset();
  m_h_L1CDCKLM1x1->Reset();
  m_h_L1CDCKLM1x3->Reset();

}

void PhysicsObjectsMiraBelleTauModule::event()
{

  StoreObjPtr<SoftwareTriggerResult> result;
  if (!result.isValid()) {
    B2WARNING("SoftwareTriggerResult object not available but needed to select events for the histograms.");
    return;
  }

  const std::map<std::string, int>& results = result->getResults();

  StoreObjPtr<TRGSummary> m_trgSummary;
  if (m_trgSummary.isValid()) {
    try {
      m_ECL_ref = (m_trgSummary->testFtdl("hie") || m_trgSummary->testFtdl("c4") || m_trgSummary->testFtdl("eclmumu"));
    } catch (const std::exception&) {
      m_ECL_ref = false;
    }
    try {
      m_CDC_ref = (m_trgSummary->testFtdl("fff") || m_trgSummary->testFtdl("ffo"));
    } catch (const std::exception&) {
      m_CDC_ref = false;
    }
  }


  //--- Monitor efficiency of L1 bits with 1x1 tau pairs ---//
  if (results.find(m_triggerIdentifierTau2trk) == results.end()) {
    B2WARNING("PhysicsObjectsDQM: Can't find trigger identifier: " << m_triggerIdentifierTau2trk);
  } else {

    //Get 1x1 tau pairs for L1 efficiency calculation
    StoreObjPtr<ParticleList> Z0Particles1x1(m_tautau1x1PListName);
    if (Z0Particles1x1.isValid()) {
      for (unsigned int i = 0; i < Z0Particles1x1->getListSize(); i++) {
        // Fill ECL histogram
        // Reference: CDC bits (fff||ffo), Target: ECL bits
        // Bin x=1: N(reference), Bin x>1: N(reference&&target) [Efficiency of ECL bit in x>1 bin = N(x>1)/N(x=1)]
        if (m_CDC_ref)
          m_h_L1ECL1x1->Fill(0);
        int bin = 1;
        // Loop over ECL bits
        for (const auto& bit : m_ECLTrgBit) {
          if (m_trgSummary.isValid()) {
            bool m_ECL_target = false;
            try {
              m_ECL_target = m_trgSummary->testFtdl(bit.c_str());
            } catch (const std::exception&) {
              m_ECL_target = false;
            }

            if (m_CDC_ref && m_ECL_target)
              m_h_L1ECL1x1->Fill(bin);
          }
          bin++;
        }

        // Fill CDC histogram
        // Reference: ECL bits (hie||c4||eclmumu), Target: CDC bits
        // Bin x=1: N(reference), Bin x>1: N(reference&&target) [Efficiency of CDC bit in x>1 bin = N(x>1)/N(x=1)]
        if (m_ECL_ref)
          m_h_L1CDC1x1->Fill(0);
        bin = 1;
        // Loop over CDC bits
        for (const auto& bit : m_CDCTrgBit) {
          if (m_trgSummary.isValid()) {
            bool m_CDC_target = false;
            try {
              m_CDC_target = m_trgSummary->testFtdl(bit.c_str());
            } catch (const std::exception&) {
              m_CDC_target = false;
            }

            if (m_ECL_ref && m_CDC_target)
              m_h_L1CDC1x1->Fill(bin);
          }
          bin++;
        }

        // Fill CDC-KLM histogram
        // Reference: ECL bits (hie||c4||eclmumu), Target: CDC-KLM bits
        // Bin x=1: N(reference), Bin x>1: N(reference&&target) [Efficiency of CDC-KLM bit in x>1 bin = N(x>1)/N(x=1)]
        if (m_ECL_ref)
          m_h_L1CDCKLM1x1->Fill(0);
        bin = 1;
        // Loop over CDC bits
        for (const auto& bit : m_CDCKLMTrgBit) {
          if (m_trgSummary.isValid()) {
            bool m_CDCKLM_target = false;
            try {
              m_CDCKLM_target = m_trgSummary->testFtdl(bit.c_str());
            } catch (const std::exception&) {
              m_CDCKLM_target = false;
            }

            if (m_ECL_ref && m_CDCKLM_target)
              m_h_L1CDCKLM1x1->Fill(bin);
          }
          bin++;
        }

      }

    }

  }

  //--- Monitor efficiency of L1 bits with 1x3 tau pairs ---//
  if (results.find(m_triggerIdentifierTauNtrk) == results.end()) {
    B2WARNING("PhysicsObjectsDQM: Can't find trigger identifier: " << m_triggerIdentifierTauNtrk);
  } else {

    //Get 1x3 tau pairs for L1 efficiency calculation
    StoreObjPtr<ParticleList> Z0Particles1x3(m_tautau1x3PListName);
    if (Z0Particles1x3.isValid()) {
      for (unsigned int i = 0; i < Z0Particles1x3->getListSize(); i++) {
        // Fill ECL histogram
        // Reference: CDC bits (fff||ffo), Target: ECL bits
        // Bin x=1: N(reference), Bin x>1: N(reference&&target) [Efficiency of ECL bit in x>1 bin = N(x>1)/N(x=1)]
        if (m_CDC_ref)
          m_h_L1ECL1x3->Fill(0);
        int bin = 1;
        // Loop over ECL bits
        for (const auto& bit : m_ECLTrgBit) {
          if (m_trgSummary.isValid()) {
            bool m_ECL_target = false;
            try {
              m_ECL_target = m_trgSummary->testFtdl(bit.c_str());
            } catch (const std::exception&) {
              m_ECL_target = false;
            }

            if (m_CDC_ref && m_ECL_target)
              m_h_L1ECL1x3->Fill(bin);
          }
          bin++;
        }
        // Fill CDC histogram
        // Reference: ECL bits (hie||c4||eclmumu), Target: CDC bits
        // Bin x=1: N(reference), Bin x>1: N(reference&&target) [Efficiency of CDC bit in x>1 bin = N(x>1)/N(x=1)]
        if (m_ECL_ref)
          m_h_L1CDC1x3->Fill(0);
        bin = 1;
        // Loop over CDC bits
        for (const auto& bit : m_CDCTrgBit) {
          if (m_trgSummary.isValid()) {
            bool m_CDC_target = false;
            try {
              m_CDC_target = m_trgSummary->testFtdl(bit.c_str());
            } catch (const std::exception&) {
              m_CDC_target = false;
            }

            if (m_ECL_ref && m_CDC_target)
              m_h_L1CDC1x3->Fill(bin);
          }
          bin++;
        }
        // Fill CDC-KLM histogram
        // Reference: ECL bits (hie||c4||eclmumu), Target: CDC-KLM bits
        // Bin x=1: N(reference), Bin x>1: N(reference&&target) [Efficiency of CDC-KLM bit in x>1 bin = N(x>1)/N(x=1)]
        if (m_ECL_ref)
          m_h_L1CDCKLM1x3->Fill(0);
        bin = 1;
        // Loop over CDC bits
        for (const auto& bit : m_CDCKLMTrgBit) {
          if (m_trgSummary.isValid()) {
            bool m_CDCKLM_target = false;
            try {
              m_CDCKLM_target = m_trgSummary->testFtdl(bit.c_str());
            } catch (const std::exception&) {
              m_CDCKLM_target = false;
            }

            if (m_ECL_ref && m_CDCKLM_target)
              m_h_L1CDCKLM1x3->Fill(bin);
          }
          bin++;
        }

      }

    }

  }



}

void PhysicsObjectsMiraBelleTauModule::endRun()
{
}

void PhysicsObjectsMiraBelleTauModule::terminate()
{
}

