/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contrbutors: Marko Petric                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// Own include

#include <top/modules/TOPTutorial/TOPTutorialModule.h>
#include <framework/core/ModuleManager.h>
#include <time.h>

// Hit classes
#include <top/dataobjects/TOPTrack.h>
#include <top/dataobjects/TOPLikelihoods.h>
#include <arich/dataobjects/ARICHLikelihoods.h>
#include <arich/dataobjects/ARICHAeroHit.h>


#include <GFTrack.h>
#include <generators/dataobjects/MCParticle.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <TClonesArray.h>
// Framwrok
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>
//BOOST
#include <boost/format.hpp>
#include <boost/foreach.hpp>
//Generators
#include <generators/utilities/cm2LabBoost.h>

using namespace std;
using namespace boost;

namespace Belle2 {
  namespace TOP {
    //-----------------------------------------------------------------
    //                 Register the Module
    //-----------------------------------------------------------------

    REG_MODULE(TOPTutorial)


    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    TOPTutorialModule::TOPTutorialModule() : Module()
    {
      // Set description()
      setDescription("TOPTutorial");

      // Add parameters
    }

    TOPTutorialModule::~TOPTutorialModule()
    {

    }

    void TOPTutorialModule::initialize()
    {
      // Initialize variables
      m_nRun    = 0 ;
      m_nEvent  = 0 ;

      // Print set parameters
      printModuleParams();

      // CPU time start
      addParam("MCParticlesColName", m_mcParticlesColName, "MCParticles collection name" , string(""));
      addParam("GFTracksColName", m_gfTracksColName, "GFTracks collection name", string(""));
      addParam("outputFileName", m_dataOutFileName, "Output root file name of this module. Suffixes to distinguish them will be added automatically", string("Tutorial"));
      // Initializing the output root file
      string dataFileName = m_dataOutFileName + ".root";
      m_rootFile = new TFile(dataFileName.c_str(), "RECREATE");
      m_tree     = new TTree("m_tree", "BtoPhiKshortStudy tree");

      m_tree->Branch("m_bc", &m_m_bc, "m_bc/D");
      m_tree->Branch("deltae", &m_deltae, "deltae/D");
    }

    void TOPTutorialModule::beginRun()
    {
      // Print run number
      B2INFO("TOPTutorial: Processing run: ");

    }

    const TOPLikelihoods* TOPTutorialModule::getTOPLikelihoods(const MCParticle* particle)
    {

      StoreArray<MCParticle> mcParticles;
      StoreArray<TOPTrack>  topTracks;
      StoreArray<TOPLikelihoods> toplogL;

      RelationArray  testrelTrackLikelihoods(topTracks, toplogL);
      RelationArray  testrelMCParticleToTOPTrack(mcParticles, topTracks);

      if (!(testrelTrackLikelihoods && testrelMCParticleToTOPTrack)) {
        return 0;
      }

      RelationIndex< TOPTrack, TOPLikelihoods > relTrackLikelihoods(topTracks, toplogL);
      RelationIndex<MCParticle, TOPTrack> relMCParticleToTOPTrack(mcParticles, topTracks);

      if (!(relTrackLikelihoods && relMCParticleToTOPTrack)) {
        return 0;
      }

      if (relMCParticleToTOPTrack.getFirstTo(particle)) {
        const TOPTrack* track = relMCParticleToTOPTrack.getFirstTo(particle)->to;

        if (relTrackLikelihoods.getFirstTo(track)) {

          return relTrackLikelihoods.getFirstTo(track)->to;

        }
      }

      return 0;
    }

    const ARICHLikelihoods* TOPTutorialModule::getARICHLikelihoods(const MCParticle* particle)
    {

      StoreArray<ARICHLikelihoods> arichLikelihoods;
      StoreArray<ARICHAeroHit> arichAeroHits;
      StoreArray<MCParticle> mcParticles;

      RelationArray testarichAeroHitRel(mcParticles, arichAeroHits);
      RelationArray testrelAeroToLikelihood(arichAeroHits, arichLikelihoods);

      if (!(testarichAeroHitRel && testrelAeroToLikelihood)) {
        return 0;
      }


      RelationIndex<MCParticle, ARICHAeroHit> arichAeroHitRel(mcParticles, arichAeroHits);
      RelationIndex<ARICHAeroHit, ARICHLikelihoods> relAeroToLikelihood(arichAeroHits, arichLikelihoods);

      if (!(arichAeroHitRel && relAeroToLikelihood)) {
        return 0;
      }

      if (arichAeroHitRel.getFirstTo(particle)) {
        const ARICHAeroHit* track = arichAeroHitRel.getFirstTo(particle)->to;
        if (relAeroToLikelihood.getFirstTo(track)) {

          return relAeroToLikelihood.getFirstTo(track)->to;

        }
      }



      return 0;
    }

