/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Module manager
#include <framework/core/ModuleManager.h>

// Own include
#include <top/modules/TOPReconstruction/TOPReconstructorModule.h>
#include <top/reconstruction/TOPreco.h>
#include <top/reconstruction/TOPtrack.h>
#include <top/reconstruction/TOPutil.h>
#include <top/reconstruction/TOPconfigure.h>

// Hit classes
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPBarHit.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <geometry/bfieldmap/BFieldMap.h>

// ROOT
#include <TVector3.h>

using namespace std;
using namespace boost;

namespace Belle2 {
  namespace TOP {
    //-----------------------------------------------------------------
    //                 Register the Module
    //-----------------------------------------------------------------

    REG_MODULE(TOPReconstructor)


    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    TOPReconstructorModule::TOPReconstructorModule() : Module(),
      m_debugLevel(0),
      m_smearTrack(false),
      m_topgp(TOPGeometryPar::Instance()),
      m_R1(0),
      m_R2(0),
      m_Z1(0),
      m_Z2(0)
    {
      // Set description
      setDescription("Reconstruction for TOP counter. Uses reconstructed tracks extrapolated to TOP and TOPDigits to calculate log likelihoods for e, mu, pi, K, p.");

      // Set property flags
      setPropertyFlags(c_ParallelProcessingCertified);

      // Add parameters
      addParam("inputTracks", m_inputTracks, "Mdst tracks", string(""));
      addParam("inputExtHits", m_inputExtHits, "Extrapolated tracks", string(""));
      addParam("inputDigits", m_inputDigits, "TOP digits", string(""));
      addParam("inputBarHits", m_inputBarHits, "MCParticle hits at bars", string(""));
      addParam("outputLikelihoods", m_outputLikelihoods, "TOP likelihoods", string(""));
      addParam("minBkgPerBar", m_minBkgPerBar,
               "Minimal number of background photons per bar", 0.0);
      addParam("scaleN0", m_scaleN0, "Scale factor for N0", 1.0);
      addParam("sigmaRphi", m_sigmaRphi, "track smearing sigma in Rphi [cm]", 0.0);
      addParam("sigmaZ", m_sigmaZ, "track smearing sigma in Z [cm]", 0.0);
      addParam("sigmaTheta", m_sigmaTheta, "track smearing sigma in Theta [radians]",
               0.0);
      addParam("sigmaPhi", m_sigmaPhi, "track smearing sigma in Phi [radians]", 0.0);
      addParam("maxTime", m_maxTime,
               "time limit for photons [ns] (0 = use default one)", 0.0);

      for (int i = 0; i < c_Nhyp; i++) {m_Masses[i] = 0;}

    }

    TOPReconstructorModule::~TOPReconstructorModule()
    {
    }

    void TOPReconstructorModule::initialize()
    {
      // check for module debug level

      if (getLogConfig().getLogLevel() == LogConfig::c_Debug) {
        m_debugLevel = getLogConfig().getDebugLevel();
      }

      // Initialize masses

      m_Masses[0] = Const::electron.getMass();
      m_Masses[1] = Const::muon.getMass();
      m_Masses[2] = Const::pion.getMass();
      m_Masses[3] = Const::kaon.getMass();
      m_Masses[4] = Const::proton.getMass();

      // set track smearing flag

      m_smearTrack = m_sigmaRphi > 0 || m_sigmaZ > 0 || m_sigmaTheta > 0 ||
                     m_sigmaPhi > 0;

      // Data store registration

      StoreArray<TOPLikelihood>::registerPersistent(m_outputLikelihoods);
      RelationArray::registerPersistent<Track, TOPLikelihood>(m_inputTracks, m_outputLikelihoods);
      RelationArray::registerPersistent<TOPLikelihood, ExtHit>(m_outputLikelihoods, m_inputExtHits);
      RelationArray::registerPersistent<TOPLikelihood, TOPBarHit>(m_outputLikelihoods, m_inputBarHits);

      StoreArray<TOPDigit>::required();
      StoreArray<Track>::required();
      StoreArray<ExtHit>::required();
      StoreArray<MCParticle>::optional();
      StoreArray<TOPBarHit>::optional();

      // Configure TOP detector

      TOPconfigure config;
      m_R1 = config.getR1();
      m_R2 = config.getR2();
      m_Z1 = config.getZ1();
      m_Z2 = config.getZ2();
      if (m_debugLevel > 0) config.print();
    }

    void TOPReconstructorModule::beginRun()
    {

    }

