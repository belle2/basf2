/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
// Own header.

#include <arich/modules/arichReconstruction/ARICHReconstructorModule.h>
#include <time.h>

// Hit classes
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <arich/dataobjects/ARICHAeroHit.h>
#include <arich/dataobjects/ARICHLikelihood.h>
#include <arich/dataobjects/ARICHTrack.h>
#include <arich/dataobjects/ARICHHit.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

using namespace boost;

namespace Belle2 {

  //-----------------------------------------------------------------
  ///                Register the Module
  //-----------------------------------------------------------------

  REG_MODULE(ARICHReconstructor);

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ARICHReconstructorModule::ARICHReconstructorModule() :
    m_ana(NULL)
  {
    // Set description()
    setDescription("This module calculates the ARICHLikelihood values for all particle id. hypotheses, for all tracks that enter ARICH in the event.");

    // Set property flags
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("trackPositionResolution", m_trackPositionResolution,
             "Resolution of track position on aerogel plane (for additional smearing of MC tracks)", 1.0 * Unit::mm);
    addParam("trackAngleResolution", m_trackAngleResolution,
             "Resolution of track direction angle on aerogel plane (for additional smearing of MC tracks)", 2.0 * Unit::mrad);
    addParam("inputTrackType", m_inputTrackType, "Input tracks switch: tracking (0), from AeroHits - MC info (1)", 0);
    addParam("storePhotons", m_storePhot, "Set to 1 to store reconstructed photon information (Ch. angle,...)", 0);
    addParam("useAlignment", m_align, "Use ARICH global position alignment constants", true);
    addParam("useMirrorAlignment", m_alignMirrors, "Use ARICH mirror alignment constants", true);
  }

  ARICHReconstructorModule::~ARICHReconstructorModule()
  {
    if (m_ana) delete m_ana;
  }

  void ARICHReconstructorModule::initialize()
  {
    // Initialize variables
    if (m_ana) delete m_ana;
    m_ana = new ARICHReconstruction(m_storePhot);
    m_ana->setTrackPositionResolution(m_trackPositionResolution);
    m_ana->setTrackAngleResolution(m_trackAngleResolution);
    m_ana->useMirrorAlignment(m_alignMirrors);

    // Input: ARICHDigits
    m_ARICHHits.isRequired();

    m_Tracks.isOptional();
    m_ExtHits.isOptional();
    m_aeroHits.isOptional();

    if (!m_aeroHits.isOptional()) {
      m_Tracks.isRequired();
      m_ExtHits.isRequired();
    }

    StoreArray<MCParticle> MCParticles;
    MCParticles.isOptional();

    // Output - log likelihoods
    m_ARICHLikelihoods.registerInDataStore();

    m_ARICHTracks.registerInDataStore();

    if (m_inputTrackType) m_ARICHTracks.registerRelationTo(m_ARICHLikelihoods);
    else {
      m_ARICHTracks.registerRelationTo(m_ExtHits);
      m_Tracks.registerRelationTo(m_ARICHLikelihoods);
    }
    //m_ARICHTracks.registerInDataStore(DataStore::c_DontWriteOut);
    //m_ARICHTracks.registerRelationTo(m_ARICHLikelihoods, DataStore::c_Event, DataStore::c_DontWriteOut);
    m_ARICHTracks.registerRelationTo(m_aeroHits);
    printModuleParams();
  }

  void ARICHReconstructorModule::beginRun()
  {
    m_ana->initialize();
  }

  void ARICHReconstructorModule::event()
  {
    // using track information form tracking system (mdst Track)
    if (m_inputTrackType == 0) {

      Const::EDetector myDetID = Const::EDetector::ARICH; // arich
      Const::ChargedStable hypothesis = Const::pion;
      int pdgCode = abs(hypothesis.getPDGCode());

      for (const Track& track : m_Tracks) {
        const TrackFitResult* fitResult = track.getTrackFitResultWithClosestMass(hypothesis);
        if (!fitResult) {
          B2ERROR("No TrackFitResult for " << hypothesis.getPDGCode());
          continue;
        }

        const MCParticle* mcParticle = track.getRelated<MCParticle>();

        ARICHAeroHit* aeroHit = NULL;
        if (mcParticle) aeroHit = mcParticle->getRelated<ARICHAeroHit>();

        RelationVector<ExtHit> extHits = DataStore::getRelationsWithObj<ExtHit>(&track);
        ARICHTrack* arichTrack = NULL;

        //const ExtHit* arich1stHit = NULL;
        const ExtHit* arich2ndHit = NULL;
        const ExtHit* arichWinHit = NULL;

        for (unsigned i = 0; i < extHits.size(); i++) {
          const ExtHit* extHit = extHits[i];
          if (abs(extHit->getPdgCode()) != pdgCode or
              extHit->getDetectorID() != myDetID or
              extHit->getStatus() != EXT_EXIT or // particles registered at the EXIT of the Al plate
              extHit->getMomentum().Z() < 0.0 or // track passes in backward
              extHit->getCopyID() == 12345) { continue;}
          if (extHit->getCopyID() == 6789) {  arich2ndHit = extHit; continue;}
          arichWinHit = extHit;
        }

        if (arich2ndHit) {
          // if aeroHit cannot be found using MCParticle check if it was already related to the extHit (by ARICHRelate module)
          if (!aeroHit) aeroHit = arich2ndHit->getRelated<ARICHAeroHit>();

          // make new ARICHTrack
          arichTrack = m_ARICHTracks.appendNew(arich2ndHit);
          if (arichWinHit) arichTrack->setHapdWindowHit(arichWinHit);
        }

        // skip if track has no extHit in ARICH
        if (!arichTrack) continue;
        // transform track parameters to ARICH local frame
        m_ana->transformTrackToLocal(*arichTrack, m_align);
        // make new ARICHLikelihood
        ARICHLikelihood* like = m_ARICHLikelihoods.appendNew();
        // calculate and set likelihood values
        m_ana->likelihood2(*arichTrack, m_ARICHHits, *like);
        // make relations
        track.addRelationTo(like);
        arichTrack->addRelationTo(arich2ndHit);
        if (aeroHit) arichTrack->addRelationTo(aeroHit);

      } // Tracks loop
    } // input type if


    // using track information form MC (stored in ARICHAeroHit)
    else {

      // Loop over all ARICHAeroHits
      for (const ARICHAeroHit& aeroHit : m_aeroHits) {

        // make new ARICHTrack
        ARICHTrack* arichTrack = m_ARICHTracks.appendNew(&aeroHit);
        // smearing of track parameters (to mimic tracking system resolutions)
        m_ana->smearTrack(*arichTrack);
        // transform track parameters to ARICH local frame
        m_ana->transformTrackToLocal(*arichTrack, m_align);
        // make associated ARICHLikelihood
        ARICHLikelihood* like = m_ARICHLikelihoods.appendNew();
        // calculate and set likelihood values
        m_ana->likelihood2(*arichTrack, m_ARICHHits, *like);
        // make relation
        arichTrack->addRelationTo(like);
        arichTrack->addRelationTo(&aeroHit);
      } // for iTrack

    }
  }

  void ARICHReconstructorModule::printModuleParams()
  {
    if (m_inputTrackType == 0) { B2DEBUG(100, "ARICHReconstructorModule: track infromation is taken from mdst Tracks.");}
    else  B2DEBUG(100, "ARICHReconstructorModule: track information is taken from MC (ARICHAeroHit).");
  }

} // namespace Belle2
