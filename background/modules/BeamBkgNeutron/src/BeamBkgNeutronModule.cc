/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <background/modules/BeamBkgNeutron/BeamBkgNeutronModule.h>

#include <time.h>

// Hit classes

#include <simulation/dataobjects/BeamBackHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <arich/dataobjects/ARICHSimHit.h>
#include <top/dataobjects/TOPSimHit.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <klm/dataobjects/KLMSimHit.h>
#include <simulation/dataobjects/MCParticleTrajectory.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <framework/geometry/B2Vector3.h>

using namespace std;
using namespace boost;

namespace Belle2 {
  //-----------------------------------------------------------------
  ///                Register the Module
  //-----------------------------------------------------------------

  REG_MODULE(BeamBkgNeutron);


  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  BeamBkgNeutronModule::BeamBkgNeutronModule() : Module(), m_iEntry(0), m_ff(NULL), m_tree1(NULL), m_tree2(NULL)
  {
    // Set description()
    setDescription("BeamBkgNeutronModule module. Used to extract information relevant for the neutron background from background files");

    // Add parameters
    addParam("FileName", m_filename, "output file name", string("mytree.root"));
  }

  BeamBkgNeutronModule::~BeamBkgNeutronModule()
  {
  }

  void BeamBkgNeutronModule::initialize()
  {
    // create/open the file
    m_ff = new TFile(m_filename.c_str(), "RECREATE");

    // create trees
    m_tree1 = new TTree("tree1", "BeamBackHit data");
    m_tree2 = new TTree("tree2", "SimHits data");

    // create branches
    m_tree1->Branch("event", &m_iEvent, "event/I");
    m_tree1->Branch("subDet", &m_subDet, "subDet/I");
    m_tree1->Branch("iden", &m_iden, "iden/I");
    m_tree1->Branch("PDG", &m_PDG, "PDG/I");
    m_tree1->Branch("trackID", &m_trackID, "trackID/I");
    m_tree1->Branch("momentumX", &m_momentumX, "momentumX/F");
    m_tree1->Branch("momentumY", &m_momentumY, "momentumY/F");
    m_tree1->Branch("momentumZ", &m_momentumZ, "momentumZ/F");
    m_tree1->Branch("positionX", &m_positionX, "positionX/F");
    m_tree1->Branch("positionY", &m_positionY, "positionY/F");
    m_tree1->Branch("positionZ", &m_positionZ, "positionZ/F");
    m_tree1->Branch("E_start", &m_E_start, "E_start/F");
    m_tree1->Branch("E_end", &m_E_end, "E_end/F");
    m_tree1->Branch("eDep", &m_eDep, "eDep/F");
    m_tree1->Branch("trackLength", &m_trackLength, "trackLength/F");
    m_tree1->Branch("nWeight", &m_nWeight, "nWeight/F");
    m_tree1->Branch("E_init", &m_E_init, "E_init/F");
    m_tree1->Branch("mass", &m_mass, "mass/F");
    m_tree1->Branch("lifeTime", &m_lifeTime, "lifeTime/F");
    m_tree1->Branch("vtxProdX", &m_vtxProdX, "vtxProdX/F");
    m_tree1->Branch("vtxProdY", &m_vtxProdY, "vtxProdY/F");
    m_tree1->Branch("vtxProdZ", &m_vtxProdZ, "vtxProdZ/F");
    m_tree1->Branch("trj_x", &m_trj_x);
    m_tree1->Branch("trj_y", &m_trj_y);
    m_tree1->Branch("trj_z", &m_trj_z);
    m_tree1->Branch("trj_px", &m_trj_px);
    m_tree1->Branch("trj_py", &m_trj_py);
    m_tree1->Branch("trj_pz", &m_trj_pz);

    m_tree2->Branch("nSimHits", m_nSimHits, "nSimHits[13]/I");
    m_tree2->Branch("hitPDG", m_hitPDG, "hitPDG[13]/I");
    m_tree2->Branch("momPDG", m_momPDG, "momPDG[13]/I");
//    std::string xyzvector_typedef = "std::vector<ROOT::Math::DisplacementVector3D<ROOT::Math::Cartesian3D<double>,ROOT::Math::DefaultCoordinateSystemTag>>";
//    tt->Branch("vtxProd", "xyzvector_typedef.c_str()", &m_vtxProd);
  }

