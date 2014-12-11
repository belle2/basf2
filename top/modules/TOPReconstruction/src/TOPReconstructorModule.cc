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
#include <top/reconstruction/TOPconfigure.h>

// Hit classes
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPBarHit.h>
#include <top/dataobjects/TOPRecBunch.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TVector3.h>

using namespace std;
using namespace boost;

namespace Belle2 {
  using namespace TOP;
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
    setDescription("Reconstruction for TOP counter. Uses reconstructed tracks "
                   "extrapolated to TOP and TOPDigits to calculate log likelihoods "
                   "for charged stable particles");

    // Set property flags
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("minBkgPerBar", m_minBkgPerBar,
             "Minimal number of background photons per bar", 0.0);
    addParam("scaleN0", m_scaleN0,
             "Scale factor for N0", 1.0);
    addParam("sigmaRphi", m_sigmaRphi,
             "track smearing sigma in Rphi [cm]", 0.0);
    addParam("sigmaZ", m_sigmaZ,
             "track smearing sigma in Z [cm]", 0.0);
    addParam("sigmaTheta", m_sigmaTheta,
             "track smearing sigma in Theta [radians]", 0.0);
    addParam("sigmaPhi", m_sigmaPhi,
             "track smearing sigma in Phi [radians]", 0.0);
    addParam("maxTime", m_maxTime,
             "time limit for photons [ns] (0 = use full TDC range)", 51.2);

    for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) {m_masses[i] = 0;}
    for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) {m_pdgCodes[i] = 0;}

  }


  TOPReconstructorModule::~TOPReconstructorModule()
  {
  }


  void TOPReconstructorModule::initialize()
  {
    // input

    StoreArray<TOPDigit> topDigits;
    topDigits.isRequired();

    StoreArray<Track> tracks;
    tracks.isRequired();

    StoreArray<ExtHit> extHits;
    extHits.isRequired();

    StoreArray<MCParticle> mcParticles;
    mcParticles.isOptional();

    StoreArray<TOPBarHit> barHits;
    barHits.isOptional();

    StoreObjPtr<TOPRecBunch> recBunch;
    recBunch.isOptional(); // to be changed to required in future

    // output

    StoreArray<TOPLikelihood> topLikelihoods;
    topLikelihoods.registerInDataStore();
    topLikelihoods.registerRelationTo(extHits);
    topLikelihoods.registerRelationTo(barHits);
    tracks.registerRelationTo(topLikelihoods);

    // check for module debug level

    if (getLogConfig().getLogLevel() == LogConfig::c_Debug) {
      m_debugLevel = getLogConfig().getDebugLevel();
    }

    // Initialize masses

    for (const auto & part : Const::chargedStableSet) {
      m_masses[part.getIndex()] = part.getMass();
      m_pdgCodes[part.getIndex()] = abs(part.getPDGCode());
    }

    // set track smearing flag

    m_smearTrack = m_sigmaRphi > 0 || m_sigmaZ > 0 || m_sigmaTheta > 0 ||
                   m_sigmaPhi > 0;

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

    StoreArray<TOPLikelihood> topLikelihoods;
    topLikelihoods.create();

    // create reconstruction object

    TOPreco reco(Const::ChargedStable::c_SetSize, m_masses, m_minBkgPerBar, m_scaleN0);
    reco.setHypID(Const::ChargedStable::c_SetSize, m_pdgCodes);

    // set time limit for photons lower than that given by TDC range (optional)

    if (m_maxTime > 0) reco.setTmax(m_maxTime);

    // get reconstructed bunch time (if available)

    double bunchTime = 0;
    StoreObjPtr<TOPRecBunch> recBunch;
    if (recBunch.isValid()) {
      bunchTime = recBunch->getTime();
      if (recBunch->isSimulated() and !recBunch->isReconstructed())
        B2WARNING("TOPReconstructor: bunch time is simulated but not reconstructed");
    }

    // add photons

    StoreArray<TOPDigit> topDigits;
    for (int i = 0; i < topDigits.getEntries(); ++i) {
      const TOPDigit* data = topDigits[i];
      if (data->getHitQuality() == TOPDigit::EHitQuality::c_Good)
        reco.addData(data->getBarID(), data->getChannelID(), data->getTDC(), bunchTime);
    }

    // reconstruct track-by-track and store the results

    StoreArray<Track> tracks;
    for (int i = 0; i < tracks.getEntries(); ++i) {
      const Track* track = tracks[i];

      // construct TOPtrack from mdst track
      TOPtrack trk(track);
      if (!trk.isValid()) continue;

      // optional track smearing (needed for some MC studies)
      if (m_smearTrack) {
        trk.smear(m_sigmaRphi, m_sigmaZ, m_sigmaTheta, m_sigmaPhi);
        B2INFO("TOPReconstructor: additional smearing of track parameters done");
      }

      // reconstruct
      reco.reconstruct(trk);
      if (m_debugLevel > 1) {
        trk.dump();
        reco.dumpTrackHit(Local);
        reco.dumpLogL(Const::ChargedStable::c_SetSize);
      }

      // get results
      double logl[Const::ChargedStable::c_SetSize];
      double estPhot[Const::ChargedStable::c_SetSize];
      int nphot = 0;
      reco.getLogL(Const::ChargedStable::c_SetSize, logl, estPhot, nphot);
      double estBkg = reco.getExpectedBG();

      // store results
      TOPLikelihood* topL = topLikelihoods.appendNew(reco.getFlag(), nphot,
                                                     logl, estPhot, estBkg);
      // make relations:
      track->addRelationTo(topL);
      topL->addRelationTo(trk.getExtHit());
      topL->addRelationTo(trk.getBarHit());
    }

  }


  void TOPReconstructorModule::endRun()
  {
  }

  void TOPReconstructorModule::terminate()
  {
  }



} // end Belle2 namespace