    const MCParticle* TOPTutorialModule::getMCParticle(const GFTrack* track)
    {
      StoreArray<MCParticle> mcParticles;
      StoreArray<GFTrack> gfTracks;

      RelationArray testgftracktomc(gfTracks, mcParticles);

      if (! testgftracktomc) {
        return 0;
      }

      RelationIndex<GFTrack, MCParticle> gftracktomc(gfTracks, mcParticles);

      if (gftracktomc.getFirstTo(track)) {

        return gftracktomc.getFirstTo(track)->to;

      }

      return 0;

    }

    double TOPTutorialModule::TOP_ARICH_PID(int hyp1, int hyp2, const GFTrack* particle)
    {

      double logl1 = 0.;
      double logl2 = 0.;

      if (hyp1 == hyp2 || hyp1 > 5 || hyp1 < 1 || hyp2 > 5 || hyp2 < 1) {
        return 0;
      }


      if (getMCParticle(particle)) {
        const MCParticle* mctrack = getMCParticle(particle);

        if (getTOPLikelihoods(mctrack)) {

          if (getTOPLikelihoods(mctrack)->getFlag()) {

            if (hyp1 == 1) {
              logl1 = getTOPLikelihoods(mctrack)->getLogL_e();
            }
            if (hyp1 == 2) {
              logl1 = getTOPLikelihoods(mctrack)->getLogL_mu();
            }
            if (hyp1 == 3) {
              logl1 = getTOPLikelihoods(mctrack)->getLogL_pi();
            }
            if (hyp1 == 4) {
              logl1 = getTOPLikelihoods(mctrack)->getLogL_K();
            }
            if (hyp1 == 5) {
              logl1 = getTOPLikelihoods(mctrack)->getLogL_p();
            }

            if (hyp2 == 1) {
              logl2 = getTOPLikelihoods(mctrack)->getLogL_e();
            }
            if (hyp2 == 2) {
              logl2 = getTOPLikelihoods(mctrack)->getLogL_mu();
            }
            if (hyp2 == 3) {
              logl2 = getTOPLikelihoods(mctrack)->getLogL_pi();
            }
            if (hyp2 == 4) {
              logl2 = getTOPLikelihoods(mctrack)->getLogL_K();
            }
            if (hyp2 == 5) {
              logl2 = getTOPLikelihoods(mctrack)->getLogL_p();
            }
            return logl1 - logl2;
          }
        }

        if (getARICHLikelihoods(mctrack)) {

          if (getARICHLikelihoods(mctrack)->getFlag()) {
            if (hyp1 == 1) {
              logl1 = getARICHLikelihoods(mctrack)->getLogL_e();
            }
            if (hyp1 == 2) {
              logl1 = getARICHLikelihoods(mctrack)->getLogL_mu();
            }
            if (hyp1 == 3) {
              logl1 = getARICHLikelihoods(mctrack)->getLogL_pi();
            }
            if (hyp1 == 4) {
              logl1 = getARICHLikelihoods(mctrack)->getLogL_K();
            }
            if (hyp1 == 5) {
              logl1 = getARICHLikelihoods(mctrack)->getLogL_p();
            }

            if (hyp2 == 1) {
              logl2 = getARICHLikelihoods(mctrack)->getLogL_e();
            }
            if (hyp2 == 2) {
              logl2 = getARICHLikelihoods(mctrack)->getLogL_mu();
            }
            if (hyp2 == 3) {
              logl2 = getARICHLikelihoods(mctrack)->getLogL_pi();
            }
            if (hyp2 == 4) {
              logl2 = getARICHLikelihoods(mctrack)->getLogL_K();
            }
            if (hyp2 == 5) {
              logl2 = getARICHLikelihoods(mctrack)->getLogL_p();
            }
            return logl1 - logl2;
          }
        }


      }

      return 0;


    }

    void TOPTutorialModule::makePi(std::vector<TLorentzVector> &pip, std::vector<TLorentzVector> &pim, double cut)
    {
      StoreArray<GFTrack> gfTracks(m_gfTracksColName);

      double pimass = 0.13957018;

      int nHits = gfTracks->GetLast() + 1;

      for (int iHit = 0; iHit < nHits; ++iHit) {

        GFTrack* particle  = gfTracks[iHit];

        TVector3 point(0., 0., 0.);
        TVector3 poca(0., 0., 0.);
        TVector3 dir(0., 0., 0.);

        try {
          particle->getCardinalRep()->extrapolateToPoint(point, poca, dir);
        } catch (...) {
          B2WARNING("Track extrapolation failed!");
          continue;
        }

        if (TOP_ARICH_PID(3, 4, particle) > cut) {
          TLorentzVector tmp;

          tmp.SetXYZM(particle->getMom().Mag()*dir.X(), particle->getMom().Mag()*dir.Y(), particle->getMom().Mag()*dir.Z(), pimass);


          if (particle->getCharge() > 0) {
            pip.push_back(tmp);
          } else {
            pim.push_back(tmp);
          }
        }
      }
    }


