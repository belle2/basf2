/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett, Todd Pedlar
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/CombinedPIDPerformance/CombinedPIDPerformanceModule.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationVector.h>

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

  addParam("outputFileName", m_rootFileName, "Name of output root file.",
           std::string("CombinedPIDPerformance_output.root"));
  addParam("mdstType", m_mdstType, "Type of mdst file (Belle/BelleII).",
           std::string("BelleII"));

  addParam("numberOfBins", m_nbins, "Number of momentum bins.", int(100));
  addParam("pLow", m_pLow, "Lower bound of momentum range.", double(0.0));
  addParam("pHigh", m_pHigh, "Upper bound of momentum range.", double(5.0));
}

CombinedPIDPerformanceModule::~CombinedPIDPerformanceModule() { }

void CombinedPIDPerformanceModule::initialize()
{

  B2INFO("Making PID Performance plots...");

  // required input
  m_tracks.isRequired();
  m_trackFitResults.isRequired();
  m_pidLikelihoods.isRequired();
  m_mcParticles.isRequired();

  // create list of histograms to be saved in the rootfile
  m_histoList = new TList;

  // create the output ROOT File
  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  // determine which detectors to use
  // cannot add more than one at a time...
  if (m_mdstType == "BelleII") {
    chargedSet += Const::SVD; detset.push_back(svd);
  }
  chargedSet += Const::CDC; detset.push_back(cdc);
  chargedSet += Const::TOP; detset.push_back(top);
  chargedSet += Const::ARICH; detset.push_back(arich);
  chargedSet += Const::KLM; detset.push_back(klm);
  detset.push_back(dedx);
  detset.push_back(dedxtop);
  detset.push_back(all);

  if (m_mdstType == "BelleII") {
    electronSet += Const::SVD; edetset.push_back(svd);
  }
  electronSet += Const::CDC; edetset.push_back(cdc);
  electronSet += Const::ECL; edetset.push_back(ecl);
  edetset.push_back(dedx);
  edetset.push_back(dedxecl);

  muonSet += Const::KLM;

  // parameters for ROCs
  const int npbins = 18;
  const int npidbins = 101;
  const float pidlow = 0.;
  const float pidhigh = 1.01;
  const char* names[] = { "pi", "k", "e", "mu", "p" };

  // roc plots: h_ROC[Hypothesis][det](true particle id, pidvalue, momentum)
  // pion=0, kaon=1, electron=2, muon=3, proton=4;
  for (int Hypo = 0; Hypo < 5; Hypo++) {
    for (int k = 0; k < 10; k++) {
      h_ROC[Hypo][k] = new TH3F(Form("ROC_%s_%d", names[Hypo], k), Form(";PID(%s);N;p (GeV)", names[Hypo]), 5, 0, 5, npidbins, pidlow,
                                pidhigh, npbins, m_pLow, m_pHigh);
      if (m_histoList) m_histoList->Add(h_ROC[Hypo][k]);
    }
  }

  // create the efficiency and fake rate objects for hadrons
  for (unsigned int i = 0; i < chargedSet.size() + 3; ++i) {
    m_piK_Efficiencies.push_back(createEfficiency(TString::Format("epik_%d", i), "#pi efficiency;p  [GeV/c];Efficiency", m_nbins,
                                                  m_pLow,
                                                  m_pHigh, m_histoList));
    m_Kpi_Efficiencies.push_back(createEfficiency(TString::Format("ekpi_%d", i), "K efficiency;p  [GeV/c];Efficiency", m_nbins, m_pLow,
                                                  m_pHigh, m_histoList));
    m_ppi_Efficiencies.push_back(createEfficiency(TString::Format("eppi_%d", i), "p efficiency;p  [GeV/c];Efficiency", m_nbins, m_pLow,
                                                  m_pHigh, m_histoList));
    m_pK_Efficiencies.push_back(createEfficiency(TString::Format("epk_%d", i), "p efficiency;p  [GeV/c];Efficiency", m_nbins, m_pLow,
                                                 m_pHigh, m_histoList));
    m_dpi_Efficiencies.push_back(createEfficiency(TString::Format("edpi_%d", i), "d efficiency;p  [GeV/c];Efficiency", m_nbins, m_pLow,
                                                  m_pHigh, m_histoList));

    m_piK_FakeRates.push_back(createEfficiency(TString::Format("fpik_%d", i), "#pi fake rate;p  [GeV/c];Fake Rate", m_nbins, m_pLow,
                                               m_pHigh, m_histoList));
    m_Kpi_FakeRates.push_back(createEfficiency(TString::Format("fkpi_%d", i), "K fake rate;p  [GeV/c];Fake Rate", m_nbins, m_pLow,
                                               m_pHigh,
                                               m_histoList));
    m_ppi_FakeRates.push_back(createEfficiency(TString::Format("fppi_%d", i), "p fake rate;p  [GeV/c];Fake Rate", m_nbins, m_pLow,
                                               m_pHigh,
                                               m_histoList));
    m_pK_FakeRates.push_back(createEfficiency(TString::Format("fpk_%d", i), "p fake rate;p  [GeV/c];Fake Rate", m_nbins, m_pLow,
                                              m_pHigh,
                                              m_histoList));
    m_dpi_FakeRates.push_back(createEfficiency(TString::Format("fdpi_%d", i), "d fake rate;p  [GeV/c];Fake Rate", m_nbins, m_pLow,
                                               m_pHigh,
                                               m_histoList));
  }

  // create the efficiency and fake rate objects for electrons
  for (unsigned int i = 0; i < electronSet.size() + 2; ++i) {
    m_epi_Efficiencies.push_back(createEfficiency(TString::Format("eepi_%d", i), "e efficiency;p  [GeV/c];Efficiency", m_nbins, m_pLow,
                                                  m_pHigh, m_histoList));

    m_epi_FakeRates.push_back(createEfficiency(TString::Format("fepi_%d", i), "e fake rate;p  [GeV/c];Fake Rate", m_nbins, m_pLow,
                                               m_pHigh,
                                               m_histoList));
  }

  // create the efficiency and fake rate objects for muons
  for (unsigned int i = 0; i < muonSet.size(); ++i) {
    m_mpi_Efficiencies.push_back(createEfficiency(TString::Format("empi_%d", i), "#mu efficiency;p  [GeV/c];Efficiency", m_nbins,
                                                  m_pLow,
                                                  m_pHigh, m_histoList));

    m_mpi_FakeRates.push_back(createEfficiency(TString::Format("fmpi_%d", i), "#mu fake rate;p  [GeV/c];Fake Rate", m_nbins, m_pLow,
                                               m_pHigh, m_histoList));
  }

  // color the fake rate objects red here for simplicity later
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
  for (const auto& track : m_tracks) {

    const TrackFitResult* trackFit = track.getTrackFitResultWithClosestMass(Const::pion);
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
    if (!mcParticle) continue;
    if (mcParticle) pdg = mcParticle->getPDG();

    // apply some loose cuts on track quality and production vertex
    if (trackFit->getPValue() < 0.001) continue;
    if (mcParticle->getProductionVertex().Perp() > 1.0) continue;
    if (!(mcParticle->getStatus(MCParticle::c_PrimaryParticle))) continue;

    // fill the efficiencies and fake rates
    fillEfficiencyHistos(trackFit, pid, pdg);
  }
}

