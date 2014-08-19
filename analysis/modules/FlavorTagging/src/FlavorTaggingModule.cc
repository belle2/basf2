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
#include <framework/dataobjects/EventMetaData.h>

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
#include <analysis/utility/MCMatching.h>

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
    addParam("usingMode", m_mode, "Mode of Use of the Module (0 for Teacher in Track Level, 1 for Expert in Track Level,"
             " 2 for Teacher in Event Level, 3 for Expert in Event Level,"
             " 4 Particle Combiner Teacher, 6 for Particle Combiner Expert ) ", 0);
    addParam("listName", m_listName, "name of particle list", string(""));

    static const char* localdir = getenv("BELLE2_LOCAL_DIR");
    m_workingDirectory = string(localdir) + string("/analysis/modules/FlavorTagging/TrainedMethods");

//   Settings for Teaching
    m_prepareOption = "SplitMode=Alternate:!V";//:nTrain_Signal=10000:nTest_Signal=10000";

    //training definition for Muons
//  Teacher in Track Level
    m_methodPrefix_Muon_TL = "TMVA_Muon_TL";
    m_target_Muon_TL = "isMuonFromB";
    m_variables_Muon_TL = {"charge", "p_CMS", "cosTheta", "muid"}; // input variables for the TMVA method */P.
    tuple<std::string, std::string, std::string> method_Muon_TL_0("MLP", "MLP", "H:!V:VarTransform=Norm:NCycles=10000:HiddenLayers=3*N:NeuronType=tanh:TrainingMethod=BP:TestRate=10:EpochMonitoring=False:Sampling=1:SamplingEpoch=1:BPMode=sequential:ConvergenceImprove=1e-30:ConvergenceTests=-1:UseRegulator=False:UpdateLimit=10000:CalculateErrors=False");
    tuple<std::string, std::string, std::string> method_Muon_TL_1("FastBDT", "Plugin", "H:!V:CreateMVAPdfs:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3");
    tuple<std::string, std::string, std::string> method_Muon_TL_2("Fisher", "Fisher", "H:!V:Fisher:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10");
    tuple<std::string, std::string, std::string> method_Muon_TL_3("BDTGradient", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100:BoostType=Grad:Shrinkage=0.10:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=200:MaxDepth=2");
    tuple<std::string, std::string, std::string> method_Muon_TL_4("PDEFoamBoost", "PDEFoam", "!H:!V:CreateMVAPdfs:Boost_Num=10:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T");
    m_methods_Muon_TL = {method_Muon_TL_0, method_Muon_TL_1, method_Muon_TL_2, method_Muon_TL_3, method_Muon_TL_4};
    m_factoryOption_Muon_TL = "!V:Color=True:Silent=False:DrawProgressBar=True:AnalysisType=Classification";
    m_teacher_Muon_TL = nullptr;
// Expert in Track Level
    m_methodName_Muon_TL = "MLP";
    m_signalCluster_Muon_TL = 1;
    m_signalToBackgroundRatio = -1.0f;
    m_expert_Muon_TL = nullptr;
    m_signalProbabilityName = "y_Class_Track_";
    m_signalRealValueName = "y_MC_Track_";
    N_Events = 0;
    N_noParticle = 0;
    N_Background = 0;
    N_noTracks = 0;
    N_Target_in_Breco = 0;
    N_Signal = 0;
    N_Corr = 0;
    N_Wrong = 0;
    N_Wrong_rightFlavor = 0;
//  Teacher in Event Level
    m_methodPrefix_Muon_EL = "TMVA_Muon_EL";
    m_target_Muon_EL = "BtagMCFlavor";
    m_variables_Muon_EL = {"BtagClassFlavor", "p_CMS_missing", "cosTheta_missing",  "mRecoilBtag"}; // input variables for the TMVA method */P.
    tuple<std::string, std::string, std::string> method_Muon_EL_0("MLP", "MLP", "H:!V:VarTransform=Norm:NCycles=10000:HiddenLayers=3*N:NeuronType=tanh:TrainingMethod=BP:TestRate=10:EpochMonitoring=False:Sampling=1:SamplingEpoch=1:BPMode=sequential:ConvergenceImprove=1e-30:ConvergenceTests=-1:UseRegulator=False:UpdateLimit=10000:CalculateErrors=False");
    tuple<std::string, std::string, std::string> method_Muon_EL_1("FastBDT", "Plugin", "H:!V:CreateMVAPdfs:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3");
    tuple<std::string, std::string, std::string> method_Muon_EL_2("Fisher", "Fisher", "H:!V:Fisher:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10");
    tuple<std::string, std::string, std::string> method_Muon_EL_3("BDTGradient", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100:BoostType=Grad:Shrinkage=0.10:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=200:MaxDepth=2");
    tuple<std::string, std::string, std::string> method_Muon_EL_4("PDEFoamBoost", "PDEFoam", "!H:!V:CreateMVAPdfs:Boost_Num=10:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T");
    m_methods_Muon_EL = {method_Muon_EL_0, method_Muon_EL_1, method_Muon_EL_2, method_Muon_EL_3, method_Muon_EL_4};
    m_factoryOption_Muon_EL = "!V:Color=True:Silent=False:DrawProgressBar=True:AnalysisType=Classification";
    m_teacher_Muon_EL = nullptr;

    //training definition for Kaons
//  Teacher in Track Level
    m_methodPrefix_Kaon_TL = "TMVA_Kaon_TL";
    m_target_Kaon_TL = "isKaonFromB";
    m_variables_Kaon_TL = {"charge", "p_CMS", "cosTheta", "K_vs_piid"};
    tuple<std::string, std::string, std::string> method_Kaon_TL_0("MLP", "MLP", "H:!V:VarTransform=Norm:NCycles=10000:HiddenLayers=3*N:NeuronType=tanh:TrainingMethod=BP:TestRate=10:EpochMonitoring=False:Sampling=1:SamplingEpoch=1:BPMode=sequential:ConvergenceImprove=1e-30:ConvergenceTests=-1:UseRegulator=False:UpdateLimit=10000:CalculateErrors=False");
    tuple<std::string, std::string, std::string> method_Kaon_TL_1("FastBDT", "Plugin", "H:!V:CreateMVAPdfs:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3");
    tuple<std::string, std::string, std::string> method_Kaon_TL_2("Fisher", "Fisher", "H:!V:Fisher:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10");
    tuple<std::string, std::string, std::string> method_Kaon_TL_3("BDTGradient", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100:BoostType=Grad:Shrinkage=0.10:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=200:MaxDepth=2");
    tuple<std::string, std::string, std::string> method_Kaon_TL_4("PDEFoamBoost", "PDEFoam", "!H:!V:CreateMVAPdfs:Boost_Num=10:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T");
    m_methods_Kaon_TL = {method_Kaon_TL_0, method_Kaon_TL_1, method_Kaon_TL_2, method_Kaon_TL_3, method_Kaon_TL_4};
    m_factoryOption_Kaon_TL = "!V:Color=True:Silent=False:DrawProgressBar=True:AnalysisType=Classification";
    m_teacher_Kaon_TL = nullptr;

    //training definition for Slow Pions
//  Teacher in Track Level
    m_methodPrefix_SlowPion_TL = "TMVA_SlowPion_TL";
    m_target_SlowPion_TL = "isSlowPionFromB";
    m_variables_SlowPion_TL = {"charge", "p", "cosTheta", "pi_vs_edEdxid", "cosTPTO"};
    tuple<std::string, std::string, std::string> method_SlowPion_TL_0("MLP", "MLP", "H:!V:VarTransform=Norm:NCycles=10000:HiddenLayers=3*N:NeuronType=tanh:TrainingMethod=BP:TestRate=10:EpochMonitoring=False:Sampling=1:SamplingEpoch=1:BPMode=sequential:ConvergenceImprove=1e-30:ConvergenceTests=-1:UseRegulator=False:UpdateLimit=10000:CalculateErrors=False");
    tuple<std::string, std::string, std::string> method_SlowPion_TL_1("FastBDT", "Plugin", "H:!V:CreateMVAPdfs:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3");
    tuple<std::string, std::string, std::string> method_SlowPion_TL_2("Fisher", "Fisher", "H:!V:Fisher:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10");
    tuple<std::string, std::string, std::string> method_SlowPion_TL_3("BDTGradient", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100:BoostType=Grad:Shrinkage=0.10:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=200:MaxDepth=2");
    tuple<std::string, std::string, std::string> method_SlowPion_TL_4("PDEFoamBoost", "PDEFoam", "!H:!V:CreateMVAPdfs:Boost_Num=10:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T");
    m_methods_SlowPion_TL = {method_SlowPion_TL_0, method_SlowPion_TL_1, method_SlowPion_TL_2, method_SlowPion_TL_3, method_SlowPion_TL_4};
    m_factoryOption_SlowPion_TL = "!V:Color=True:Silent=False:DrawProgressBar=True:AnalysisType=Classification";
    m_teacher_SlowPion_TL = nullptr;

    //further expert definitions
    //expert_Kaon = new TMVAInterface::Expert("training_Kaon", ".","Fisher",1);

  }

  FlavorTaggingModule::~FlavorTaggingModule()
  {
  }

  void FlavorTaggingModule::initialize()
  {

    //TO DO: choose method etc in steering file also adapt design: see teacher.py
    StoreArray<MCParticle>::optional();
    //store array for dummy particle:
    StoreArray<Particle> particles;
    particles.isRequired(); //if we don't need them anymore, ok. If not, we have to adapt it for each category.
    StoreArray<ContinuumSuppression> csArray;
    csArray.registerInDataStore();
    particles.registerRelationTo(csArray);

    if ((m_mode == 1) || (m_mode == 2)) {
      m_expert_Muon_TL = new TMVAInterface::Expert(m_methodPrefix_Muon_EL, m_workingDirectory, m_methodName_Muon_TL, m_signalCluster_Muon_TL);

    }

  }

  void FlavorTaggingModule::beginRun()
  {

    if (m_mode == 0) {

      std::vector<TMVAInterface::Method> methods_Muon_TL;
      for (auto & x : m_methods_Muon_TL) {
        methods_Muon_TL.push_back(TMVAInterface::Method(std::get<0>(x), std::get<1>(x), std::get<2>(x), m_variables_Muon_TL));
      }
      m_teacher_Muon_TL = new TMVAInterface::Teacher(m_methodPrefix_Muon_TL, m_workingDirectory, m_target_Muon_TL, methods_Muon_TL, false);

      std::vector<TMVAInterface::Method> methods_Kaon_TL;
      for (auto & x : m_methods_Kaon_TL) {
        methods_Kaon_TL.push_back(TMVAInterface::Method(std::get<0>(x), std::get<1>(x), std::get<2>(x), m_variables_Kaon_TL));
      }
      m_teacher_Kaon_TL = new TMVAInterface::Teacher(m_methodPrefix_Kaon_TL, m_workingDirectory, m_target_Kaon_TL, methods_Kaon_TL, false);

      std::vector<TMVAInterface::Method> methods_SlowPion_TL;
      for (auto & x : m_methods_SlowPion_TL) {
        methods_SlowPion_TL.push_back(TMVAInterface::Method(std::get<0>(x), std::get<1>(x), std::get<2>(x), m_variables_SlowPion_TL));
      }
      m_teacher_SlowPion_TL = new TMVAInterface::Teacher(m_methodPrefix_SlowPion_TL, m_workingDirectory, m_target_SlowPion_TL, methods_SlowPion_TL, false);

    }

    if (m_mode == 2) {
      std::vector<TMVAInterface::Method> methods_Muon_EL;
      for (auto & x : m_methods_Muon_EL) {
        methods_Muon_EL.push_back(TMVAInterface::Method(std::get<0>(x), std::get<1>(x), std::get<2>(x), m_variables_Muon_EL));
      }
      m_teacher_Muon_EL = new TMVAInterface::Teacher(m_methodPrefix_Muon_EL, m_workingDirectory, m_target_Muon_EL, methods_Muon_EL);
    }


  }

  void FlavorTaggingModule::event()
  {

    N_Events++;
    StoreObjPtr<ParticleList> plist(m_listName);
    if (plist->getListSize() == 0) N_noParticle++;
    if (!plist) {
      B2ERROR("ParticleList " << m_listName << " not found");
      return;
    }

    StoreArray<Particle> Particles(plist->getParticleCollectionName());

    std::vector<unsigned int> toRemove;
    if (plist -> getListSize() > 0) {
      for (unsigned i = 0; i < plist -> getListSize(); i++) {
        Particle* particle =  plist->getParticle(i);
        bool ok = getTagObjects(particle);
        bool mc_ok = ((MCMatching::getMCTruthStatus(particle, particle ->getRelated<MCParticle>()) == 0) && (TMath::Abs(particle -> getPDGCode()) == 511));
        if (ok) {
          Muon_Category(particle);
          Kaon_Category();
          SlowPion_Category();
        }
        if (!ok) toRemove.push_back(i);
        if ((!ok) && (mc_ok)) {
          N_noParticle++;
        }
        if (!(mc_ok)) toRemove.push_back(i);
      }
    }


    plist->removeParticles(toRemove);
  }

  bool FlavorTaggingModule::getTagObjects(Particle* Breco)
  {

    const RestOfEvent* roe = Breco->getRelatedTo<RestOfEvent>();

    if (roe) {
      m_tagTracks = roe->getTracks();
      m_tagECLClusters = roe-> getECLClusters();
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

//Muon Category
  bool FlavorTaggingModule::Muon_Category(Particle* Breco)
  {
    //B-> Mu + Anti-v + X

    //Variables neede for Event level
    TLorentzVector momX; //Momentum of X in CMS-System
    TLorentzVector momMu;  //Momentum of Mu in CMS-System
    TLorentzVector momMiss;  //Momentum of Anti-v  in CMS-System
    PCmsLabTransform T;

    float p_CMS_missing;
    float cosTheta_missing;
    float recoilMassBtag;
//      float E_W_90;


    StoreArray<Particle> particles;

    float Track_y_Class[m_tagTracks.size()];
    float Track_y_MC[m_tagTracks.size()];
    int TrackID;
    TrackID = 0;

    m_tagDummies.resize(m_tagTracks.size());

    for (unsigned int i = 0; i < m_tagTracks.size(); i++) {

      const Track* tracki = m_tagTracks[i];
      const Belle2::Particle* particle_i = particles.appendNew(tracki, Const::muon); //make a dummy particle from track
      const TrackFitResult* trackiRes = NULL;
      if (tracki) trackiRes = tracki->getTrackFitResult(Const::muon);

      //add a relation: dummy particle related to its mcparticle and pid likelihood - now we can use Variables!
      if (tracki->getRelated<PIDLikelihood>()) particle_i->addRelationTo(tracki->getRelated<PIDLikelihood>());
      if (tracki->getRelated<MCParticle>()) particle_i->addRelationTo(tracki->getRelated<MCParticle>());
      addContinuumSuppression(particle_i);
      m_tagDummies[i] = particle_i;




      if (m_mode == 0) {
        if (trackiRes) {
//  cout << "This is the Real Value= " << Variable::Manager::Instance().getVariable(m_target_Muon)->function(particle_i) << endl;
          m_teacher_Muon_TL->addSample(particle_i);
        } else continue;
      }


      if ((m_mode == 1) || (m_mode == 2)) {
        string TrackNumber = static_cast<ostringstream*>(&(ostringstream() << TrackID))->str();
        if (trackiRes) {

          float ClassValue = m_expert_Muon_TL->analyse(particle_i, m_signalToBackgroundRatio);
          Track_y_Class[i] = ClassValue;
//            cout << "This is the MLP Value= " << ClassValue << endl;
          Breco->addExtraInfo(m_signalProbabilityName + TrackNumber, ClassValue);
          float MCValue = Variable::Manager::Instance().getVariable(m_target_Muon_TL)->function(particle_i);
          Track_y_MC[i] = MCValue;
//              cout << "This is the Real Value= " << Variable::Manager::Instance().getVariable(m_target_Muon)->function(particle_i) << endl;
//              cout << "Track_y_MC=" << Track_y_MC[i] << endl;
          Breco->addExtraInfo(m_signalRealValueName + TrackNumber, MCValue);
          if (m_mode == 2) momX += T.rotateLabToCms() * particle_i->get4Vector();
          TrackID++;
        }
        if (!trackiRes) {
          Track_y_Class[i] = 0;
          Track_y_MC[i] = 0;
        }

      }

    }
    if (m_mode == 1) {
//       cout<< "Breco PDG" <<Breco-> getPDGCode() <<endl;
//       cout<< "BMC PDG" <<Breco-> getRelated<MCParticle>() ->getPDG() <<endl;
      Breco ->addExtraInfo("N_Tracks_in_Event", TrackID);
      if ((TrackID > 0) && (int(Track_y_MC[TMath::LocMax(m_tagTracks.size(), Track_y_MC)]) != 0)) {
//         cout << "Position of Found track=" <<  TMath::LocMax(m_tagTracks.size(), Track_y_Class) << "Value" <<  Track_y_Class[TMath::LocMax(m_tagTracks.size(), Track_y_Class)] << endl;
//         cout << "Position of Correct track=" <<  TMath::LocMax(m_tagTracks.size(), Track_y_MC) << " Value" << int(Track_y_MC[TMath::LocMax(m_tagTracks.size(), Track_y_MC)]) << endl;
        N_Signal++;
        if (TMath::LocMax(m_tagTracks.size(), Track_y_Class) == TMath::LocMax(m_tagTracks.size(), Track_y_MC)) { //If the track with the highest probability corresponds to target Muon then the event has been correctly classified
          N_Corr++;
        }  else N_Wrong++;
//         cout << "Charge of Found track=" <<  m_tagTracks[TMath::LocMax(m_tagTracks.size(), Track_y_Class)]->getTrackFitResult(Const::muon)-> getChargeSign() << endl;
//         cout << "Charge of Correct track=" <<  m_tagTracks[TMath::LocMax(m_tagTracks.size(), Track_y_MC)]->getTrackFitResult(Const::muon)-> getChargeSign() << endl;
        if ((TMath::LocMax(m_tagTracks.size(), Track_y_Class) != TMath::LocMax(m_tagTracks.size(), Track_y_MC))
            && (m_tagTracks[TMath::LocMax(m_tagTracks.size(), Track_y_Class)]->getTrackFitResult(Const::muon)-> getChargeSign() ==
                m_tagTracks[TMath::LocMax(m_tagTracks.size(), Track_y_MC)]->getTrackFitResult(Const::muon)-> getChargeSign())) {
          N_Wrong_rightFlavor++;
          Breco -> addExtraInfo("MC_PDG_of_Class_Track_Wrong_rightFlavor",  TMath::Abs(m_tagTracks[TMath::LocMax(m_tagTracks.size(), Track_y_Class)] ->getRelated<MCParticle>()-> getPDG()));
          Breco -> addExtraInfo("MC_PDG_of_Class_Track_Mother_Wrong_rightFlavor", TMath::Abs(m_tagTracks[TMath::LocMax(m_tagTracks.size(), Track_y_Class)] ->getRelated<MCParticle>()-> getMother()-> getPDG()));
        } else {
          Breco -> addExtraInfo("MC_PDG_of_Class_Track_Wrong_rightFlavor",  0);
          Breco -> addExtraInfo("MC_PDG_of_Class_Track_Mother_Wrong_rightFlavor", 0);
        }
        Breco ->addExtraInfo("Class_Flavor", m_tagTracks[TMath::LocMax(m_tagTracks.size(), Track_y_Class)]->getTrackFitResult(Const::muon)-> getChargeSign());
        Breco ->addExtraInfo("MC_Flavor", m_tagTracks[TMath::LocMax(m_tagTracks.size(), Track_y_MC)]->getTrackFitResult(Const::muon)-> getChargeSign());
        Breco ->addExtraInfo("MC_PDG_of_Class_Track", TMath::Abs(m_tagTracks[TMath::LocMax(m_tagTracks.size(), Track_y_Class)] ->getRelated<MCParticle>()-> getPDG()));
        Breco ->addExtraInfo("MC_PDG_of_Class_Track_Mother", TMath::Abs(m_tagTracks[TMath::LocMax(m_tagTracks.size(), Track_y_Class)] ->getRelated<MCParticle>()-> getMother()-> getPDG()));
        Breco ->addExtraInfo("Class_Prob",  Track_y_Class[TMath::LocMax(m_tagTracks.size(), Track_y_Class)]);
//  cout<< "OUT="<< Track_y_Class[TMath::LocMax(m_tagTracks.size(), Track_y_Class)]<<"   REAL="<<Track_y_MC[TMath::LocMax(m_tagTracks.size(), Track_y_MC)] <<endl;
        Breco ->addExtraInfo("PID_MonteCarlo", Track_y_MC[TMath::LocMax(m_tagTracks.size(), Track_y_MC)]);
      } else if ((TrackID > 0) && (int(Track_y_MC[TMath::LocMax(m_tagTracks.size(), Track_y_MC)]) == 0)) {
        N_Background++;
        Breco ->addExtraInfo("Class_Flavor", 0);
        Breco ->addExtraInfo("MC_Flavor", 0);
        Breco ->addExtraInfo("MC_PDG_of_Class_Track", 0);
        Breco ->addExtraInfo("MC_PDG_of_Class_Track_Mother", 0);
        Breco ->addExtraInfo("Class_Prob",  0);
        Breco ->addExtraInfo("PID_MonteCarlo", 0);
        Breco -> addExtraInfo("MC_PDG_of_Class_Track_Wrong_rightFlavor",  0);
        Breco -> addExtraInfo("MC_PDG_of_Class_Track_Mother_Wrong_rightFlavor", 0);
      } else if (TrackID == 0) {
        N_noTracks++;
        Breco ->addExtraInfo("Class_Flavor", 0);
        Breco ->addExtraInfo("MC_Flavor", 0);
        Breco ->addExtraInfo("MC_PDG_of_Class_Track", 0);
        Breco ->addExtraInfo("MC_PDG_of_Class_Track_Mother", 0);
        Breco ->addExtraInfo("Class_Prob",  0);
        Breco ->addExtraInfo("PID_MonteCarlo", 0);
        Breco -> addExtraInfo("MC_PDG_of_Class_Track_Wrong_rightFlavor",  0);
        Breco -> addExtraInfo("MC_PDG_of_Class_Track_Mother_Wrong_rightFlavor", 0);
      }
      for (auto & x : Breco -> getDaughters()) {
        if ((TMath::Abs(x ->getRelated<MCParticle>()->getPDG()) == 13) && (TMath::Abs(x ->getRelated<MCParticle>()->getMother()->getPDG()) == 511)) N_Target_in_Breco++;
      }
//       cout << "N_Events=" << N_Events << endl;
//       cout << "Sum_allParticles=" << int(N_noParticle + N_Signal + N_Background + N_noTracks) << endl;
//       cout << "N_noParticle=" << N_noParticle << endl;
//       cout << "N_Background=" << N_Background << endl;
//       cout << "N_noTracks=" << N_noTracks << endl;
//       cout << "N_Target_in_Breco=" << N_Target_in_Breco << endl;
//       cout << "N_Signal=" << N_Signal << endl;
//       cout << "N_Corr/N_Signal=" <<  float(N_Corr) / float(N_Signal) << "   N_Wrong/N_Signal=" << float(N_Wrong) / float(N_Signal) << "   N_Wrong_Right_Flavor/N_Signal=" << float(N_Wrong_rightFlavor) / float(N_Signal) << "   (N_Corr+N_Wrong_Right_Flavor)/N_Signal=" << float(N_Corr + N_Wrong_rightFlavor) / float(N_Signal) << endl;


    }

    if (m_mode == 2) {

      if ((TrackID > 0) && (int(Track_y_MC[TMath::LocMax(m_tagTracks.size(), Track_y_MC)]) != 0)) {

        momMu = T.rotateLabToCms() * m_tagDummies[TMath::LocMax(m_tagTracks.size(), Track_y_Class)] -> get4Vector();
        momX = (momX - momMu) - momMu;
        momMiss = -(momX + momMu);
        p_CMS_missing = momMiss.Vect().Mag();
        cosTheta_missing = TMath::Cos(momMu.Angle(momMiss.Vect()));
        recoilMassBtag = momX.M();

        /*        cout << "Charge of Found track=" <<  m_tagTracks[TMath::LocMax(m_tagTracks.size(), Track_y_Class)]->getTrackFitResult(Const::muon)-> getChargeSign() << endl;      */
        /*        cout << "Charge of Correct track=" <<  m_tagDummies[TMath::LocMax(m_tagTracks.size(), Track_y_MC)] -> getCharge() << endl;   */
        /*        cout << "P of Found track=" <<  m_tagTracks[TMath::LocMax(m_tagTracks.size(), Track_y_Class)]->getTrackFitResult(Const::muon)-> getMomentum().Mag() << endl;       */
        /*        cout << "P of Correct track=" <<  m_tagDummies[TMath::LocMax(m_tagTracks.size(), Track_y_MC)]-> getMomentumMagnitude() << endl;  */

        Breco -> addExtraInfo("Class_Flavor", m_tagTracks[TMath::LocMax(m_tagTracks.size(), Track_y_Class)]->getTrackFitResult(Const::muon)-> getChargeSign());
        Breco -> addExtraInfo("MC_Flavor", m_tagTracks[TMath::LocMax(m_tagTracks.size(), Track_y_MC)]->getTrackFitResult(Const::muon)-> getChargeSign());
        Breco -> addExtraInfo("p_CMS_missing", p_CMS_missing);
        Breco -> addExtraInfo("cosTheta_missing", cosTheta_missing);
        Breco -> addExtraInfo("recoilMassBtag", recoilMassBtag);

        m_teacher_Muon_EL -> addSample(Breco);

      }
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
      const Belle2::Particle* particle_i = particles.appendNew(tracki, Const::kaon); //make a dummy particle from track
      const TrackFitResult* trackiRes = NULL;
      if (tracki) trackiRes = tracki->getTrackFitResult(Const::kaon); //

      //add a relation: dummy particle related to its mcparticle and pid likelihood - now we can use Variables!
      if (tracki->getRelated<PIDLikelihood>()) particle_i->addRelationTo(tracki->getRelated<PIDLikelihood>());
      if (tracki->getRelated<MCParticle>()) particle_i->addRelationTo(tracki->getRelated<MCParticle>());
      addContinuumSuppression(particle_i);




      if (m_mode == 0) {
        if (trackiRes) {
          m_teacher_Kaon_TL->addSample(particle_i);
        } else continue;
      }

      //const ContinuumSuppression* continuumSuppression = p->getRelated<ContinuumSuppression>();
      //endrun
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
      const Belle2::Particle* particle_i = particles.appendNew(tracki, Const::kaon); //make a dummy particle from track
      const TrackFitResult* trackiRes = NULL;
      if (tracki) trackiRes = tracki->getTrackFitResult(Const::kaon); //

      //add a relation: dummy particle related to its mcparticle and pid likelihood - now we can use Variables!
      if (tracki->getRelated<PIDLikelihood>()) particle_i->addRelationTo(tracki->getRelated<PIDLikelihood>());
      if (tracki->getRelated<MCParticle>()) particle_i->addRelationTo(tracki->getRelated<MCParticle>());
      addContinuumSuppression(particle_i);

      if (m_mode == 0) {
        if (trackiRes) {
          m_teacher_SlowPion_TL->addSample(particle_i);
        } else continue;
      }
      //p->addExtraInfo("signalProbabilitySlowPion", expert_SlowPion->analyse(p));
      //p->getExtraInfo("signalProbabilitySlowPion");
    }

    return true;
  }

//TO DO: add more categories...

  void FlavorTaggingModule::endRun()
  {
    if (m_mode == 0) {
      m_teacher_Muon_TL->train(m_factoryOption_Muon_TL, m_prepareOption);
      delete m_teacher_Muon_TL;
      m_teacher_Muon_TL = nullptr;

      m_teacher_Kaon_TL->train(m_factoryOption_Kaon_TL, m_prepareOption);
      delete m_teacher_Kaon_TL;
      m_teacher_Kaon_TL = nullptr;

      m_teacher_SlowPion_TL->train(m_factoryOption_SlowPion_TL, m_prepareOption);
      delete m_teacher_SlowPion_TL;
      m_teacher_SlowPion_TL = nullptr;

    }
    if (m_mode == 2) {
      m_teacher_Muon_EL->train(m_factoryOption_Muon_EL, m_prepareOption);
      delete m_teacher_Muon_EL;
      m_teacher_Muon_EL = nullptr;
    }
  }

  void FlavorTaggingModule::terminate()
  {

    if ((m_mode == 1) || (m_mode == 2)) {
      if (m_expert_Muon_TL !=  nullptr) {
        delete m_expert_Muon_TL;
        m_expert_Muon_TL = nullptr;
      }
    }

  }

} // end Belle2 namespace