    void TOPTutorialModule::makeK(std::vector<TLorentzVector> &kp, std::vector<TLorentzVector> &km, double cut)
    {
      StoreArray<GFTrack> gfTracks(m_gfTracksColName);

      double Kmass = 0.493667;

      int nHits = gfTracks->GetLast() + 1;

      for (int iHit = 0; iHit < nHits; ++iHit) {

        GFTrack* particle  = gfTracks[iHit];

        TVector3 point(0., 0., 0.);
        TVector3 poca(0., 0., 0.);
        TVector3 dir(0., 0., 0.);

        try {
          particle->getCardinalRep()->extrapolateToPoint(point, poca, dir);
        } catch (...) {
          B2WARNING("Track extrapolation failed!");
          continue;
        }


        if (TOP_ARICH_PID(3, 4, particle) < cut) {
          TLorentzVector tmp;
          tmp.SetXYZM(particle->getMom().Mag()*dir.X(), particle->getMom().Mag()*dir.Y(), particle->getMom().Mag()*dir.Z(), Kmass);

          if (particle->getCharge() > 0) {
            kp.push_back(tmp);
          } else {
            km.push_back(tmp);
          }
        }
      }
    }

    void TOPTutorialModule::combination(std::vector<TLorentzVector>& mother, std::vector<TLorentzVector>& child1, std::vector<TLorentzVector>& child2)
    {

      int size1 = child1.size();
      int size2 = child2.size();

      for (int i = 0; i < size1; i++) {
        for (int j = 0; j < size2; j++) {
          mother.push_back(child1[i] + child2[j]);
        }
      }

    }

    void TOPTutorialModule::event()
    {

      vector<TLorentzVector> pip;
      vector<TLorentzVector> pim;
      vector<TLorentzVector> kp;
      vector<TLorentzVector> km;

      makePi(pip, pim, 0);
      makeK(kp, km, 0);

      vector<TLorentzVector> B0;
      vector<TLorentzVector> B0bar;


      combination(B0, kp, pim);
      combination(B0bar, km, pip);

      StoreArray<MCParticle> mcParticles(m_mcParticlesColName);

      TVector3 boostToCMS(0., 0., 0.);
      double cmsE = 0;

      for (int i = 0; i < mcParticles.getEntries(); i++) {
        MCParticle* mc = mcParticles[i];

        if (mc->getPDG() == 300553) {
          // get the CMS boost vector end energy
          TLorentzVector cmsLV = mc->get4Vector();
          boostToCMS = -(cmsLV.BoostVector());
          cmsLV.Boost(boostToCMS);
          cmsE = cmsLV.E() / 2.0;
        }
      }

      int size = B0.size();

      for (int i = 0; i < size; i++) {
        // calculate B0 invariant mass

        TLorentzVector B = B0[i];

        //double B0mass = B.Mag();

        B.Boost(boostToCMS);
        m_deltae = B.E() - cmsE;
        m_m_bc = TMath::Sqrt((cmsE * cmsE) - B.Vect().Mag2());
        m_tree->Fill();
        /*
         cout <<"-----------------------------------"<<endl;
         cout <<"B0bar: deltaE"<< m_deltae<<"\t M_bc"<<m_m_bc<<"\t"<<B0mass<<endl;
         cout <<"-----------------------------------"<<endl;
         */

      }

      size = B0bar.size();

      for (int i = 0; i < size; i++) {
        // calculate B0 invariant mass

        TLorentzVector B = B0bar[i];

        //double B0mass = B.Mag();

        B.Boost(boostToCMS);
        m_deltae = B.E() - cmsE;
        m_m_bc = TMath::Sqrt((cmsE * cmsE) - B.Vect().Mag2());
        m_tree->Fill();
        /*
         cout <<"-----------------------------------"<<endl;
         cout <<"B0bar: deltaE"<< m_deltae<<"\t M_bc"<<m_m_bc<<"\t"<<B0mass<<endl;
         cout <<"-----------------------------------"<<endl;
         */
      }

    }

    void TOPTutorialModule::endRun()
    {
      m_nRun++;
    }

    void TOPTutorialModule::terminate()
    {
      m_tree->Write();
      m_rootFile->Close();

      // Announce
      B2INFO("TOTutorial finished");


    }

    void TOPTutorialModule::printModuleParams() const
    {

    }


  } // end top namespace
} // end Belle2 namespace
