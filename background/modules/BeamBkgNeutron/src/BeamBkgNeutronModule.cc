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
#include <klm/dataobjects/eklm/EKLMSimHit.h>
#include <klm/dataobjects/bklm/BKLMSimHit.h>
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

  BeamBkgNeutronModule::BeamBkgNeutronModule() : Module(), iEntry(0), ff(NULL), tree1(NULL), tree2(NULL)
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
    // Print set parameters
    printModuleParams();

    // create/open the file
    ff = new TFile(m_filename.c_str(), "RECREATE");

    // create trees
    tree1 = new TTree("tree1", "BeamBackHit data");
    tree2 = new TTree("tree2", "SimHits data");

    // create branches
    tree1->Branch("event", &iEvent, "event/I");
    tree1->Branch("subDet", &subDet, "subDet/I");
    tree1->Branch("iden", &iden, "iden/I");
    tree1->Branch("PDG", &PDG, "PDG/I");
    tree1->Branch("trackID", &trackID, "trackID/I");
    tree1->Branch("momentumX", &momentumX, "momentumX/F");
    tree1->Branch("momentumY", &momentumY, "momentumY/F");
    tree1->Branch("momentumZ", &momentumZ, "momentumZ/F");
    tree1->Branch("positionX", &positionX, "positionX/F");
    tree1->Branch("positionY", &positionY, "positionY/F");
    tree1->Branch("positionZ", &positionZ, "positionZ/F");
    tree1->Branch("E_start", &E_start, "E_start/F");
    tree1->Branch("E_end", &E_end, "E_end/F");
    tree1->Branch("eDep", &eDep, "eDep/F");
    tree1->Branch("trackLength", &trackLength, "trackLength/F");
    tree1->Branch("nWeight", &nWeight, "nWeight/F");
    tree1->Branch("E_init", &E_init, "E_init/F");
    tree1->Branch("mass", &mass, "mass/F");
    tree1->Branch("lifeTime", &lifeTime, "lifeTime/F");
    tree1->Branch("vtxProdX", &vtxProdX, "vtxProdX/F");
    tree1->Branch("vtxProdY", &vtxProdY, "vtxProdY/F");
    tree1->Branch("vtxProdZ", &vtxProdZ, "vtxProdZ/F");
    tree1->Branch("trj_x", &trj_x);
    tree1->Branch("trj_y", &trj_y);
    tree1->Branch("trj_z", &trj_z);
    tree1->Branch("trj_px", &trj_px);
    tree1->Branch("trj_py", &trj_py);
    tree1->Branch("trj_pz", &trj_pz);

    tree2->Branch("nSimHits", nSimHits, "nSimHits[13]/I");
