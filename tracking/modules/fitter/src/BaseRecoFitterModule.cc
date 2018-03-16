/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/fitter/BaseRecoFitterModule.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <genfit/AbsKalmanFitter.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/FitStatus.h>
#include <genfit/MaterialEffects.h>
#include <genfit/FieldManager.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

using namespace std;
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
}

void BaseRecoFitterModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2FATAL("Material effects not set up.  Please use SetupGenfitExtrapolationModule.");
  }

  if (!genfit::FieldManager::getInstance()->isInitialized()) {
    B2FATAL("Magnetic field not set up. Please use SetupGenfitExtrapolationModule.");
  }
}


void BaseRecoFitterModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);

  // The used fitting algorithm class.
  TrackFitter fitter(m_param_pxdHitsStoreArrayName, m_param_svdHitsStoreArrayName, m_param_cdcHitsStoreArrayName,
                     m_param_bklmHitsStoreArrayName, m_param_eklmHitsStoreArrayName);

  const std::shared_ptr<genfit::AbsFitter>& genfitFitter = createFitter();
  if (genfitFitter) {
    fitter.resetFitter(genfitFitter);
  }

  B2DEBUG(100, "Number of reco track candidates to process: " << recoTracks.getEntries());
  unsigned int recoTrackCounter = 0;

  for (RecoTrack& recoTrack : recoTracks) {
    if (recoTrack.getNumberOfTotalHits() < 3) {
      B2WARNING("Genfit2Module: only " << recoTrack.getNumberOfTotalHits() << " were assigned to the Track! " <<
                "This Track will not be fitted!");
      continue;
    }

    B2DEBUG(100, "Fitting reco track candidate number " << recoTrackCounter);
    B2DEBUG(100, "Reco track candidate has start values: ");
    B2DEBUG(100, "Momentum: " << recoTrack.getMomentumSeed().X() << " " << recoTrack.getMomentumSeed().Y() << " " <<
            recoTrack.getMomentumSeed().Z());
    B2DEBUG(100, "Position: " << recoTrack.getPositionSeed().X() << " " << recoTrack.getPositionSeed().Y() << " " <<
            recoTrack.getPositionSeed().Z());
    B2DEBUG(100, "Charge: " << recoTrack.getChargeSeed());
    B2DEBUG(100, "Total number of hits assigned to the track: " << recoTrack.getNumberOfTotalHits());

    for (const unsigned int pdgCodeToUseForFitting : m_param_pdgCodesToUseForFitting) {
      Const::ChargedStable particleUsedForFitting(pdgCodeToUseForFitting);
      B2DEBUG(100, "PDG: " << pdgCodeToUseForFitting);
      const bool wasFitSuccessful = fitter.fit(recoTrack, particleUsedForFitting);
      const genfit::AbsTrackRep* trackRep = TrackFitter::getTrackRepresentationForPDG(pdgCodeToUseForFitting, recoTrack);

      B2DEBUG(99, "-----> Fit results:");
      if (wasFitSuccessful) {
        const genfit::FitStatus* fs = recoTrack.getTrackFitStatus(trackRep);
        const genfit::KalmanFitStatus* kfs = dynamic_cast<const genfit::KalmanFitStatus*>(fs);
        B2DEBUG(99, "       Chi2 of the fit: " << kfs->getChi2());
        B2DEBUG(99, "       NDF of the fit: " << kfs->getBackwardNdf());
        //Calculate probability
        double pValue = recoTrack.getTrackFitStatus(trackRep)->getPVal();
        B2DEBUG(99, "       pValue of the fit: " << pValue);
        const genfit::MeasuredStateOnPlane& mSoP = recoTrack.getMeasuredStateOnPlaneFromFirstHit(trackRep);
        B2DEBUG(99, "Charge after fit " << mSoP.getCharge());
        B2DEBUG(99, "Position after fit " << mSoP.getPos().X() << " " << mSoP.getPos().Y() << " " << mSoP.getPos().Z());
        B2DEBUG(99, "Momentum after fit " << mSoP.getMom().X() << " " << mSoP.getMom().Y() << " " << mSoP.getMom().Z());
      } else {
        B2DEBUG(99, "       fit failed!");
      }
    }
    recoTrackCounter += 1;
  }
}
