/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/fitter/BaseRecoFitterModule.h>

#include <genfit/KalmanFitStatus.h>
#include <genfit/FitStatus.h>
#include <genfit/MaterialEffects.h>
#include <genfit/FieldManager.h>

#include <tracking/trackFitting/fitter/base/TrackFitter.h>

#include <simulation/monopoles/MonopoleConstants.h>

using namespace Belle2;

BaseRecoFitterModule::BaseRecoFitterModule() :
  Module()
{
  setDescription("Fit the given reco tracks with the given fitter parameters.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("recoTracksStoreArrayName", m_param_recoTracksStoreArrayName, "StoreArray name of the input and output reco tracks.",
           m_param_recoTracksStoreArrayName);

  addParam("pxdHitsStoreArrayName", m_param_pxdHitsStoreArrayName, "StoreArray name of the input PXD hits.",
           m_param_pxdHitsStoreArrayName);
  addParam("svdHitsStoreArrayName", m_param_svdHitsStoreArrayName, "StoreArray name of the input SVD hits.",
           m_param_svdHitsStoreArrayName);
  addParam("cdcHitsStoreArrayName", m_param_cdcHitsStoreArrayName, "StoreArray name of the input CDC hits.",
           m_param_cdcHitsStoreArrayName);
  addParam("bklmHitsStoreArrayName", m_param_bklmHitsStoreArrayName, "StoreArray name of the input BKLM hits.",
           m_param_bklmHitsStoreArrayName);
  addParam("eklmHitsStoreArrayName", m_param_eklmHitsStoreArrayName, "StoreArray name of the input EKLM hits.",
           m_param_eklmHitsStoreArrayName);

  addParam("pdgCodesToUseForFitting", m_param_pdgCodesToUseForFitting,
           "Use these particle hypotheses for fitting. Please use positive pdg codes only.",
           m_param_pdgCodesToUseForFitting);

  addParam("resortHits", m_param_resortHits, "Resort the hits while fitting.",
           m_param_resortHits);

  addParam("initializeCDCTranslators", m_param_initializeCDCTranslators,
           "Configures whether the CDC Translators should be initialized by the FitterModule",
           m_param_initializeCDCTranslators);
  addParam("monopoleMagCharge", Monopoles::monopoleMagCharge,
           "Sets monopole magnetic charge hypothesis if it is in the pdgCodesToUseForFitting",
           Monopoles::monopoleMagCharge);

  addParam("correctSeedCharge", m_correctSeedCharge,
           "If true changes seed charge of the RecoTrack to the one found by the track fit (if it differs).",
           m_correctSeedCharge);
}

void BaseRecoFitterModule::initialize()
{
  m_recoTracks.isRequired(m_param_recoTracksStoreArrayName);

  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2FATAL("Material effects not set up.  Please use SetupGenfitExtrapolationModule.");
  }

  if (!genfit::FieldManager::getInstance()->isInitialized()) {
    B2FATAL("Magnetic field not set up. Please use SetupGenfitExtrapolationModule.");
  }

  genfit::MaterialEffects::getInstance()->setMagCharge(Monopoles::monopoleMagCharge);
}