//    std::string xyzvector_typedef = "std::vector<ROOT::Math::DisplacementVector3D<ROOT::Math::Cartesian3D<double>,ROOT::Math::DefaultCoordinateSystemTag>>";
//    tt->Branch("vtxProd", "xyzvector_typedef.c_str()", &vtxProd);
  }

  void BeamBkgNeutronModule::beginRun()
  {
    // Print run number
    B2INFO("BeamBkgNeutron: Processing. ");
  }

  void BeamBkgNeutronModule::event()
  {
    // SimHits
    StoreArray<PXDSimHit>   PXDSimHits;
    StoreArray<SVDSimHit>   SVDSimHits;
    StoreArray<CDCSimHit>   CDCSimHits;
    StoreArray<ARICHSimHit> ARICHSimHits;
    StoreArray<TOPSimHit>   TOPSimHits;
    StoreArray<ECLSimHit>   ECLSimHits;
    StoreArray<EKLMSimHit>  EKLMSimHits;
    StoreArray<BKLMSimHit>  BKLMSimHits;

    nSimHits[0] = -1;
    nSimHits[1] = PXDSimHits.getEntries();
    nSimHits[2] = SVDSimHits.getEntries();
    nSimHits[3] = CDCSimHits.getEntries();
    nSimHits[4] = ARICHSimHits.getEntries();
    nSimHits[5] = TOPSimHits.getEntries();
    nSimHits[6] = ECLSimHits.getEntries();
    nSimHits[7] = EKLMSimHits.getEntries();
    nSimHits[8] = BKLMSimHits.getEntries();

    // loop over EKLM simHits
    nSimHits[9] = 0;
    nSimHits[10] = 0;
    nSimHits[11] = 0;
    nSimHits[12] = 0;
    for (Int_t hit = 0; hit < nSimHits[7]; hit++) {
      // get EKLMSimHit
      EKLMSimHit* simHit = EKLMSimHits[hit];
      Float_t posZ = simHit->getPositionZ();
      if (280.0 < posZ && posZ <  288.0) nSimHits[9]++;   // FWD EKLM innermost layer
      else if (400.0 < posZ && posZ <  406.0) nSimHits[10]++;  // FWD EKLM outermost layer
      else if (-194.0 < posZ && posZ < -186.0) nSimHits[11]++; // BWD EKLM innermost layer
      else if (-294.0 < posZ && posZ < -286.0) nSimHits[12]++; // BWD EKLM outermost layer
    }

    // fill the tree
    tree2->Fill();

    // BeamBkgHits
    StoreArray<MCParticle>  McParticles;
    StoreArray<BeamBackHit> BeamBackHits;
    RelationIndex<MCParticle, BeamBackHit> relBeamBackHitToMCParticle(McParticles, BeamBackHits);

    Int_t nHits = BeamBackHits.getEntries();

    // looop over bkgHits
    for (Int_t iHit = 0; iHit < nHits; iHit++) {
      // get one bkgHit
      BeamBackHit* bkgHit = BeamBackHits[iHit];
      if (bkgHit->getPDG() == 2112) { /*neutron*/
        iEvent = iEntry;
        iden = bkgHit->getIdentifier();
        subDet = bkgHit->getSubDet();
        PDG = bkgHit->getPDG();
        trackID = bkgHit->getTrackID();
        auto position = bkgHit->getPosition();
        positionX = position.X();
        positionY = position.Y();
        positionZ = position.Z();
        auto momentum = bkgHit->getMomentum();
        momentumX = momentum.X();
        momentumY = momentum.Y();
        momentumZ = momentum.Z();
        E_start = bkgHit->getEnergy();
        E_end = bkgHit->getEnergyAtExit();
        eDep = bkgHit->getEnergyDeposit();
        trackLength = bkgHit->getTrackLength();
        nWeight = bkgHit->getNeutronWeight();

        trj_x.clear();
        trj_y.clear();
        trj_z.clear();
        trj_px.clear();
        trj_py.clear();
        trj_pz.clear();

        // get related MCparticle
        if (relBeamBackHitToMCParticle.getFirstElementTo(bkgHit)) {
          const MCParticle* currParticle = relBeamBackHitToMCParticle.getFirstElementTo(bkgHit)->from;
          auto vtxProd = currParticle->getVertex();
          E_init = currParticle->getEnergy();
          mass = currParticle->getMass();
          lifeTime = currParticle->getLifetime();
          vtxProdX = vtxProd.X();
          vtxProdY = vtxProd.Y();
          vtxProdZ = vtxProd.Z();

          // get trajectory stored for this particle
          const auto mcTrajectories = currParticle->getRelationsTo<MCParticleTrajectory>();
          for (auto rel : mcTrajectories.relations()) {
            // the trajectories with negative weight are from secondary daughters which were ignored so we don't use them
            if (rel.weight <= 0)  continue;
            // get the trajectory
            const MCParticleTrajectory& trajectory = dynamic_cast<const MCParticleTrajectory&>(*rel.object);
            // loop over each point
            for (const MCTrajectoryPoint& pt : trajectory) {
              trj_x.push_back(pt.x);
              trj_y.push_back(pt.y);
              trj_z.push_back(pt.z);
              trj_px.push_back(pt.px);
              trj_py.push_back(pt.py);
              trj_pz.push_back(pt.pz);
            }
          }
        }

        // fill the tree
        tree1->Fill();
      }
    }
    // increase the entry counter
    iEntry++;
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
    ff->cd();
    tree1->Write();
    tree2->Write();
    // close the tree
    ff->Close();
  }

  void BeamBkgNeutronModule::printModuleParams() const
  {
  }
} // end Belle2 namespace
