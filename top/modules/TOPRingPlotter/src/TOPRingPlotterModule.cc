/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2020 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Umberto Tamponi                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <top/modules/TOPRingPlotter/TOPRingPlotterModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/MakeROOTCompatible.h>

#include <iostream>
#include <TTree.h>
#include <TF2.h>
#include <TH2F.h>
#include <TFile.h>
#include <TRandom.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <framework/dataobjects/EventMetaData.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <top/dataobjects/TOPBarHit.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>

#include <analysis/VariableManager/Manager.h>

#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction/TOPreco.h>
#include <top/reconstruction/TOPtrack.h>
#include <top/reconstruction/TOPconfigure.h>

using namespace Belle2;
using namespace TOP;
using namespace Belle2::Variable;


REG_MODULE(TOPRingPlotter)

TOPRingPlotterModule::TOPRingPlotterModule() : Module()
{
  // Set module properties
  setDescription(
    R"DOC(A module to plot the x-t images from TOP, and in general study the distribution of the digits associated to the particles in a particleList)DOC");

  // Parameter definitions
  addParam("particleList", m_particleList, "List of particles to be used for plotting", std::string("pi+:all"));
  addParam("variables", m_variables, "List of variables to be saved", {});
  addParam("outputName", m_outputName, "Name of the output file", std::string("TOPRings.root"));
  addParam("toyNumber", m_toyNumber, "Number of toys used to fill the PDF maps", 1);
  addParam("saveHistograms", m_saveHistograms, "Set true to save the histograms of the maps", false);
  addParam("saveDigitTree", m_saveDigitTree, "Set true to Save the branch with the digits", true);

}



void TOPRingPlotterModule::fillPDF(TH2F* histo,  Belle2::TOP::TOPreco reco)
{
  for (int iSim = 0; iSim < m_toyNumber; iSim++) {
    for (int pixelID = 1; pixelID <= 512; pixelID++) {
      for (int peak = 0; peak < reco.getNumofPDFPeaks(pixelID); peak++) {
        float t0 = 0;
        float sigma = 0;
        float numPhot = 0;
        reco.getPDFPeak(pixelID, peak, t0, sigma, numPhot);
        for (int rn = 0; rn < gRandom->Poisson(numPhot); rn++) {
          double time = gRandom->Gaus(t0, sigma);
          int pixelCol = (pixelID - 1) % 64 + 1;
          histo->Fill(pixelCol, time);
        }
      }
    }
  }
}


void TOPRingPlotterModule::initialize()
{
  B2INFO("creating a tree for TOPRingPlotterModule");
  m_outputFile = new TFile(m_outputName.c_str(), "recreate");
  m_tree = new TTree("tree", "tree");

  if (m_saveHistograms) {
    m_tree->Branch("hitMapMCE", "TH2F", &m_hitMapMCE, 32000, 0);
    m_tree->Branch("hitMapMCMU", "TH2F", &m_hitMapMCMU, 32000, 0);
    m_tree->Branch("hitMapMCPi", "TH2F", &m_hitMapMCPi, 32000, 0);
    m_tree->Branch("hitMapMCK", "TH2F", &m_hitMapMCK, 32000, 0);
    m_tree->Branch("hitMapMCP", "TH2F", &m_hitMapMCP, 32000, 0);
  }

  m_tree->Branch("nDigits", &m_nDigits, "m_nDigits/I");

  m_tree->Branch("digitTime", m_digitTime, "m_digitTime[m_nDigits]/F");
  m_tree->Branch("digitAmplitude", m_digitAmplitude, "m_digitAmplitude[m_nDigits]/F");
  m_tree->Branch("digitWidth", m_digitWidth, "m_digitWidth[m_nDigits]/F");
  m_tree->Branch("digitChannel", m_digitChannel, "m_digitChannel[m_nDigits]/S");
  m_tree->Branch("digitPixelCol", m_digitPixelCol, "m_digitPixelCol[m_nDigits]/S");
  m_tree->Branch("digitPixelRow", m_digitPixelRow, "m_digitPixelRow[m_nDigits]/S");
  m_tree->Branch("digitASICChannel", m_digitASICChannel, "m_digitASICChannel[m_nDigits]/S");
  m_tree->Branch("digitPMTNumber", m_digitPMTNumber, "m_digitPMTNumber[m_nDigits]/S");




  // declare branches and get the variable strings. This is copy-pasted from VariablesToNtupleModule
  m_variables = Variable::Manager::Instance().resolveCollections(m_variables);
  m_branchAddresses.resize(m_variables.size() + 1);
  size_t enumerate = 0;
  for (const std::string& varStr : m_variables) {
    std::string branchName = makeROOTCompatible(varStr);
    m_tree->Branch(branchName.c_str(), &m_branchAddresses[enumerate], (branchName + "/D").c_str());

    // also collection function pointers
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(varStr);
    if (!var) {
      B2ERROR("Variable '" << varStr << "' is not available in Variable::Manager!");
    } else {
      if (m_particleList.empty() && var->description.find("[Eventbased]") == std::string::npos) {
        B2ERROR("Variable '" << varStr << "' is not an event-based variable, "
                "but you are using VariablesToNtuple without a decay string, i.e. in the event-wise mode.\n"
                "If you have created an event-based alias you can wrap your alias with `eventCached` to "
                "declare it as event based, which avoids this error.\n\n"
                "vm.addAlias('myAliasName', 'eventCached(myAlias)')");
        continue;
      }
      m_functions.push_back(var->function);
    }
    enumerate++;
  }



  StoreArray<Track> tracks;
  tracks.isRequired();
  StoreArray<ExtHit> extHits;
  extHits.isRequired();
  StoreArray<TOPLikelihood> likelihoods;
  likelihoods.isRequired();
  StoreArray<TOPDigit> digits;
  digits.isRequired();
  StoreArray<TOPBarHit> barHits;
  barHits.isOptional();

  TOPconfigure config;
}


