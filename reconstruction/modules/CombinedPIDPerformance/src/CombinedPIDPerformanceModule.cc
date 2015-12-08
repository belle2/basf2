/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/CombinedPIDPerformance/CombinedPIDPerformanceModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationVector.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <root/TTree.h>
#include <root/TAxis.h>
#include <root/TObject.h>

#include <boost/foreach.hpp>

#include <typeinfo>
#include <cxxabi.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CombinedPIDPerformance)

CombinedPIDPerformanceModule::CombinedPIDPerformanceModule() : Module() , m_rootFilePtr(NULL)
{
  setDescription("This module evaluates the combined PID performance");

  addParam("outputFileName", m_rootFileName, "Name of output root file",
           std::string("CombinedPIDPerformance_output.root"));
  addParam("numberOfBins", m_nbins, "Number of bins in TEfficiency", int(100));
  addParam("momentumLower", m_p_lower, "Lower limit of momentum range", double(0.0));
  addParam("momentumUpper", m_p_upper, "Upper limit of momentum range", double(5.0));
}

CombinedPIDPerformanceModule::~CombinedPIDPerformanceModule() { }

void CombinedPIDPerformanceModule::initialize()
{

  B2INFO("Making PID Performance plots...");

  // MCParticles, Tracks, PIDLikelihoods needed for this module
  StoreArray<MCParticle>::required();
  StoreArray<Track>::required();
  StoreArray<TrackFitResult>::required();
  StoreArray<PIDLikelihood>::required();

  // create list of histograms to be saved in the rootfile
  m_histoList = new TList;

  // create the ROOT File
  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  // determine which detectors to use
  // cannot add more than one at a time...
  chargedSet += Const::SVD;
  chargedSet += Const::CDC;
  chargedSet += Const::TOP;
  chargedSet += Const::ARICH;
  chargedSet += Const::KLM;
  electronSet += Const::SVD;
  electronSet += Const::CDC;
  electronSet += Const::ECL;
  muonSet += Const::KLM;

  // now create the histograms
  //  for( const auto& det : chargedSet ){
  for (unsigned int i = 0; i < chargedSet.size() + 3; ++i) {
    m_piK_Efficiencies.push_back(createEfficiency(TString::Format("epik_%d", i), "#pi efficiency;p  [GeV/c];Efficiency", m_nbins,
                                                  m_p_lower, m_p_upper,
                                                  m_histoList));
    m_Kpi_Efficiencies.push_back(createEfficiency(TString::Format("ekpi_%d", i), "K efficiency;p  [GeV/c];Efficiency", m_nbins,
                                                  m_p_lower, m_p_upper,
                                                  m_histoList));
    m_ppi_Efficiencies.push_back(createEfficiency(TString::Format("eppi_%d", i), "p efficiency;p  [GeV/c];Efficiency", m_nbins,
                                                  m_p_lower, m_p_upper,
                                                  m_histoList));
    m_pK_Efficiencies.push_back(createEfficiency(TString::Format("epk_%d", i), "p efficiency;p  [GeV/c];Efficiency", m_nbins, m_p_lower,
                                                 m_p_upper,
                                                 m_histoList));
    m_dpi_Efficiencies.push_back(createEfficiency(TString::Format("edpi_%d", i), "d efficiency;p  [GeV/c];Efficiency", m_nbins,
                                                  m_p_lower, m_p_upper,
                                                  m_histoList));

    m_piK_FakeRates.push_back(createEfficiency(TString::Format("fpik_%d", i), "#pi fake rate;p  [GeV/c];Fake Rate", m_nbins, m_p_lower,
                                               m_p_upper,
                                               m_histoList));
    m_Kpi_FakeRates.push_back(createEfficiency(TString::Format("fkpi_%d", i), "K fake rate;p  [GeV/c];Fake Rate", m_nbins, m_p_lower,
                                               m_p_upper,
                                               m_histoList));
    m_ppi_FakeRates.push_back(createEfficiency(TString::Format("fppi_%d", i), "p fake rate;p  [GeV/c];Fake Rate", m_nbins, m_p_lower,
                                               m_p_upper,
                                               m_histoList));
    m_pK_FakeRates.push_back(createEfficiency(TString::Format("fpk_%d", i), "p fake rate;p  [GeV/c];Fake Rate", m_nbins, m_p_lower,
                                              m_p_upper,
                                              m_histoList));
    m_dpi_FakeRates.push_back(createEfficiency(TString::Format("fdpi_%d", i), "d fake rate;p  [GeV/c];Fake Rate", m_nbins, m_p_lower,
                                               m_p_upper,
                                               m_histoList));
  }

  //  for( const auto& det : electronSet ){
  for (unsigned int i = 0; i < electronSet.size() + 2; ++i) {
    m_epi_Efficiencies.push_back(createEfficiency(TString::Format("eepi_%d", i), "e efficiency;p  [GeV/c];Efficiency", m_nbins,
                                                  m_p_lower, m_p_upper,
                                                  m_histoList));

    m_epi_FakeRates.push_back(createEfficiency(TString::Format("fepi_%d", i), "e fake rate;p  [GeV/c];Fake Rate", m_nbins, m_p_lower,
                                               m_p_upper,
                                               m_histoList));
  }

  //  for( const auto& det : muonSet ){
  for (unsigned int i = 0; i < muonSet.size(); ++i) {
    m_mpi_Efficiencies.push_back(createEfficiency(TString::Format("empi_%d", i), "#mu efficiency;p  [GeV/c];Efficiency", m_nbins,
                                                  m_p_lower, m_p_upper,
                                                  m_histoList));

    m_mpi_FakeRates.push_back(createEfficiency(TString::Format("fmpi_%d", i), "#mu fake rate;p  [GeV/c];Fake Rate", m_nbins, m_p_lower,
                                               m_p_upper,
                                               m_histoList));
  }

  // color the histograms of fake rates red
  for (unsigned int i = 0; i < chargedSet.size() + 3; ++i) {
    m_piK_FakeRates[i]->SetMarkerColor(kRed);
    m_piK_FakeRates[i]->SetLineColor(kRed);
    m_Kpi_FakeRates[i]->SetMarkerColor(kRed);
    m_Kpi_FakeRates[i]->SetLineColor(kRed);
    m_ppi_FakeRates[i]->SetMarkerColor(kRed);
    m_ppi_FakeRates[i]->SetLineColor(kRed);
    m_pK_FakeRates[i]->SetMarkerColor(kRed);
    m_pK_FakeRates[i]->SetLineColor(kRed);
    m_dpi_FakeRates[i]->SetMarkerColor(kRed);
    m_dpi_FakeRates[i]->SetLineColor(kRed);
    if (i < electronSet.size() + 2) {
      m_epi_FakeRates[i]->SetMarkerColor(kRed);
      m_epi_FakeRates[i]->SetLineColor(kRed);
    }
    if (i < muonSet.size()) {
      m_mpi_FakeRates[i]->SetMarkerColor(kRed);
      m_mpi_FakeRates[i]->SetLineColor(kRed);
    }
  }
}