void CombinedPIDPerformanceModule::terminate()
{

  // write out the objects
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

  bool pass; // used to pass or fail events based on coverage
  float pidval = -1.0;

  // fill rocs, efficiencies, and fake rates for hadrons
  for (unsigned int i = 0; i < chargedSet.size() + 3; ++i) {
    int detnum = detset[i];

    Const::DetectorSet det;
    if (i < chargedSet.size()) det = chargedSet[i];
    if (i == chargedSet.size()) { // Combined dE/dx
      if (m_mdstType == "BelleII") det += Const::SVD;
      det += Const::CDC;
    }
    if (i == chargedSet.size() + 1) { // Combined dE/dx, TOP
      if (m_mdstType == "BelleII") det += Const::SVD;
      det += Const::CDC;
      det += Const::TOP;
    }
    if (i == chargedSet.size() + 2) { // Combined dE/dx, TOP, ARICH
      if (m_mdstType == "BelleII") det += Const::SVD;
      det += Const::CDC;
      det += Const::TOP;
      det += Const::ARICH;
    }

    // get pid LogL values for hadrons
    double logl_pi = 0, logl_k = 0, logl_p = 0;
    if (pidavail(pid, det)) {
      logl_pi = pid->getLogL(Const::pion, det);
      logl_k = pid->getLogL(Const::kaon, det);
      logl_p = pid->getLogL(Const::proton, det);
    }

    // fill rocs, efficiencies, and fake rates for true pions
    if (pidavail(pid, det) and abs(pdg) == 211) {
      pass = (pid->getDeltaLogL(Const::pion, Const::kaon, det) > 0) ? true : false;
      m_piK_Efficiencies[i]->Fill(pass, trackFit->getMomentum().Mag());

      pass = (pid->getDeltaLogL(Const::kaon, Const::pion, det) > 0) ? true : false;
      m_Kpi_FakeRates[i]->Fill(pass, trackFit->getMomentum().Mag());

      pass = (pid->getDeltaLogL(Const::proton, Const::pion, det) > 0) ? true : false;
      m_ppi_FakeRates[i]->Fill(pass, trackFit->getMomentum().Mag());

      pass = (pid->getDeltaLogL(Const::deuteron, Const::pion, det) > 0) ? true : false;
      m_dpi_FakeRates[i]->Fill(pass, trackFit->getMomentum().Mag());

      pidval = pidvalue(logl_pi, logl_k);
      h_ROC[0][detnum]->Fill(0.0, pidval, trackFit->getMomentum().Mag()); // pion eff

      pidval = pidvalue(logl_k, logl_pi);
      h_ROC[1][detnum]->Fill(0.0, pidval, trackFit->getMomentum().Mag()); // pion faking k

      pidval = pidvalue(logl_p, logl_pi);
      h_ROC[4][detnum]->Fill(0.0, pidval, trackFit->getMomentum().Mag()); // pion faking proton
    }

    // fill rocs, efficiencies, and fake rates for true kaons
    if (pidavail(pid, det) and abs(pdg) == 321) {
      pass = (pid->getDeltaLogL(Const::kaon, Const::pion, det) > 0) ? true : false;
      m_Kpi_Efficiencies[i]->Fill(pass, trackFit->getMomentum().Mag());

      pass = (pid->getDeltaLogL(Const::pion, Const::kaon, det) > 0) ? true : false;
      m_piK_FakeRates[i]->Fill(pass, trackFit->getMomentum().Mag());

      pass = (pid->getDeltaLogL(Const::proton, Const::kaon, det) > 0) ? true : false;
      m_pK_FakeRates[i]->Fill(pass, trackFit->getMomentum().Mag());

      pidval = pidvalue(logl_k, logl_pi);
      h_ROC[1][detnum]->Fill(1.0, pidval, trackFit->getMomentum().Mag()); // kaon eff

      pidval = pidvalue(logl_pi, logl_k);
      h_ROC[0][detnum]->Fill(1.0, pidval, trackFit->getMomentum().Mag()); // kaon faking pion

      pidval = pidvalue(logl_p, logl_k);
      h_ROC[4][detnum]->Fill(1.0, pidval, trackFit->getMomentum().Mag()); // kaon faking proton
    }

    // fill rocs and efficiencies for true protons
    if (pidavail(pid, det) and abs(pdg) == 2212) {
      pass = (pid->getDeltaLogL(Const::proton, Const::pion, det) > 0) ? true : false;
      m_ppi_Efficiencies[i]->Fill(pass, trackFit->getMomentum().Mag());

      pass = (pid->getDeltaLogL(Const::proton, Const::kaon, det) > 0) ? true : false;
      m_pK_Efficiencies[i]->Fill(pass, trackFit->getMomentum().Mag());

      pidval = pidvalue(logl_p, logl_pi + logl_k);
      h_ROC[4][detnum]->Fill(4.0, pidval, trackFit->getMomentum().Mag()); // proton eff vs. pion and kaon
    }

    // fill efficiencies for true deuterons
    if (pidavail(pid, det) and abs(pdg) == 1000010020) {
      pass = (pid->getDeltaLogL(Const::deuteron, Const::pion, det) > 0) ? true : false;
      m_dpi_Efficiencies[i]->Fill(pass, trackFit->getMomentum().Mag());
    }
  } // end of loop for hadrons

  // fill rocs, efficiencies, and fake rates for electrons
  for (unsigned int i = 0; i <= electronSet.size() + 1; ++i) {
    int detnum = edetset[i];

    Const::DetectorSet det;
    if (i < electronSet.size()) det = electronSet[i];
    if (i == electronSet.size()) { // Combined dE/dx
      if (m_mdstType == "BelleII") det += Const::SVD;
      det += Const::CDC;
    }
    if (i == electronSet.size() + 1) // Combined dE/dx, ECL
      det = electronSet;

    // get pid LogL values for electrons and pions
    double logl_e = 0, logl_pi_e = 0;
    if (pidavail(pid, det)) {
      logl_e = pid->getLogL(Const::electron, det);
      logl_pi_e = pid->getLogL(Const::pion, det);
    }

    // fill rocs and efficiencies for true electrons
    if (pidavail(pid, det) and abs(pdg) == 11) {
      pass = (pid->getDeltaLogL(Const::electron, Const::pion, det) > 0) ? true : false;
      m_epi_Efficiencies[i]->Fill(pass, trackFit->getMomentum().Mag());

      pidval = pidvalue(logl_e, logl_pi_e);
      h_ROC[2][detnum]->Fill(2.0, pidval, trackFit->getMomentum().Mag()); // electron eff vs pion
    }

    // fill rocs and fake rates for true pions
    if (pidavail(pid, det) and abs(pdg) == 211) {
      pass = (pid->getDeltaLogL(Const::electron, Const::pion, det) > 0) ? true : false;
      m_epi_FakeRates[i]->Fill(pass, trackFit->getMomentum().Mag());

      pidval = pidvalue(logl_pi_e, logl_e);
      h_ROC[0][detnum]->Fill(0.0, pidval, trackFit->getMomentum().Mag()); // pion faking electron
    }
  } // end of loop for electrons

  // fill rocs, efficiencies, and fake rates for muons
  for (unsigned int i = 0; i < muonSet.size(); ++i) {
    Const::EDetector det = muonSet[i];

    // get pid LogL values for electrons and pions
    double logl_mu = 0, logl_pi_mu = 0;
    if (pidavail(pid, det)) {
      logl_mu = pid->getLogL(Const::muon, det);
      logl_pi_mu = pid->getLogL(Const::pion, det);
    }

    // fill rocs and efficiencies for true muons
    if (pidavail(pid, det) and abs(pdg) == 13) {
      pass = (pid->getDeltaLogL(Const::muon, Const::pion, det) > 0) ? true : false;
      m_mpi_Efficiencies[i]->Fill(pass, trackFit->getMomentum().Mag());

      pidval = pidvalue(logl_mu, logl_pi_mu);
      h_ROC[3][klm]->Fill(3.0, pidval, trackFit->getMomentum().Mag()); // muon eff vs. pion
    }

    // fill rocs and fake ratesfor true pions
    if (pidavail(pid, det) and abs(pdg) == 211) {
      pass = (pid->getDeltaLogL(Const::muon, Const::pion, det) > 0) ? true : false;
      m_mpi_FakeRates[i]->Fill(pass, trackFit->getMomentum().Mag());

      pidval = pidvalue(logl_pi_mu, logl_mu);
      h_ROC[3][klm]->Fill(0.0, pidval, trackFit->getMomentum().Mag()); // pion faking muon
    }
  } // end of loop for muons
}

TEfficiency* CombinedPIDPerformanceModule::createEfficiency(const char* name, const char* title,
                                                            Int_t nbins, Double_t min, Double_t max, TList* histoList)
{

  TEfficiency* h = new TEfficiency(name, title, nbins, min, max);

  if (histoList)
    histoList->Add(h);

  return h;
}


double CombinedPIDPerformanceModule::pidvalue(float logl_a, float logl_b)
{
  // returns likelihood ratio

  double val = -1.0;
  float dl = logl_b - logl_a;

  if (dl < 0) {
    val = 1 / (1 + exp(dl));
  } else {
    val = exp(-1 * dl) / (1 + exp(-1 * dl));
  }

  return val;
}

bool CombinedPIDPerformanceModule::pidavail(const PIDLikelihood* pidl, Const::DetectorSet dets)
{
  // returns true if at least one detector in the detectors is available.

  bool avail = false;
  for (unsigned int i = 0; i < dets.size(); ++i) {
    if (pidl->isAvailable(dets[i])) {
      avail = true;
    }
  }

  return avail;
}


