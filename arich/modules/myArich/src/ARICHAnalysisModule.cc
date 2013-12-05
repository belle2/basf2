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
    m_eventNo(-1),
    m_trackNo(-1),
    file(new TFile),
    tree(new TTree),
    m_chi2(0),
    m_pdg(0),
    m_primary(-1),
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

    for (int i = 0; i < 3; i++) {
      m_truePosition[i] = 0;
      m_position[i] = 0;
      m_trueMomentum[i] = 0;
      m_momentum[i] = 0;
    };
    for (int i = 0; i < 5; i++) {
      m_logl[i] = 0;
      m_expPhotons[i] = 0;
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
    tree->Branch("m_primary", &m_primary, "m_primary/I");
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
    B2DEBUG(50, "Event" << m_eventNo);

    if (m_inputTrackType == 0) {

      // Input particles
      StoreArray<Track> tracks;
      if (!tracks.isValid()) return;

      m_trackNo = tracks.getEntries();
      B2DEBUG(50, "No. of tracks " << m_trackNo);

      for (int iTrack = 0; iTrack < tracks.getEntries(); iTrack++) {
        // Get the track and related DataStore entries
        const Track* track = tracks[iTrack];
        const TrackFitResult* fitResult = track->getTrackFitResult(Const::pion);
        m_chi2 = fitResult->getPValue();
        B2DEBUG(50, "Track.Chi2 probability " << m_chi2);
        const MCParticle* particle = DataStore::getRelated<MCParticle>(track);
        if (!particle) continue;
        m_pdg = particle->getPDG();
        B2DEBUG(50, "PDG " << m_pdg);
        m_primary = particle->getStatus(MCParticle::c_PrimaryParticle);
        B2DEBUG(50, "MC flag: primary " << m_primary);
        const ARICHLikelihood* arich = DataStore::getRelated<ARICHLikelihood>(track);
        if (!arich) continue;
        const ARICHAeroHit* aeroHit = DataStore::getRelated<ARICHAeroHit>(arich);
        if (!aeroHit) continue;
        const ExtHit* extHit = DataStore::getRelated<ExtHit>(arich);
        if (!extHit) continue;

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
        m_primary = particle->getStatus(MCParticle::c_PrimaryParticle);
        B2DEBUG(50, "MC flag: primary " << m_primary);
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

