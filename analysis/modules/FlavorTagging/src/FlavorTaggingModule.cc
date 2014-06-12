/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Fernando Abudinen, Pablo Goldenzweig, Luigi Li Gioi      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/



#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>

#include <analysis/modules/FlavorTagging/FlavorTaggingModule.h>
#include <analysis/TMVAInterface/Teacher.h>


// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/core/ModuleManager.h>


// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/RestOfEvent.h>

// utilities
#include <analysis/utility/PCmsLabTransform.h>

// MC particle
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>


using namespace std;

namespace Belle2 {


  //-----------------------------------------------------------------
  //                 Register the Module
  //-----------------------------------------------------------------
  REG_MODULE(FlavorTagging)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  FlavorTaggingModule::FlavorTaggingModule() : Module() , m_mode(0)//, m_fitPval(0)
  {
    // Set module properties
    setDescription("Flavor tagging neural Networks for modular analysis");

    // Parameter definitions

    addParam("UsingMode", m_mode, "Mode of Use of the Module (0 for Teacher, 1 for Expert) ", 0);
    addParam("ListName", m_listName, "name of particle list", string(""));




//     addParam("ConfidenceLevel", m_confidenceLevel,
//              "required confidence level of fit to keep particles in the list", 0.001);
    //addParam("EventType", m_EventType, "Btag decay type", std::string(""));

  }

  FlavorTaggingModule::~FlavorTaggingModule()
  {
  }

  void FlavorTaggingModule::initialize()
  {
  }

  void FlavorTaggingModule::beginRun()
  {
  }

  void FlavorTaggingModule::event()
  {

    StoreObjPtr<ParticleList> plist(m_listName);
    if (!plist) {
      B2ERROR("ParticleList " << m_listName << " not found");
      return;
    }

    StoreArray<Particle> Particles(plist->getParticleCollectionName());

    std::vector<unsigned int> toRemove;
    for (unsigned i = 0; i < plist->getListSize(); i++) {
      Particle* particle =  plist->getParticle(i);
      bool ok = getTagObjects(particle);
      if (ok) {
        getMC_PDGcodes();
        Muon_Cathegory();
      };
      if (!ok) toRemove.push_back(i);
    }
    plist->removeParticles(toRemove);

  }

  bool FlavorTaggingModule::getTagObjects(Particle* Breco)
  {

    const RestOfEvent* roe = Breco->getRelatedTo<RestOfEvent>();

    if (roe) {
      tagTracks = roe->getTracks();
      tagECLClusters = roe-> getECLClusters();
      tagKLMClusters = roe-> getKLMClusters();

    } else {
      return false;
    }

    return true;
  }

  bool FlavorTaggingModule::getMC_PDGcodes()// Print out PDG codes of MC Simulation
  {
    StoreArray<MCParticle> mcParticles;
    for (int qu = 0; qu < mcParticles.getEntries(); qu++) {
      if ((mcParticles[ qu ]-> getPDG()) == 511) {
        const std::vector<Belle2::MCParticle*>  Daughters_of_B0 = mcParticles[ qu ]->getDaughters();
//    cout << "PDGCODES of B0 Daughters:" << endl;
//          for (int dau = 0; dau < Daughters_of_B0.size(); dau++) {
//            cout << "PDGCODE:" << Daughters_of_B0[dau]->getPDG() << endl;
//          }

      }
      if ((mcParticles[ qu ]-> getPDG()) == -511) {
        const std::vector<Belle2::MCParticle*>  Daughters_of_anti_B0 = mcParticles[ qu ]->getDaughters();
//    cout << "PDGCODES of Anti-B0 Daughters:" << endl;
//          for (int dau = 0; dau < Daughters_of_anti_B0.size(); dau++) {
//            cout << "PDGCODE:" << Daughters_of_anti_B0[dau]->getPDG() << endl;
//          }
      } else {
        return false;
      }
    }
    return true;

  }

  //Variables needed for Muon Cathegory

  double Charge(Track* track)
  {
    const TrackFitResult* TrackRes = track->getTrackFitResult(Const::muon);
    return TrackRes -> getChargeSign();
  }

  double p_cms(Track* track)
  {
    const TrackFitResult* TrackRes = track->getTrackFitResult(Const::muon);
    TLorentzVector p_lab(TrackRes->getMomentum(), TrackRes -> getParticleType().getMass());
    PCmsLabTransform T;
    TLorentzVector p_cms_vec = T.rotateLabToCms() * p_lab;
    return p_cms_vec.Vect().Mag();
  }

  double Theta_Lab(Track* track)
  {
    const TrackFitResult* TrackRes = track->getTrackFitResult(Const::muon);
    return TrackRes -> getCotTheta();
  }

  double PID_MonteCarlo(Track* track)
  {
    const MCParticle* mcParticle = track->getRelated<MCParticle>();
    if ((TMath::Abs(mcParticle->getPDG()) == 13) && (TMath::Abs(mcParticle->getMother()->getPDG()) == 511)) {
      return 1.0;
    } else return 0.0;
  }

  double PID_Likelihood(Track* track)
  {
    const PIDLikelihood* pid_Likelihood = track->getRelated<PIDLikelihood>();
    return pid_Likelihood->getProbability(Const::muon, Const::pion, Const::PIDDetectorSet::set());
  }

  bool FlavorTaggingModule::Muon_Cathegory()
  {
    //B-> Mu + Anti-v + X
//     TLorentzVector momX; //Momentum of X in CMS-System
//     TLorentzVector momMu;  //Momentum of Mu in CMS-System
//     TLorentzVector momMiss;  //Momentum of Anti-v  in CMS-System
//
//
//     double p_cms_miss;
//     double Cos_Theta_miss;
//     double M_recoil;
//     double E_W_90;
//

    for (unsigned int i = 0; i < tagTracks.size(); i++) {

      const Track* tracki = tagTracks[i];

      const TrackFitResult* trakiRes = NULL;

      if (tracki) trakiRes = tracki->getTrackFitResult(Const::muon);

//        if (TrackRes){}


//        TVector3 mom;
//       momMiss = -(momX+momMu);
//       p_cms_miss = momMiss.Vect().Mag();
//       Cos_Theta_miss = momMu.Angle(momMiss.Vect());
//       M_recoil = momX.M();



      if (!trakiRes) continue;


    }
    return true;
  }



  void FlavorTaggingModule::endRun()
  {
  }

  void FlavorTaggingModule::terminate()
  {
  }

} // end Belle2 namespace
