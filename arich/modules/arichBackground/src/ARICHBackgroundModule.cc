/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
// Own include

#include <arich/modules/arichBackground/ARICHBackgroundModule.h>

#include <time.h>

// Hit classes

#include <simulation/dataobjects/BeamBackHit.h>
#include <arich/dataobjects/ARICHSimHit.h>
#include <arich/dataobjects/ARICHDigit.h>

#include <mdst/dataobjects/MCParticle.h>

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
  namespace arich {
    //-----------------------------------------------------------------
    ///                Register the Module
    //-----------------------------------------------------------------

    REG_MODULE(ARICHBackground);


    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    ARICHBackgroundModule::ARICHBackgroundModule() : Module(), m_phpos(TVector3()), m_phmom(TVector3()),
      m_phVtx(TVector3()), m_phMmom(TVector3()), m_phMvtx(TVector3()), m_phPvtx(TVector3()), m_phPmom(TVector3()),
      m_phGMvtx(TVector3()), m_phGMmom(TVector3()), m_modOrig(TVector3()), m_source(0), m_phPDG(0), m_phMPDG(0), m_phPPDG(0),
      m_phGMPDG(0), m_type(0), m_edep(0.0), m_ttime(0.0), m_moduleID(0), m_phnw(0.0), m_trackLength(0.0), m_energy(0.0),
      m_outputFile(NULL),
      m_outputTree(NULL)
    {
      // Set description()
      setDescription("ARICHBackground module. Used to extract information relevant for ARICH background from background files");

      // Add parameters
      addParam("FileName", m_filename, "output file name", string("mytree.root"));
      addParam("BkgTag", m_bkgTag, "background type tag (appended to hits in the output tree", 0);

    }

    ARICHBackgroundModule::~ARICHBackgroundModule()
    {

    }

    void ARICHBackgroundModule::initialize()
    {
      // Print set parameters
      printModuleParams();

      m_outputFile = new TFile(m_filename.c_str(), "RECREATE");

      m_outputTree = new TTree("m_outputTree", "tree of arich background hits");
      m_outputTree->Branch("source", &m_source, "m_source/I");
      m_outputTree->Branch("phpos", "TVector3", &m_phpos);
      m_outputTree->Branch("phmom", "TVector3", &m_phmom);
      m_outputTree->Branch("phVtx", "TVector3", &m_phVtx);
      m_outputTree->Branch("phMmom", "TVector3", &m_phMmom);
      m_outputTree->Branch("phMvtx", "TVector3", &m_phMvtx);
      m_outputTree->Branch("phPvtx", "TVector3", &m_phPvtx);
      m_outputTree->Branch("phPmom", "TVector3", &m_phPmom);
      m_outputTree->Branch("phGMvtx", "TVector3", &m_phGMvtx);
      m_outputTree->Branch("phGMmom", "TVector3", &m_phGMmom);
      m_outputTree->Branch("modOrig", "TVector3", &m_modOrig);
      m_outputTree->Branch("phPDG", &m_phPDG, "m_phPDG/I");
      m_outputTree->Branch("phMPDG", &m_phMPDG, "m_phMPDG/I");
      m_outputTree->Branch("phPPDG", &m_phPPDG, "m_phPPDG/I");
      m_outputTree->Branch("phGMPDG", &m_phGMPDG, "m_phGMPDG/I");
      m_outputTree->Branch("type", &m_type, "m_type/I");
      m_outputTree->Branch("edep", &m_edep, "m_edep/D");
      m_outputTree->Branch("ttime", &m_ttime, "m_ttime/D");
      m_outputTree->Branch("moduleID", &m_moduleID, "m_moduleID/I");
      m_outputTree->Branch("phnw", &m_phnw, "m_phnw/D");
      m_outputTree->Branch("trackLength", &m_trackLength, "m_trackLength/D");
      m_outputTree->Branch("energy", &m_energy, "m_energy/D");

      m_source = m_bkgTag;
    }

    void ARICHBackgroundModule::beginRun()
    {
      // Print run number
      B2INFO("ARICHBackground: Processing. ");

    }

    void ARICHBackgroundModule::event()
    {
      int nHits = m_BeamBackHits.getEntries();

      for (int iHit = 0; iHit < nHits; ++iHit) {

        BeamBackHit* arichhit = m_BeamBackHits[iHit];
        int subdet = arichhit->getSubDet();
        if (subdet != 4) continue;
        m_type = 0;
        if (arichhit->getIdentifier() == 1) m_type = 1;
        m_edep = arichhit->getEnergyDeposit();
        m_ttime = arichhit->getTime();
        m_phPDG = arichhit->getPDG();
        m_phpos = arichhit->getPosition();
        m_phmom = arichhit->getMomentum();
        m_moduleID = m_arichgp->getDetectorPlane().pointSlotID(m_phpos.X(), m_phpos.Y());
        double r = m_arichgp->getDetectorPlane().getSlotR(m_moduleID);
        double phi = m_arichgp->getDetectorPlane().getSlotPhi(m_moduleID);
        m_modOrig = TVector3(r * cos(phi), r * sin(phi), 0);
        m_energy = arichhit->getEnergy();

        if (m_phPDG == Const::neutron.getPDGCode()) {
          m_phnw = arichhit->getNeutronWeight();
          m_trackLength = arichhit->getTrackLength();
        }
        m_phVtx = TVector3(0, 0, 0); m_phMvtx = TVector3(0, 0, 0);  m_phMmom = TVector3(0, 0, 0);
        m_phMPDG = -1; m_phPPDG = -1; m_phGMPDG = -1;
        m_phPvtx = TVector3(0, 0, 0); m_phPmom = TVector3(0, 0, 0); m_phGMvtx = TVector3(0, 0, 0); m_phGMmom = TVector3(0, 0, 0);

        RelationIndex<MCParticle, BeamBackHit> relBeamBackHitToMCParticle(m_MCParticles, m_BeamBackHits);
        if (relBeamBackHitToMCParticle.getFirstElementTo(arichhit)) {
          const MCParticle* currParticle = relBeamBackHitToMCParticle.getFirstElementTo(arichhit)->from;
          m_phVtx = B2Vector3D(currParticle->getVertex());
          const MCParticle* mother = currParticle->getMother();
          int mm = 0;
          while (mother) {
            if (mm == 0) {
              m_phMPDG = mother->getPDG();
              m_phMvtx = B2Vector3D(mother->getVertex());
              m_phMmom = B2Vector3D(mother->getMomentum());
            }
            if (mm == 1) {
              m_phGMPDG = mother->getPDG();
              m_phGMvtx = B2Vector3D(mother->getVertex());
              m_phGMmom = B2Vector3D(mother->getMomentum());
            }
            const MCParticle* pommother = mother->getMother();
            if (!pommother) {
              m_phPPDG = mother->getPDG();
              m_phPvtx = B2Vector3D(mother->getVertex());
              m_phPmom = B2Vector3D(mother->getMomentum());
            }
            mother = pommother;
            mm++;
          }

        }
        m_outputTree->Fill();
      }

      nHits = m_ARICHSimHits.getEntries();

      for (int iHit = 0; iHit < nHits; ++iHit) {
        ARICHSimHit* simHit = m_ARICHSimHits[iHit];
        m_moduleID = simHit->getModuleID();
        m_type = 2;
        m_phPDG = 0;
        m_edep = 0;
        m_ttime = simHit->getGlobalTime();

        m_phVtx = TVector3(0, 0, 0); m_phMvtx = TVector3(0, 0, 0);  m_phMmom = TVector3(0, 0, 0);
        m_phMPDG = -1; m_phPPDG = -1; m_phGMPDG = -1; m_phmom = TVector3(0, 0, 0);
        m_phPvtx = TVector3(0, 0, 0); m_phPmom = TVector3(0, 0, 0); m_phGMvtx = TVector3(0, 0, 0); m_phGMmom = TVector3(0, 0, 0);
        RelationIndex<MCParticle, ARICHSimHit> relSimHitToMCParticle(m_MCParticles, m_ARICHSimHits);
        if (relSimHitToMCParticle.getFirstElementTo(simHit)) {
          const MCParticle* currParticle = relSimHitToMCParticle.getFirstElementTo(simHit)->from;
          m_phVtx = B2Vector3D(currParticle->getVertex());
          const MCParticle* mother = currParticle->getMother();
          int mm = 0;
          while (mother) {
            if (mm == 0) {
              m_phMPDG = mother->getPDG();
              m_phMvtx = B2Vector3D(mother->getVertex());
              m_phMmom = B2Vector3D(mother->getMomentum());
            }
            if (mm == 1) {
              m_phGMPDG = mother->getPDG();
              m_phGMvtx = B2Vector3D(mother->getVertex());
              m_phGMmom = B2Vector3D(mother->getMomentum());
            }
            const MCParticle* pommother = mother->getMother();
            if (!pommother) {
              m_phPPDG = mother->getPDG();
              m_phPvtx = B2Vector3D(mother->getVertex());
              m_phPmom = B2Vector3D(mother->getMomentum());
            }
            mother = pommother;
            mm++;
          }

        }
        m_outputTree->Fill();
      }
    }

    void ARICHBackgroundModule::endRun()
    {
    }

    void ARICHBackgroundModule::terminate()
    {
      // CPU time end

      // Announce
      B2INFO("ARICHBackground finished.");

      //m_outputTree->Write();
      m_outputFile->Write();
      m_outputFile->Close();

    }

    void ARICHBackgroundModule::printModuleParams() const
    {

    }


  } // end arich namespace
} // end Belle2 namespace
