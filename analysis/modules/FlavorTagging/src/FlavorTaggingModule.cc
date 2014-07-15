/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Fernando Abudinen, Moritz Gelb, Pablo Goldenzweig,       *
 *               Luigi Li Gioi                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/



#include <analysis/modules/FlavorTagging/FlavorTaggingModule.h>
#include <analysis/TMVAInterface/Teacher.h>
#include <analysis/TMVAInterface/Expert.h>
#include <analysis/ContinuumSuppression/ContinuumSuppression.h>


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
#include <analysis/dataobjects/ContinuumSuppression.h>

// utilities
#include <analysis/utility/PCmsLabTransform.h>

// MC particle
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <iostream>


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

    addParam("usingMode", m_mode, "Mode of Use of the Module (0 for Teacher, 1 for Expert) ", 0);
    addParam("listName", m_listName, "name of particle list", string(""));
    //addParam("trainingMethod", m_training, "Specifify your method"); //please one default, please several targets
    //addParam("trainingVariables", m_variables, "Specify your variables for training");
    //addParam("trainingTarget", m_target, "Specify your target");
    //addParam("output_training_filename", m_trainingoutputfilename, "Training Output File Name")



//     addParam("ConfidenceLevel", m_confidenceLevel,
//              "required confidence level of fit to keep particles in the list", 0.001);
    //addParam("EventType", m_EventType, "Btag decay type", std::string(""));

  }

  FlavorTaggingModule::~FlavorTaggingModule()
  {
  }

  void FlavorTaggingModule::initialize()
  {
    //TO DO: choose method etc in steering file also adapt design: see teacher.py
    StoreArray<MCParticle>::optional();
    //store array for dummy particle:
    StoreArray<Particle>::required(); //if we don't need them anymore, ok. If not, we have to adapt it for each category.
    StoreArray<ContinuumSuppression>::registerPersistent();
    RelationArray::registerPersistent<Particle, ContinuumSuppression>();

    //training definition for Kaons
    std::string target_Kaon = "isKaon";
    std::vector<std::string> variables_Kaon = {"charge", "p_CMS", "cosTheta", "K_vs_piid"};
    TMVAInterface::Method method_Kaon_1("FastBDT", "Plugin", "H:!V:CreateMVAPdfs:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3", variables_Kaon);
    TMVAInterface::Method method_Kaon_2("Fisher", "Fisher", "H:!V:Fisher:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10", variables_Kaon);
    TMVAInterface::Method method_Kaon_3("BDTGradient", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100:BoostType=Grad:Shrinkage=0.10:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=200:MaxDepth=2", variables_Kaon);
    TMVAInterface::Method method_Kaon_4("PDEFoamBoost", "PDEFoam", "!H:!V:CreateMVAPdfs:Boost_Num=10:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T", variables_Kaon);
    teacher_Kaon = new TMVAInterface::Teacher("training_Kaon", ".", target_Kaon, {method_Kaon_1, method_Kaon_2, method_Kaon_3, method_Kaon_4});

    //training definition for Slow Pions
    std::string target_SlowPion = "isSlowPion";
    std::vector<std::string> variables_SlowPion = {"charge", "p", "cosTheta", "pi_vs_edEdxid", "a_thrust"};
    TMVAInterface::Method method_SlowPion_1("FastBDT", "Plugin", "H:!V:CreateMVAPdfs:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3", variables_SlowPion);
    TMVAInterface::Method method_SlowPion_2("Fisher", "Fisher", "H:!V:Fisher:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10", variables_SlowPion);
    TMVAInterface::Method method_SlowPion_3("BDTGradient", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100:BoostType=Grad:Shrinkage=0.10:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=200:MaxDepth=2", variables_SlowPion);
    TMVAInterface::Method method_SlowPion_4("PDEFoamBoost", "PDEFoam", "!H:!V:CreateMVAPdfs:Boost_Num=10:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T", variables_SlowPion);
    teacher_SlowPion = new TMVAInterface::Teacher("training_SlowPion", ".", target_SlowPion, {method_SlowPion_1, method_SlowPion_2, method_SlowPion_3, method_SlowPion_4});

    //further expert definitions
    //expert_Kaon = new TMVAInterface::Expert("training_Kaon", ".","Fisher",1);
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
        //Muon_Cathegory();
        Kaon_Category();
        SlowPion_Category();
      };
      if (!ok) toRemove.push_back(i);
    }
    plist->removeParticles(toRemove);

  }

  bool FlavorTaggingModule::getTagObjects(Particle* Breco)
  {

    const RestOfEvent* roe = Breco->getRelatedTo<RestOfEvent>();

    if (roe) {
      m_tagTracks = roe->getTracks();
      //m_Clusters = roe-> getECLClusters();

      //BUG!!! RoE Dataobject error!!!!
      //m_tagKLMClusters = roe-> getKLMClusters();

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

    for (unsigned int i = 0; i < m_tagTracks.size(); i++) {

      const Track* tracki = m_tagTracks[i];

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

//Kaon Category
  bool FlavorTaggingModule::Kaon_Category()
  {
    //Bbar-> D+(K- 2pi+) K-

    //for storing dummy-particle in datastore - otherwise we cannot set a relation
    StoreArray<Particle> particles;

    for (unsigned int i = 0; i < m_tagTracks.size(); i++) {

      const Track* tracki = m_tagTracks[i];

      const TrackFitResult* trakiRes = NULL;

      if (tracki) trakiRes = tracki->getTrackFitResult(Const::kaon); //
      if (!trakiRes) continue;

      //make a dummy particle from track
      Particle* p = particles.appendNew(tracki, Const::kaon);

      //add a relation: dummy particle related to its mcparticle and pid likelihood - now we can use Variables!
      const PIDLikelihood* pid = tracki->getRelated<PIDLikelihood>();
      const MCParticle* mcParticle = tracki->getRelated<MCParticle>();


      addContinuumSuppression(p);
      //const ContinuumSuppression* continuumSuppression = p->getRelated<ContinuumSuppression>();
      if (pid) p->addRelationTo(pid);
      if (mcParticle) p->addRelationTo(mcParticle);

      teacher_Kaon->addSample(p); //endrun

      //p->addExtraInfo("signalProbabilityKaon", expert_Kaon->analyse(p));
    }

    return true;
  }

//Slow Pion Category
  bool FlavorTaggingModule::SlowPion_Category()
  {
    //B->  D*-(Dbar0 pion-) X

    //for storing dummy-particle in datastore - otherwise we cannot set a relation
    StoreArray<Particle> particles;

    for (unsigned int i = 0; i < m_tagTracks.size(); i++) {

      const Track* tracki = m_tagTracks[i];
      const TrackFitResult* trakiRes = NULL;

      if (tracki) trakiRes = tracki->getTrackFitResult(Const::pion);
      if (!trakiRes) continue;

      //make a dummy particle from track
      Particle* p = particles.appendNew(tracki, Const::pion);

      //add a relation: dummy particle related to its mcparticle and pid likelihood - now we can use Variables!
      const PIDLikelihood* pid = tracki->getRelated<PIDLikelihood>();
      const MCParticle* mcParticle = tracki->getRelated<MCParticle>();

      addContinuumSuppression(p);
      //const ContinuumSuppression* continuumSuppression = tracki->getRelated<ContinuumSuppression>();
      if (pid) p->addRelationTo(pid);
      if (mcParticle) p->addRelationTo(mcParticle);

      teacher_SlowPion->addSample(p); //endrun
      //p->addExtraInfo("signalProbabilitySlowPion", expert_SlowPion->analyse(p));
      //p->getExtraInfo("signalProbabilitySlowPion");
    }

    return true;
  }

//TO DO: add more categories...

  void FlavorTaggingModule::endRun()
  {

  }

  void FlavorTaggingModule::terminate()
  {
    //perform training
    //teacher_Kaon->train();
    //teacher_SlowPion->train();

    delete teacher_Kaon;
    delete teacher_SlowPion;

  }

} // end Belle2 namespace
