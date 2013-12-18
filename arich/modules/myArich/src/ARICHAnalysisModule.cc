/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dino Tahirovic                                           *
 *                                                                        *
 * The purpose of this module is to test the reconstruction of            *
 * the particles with ARICH.                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <arich/modules/myArich/ARICHAnalysisModule.h>

#include <framework/core/ModuleManager.h>

// Hit classes
#include <tracking/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <arich/dataobjects/ARICHLikelihood.h>
#include <arich/dataobjects/ARICHAeroHit.h>
#include <generators/dataobjects/MCParticle.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TVector3.h>


using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ARICHAnalysis)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ARICHAnalysisModule::ARICHAnalysisModule() : Module(),
    m_eventNo(0),
    m_trackNo(-1),
    file(new TFile),
    tree(new TTree),
    m_chi2(0),
    m_pdg(0),
    m_charge(999),
    m_flag(-1),
    m_daughter(-1),
    m_lifetime(-1),
    m_detPhotons(0)

  {
    // set module description (e.g. insert text)
    setDescription("The module saves variables needed for performance analysis, such as postion and momentum of the hit, likelihoods for hypotheses and number of photons.");
    setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

    // Add parameters
    addParam("outputFile", m_outputFile, "ROOT output file name", string("extArichTest.root"));
    addParam("inputTrackType", m_inputTrackType, "Input tracks switch: tracking (0) or AeroHit (1)", 0);

    //    addParam("InputName", m_inName, "some name",string(""));
    //    vector<string> defaultList;
    //    addParam("ListName", m_list, "some list of names", defaultList);
    //    addParam("ModuleType", m_type, "some int number", 0);

    for (int j = 0; j < 3; j++) {
      m_decayVertex[j] = 0;
      m_truePosition[j] = 0;
      m_position[j] = 0;
      m_trueMomentum[j] = 0;
      m_momentum[j] = 0;
    };
    for (int k = 0; k < 5; k++) {
      m_logl[k] = 0;
      m_expPhotons[k] = 0;
    };

  }

  ARICHAnalysisModule::~ARICHAnalysisModule()
  {
  }

  void ARICHAnalysisModule::initialize()
  {
    B2DEBUG(50, "Initialize");

    char filename[m_outputFile.size() + 1];
    strcpy(filename, m_outputFile.c_str());
    file = new TFile(filename, "RECREATE");

    tree = new TTree("tree", "ext at ARICH");
    tree->Branch("m_eventNo", &m_eventNo, "m_eventNo/I");
    tree->Branch("m_trackNo", &m_trackNo, "m_trackNo/I");
    tree->Branch("m_chi2", &m_chi2, "m_chi2/F");
    tree->Branch("m_pdg", &m_pdg, "m_pdg/I");
    tree->Branch("m_charge", &m_charge, "m_charge/I");
    tree->Branch("m_flag", &m_flag, "m_flag/I");
    tree->Branch("m_daughter", &m_daughter, "m_daughter/I");
    tree->Branch("m_lifetime", &m_lifetime, "m_lifetime/F");
    tree->Branch("m_decayVertex", &m_decayVertex, "x/F:y/F:z/F");
    tree->Branch("m_logl", &m_logl, "e/F:mu/F:pi/F:K/F:p/F");
    tree->Branch("m_detPhotons", &m_detPhotons, "m_detPhotons/I");
    tree->Branch("m_expPhotons", &m_expPhotons, "e/F:mu/F:pi/F:K/F:p/F");
    tree->Branch("m_truePosition", &m_truePosition, "x/F:y/F:z/F");
    tree->Branch("m_position", &m_position, "x/F:y/F:z/F");
    tree->Branch("m_trueMomentum", &m_trueMomentum, "x/F:y/F:z/F");
    tree->Branch("m_momentum", &m_momentum, "x/F:y/F:z/F");

  }

  void ARICHAnalysisModule::beginRun()
  {
    B2DEBUG(50, "BeginRun");
  }

  void ARICHAnalysisModule::event()
  {
    m_eventNo++;
    B2DEBUG(50, "Event number: " << m_eventNo);

    if (m_inputTrackType == 0) {

      // Input particles
      StoreArray<ARICHAeroHit> aeroHits;
      if (!aeroHits.isValid()) return;

      m_trackNo = aeroHits.getEntries();
      B2DEBUG(50, "No. of hits " << m_trackNo);

      for (int iHit = 0; iHit < aeroHits.getEntries(); iHit++) {
        // Get the track and related DataStore entries
        const ARICHAeroHit* aeroHit = aeroHits[iHit];
        const MCParticle* particle = DataStore::getRelated<MCParticle>(aeroHit);
        if (!particle) {
          B2DEBUG(50, "No MCParticle for AeroHit!");
          B2DEBUG(50, "Particle momentum: " << aeroHit->getMomentum().Mag());
          B2DEBUG(50, "Particle PDG: " << aeroHit->getPDG());
          continue;
        }
        if (!(particle->hasStatus(MCParticle::c_PrimaryParticle) and particle->hasStatus(MCParticle::c_StableInGenerator))) continue;
        m_pdg = particle->getPDG();
        B2DEBUG(50, "PDG " << m_pdg);
        m_charge = particle->getCharge();
        m_flag = particle->getStatus();
        B2DEBUG(50, "MC flag: " << m_flag << " flag & 1: " << (m_flag & 1));
        vector<MCParticle*> daughters = particle->getDaughters();
        if (daughters.size()) m_daughter = daughters[0]->getPDG();
        B2DEBUG(50, "daughter pdg " << m_daughter);
        if (!(m_flag & 1)) B2DEBUG(50, "m_flag & 1: " << (m_flag & 1));
        m_lifetime = particle->getLifetime();
        TVector3 decayVertex = particle->getDecayVertex();
        m_decayVertex[0] = decayVertex.X();
        m_decayVertex[1] = decayVertex.Y();
        m_decayVertex[2] = decayVertex.Z();

        const ARICHLikelihood* arich = DataStore::getRelated<ARICHLikelihood>(aeroHit);
        if (!arich) {
          B2DEBUG(50, "No ARICHLikelihood related to AeroHit");
          continue;
        }

        const Track* track = DataStore::getRelated<Track>(particle);
        if (!track) {
          B2DEBUG(50, "No Track relation to MCParticle");
          B2DEBUG(50, "PDG " << m_pdg);
          B2DEBUG(50, "Generated momentum: " << particle->getMomentum().Mag());
          continue;
        }
        const TrackFitResult* fitResult = track->getTrackFitResult(Const::pion);
        m_chi2 = fitResult->getPValue();
        B2DEBUG(50, "Track.Chi2 probability " << m_chi2);

        const ExtHit* extHit = DataStore::getRelated<ExtHit>(arich);
        if (!extHit) {
          B2DEBUG(50, "No extHit related with Track");
          continue;
        }

        B2DEBUG(50, "ExtHit particle with pion ARICH likelihood: " << arich->getLogL_pi() <<
                " and PDG: " << extHit->getPdgCode());
        B2DEBUG(100, "aeroHit.x " << aeroHit->getPosition().X());
        B2DEBUG(100, "extHit.x " << extHit->getPosition().X());

        // Get the values
        m_truePosition[0] = aeroHit->getPosition().X();
        m_truePosition[1] = aeroHit->getPosition().Y();
        m_truePosition[2] = aeroHit->getPosition().Z();
        m_position[0] = extHit->getPosition().X();
        m_position[1] = extHit->getPosition().Y();
        m_position[2] = extHit->getPosition().Z();

        m_trueMomentum[0] = aeroHit->getMomentum().X();
        m_trueMomentum[1] = aeroHit->getMomentum().Y();
        m_trueMomentum[2] = aeroHit->getMomentum().Z();
        m_momentum[0] = extHit->getMomentum().X();
        m_momentum[1] = extHit->getMomentum().Y();
        m_momentum[2] = extHit->getMomentum().Z();

        m_logl[0] = arich->getLogL_e();
        m_logl[1] = arich->getLogL_mu();
        m_logl[2] = arich->getLogL_pi();
        m_logl[3] = arich->getLogL_K();
        m_logl[4] = arich->getLogL_p();
        m_detPhotons = arich->getNphot();
        m_expPhotons[0] = arich->getNphot_e();
        m_expPhotons[1] = arich->getNphot_mu();
        m_expPhotons[2] = arich->getNphot_pi();
        m_expPhotons[3] = arich->getNphot_K();
        m_expPhotons[4] = arich->getNphot_p();

        tree->Fill();
      }
    } else if (m_inputTrackType == 1) {

      // Input particles
      StoreArray<ARICHAeroHit> aeroHits;
      if (!aeroHits.isValid()) return;

      m_trackNo = aeroHits.getEntries();
      B2DEBUG(50, "No. of hits on aerogel " << m_trackNo);

      for (int iHit = 0; iHit < m_trackNo; iHit++) {
        const ARICHAeroHit* aeroHit = aeroHits[iHit];
        const MCParticle* particle = DataStore::getRelated<MCParticle>(aeroHit);
        if (!particle) {
          B2DEBUG(50, "No valid relation AeroHit -> MCParticle");
          continue;
        }
        m_pdg = particle->getPDG();
        B2DEBUG(50, "PDG " << m_pdg);
        m_charge = particle->getCharge();
        m_flag = particle->getStatus(MCParticle::c_PrimaryParticle);
        B2DEBUG(50, "MC flag: primary " << m_flag);
        const ARICHLikelihood* arich = DataStore::getRelated<ARICHLikelihood>(aeroHit);
        if (!arich) continue;

        m_logl[0] = arich->getLogL_e();
        m_logl[1] = arich->getLogL_mu();
        m_logl[2] = arich->getLogL_pi();
        m_logl[3] = arich->getLogL_K();
        m_logl[4] = arich->getLogL_p();
        m_detPhotons = arich->getNphot();
        m_expPhotons[0] = arich->getNphot_e();
        m_expPhotons[1] = arich->getNphot_mu();
        m_expPhotons[2] = arich->getNphot_pi();
        m_expPhotons[3] = arich->getNphot_K();
        m_expPhotons[4] = arich->getNphot_p();

        B2DEBUG(110, "aeroHit.x " << aeroHit->getPosition().X());
        m_truePosition[0] = aeroHit->getPosition().X();
        m_truePosition[1] = aeroHit->getPosition().Y();
        m_truePosition[2] = aeroHit->getPosition().Z();

        m_trueMomentum[0] = aeroHit->getMomentum().X();
        m_trueMomentum[1] = aeroHit->getMomentum().Y();
        m_trueMomentum[2] = aeroHit->getMomentum().Z();

        tree->Fill();
      }
    }

  }


  void ARICHAnalysisModule::endRun()
  {
  }

  void ARICHAnalysisModule::terminate()
  {
    file->Write();
    file->Close();
    B2INFO("ARICHAnalysisModule: finished. Output " << m_outputFile << " written.");
  }

  void ARICHAnalysisModule::printModuleParams() const
  {
  }



} // end Belle2 namespace

