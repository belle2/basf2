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
  addParam("nToys", m_toyNumber,
           "Number of toys used to sample the PDFs. Set it to a large number for a precise sampling of the PDF in the histograms (suggested for small number of events) ",
           1);
  addParam("saveHistograms", m_saveHistograms,
           "Set true to save the histograms of the maps in addition to the list of sampled pseudo-digits", false);
}


void TOPRingPlotterModule::resetTree()
{
  m_pdfSamplesP = 0;
  m_pdfToysP = 0;
  m_pdfSamplesK = 0;
  m_pdfToysK = 0;
  m_pdfSamplesPI = 0;
  m_pdfToysPI = 0;
  m_pdfSamplesMU = 0;
  m_pdfToysMU = 0;
  m_pdfSamplesE = 0;
  m_pdfToysE = 0;
  m_hitMapMCK->Reset();
  m_hitMapMCPI->Reset();
  m_hitMapMCP->Reset();
  m_hitMapMCE->Reset();
  m_hitMapMCMU->Reset();
  return;
}


void TOPRingPlotterModule::fillPDF(Belle2::Const::ChargedStable ch, const Track* track, TH2F* histo, short* pixelArray,
                                   float* timeArray, int& arrayGuard, int& toyCounter)
{
  double mass = ch.getMass();
  int pdgCode = ch.getPDGCode();


  TOPtrack trkPDF(track, ch);

  if (!trkPDF.isValid())
    return;

  TOPreco reco(1, &mass, &pdgCode);
  reco.setPDFoption(TOPreco::c_Fine);
  reco.setTimeWindow(0, 200);
  reco.setMass(mass, pdgCode);
  reco.reconstruct(trkPDF, pdgCode);

  if (reco.getFlag() != 1)
    return ; // track is not in the acceptance of TOP

  arrayGuard = 0; // this keeps track of the total number of PDF samples we have saved so far

  // loop over the toys
  for (int iSim = 0; iSim < m_toyNumber; iSim++) {
    // tmp arrays. Before appending the PDF-based digits to the list, we want to be sure
    // that their total number does not exceeds teh size of pixelArray and timeArray.
    // We run the ful toy for an event storing the results here, and we copy them ro timeArray and pixelArray only
    // if there's enough room.
    std::vector<float> tmpTime;
    std::vector<short> tmpPixel;

    // loops of pixels and PDF peaks to make a full sampling of the PDF
    for (int pixelID = 1; pixelID <= 512; pixelID++) {
      for (int peak = 0; peak < reco.getNumofPDFPeaks(pixelID); peak++) {

        float t0 = 0;
        float sigma = 0;
        float numPhotAvg = 0;
        reco.getPDFPeak(pixelID, peak, t0, sigma, numPhotAvg);
        short numPhot = gRandom->Poisson(numPhotAvg);

        for (short rn = 0; rn < numPhot; rn++) {
          float time = gRandom->Gaus(t0, sigma);
          short pixelCol = (pixelID - 1) % 64 + 1;
          histo->Fill(pixelCol, time);
          tmpTime.push_back(time);
          tmpPixel.push_back(pixelID);
        }
      }
    }

    // before transferring the results of the PDF sampling into pixelArray and timeArray, check if there's
    // enough room.
    if (arrayGuard + tmpTime.size() < m_MaxPDFPhotons) {
      for (unsigned int iPh = 0; iPh < tmpTime.size(); iPh++) {
        pixelArray[arrayGuard] = tmpPixel[iPh];
        timeArray[arrayGuard] = tmpTime[iPh];
        arrayGuard++;
      }
      toyCounter++;
    } else {
      return; // we already reached the max size of the arrays for sampling, no point in keep doing this
    }
  }
  return;
}