  void BeamBkgNeutronModule::beginRun()
  {
    // Print run number
    B2INFO("BeamBkgNeutron: Processing. ");
  }

  void BeamBkgNeutronModule::event()
  {
    // MCParticles
    StoreArray<MCParticle>  McParticles;

    // SimHits
    StoreArray<PXDSimHit>   PXDSimHits;
    StoreArray<SVDSimHit>   SVDSimHits;
    StoreArray<CDCSimHit>   CDCSimHits;
    StoreArray<ARICHSimHit> ARICHSimHits;
    StoreArray<TOPSimHit>   TOPSimHits;
    StoreArray<ECLSimHit>   ECLSimHits;
    StoreArray<KLMSimHit>   KLMSimHits;

    for (Int_t i = 0; i < 13; i++) {
      m_nSimHits[i] = 0;
      m_hitPDG[i] = 0;
      m_momPDG[i] = 0;
    }

    m_nSimHits[1] = PXDSimHits.getEntries();
    m_nSimHits[2] = SVDSimHits.getEntries();
    m_nSimHits[3] = CDCSimHits.getEntries();
    m_nSimHits[4] = ARICHSimHits.getEntries();
    m_nSimHits[5] = TOPSimHits.getEntries();
    m_nSimHits[6] = ECLSimHits.getEntries();
    m_nSimHits[7] = KLMSimHits.getEntries();

    // loop over KLM simHits
    for (Int_t hit = 0; hit < m_nSimHits[7]; hit++) {
      // get KLMSimHit
//      EKLMSimHit* simHit = EKLMSimHits[hit];
      KLMSimHit* simHit = KLMSimHits[hit];
      Float_t posZ = simHit->getPositionZ();
      if (280.0 < posZ && posZ <  288.0) m_nSimHits[9]++;   // FWD EKLM innermost layer
      else if (400.0 < posZ && posZ <  406.0) m_nSimHits[10]++;  // FWD EKLM outermost layer
      else if (-194.0 < posZ && posZ < -186.0) m_nSimHits[11]++; // BWD EKLM innermost layer
      else if (-294.0 < posZ && posZ < -286.0) m_nSimHits[12]++; // BWD EKLM outermost layer
    }

    RelationIndex<MCParticle, PXDSimHit> relPXDSimHitToMCParticle(McParticles, PXDSimHits);
    RelationIndex<MCParticle, SVDSimHit> relSVDSimHitToMCParticle(McParticles, SVDSimHits);
    RelationIndex<MCParticle, CDCSimHit> relCDCSimHitToMCParticle(McParticles, CDCSimHits);
    RelationIndex<MCParticle, ARICHSimHit> relARICHSimHitToMCParticle(McParticles, ARICHSimHits);
    RelationIndex<MCParticle, TOPSimHit> relTOPSimHitToMCParticle(McParticles, TOPSimHits);
    RelationIndex<MCParticle, ECLSimHit> relECLSimHitToMCParticle(McParticles, ECLSimHits);
    RelationIndex<MCParticle, KLMSimHit> relKLMSimHitToMCParticle(McParticles, KLMSimHits);

    Int_t detID;

    //--- PXD
    detID = 1;
    // loop over simhits
    for (Int_t iHit = 0; iHit < m_nSimHits[detID]; iHit++) {
      PXDSimHit* simHit = PXDSimHits[iHit];
      // get related MCparticle
      if (relPXDSimHitToMCParticle.getFirstElementTo(simHit)) {
        const MCParticle* currParticle = relPXDSimHitToMCParticle.getFirstElementTo(simHit)->from;
        m_hitPDG[detID] = currParticle->getPDG();
        if (!currParticle->isPrimaryParticle()) {
          const MCParticle* momParticle = currParticle->getMother();
          m_momPDG[detID] = momParticle->getPDG();
        }
      }
    }

    //--- SVD
    detID = 2;
    // loop over simhits
    for (Int_t iHit = 0; iHit < m_nSimHits[detID]; iHit++) {
      SVDSimHit* simHit = SVDSimHits[iHit];
      // get related MCparticle
      if (relSVDSimHitToMCParticle.getFirstElementTo(simHit)) {
        const MCParticle* currParticle = relSVDSimHitToMCParticle.getFirstElementTo(simHit)->from;
        m_hitPDG[detID] = currParticle->getPDG();
        if (!currParticle->isPrimaryParticle()) {
          const MCParticle* momParticle = currParticle->getMother();
          m_momPDG[detID] = momParticle->getPDG();
        }
      }
    }

    //--- CDC
    detID = 3;
    // loop over simhits
    for (Int_t iHit = 0; iHit < m_nSimHits[detID]; iHit++) {
      CDCSimHit* simHit = CDCSimHits[iHit];
      // get related MCparticle
      if (relCDCSimHitToMCParticle.getFirstElementTo(simHit)) {
        const MCParticle* currParticle = relCDCSimHitToMCParticle.getFirstElementTo(simHit)->from;
        m_hitPDG[detID] = currParticle->getPDG();
        if (!currParticle->isPrimaryParticle()) {
          const MCParticle* momParticle = currParticle->getMother();
          m_momPDG[detID] = momParticle->getPDG();
        }
      }
    }

    //--- ARICH
    detID = 4;
    // loop over simhits
    for (Int_t iHit = 0; iHit < m_nSimHits[detID]; iHit++) {
      ARICHSimHit* simHit = ARICHSimHits[iHit];
      // get related MCparticle
      if (relARICHSimHitToMCParticle.getFirstElementTo(simHit)) {
        const MCParticle* currParticle = relARICHSimHitToMCParticle.getFirstElementTo(simHit)->from;
        m_hitPDG[detID] = currParticle->getPDG();
        if (!currParticle->isPrimaryParticle()) {
          const MCParticle* momParticle = currParticle->getMother();
          m_momPDG[detID] = momParticle->getPDG();
        }
      }
    }

    //--- TOP
    detID = 5;
    // loop over simhits
    for (Int_t iHit = 0; iHit < m_nSimHits[detID]; iHit++) {
      TOPSimHit* simHit = TOPSimHits[iHit];
      // get related MCparticle
      if (relTOPSimHitToMCParticle.getFirstElementTo(simHit)) {
        const MCParticle* currParticle = relTOPSimHitToMCParticle.getFirstElementTo(simHit)->from;
        m_hitPDG[detID] = currParticle->getPDG();
        if (!currParticle->isPrimaryParticle()) {
          const MCParticle* momParticle = currParticle->getMother();
          m_momPDG[detID] = momParticle->getPDG();
        }
      }
    }

    //--- ECL
    detID = 6;
    // loop over simhits
    for (Int_t iHit = 0; iHit < m_nSimHits[detID]; iHit++) {
      ECLSimHit* simHit = ECLSimHits[iHit];
      // get related MCparticle
      if (relECLSimHitToMCParticle.getFirstElementTo(simHit)) {
        const MCParticle* currParticle = relECLSimHitToMCParticle.getFirstElementTo(simHit)->from;
        m_hitPDG[detID] = currParticle->getPDG();
        if (!currParticle->isPrimaryParticle()) {
          const MCParticle* momParticle = currParticle->getMother();
          m_momPDG[detID] = momParticle->getPDG();
        }
      }
    }
    //--- KLM
    detID = 7;
    // loop over simhits
    for (Int_t iHit = 0; iHit < m_nSimHits[detID]; iHit++) {
      KLMSimHit* simHit = KLMSimHits[iHit];
      // get related MCparticle
      if (relKLMSimHitToMCParticle.getFirstElementTo(simHit)) {
        const MCParticle* currParticle = relKLMSimHitToMCParticle.getFirstElementTo(simHit)->from;
        m_hitPDG[detID] = currParticle->getPDG();
        if (!currParticle->isPrimaryParticle()) {
          const MCParticle* momParticle = currParticle->getMother();
          m_momPDG[detID] = momParticle->getPDG();
        }
      }
    }

    // fill the tree
    m_tree2->Fill();

    // BeamBkgHits
    StoreArray<BeamBackHit> BeamBackHits;
    RelationIndex<MCParticle, BeamBackHit> relBeamBackHitToMCParticle(McParticles, BeamBackHits);

    Int_t nHits = BeamBackHits.getEntries();

    // loop over bkgHits
    for (Int_t iHit = 0; iHit < nHits; iHit++) {
      // get one bkgHit
      BeamBackHit* bkgHit = BeamBackHits[iHit];
      if (bkgHit->getPDG() == 2112) { /*neutron*/
        m_iEvent = m_iEntry;
        m_iden = bkgHit->getIdentifier();
        m_subDet = bkgHit->getSubDet();
        m_PDG = bkgHit->getPDG();
        m_trackID = bkgHit->getTrackID();
        auto position = bkgHit->getPosition();
        m_positionX = position.X();
        m_positionY = position.Y();
        m_positionZ = position.Z();
        auto momentum = bkgHit->getMomentum();
        m_momentumX = momentum.X();
        m_momentumY = momentum.Y();
        m_momentumZ = momentum.Z();
        m_E_start = bkgHit->getEnergy();
        m_E_end = bkgHit->getEnergyAtExit();
        m_eDep = bkgHit->getEnergyDeposit();
        m_trackLength = bkgHit->getTrackLength();
        m_nWeight = bkgHit->getNeutronWeight();

        m_trj_x.clear();
        m_trj_y.clear();
        m_trj_z.clear();
        m_trj_px.clear();
        m_trj_py.clear();
        m_trj_pz.clear();

        // get related MCparticle
        if (relBeamBackHitToMCParticle.getFirstElementTo(bkgHit)) {
          const MCParticle* currParticle = relBeamBackHitToMCParticle.getFirstElementTo(bkgHit)->from;
          auto m_vtxProd = currParticle->getVertex();
          m_E_init = currParticle->getEnergy();
          m_mass = currParticle->getMass();
          m_lifeTime = currParticle->getLifetime();
          m_vtxProdX = m_vtxProd.X();
          m_vtxProdY = m_vtxProd.Y();
          m_vtxProdZ = m_vtxProd.Z();

          // get trajectory stored for this particle
          const auto mcTrajectories = currParticle->getRelationsTo<MCParticleTrajectory>();
          for (auto rel : mcTrajectories.relations()) {
            // the trajectories with negative weight are from secondary daughters which were ignored so we don't use them
            if (rel.weight <= 0)  continue;
            // get the trajectory
            const MCParticleTrajectory& trajectory = dynamic_cast<const MCParticleTrajectory&>(*rel.object);
            // loop over each point
            for (const MCTrajectoryPoint& pt : trajectory) {
              m_trj_x.push_back(pt.x);
              m_trj_y.push_back(pt.y);
              m_trj_z.push_back(pt.z);
              m_trj_px.push_back(pt.px);
              m_trj_py.push_back(pt.py);
              m_trj_pz.push_back(pt.pz);
            }
          }
        }

        // fill the tree
        m_tree1->Fill();
      }
    }
    // increase the entry counter
    m_iEntry++;
  }

  void BeamBkgNeutronModule::endRun()
  {
  }

  void BeamBkgNeutronModule::terminate()
  {
    // CPU time end

    // Announce
    B2INFO("BeamBkgNeutron finished.");

    // write
    m_ff->cd();
    m_tree1->Write();
    m_tree2->Write();
    // close the tree
    m_ff->Close();
  }

} // end Belle2 namespace
