//+
// File : B2BIIMCParticlesMonitorModule.cc
// Description : A module to plot MCParticles monitor histograms in basf2
//
// Contributors: Hulya Atmacan
// Date : November 21, 2015
//-

#include <b2bii/modules/B2BIIMCParticlesMonitor/B2BIIMCParticlesMonitorModule.h>

// framework
#include <framework/core/HistoModule.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/gearbox/Const.h>

// dataonjects
#include <mdst/dataobjects/MCParticle.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(B2BIIMCParticlesMonitor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

B2BIIMCParticlesMonitorModule::B2BIIMCParticlesMonitorModule() : HistoModule()
{
  //Set module properties
  setDescription("This module creates and fills B2BII MCParticles monitoring histograms.");
  //  setDescription(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
}

B2BIIMCParticlesMonitorModule::~B2BIIMCParticlesMonitorModule()
{
}

void B2BIIMCParticlesMonitorModule::defineHisto()
{
  mcPDG = new TH1F("mcPDG", "PDG values of MCParticles", 100, -22500, 22500);
  mcM = new TH1F("mcM", "M", 100, 0, 11.6);
  mcPx = new TH1F("mcPx", "Px", 100, -5.8, 5.8);
  mcPy = new TH1F("mcPy", "Py", 100, -5.8, 5.8);
  mcPz = new TH1F("mcPz", "Pz", 100, -4.2, 8.6);
  mcE = new TH1F("mcE", "E", 100, 0, 12.6);
  mcVx = new TH1F("mcVx", "Vx", 100, -4500, 5100);
  mcVy = new TH1F("mcVy", "Vy", 100, -5000, 4200);
  mcVz = new TH1F("mcVz", "Vz", 100, -9000, 9800);
  mcPiPlusMother = new TH1F("mcPiPlusMother", "Pi+ Mother PDG's", 102, -498000, 9498000);
  mcPiMinusMother = new TH1F("mcPiMinusMother", "Pi- Mother PDG's", 102, -498000, 9498000);
  mcPi0Mother = new TH1F("mcPi0Mother", "Pi0 Mother PDG's", 102, -498000, 9498000);
  mcNDau = new TH1F("mcNDau", "Number of Daughters", 56, 0, 56);
}

void B2BIIMCParticlesMonitorModule::initialize()
{
  REG_HISTOGRAM   // required to register histograms to HistoManager
}

void B2BIIMCParticlesMonitorModule::beginRun()
{
}

void B2BIIMCParticlesMonitorModule::event()
{
  StoreArray<MCParticle> mc_particles;
  if (!mc_particles) B2ERROR("Cannot find MCParticles array");

  int nentries = mc_particles.getEntries();

  for (int i = 0; i < nentries; i++) {

    const MCParticle* mcparticle = mc_particles[i];

    mcPDG->Fill(mcparticle->getPDG());

    mcM->Fill(mcparticle->getMass());
    mcPx->Fill(mcparticle->getMomentum().X());
    mcPy->Fill(mcparticle->getMomentum().Y());
    mcPz->Fill(mcparticle->getMomentum().Z());
    mcE->Fill(mcparticle->getEnergy());

    mcVx->Fill(mcparticle->getProductionVertex().X());
    mcVy->Fill(mcparticle->getProductionVertex().Y());
    mcVz->Fill(mcparticle->getProductionVertex().Z());

    int mcparticle_pdg = mcparticle->getPDG();

    if (mcparticle->getMother()) {
      if (mcparticle_pdg == Const::pion.getPDGCode()) mcPiPlusMother->Fill(mcparticle->getMother()->getPDG());
      if (mcparticle_pdg == -Const::pion.getPDGCode()) mcPiMinusMother->Fill(mcparticle->getMother()->getPDG());
      if (mcparticle_pdg == Const::pi0.getPDGCode()) mcPi0Mother->Fill(mcparticle->getMother()->getPDG());
    }

    mcNDau->Fill(mcparticle->getNDaughters());
  }
}

void B2BIIMCParticlesMonitorModule::endRun()
{
}

void B2BIIMCParticlesMonitorModule::terminate()
{
}