void BaseRecoFitterModule::event()
{
  // The used fitting algorithm class.
  TrackFitter fitter(m_param_pxdHitsStoreArrayName, m_param_svdHitsStoreArrayName, m_param_cdcHitsStoreArrayName,
                     m_param_bklmHitsStoreArrayName, m_param_eklmHitsStoreArrayName);

  const std::shared_ptr<genfit::AbsFitter>& genfitFitter = createFitter();
  if (genfitFitter) {
    fitter.resetFitter(genfitFitter);
  }

  B2DEBUG(29, "Number of reco track candidates to process: " << m_recoTracks.getEntries());
  unsigned int recoTrackCounter = 0;

  for (RecoTrack& recoTrack : m_recoTracks) {

    if (recoTrack.getNumberOfTotalHits() < 3) {
      B2WARNING("Genfit2Module: only " << recoTrack.getNumberOfTotalHits() << " were assigned to the Track! " <<
                "This Track will not be fitted!");
      continue;
    }

    B2DEBUG(29, "Fitting reco track candidate number " << recoTrackCounter);
    B2DEBUG(29, "Reco track candidate has start values: ");
    B2DEBUG(29, "Momentum: " << recoTrack.getMomentumSeed().X() << " " << recoTrack.getMomentumSeed().Y() << " " <<
            recoTrack.getMomentumSeed().Z());
    B2DEBUG(29, "Position: " << recoTrack.getPositionSeed().X() << " " << recoTrack.getPositionSeed().Y() << " " <<
            recoTrack.getPositionSeed().Z());
    B2DEBUG(29, "Charge: " << recoTrack.getChargeSeed());
    B2DEBUG(29, "Total number of hits assigned to the track: " << recoTrack.getNumberOfTotalHits());



    bool flippedCharge = false;
    for (const unsigned int pdgCodeToUseForFitting : m_param_pdgCodesToUseForFitting) {
      bool wasFitSuccessful;

      if (pdgCodeToUseForFitting != Monopoles::c_monopolePDGCode) {
        Const::ChargedStable particleUsedForFitting(pdgCodeToUseForFitting);
        B2DEBUG(29, "PDG: " << pdgCodeToUseForFitting);
        B2DEBUG(29, "resortHits: " << m_param_resortHits);

        wasFitSuccessful = fitter.fit(recoTrack, particleUsedForFitting, m_param_resortHits);

        // only flip if the current fit was the cardinal rep. and seed charge differs from fitted charge
        if (m_correctSeedCharge && wasFitSuccessful
            && recoTrack.getCardinalRepresentation() == recoTrack.getTrackRepresentationForPDG(pdgCodeToUseForFitting)) {  // charge flipping
          // If the charge after the fit (cardinal rep) is different from the seed charge,
          // we change the charge seed and refit the track
          flippedCharge |= recoTrack.getChargeSeed() != recoTrack.getMeasuredStateOnPlaneFromFirstHit().getCharge();

          // debug
          if (flippedCharge) {
            B2DEBUG(29, "Refitting with opposite charge PDG: " << pdgCodeToUseForFitting);
          }

        }  // end of charge flipping
      } else {
        // Different call signature for monopoles in order not to change Const::ChargedStable types
        wasFitSuccessful = fitter.fit(recoTrack, pdgCodeToUseForFitting, m_param_resortHits);
      }
      const genfit::AbsTrackRep* trackRep = recoTrack.getTrackRepresentationForPDG(pdgCodeToUseForFitting);

      if (!trackRep) {
        B2FATAL("TrackRepresentation for PDG id " << pdgCodeToUseForFitting << " not present in RecoTrack although it " <<
                "should have been created.");
      }

      B2DEBUG(28, "-----> Fit results:");
      if (wasFitSuccessful) {
        const genfit::FitStatus* fs = recoTrack.getTrackFitStatus(trackRep);
        const genfit::KalmanFitStatus* kfs = dynamic_cast<const genfit::KalmanFitStatus*>(fs);
        B2DEBUG(28, "       Chi2 of the fit: " << kfs->getChi2());
        B2DEBUG(28, "       NDF of the fit: " << kfs->getBackwardNdf());
        //Calculate probability
        double pValue = recoTrack.getTrackFitStatus(trackRep)->getPVal();
        B2DEBUG(28, "       pValue of the fit: " << pValue);
        const genfit::MeasuredStateOnPlane& mSoP = recoTrack.getMeasuredStateOnPlaneFromFirstHit(trackRep);
        B2DEBUG(28, "Charge after fit " << mSoP.getCharge());
        B2DEBUG(28, "Position after fit " << mSoP.getPos().X() << " " << mSoP.getPos().Y() << " " << mSoP.getPos().Z());
        B2DEBUG(28, "Momentum after fit " << mSoP.getMom().X() << " " << mSoP.getMom().Y() << " " << mSoP.getMom().Z());
      } else {
        B2DEBUG(28, "       fit failed!");
      }
    } // loop over hypothesis

    // if charge has been flipped reset seed charge and refit all track representations
    if (flippedCharge) {
      recoTrack.setChargeSeed(-recoTrack.getChargeSeed());
      // refit all present track representations
      for (const auto  trackRep : recoTrack.getRepresentations()) {
        Const::ChargedStable particleUsedForFitting(abs(trackRep->getPDG()));
        fitter.fit(recoTrack, particleUsedForFitting);
      }
    }
    recoTrackCounter += 1;
  } // loop tracks
}