void TOPRingPlotterModule::event()
{
  int Nhyp = 1;
  double mass = Const::pion.getMass();
  int pdgCode = 211;

  TOPreco reco(Nhyp, &mass, &pdgCode);
  reco.setPDFoption(TOPreco::c_Fine);
  reco.setTimeWindow(0, 200);

  StoreObjPtr<EventMetaData> evtMetaData;
  StoreArray<Track> tracks;
  StoreArray<TOPDigit> digits;
  StoreObjPtr<ParticleList> particles(m_particleList);

  if (! particles.isValid())
    return;

  int n_part = particles->getListSize();
  for (int i = 0; i < n_part; i++) {
    const Particle* particle = particles->getParticle(i);
    const Track* track = particle->getTrack();
    if (!track)
      continue;

    TOPtrack trk(track, Const::pion);

    if (!trk.isValid()) continue;

    reco.setMass(mass, m_pdg);
    reco.reconstruct(trk, 211);
    if (reco.getFlag() != 1) continue; // track is not in the acceptance of TOP


    m_nDigits = 0;
    for (const auto& digi : digits) {
      if (digi.getModuleID() == trk.getModuleID() and digi.getHitQuality() == 1) {
        m_digitTime[m_nDigits] = digi.getTime();
        m_digitChannel[m_nDigits] = digi.getChannel();
        m_digitPixelCol[m_nDigits] = digi.getPixelCol();
        m_digitPixelRow[m_nDigits] = digi.getPixelRow();
        m_digitAmplitude[m_nDigits] = digi.getPulseHeight();
        m_digitWidth[m_nDigits] = digi.getPulseWidth();
        m_digitASICChannel[m_nDigits] = digi.getASICChannel();
        m_digitPMTNumber[m_nDigits] = digi.getPMTNumber();
        m_nDigits++;
        if (m_nDigits > m_maxDigits) break;
      }
    }


    for (unsigned int iVar = 0; iVar < m_variables.size(); iVar++) {
      m_branchAddresses[iVar] = m_functions[iVar](particle);
    }


    TOPtrack trkPDFE(track, Const::electron.getMass());
    reco.setMass(Const::electron.getMass(), 11);
    reco.reconstruct(trkPDFE, 211);
    fillPDF(m_hitMapMCE, reco);

    TOPtrack trkPDFMu(track, Const::muon.getMass());
    reco.setMass(Const::muon.getMass(), 13);
    reco.reconstruct(trkPDFMu, 211);
    fillPDF(m_hitMapMCMU, reco);

    TOPtrack trkPDFPi(track, Const::pion.getMass());
    reco.setMass(Const::pion.getMass(), 211);
    reco.reconstruct(trkPDFPi, 211);
    fillPDF(m_hitMapMCPi,  reco);

    TOPtrack trkPDFK(track, Const::kaon.getMass());
    reco.setMass(Const::kaon.getMass(), 321);
    reco.reconstruct(trkPDFK, 321);
    fillPDF(m_hitMapMCK,  reco);

    TOPtrack trkPDFP(track, Const::proton.getMass());
    reco.setMass(Const::proton.getMass(), 2212);
    reco.reconstruct(trkPDFP, 2212);
    fillPDF(m_hitMapMCPi, reco);


    m_tree->Fill();
    m_hitMapMCPi->Reset();
    m_hitMapMCK->Reset();
    m_hitMapMCP->Reset();
    m_hitMapMCE->Reset();
    m_hitMapMCMU->Reset();

  }
}


void TOPRingPlotterModule::terminate()
{
  m_tree->Write();
  m_outputFile->Close();
}