void TOPRingPlotterModule::initialize()
{
  m_hitMapMCK = new TH2F("hitMapMCK", "x-t map of the kaon PDF", 64, 0, 64, 50, 0, 100);
  m_hitMapMCPI = new TH2F("hitMapMCPI", "x-t map of the pion PDF", 64, 0, 64, 50, 0, 100);
  m_hitMapMCP = new TH2F("hitMapMCP", "x-t map of the proton PDF", 64, 0, 64, 50, 0, 100);
  m_hitMapMCE = new TH2F("hitMapMCE", "x-t map of the electron PDF", 64, 0, 64, 50, 0, 100);
  m_hitMapMCMU = new TH2F("hitMapMCMU", "x-t map of the muon PDF", 64, 0, 64, 50, 0, 100);

  B2INFO("creating a tree for TOPRingPlotterModule");
  m_outputFile = new TFile(m_outputName.c_str(), "recreate");
  m_tree = new TTree("tree", "tree");

  if (m_saveHistograms) {
    m_tree->Branch("hitMapMCE", "TH2F", &m_hitMapMCE, 32000, 0);
    m_tree->Branch("hitMapMCMU", "TH2F", &m_hitMapMCMU, 32000, 0);
    m_tree->Branch("hitMapMCPI", "TH2F", &m_hitMapMCPI, 32000, 0);
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


  m_tree->Branch("pdfSamplesP", &m_pdfSamplesP, "m_pdfSamplesP/I");
  m_tree->Branch("pdfToysP", &m_pdfToysP, "m_pdfToysP/I");
  m_tree->Branch("pdfPixelP", m_pdfPixelP, "m_pdfPixelP[m_pdfSamplesP]/S");
  m_tree->Branch("pdfTimeP", m_pdfTimeP, "m_pdfTimeP[m_pdfSamplesP]/F");

  m_tree->Branch("pdfSamplesK", &m_pdfSamplesK, "m_pdfSamplesK/I");
  m_tree->Branch("pdfToysK", &m_pdfToysK, "m_pdfToysK/I");
  m_tree->Branch("pdfPixelK", m_pdfPixelK, "m_pdfPixelK[m_pdfSamplesK]/S");
  m_tree->Branch("pdfTimeK", m_pdfTimeK, "m_pdfTimeK[m_pdfSamplesK]/F");

  m_tree->Branch("pdfSamplesPI", &m_pdfSamplesPI, "m_pdfSamplesPI/I");
  m_tree->Branch("pdfToysPI", &m_pdfToysPI, "m_pdfToysPI/I");
  m_tree->Branch("pdfPixelPI", m_pdfPixelPI, "m_pdfPixelPI[m_pdfSamplesPI]/S");
  m_tree->Branch("pdfTimePI", m_pdfTimePI, "m_pdfTimePI[m_pdfSamplesPI]/F");

  m_tree->Branch("pdfSamplesMU", &m_pdfSamplesMU, "m_pdfSamplesMU/I");
  m_tree->Branch("pdfToysMU", &m_pdfToysMU, "m_pdfToysMU/I");
  m_tree->Branch("pdfPixelMU", m_pdfPixelMU, "m_pdfPixelMU[m_pdfSamplesMU]/S");
  m_tree->Branch("pdfTimeMU", m_pdfTimeMU, "m_pdfTimeMU[m_pdfSamplesMU]/F");

  m_tree->Branch("pdfSamplesE", &m_pdfSamplesE, "m_pdfSamplesE/I");
  m_tree->Branch("pdfToysE", &m_pdfToysE, "m_pdfToysE/I");
  m_tree->Branch("pdfPixelE", m_pdfPixelE, "m_pdfPixelE[m_pdfSamplesE]/S");
  m_tree->Branch("pdfTimeE", m_pdfTimeE, "m_pdfTimeE[m_pdfSamplesE]/F");


  // This parts parses the list of variables and creates the branches in the output tree.
  // This is copy-pasted from VariablesToNtupleModule
  m_variables = Variable::Manager::Instance().resolveCollections(m_variables);
  m_branchAddresses.resize(m_variables.size() + 1);
  size_t enumerate = 0;
  for (const std::string& varStr : m_variables) {
    std::string branchName = makeROOTCompatible(varStr);
    m_tree->Branch(branchName.c_str(), &m_branchAddresses[enumerate], (branchName + "/D").c_str());

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

  StoreObjPtr<EventMetaData> evtMetaData;
  StoreArray<Track> tracks;
  StoreArray<TOPDigit> digits;
  StoreObjPtr<ParticleList> particles(m_particleList);

  // skip on invalid lists
  if (! particles.isValid())
    return;

  // loops over all particles
  int n_part = particles->getListSize();
  for (int i = 0; i < n_part; i++) {
    // Reset the tree variables to the default values
    resetTree();

    const Particle* particle = particles->getParticle(i);

    const Track* track = particle->getTrack();
    if (!track)
      continue;

    // Check if one can make a TOP track before going on
    TOPtrack trk(track, Const::pion);
    if (!trk.isValid())
      continue;

    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable("topSlotID");

    if (!var)
      continue;

    auto moduleID = var->function(particle);

    if (moduleID < 1)
      continue;

    m_nDigits = 0;
    for (const auto& digi : digits) {
      if (digi.getModuleID() == moduleID and digi.getHitQuality() == 1) {
        m_digitTime[m_nDigits] = digi.getTime();
        m_digitChannel[m_nDigits] = digi.getChannel();
        m_digitPixelCol[m_nDigits] = digi.getPixelCol();
        m_digitPixelRow[m_nDigits] = digi.getPixelRow();
        m_digitAmplitude[m_nDigits] = digi.getPulseHeight();
        m_digitWidth[m_nDigits] = digi.getPulseWidth();
        m_digitASICChannel[m_nDigits] = digi.getASICChannel();
        m_digitPMTNumber[m_nDigits] = digi.getPMTNumber();
        m_nDigits++;
        if (m_nDigits > m_MaxPhotons) break;
      }
    }


    for (unsigned int iVar = 0; iVar < m_variables.size(); iVar++) {
      m_branchAddresses[iVar] = m_functions[iVar](particle);
    }


    fillPDF(Belle2::Const::electron, track, m_hitMapMCE, m_pdfPixelE, m_pdfTimeE, m_pdfSamplesE, m_pdfToysE);
    fillPDF(Belle2::Const::muon, track, m_hitMapMCMU, m_pdfPixelMU, m_pdfTimeMU, m_pdfSamplesMU, m_pdfToysMU);
    fillPDF(Belle2::Const::pion, track, m_hitMapMCPI, m_pdfPixelPI, m_pdfTimePI, m_pdfSamplesPI, m_pdfToysPI);
    fillPDF(Belle2::Const::kaon, track, m_hitMapMCK, m_pdfPixelK, m_pdfTimeK, m_pdfSamplesK, m_pdfToysK);
    fillPDF(Belle2::Const::proton, track, m_hitMapMCP, m_pdfPixelP, m_pdfTimeP, m_pdfSamplesP, m_pdfToysP);

    m_tree->Fill();
  }
}


void TOPRingPlotterModule::terminate()
{
  m_tree->Write();
  m_outputFile->Close();
}


