/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/dataobjects/RecoTrack.h>

#include <genfit/AbsKalmanFitter.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/FitStatus.h>
#include <genfit/DAF.h>
#include <genfit/MaterialEffects.h>
#include <genfit/FieldManager.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/RealisticCDCGeometryTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <tracking/modules/fitter/BaseRecoFitterModule.h>

using namespace std;
using namespace Belle2;

BaseRecoFitterModule::BaseRecoFitterModule() :
  Module()
{
  setDescription("Fit the given reco tracks with the given fitter parameters.");

  addParam("RecoTracksStoreArrayName", m_param_recoTracksStoreArrayName, "StoreArray name of the input and output reco tracks.",
           std::string("RecoTracks"));

  addParam("pdgCodeToUseForFitting", m_param_pdgCodeToUseForFitting,
           "Use this particle hypothesis for fitting. Please use the positive pdg code only.", static_cast<unsigned int>(211));

  addParam("useVXDMomentumEstimation", m_param_useVXDMomentumEstimation, "Use the momentum estimation from VXD.", false);
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

  // Create new Translators and give them to the CDCRecoHits.
  CDCRecoHit::setTranslators(new CDC::LinearGlobalADCCountTranslator(),
                             new CDC::RealisticCDCGeometryTranslator(true),
                             new CDC::RealisticTDCCountTranslator(true),
                             true);

}

void BaseRecoFitterModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);

  B2DEBUG(100, "Number of reco track candidates to process: " << recoTracks.getEntries());
  unsigned int recoTrackCounter = 0;

  const std::shared_ptr<genfit::AbsKalmanFitter>& fitter = createFitter();
  fitter->setMaxFailedHits(m_param_maxNumberOfFailedHits);


  for (RecoTrack& recoTrack : recoTracks) {
    B2DEBUG(100, "Fitting reco track candidate number " << recoTrackCounter);
    B2DEBUG(100, "Reco track candidate has start values: ")
    B2DEBUG(100, "Momentum: " << recoTrack.getMomentum().X() << " " << recoTrack.getMomentum().Y() << " " <<
            recoTrack.getMomentum().Z())
    B2DEBUG(100, "Position: " << recoTrack.getPosition().X() << " " << recoTrack.getPosition().Y() << " " <<
            recoTrack.getPosition().Z())
    B2DEBUG(100, "Total number of hits assigned to the track: " << recoTrack.getNumberOfTotalHits())

    recoTrack.fit(fitter, m_param_pdgCodeToUseForFitting, m_param_useVXDMomentumEstimation);

    B2DEBUG(100, "-----> Fit results:");
    B2DEBUG(100, "       Fitted and converged: " << recoTrack.wasLastFitSucessfull());

    if (recoTrack.wasLastFitSucessfull()) {
      genfit::FitStatus* fs = recoTrack.getFitStatus(recoTrack.getCardinalRep());
      genfit::KalmanFitStatus* kfs = dynamic_cast<genfit::KalmanFitStatus*>(fs);

      B2DEBUG(100, "       Chi2 of the fit: " << kfs->getChi2());
      B2DEBUG(100, "       NDF of the fit: " << kfs->getBackwardNdf());
      B2DEBUG(100, "       pValue of the fit: " << kfs->getPVal());
    }

    recoTrackCounter += 1;
  }
}
