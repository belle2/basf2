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
#include <analysis/modules/TMVATeacher/TMVATeacherModule.h>
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
        Input_Values_Muon();
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

  bool FlavorTaggingModule::Input_Values_Muon()
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



//     for (unsigned int i = 0; i < tagTracks.size(); i++) {
//
//       const Track* track1 = tagTracks[i];
//       const MCParticle* mcParticle = track1->getRelated<MCParticle>();
//       const PIDLikelihood* pid_Likelihood = track1->getRelated<PIDLikelihood>();
//       const TrackFitResult* trak1Res = NULL;
//
//
//       if (track1) trak1Res = track1->getTrackFitResult(Const::muon); //Const::ChargedStable(mcParticle->getPDG())
//
//       TVector3 mom;
//
//
//
//       double charge;           //Variable1
//       double p_cms;   //Variable2
//       double ThetaLab;    //Variable3
//       double PID_Likelihood;
//
//       double PID;
//
//       if (trak1Res) {
//
//
// //  Belle2::Const::ParticleType Muon= trak1Res->getParticleType();
//
//
//         mom = trak1Res->getMomentum();
//
//         charge = trak1Res->getChargeSign(); //Variable1
//
//
// //   float Particlemass = TDatabasePDG::Instance()->GetParticle(13)->Mass();
//
//         TLorentzVector p_lab(mom, TDatabasePDG::Instance()->GetParticle(13)->Mass());
//         PCmsLabTransform T;
//         TLorentzVector p_cms_vec = T.rotateLabToCms() * p_lab;
//         p_cms = p_cms_vec.Vect().Mag(); //Variable2
//
//         ThetaLab = trak1Res-> getCotTheta(); //Variable3
//
//  PID_Likelihood = pid_Likelihood->getProbability(Const::muon, Const::pion, Const::PIDDetectorSet::set());
//
//
//         if ((TMath::Abs(mcParticle->getPDG()) == 13) && (TMath::Abs(mcParticle->getMother()->getPDG()) == 511)) {
//
//    PID=1;
//
//  }
//
//  else PID=0;
//
//
//
//       }
//
// //       momMiss = -(momX+momMu);
// //       p_cms_miss = momMiss.Vect().Mag();
// //       Cos_Theta_miss = momMu.Angle(momMiss.Vect());
// //       M_recoil = momX.M();
//
// //       for (unsigned int i = 0; i < tagTracks.size(); i++) {
// //
// //
// //
// //
// //
// //       }
//
//
//       if (!trak1Res) continue;
//
//
//     }




    return true;




  }



  void FlavorTaggingModule::endRun()
  {
  }

  void FlavorTaggingModule::terminate()
  {
  }

} // end Belle2 namespace