void CombinedPIDPerformanceModule::event()
{
  StoreArray<Track> tracks;

  // loop over tracks
  for (const auto& track : tracks) {

    const TrackFitResult* trackFit = track.getTrackFitResult(Const::pion);
    if (!trackFit) {
      B2WARNING("No track fit result... Skipping.");
      continue;
    }

    const PIDLikelihood* pid = track.getRelated<PIDLikelihood>();
    if (!pid) {
      B2WARNING("No PID information... Skipping.");
      continue;
    }

    const MCParticle* mcParticle = track.getRelated<MCParticle>();
    int pdg = 0;
    if (mcParticle) pdg = mcParticle->getPDG();

    fillEfficiencyHistos(trackFit, pid, pdg);
  }
}

void CombinedPIDPerformanceModule::terminate()
{
  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    TIter nextH(m_histoList);
    TObject* obj;
    while ((obj = nextH()))
      obj->Write();


    m_rootFilePtr->Close();
  }
}

void CombinedPIDPerformanceModule::fillEfficiencyHistos(const TrackFitResult* trackFit, const PIDLikelihood* pid, int pdg)
{

  // a boolean to pass/fail events
  bool pass;

  // now fill the histograms

  // one each for pi/K/p efficiencies
  int counter = 0;
  for (unsigned int i = 0; i < chargedSet.size() + 3; ++i) {

    Const::DetectorSet det;
    if (i < chargedSet.size()) det = chargedSet[i];
    if (i == chargedSet.size()) { // Combined dE/dx
      det += Const::SVD;
      det += Const::CDC;
    }
    if (i == chargedSet.size() + 1) { // Combined dE/dx, TOP, ARICH
      det += Const::SVD;
      det += Const::CDC;
      det += Const::TOP;
      det += Const::ARICH;
    }
    if (i == chargedSet.size() + 2) // Combined dE/dx, TOP, ARICH, KLM
      det = chargedSet;

    // fill efficiencies
    if (pid->isAvailable(det) and abs(pdg) == 211) {
      pass = (pid->getDeltaLogL(Const::pion, Const::kaon, det) > 0) ? true : false;
      m_piK_Efficiencies[counter]->Fill(pass, trackFit->getMomentum().Mag());
    }
    if (pid->isAvailable(det) and abs(pdg) == 321) {
      pass = (pid->getDeltaLogL(Const::kaon, Const::pion, det) > 0) ? true : false;
      m_Kpi_Efficiencies[counter]->Fill(pass, trackFit->getMomentum().Mag());
    }
    if (pid->isAvailable(det) and abs(pdg) == 2212) {
      pass = (pid->getDeltaLogL(Const::proton, Const::pion, det) > 0) ? true : false;
      m_ppi_Efficiencies[counter]->Fill(pass, trackFit->getMomentum().Mag());
      pass = (pid->getDeltaLogL(Const::proton, Const::kaon, det) > 0) ? true : false;
      m_pK_Efficiencies[counter]->Fill(pass, trackFit->getMomentum().Mag());
    }
    if (pid->isAvailable(det) and abs(pdg) == 1000010020) {
      pass = (pid->getDeltaLogL(Const::deuteron, Const::pion, det) > 0) ? true : false;
      m_dpi_Efficiencies[counter]->Fill(pass, trackFit->getMomentum().Mag());
    }

    // and fake rates
    if (pid->isAvailable(det) and abs(pdg) == 321) {
      pass = (pid->getDeltaLogL(Const::pion, Const::kaon, det) > 0) ? true : false;
      m_piK_FakeRates[counter]->Fill(pass, trackFit->getMomentum().Mag());
      pass = (pid->getDeltaLogL(Const::proton, Const::kaon, det) > 0) ? true : false;
      m_pK_FakeRates[counter]->Fill(pass, trackFit->getMomentum().Mag());
    }
    if (pid->isAvailable(det) and abs(pdg) == 211) {
      pass = (pid->getDeltaLogL(Const::kaon, Const::pion, det) > 0) ? true : false;
      m_Kpi_FakeRates[counter]->Fill(pass, trackFit->getMomentum().Mag());
      pass = (pid->getDeltaLogL(Const::proton, Const::pion, det) > 0) ? true : false;
      m_ppi_FakeRates[counter]->Fill(pass, trackFit->getMomentum().Mag());

      pass = (pid->getDeltaLogL(Const::deuteron, Const::pion, det) > 0) ? true : false;
      m_dpi_FakeRates[counter]->Fill(pass, trackFit->getMomentum().Mag());
    }
    counter++;
  }

  // now fill the combined PID performance

  // one each for electron efficiency
  counter = 0;
  for (unsigned int i = 0; i <= electronSet.size(); ++i) {
    Const::DetectorSet det;
    if (i < electronSet.size()) det = electronSet[i];
    if (i == electronSet.size()) { // Combined dE/dx
      det += Const::SVD;
      det += Const::CDC;
    }
    if (i == electronSet.size() + 1) // Combined dE/dx, ECL
      det = electronSet;

    // fill efficiencies
    if (pid->isAvailable(det) and abs(pdg) == 11) {
      pass = (pid->getDeltaLogL(Const::electron, Const::pion, det) > 0) ? true : false;
      m_epi_Efficiencies[counter]->Fill(pass, trackFit->getMomentum().Mag());
    }

    // and fake rates
    if (pid->isAvailable(det) and abs(pdg) == 211) {
      pass = (pid->getDeltaLogL(Const::electron, Const::pion, det) > 0) ? true : false;
      m_epi_FakeRates[counter]->Fill(pass, trackFit->getMomentum().Mag());
    }
    counter++;
  }

  // and for the muon efficiency
  counter = 0;
  for (unsigned int i = 0; i < muonSet.size(); ++i) {
    Const::EDetector det = muonSet[i];

    // fill efficiencies
    if (pid->isAvailable(det) and abs(pdg) == 13) {
      pass = (pid->getDeltaLogL(Const::muon, Const::pion, det) > 0) ? true : false;
      m_mpi_Efficiencies[counter]->Fill(pass, trackFit->getMomentum().Mag());
    }

    // and fake rates
    if (pid->isAvailable(det) and abs(pdg) == 211) {
      pass = (pid->getDeltaLogL(Const::muon, Const::pion, det) > 0) ? true : false;
      m_mpi_FakeRates[counter]->Fill(pass, trackFit->getMomentum().Mag());
    }
    counter++;
  }
}

TEfficiency* CombinedPIDPerformanceModule::createEfficiency(const char* name, const char* title,
                                                            Int_t nbins, Double_t min, Double_t max, TList* histoList)
{

  TEfficiency* h = new TEfficiency(name, title, nbins, min, max);

  if (histoList)
    histoList->Add(h);

  return h;
}