    void TOPReconstructorModule::event()
    {

      // output: log likelihoods
      StoreArray<TOPLikelihood> topLikelihoods(m_outputLikelihoods);
      topLikelihoods.create();

      // create reconstruction object
      TOPreco reco(c_Nhyp, m_Masses, m_minBkgPerBar, m_scaleN0);

      // set time limit for photons lower than that given by TDC range (optional)
      if (m_maxTime > 0) reco.setTmax(m_maxTime);

      // clear reconstruction object
      reco.Clear();

      // add photons
      StoreArray<TOPDigit> topDigits(m_inputDigits);
      int nHits = topDigits.getEntries();
      for (int i = 0; i < nHits; ++i) {
        const TOPDigit* data = topDigits[i];
        reco.AddData(data->getBarID() - 1, data->getChannelID() - 1, data->getTDC());
      }

      // use pion hypothesis for track extrapolation
      Const::ChargedStable chargedStable = Const::pion;

      // reconstruct track-by-track and store the results
      StoreArray<Track> tracks(m_inputTracks);
      for (int i = 0; i < tracks.getEntries(); ++i) {

        const Track* track = tracks[i];
        const ExtHit* extHit = getExtHit(track, chargedStable);
        if (!extHit) continue;

        TVector3 point = extHit->getPosition();
        double x = point.X();
        double y = point.Y();
        double z = point.Z();
        TVector3 momentum = extHit->getMomentum();
        double px = momentum.X();
        double py = momentum.Y();
        double pz = momentum.Z();

        const TrackFitResult* fitResult = track->getTrackFitResult(chargedStable);
        if (!fitResult) {
          B2ERROR("No TrackFitResult for " << chargedStable.getPDGCode());
          continue;
        }
        int charge = fitResult->getChargeSign();

        const MCParticle* particle = track->getRelated<MCParticle>();
        int truePDGCode = 0;
        if (particle) truePDGCode = particle->getPDG();

        TOPtrack trk(x, y, z, px, py, pz, 0.0, charge, truePDGCode);
        double tof = extHit->getTOF();
        trk.setTrackLength(tof, chargedStable.getMass());

        // optional track smearing (needed for some MC studies)
        if (m_smearTrack) {
          trk.smear(m_sigmaRphi, m_sigmaZ, m_sigmaTheta, m_sigmaPhi);
          B2INFO("TOPReconstructor: additional smearing of track parameters done");
        }

        // reconstruct
        reco.Reconstruct(trk);
        if (m_debugLevel > 1) {
          trk.Dump();
          reco.DumpHit(Local);
          reco.DumpLogL(c_Nhyp);
        }

        // get results
        double logl[c_Nhyp], expPhot[c_Nhyp];
        int nphot;
        reco.GetLogL(c_Nhyp, logl, expPhot, nphot);

        // store results
        TOPLikelihood* topL = topLikelihoods.appendNew(reco.Flag(), logl, nphot, expPhot);

        // make relations:
        track->addRelationTo(topL);
        topL->addRelationTo(extHit);
        if (particle) {
          const TOPBarHit* barHit = particle->getRelated<TOPBarHit>(m_inputBarHits);
          topL->addRelationTo(barHit);
        }
      }

    }


    void TOPReconstructorModule::endRun()
    {

    }

    void TOPReconstructorModule::terminate()
    {

    }

    void TOPReconstructorModule::printModuleParams() const
    {

    }


    const ExtHit* TOPReconstructorModule::getExtHit(const Track* track,
                                                    Const::ChargedStable chargedStable)
    {

      Const::EDetector myDetID = Const::EDetector::TOP;
      int NumBars = m_topgp->getNbars();
      int pdgCode = abs(chargedStable.getPDGCode());

      RelationVector<ExtHit> extHits = track->getRelationsWith<ExtHit>(m_inputExtHits);
      for (unsigned i = 0; i < extHits.size(); i++) {
        const ExtHit* extHit = extHits[i];
        if (abs(extHit->getPdgCode()) != pdgCode) continue;
        if (extHit->getDetectorID() != myDetID) continue;
        if (extHit->getCopyID() == 0 || extHit->getCopyID() > NumBars) continue;
        if (extHit->getStatus() != EXT_ENTER) continue;
        TVector3 point = extHit->getPosition(); // do I need to x-check?
        double x = point.X();
        double y = point.Y();
        double z = point.Z();
        if (z < m_Z1 || z > m_Z2) continue;
        double r = sqrt(x * x + y * y);
        if (r < m_R1 || r > m_R2) continue;
        return extHit;
      }
      return 0;

    }


  } // end top namespace
} // end Belle2 namespace

