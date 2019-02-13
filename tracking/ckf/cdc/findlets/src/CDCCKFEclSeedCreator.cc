/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Simon Kurz, Nils Braun                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/cdc/findlets/CDCCKFEclSeedCreator.h>
#include <tracking/ckf/general/utilities/SearchDirection.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/ckf/cdc/entities/CDCCKFState.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;

CDCCKFEclSeedCreator::CDCCKFEclSeedCreator() : Super()
{

}

void CDCCKFEclSeedCreator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "inputECLshowersStoreArrayName"),
                                m_param_inputEclShowerStoreArrayName,
                                "StoreArray name of the input Shower Store Array.");

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "eclSeedRecoTrackStoreArrayName"),
                                m_param_eclSeedRecoTrackStoreArrayName,
                                "StoreArray name of the output Track Store Array.");

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimalEnRequirementCluster"),
                                m_param_minimalEnRequirement,
                                "Minimal energy requirement for the input clusters",
                                m_param_minimalEnRequirement);
}

void CDCCKFEclSeedCreator::initialize()
{
  Super::initialize();

  m_inputECLshowers.isRequired(m_param_inputEclShowerStoreArrayName);

  m_eclSeedRecoTracks.registerInDataStore(m_param_eclSeedRecoTrackStoreArrayName);
  RecoTrack::registerRequiredRelations(m_eclSeedRecoTracks);

  // TODO: add relation to ECL shower
}

void CDCCKFEclSeedCreator::apply(std::vector<CDCCKFPath>& seeds)
{
  for (auto& shower : m_inputECLshowers) {
    // TODO: TrackLoader checks if RecoTrack present in relatedRecoTrackStoreArrayName. Does this have to be done?
    // Also does relationCheckForDirection, is this necessary?

    // choose photon hypothesis since electrons not used?! Talk to Torben.
    if (shower.getHypothesisId() != ECLShower::c_nPhotons) {
      return;
    }

    const double Eclus  = shower.getEnergy();
    if (Eclus < m_param_minimalEnRequirement) {
      continue;
    }

    const double thetaClus  = shower.getTheta();
    const double phiClus  = shower.getPhi();
    const double rClus  = shower.getR();

    const TVector3 pos(rClus * sin(thetaClus) * cos(phiClus), rClus * sin(thetaClus) * sin(phiClus), rClus * cos(thetaClus));

    // the seed momentum has to point away from the center!
    const TVector3 mom(Eclus * sin(thetaClus) * cos(phiClus), Eclus * sin(thetaClus) * sin(phiClus), Eclus * cos(thetaClus));

    // electron and positron hypothesis
    RecoTrack* eclSeedEle = m_eclSeedRecoTracks.appendNew(pos, mom, -1);
    RecoTrack* eclSeedPos = m_eclSeedRecoTracks.appendNew(pos, mom, +1);

    CDCCKFState seedStateEle(eclSeedEle, nullptr);
    seeds.push_back({seedStateEle});
    CDCCKFState seedStatePos(eclSeedPos, nullptr);
    seeds.push_back({seedStatePos});

    // TODO: add relation to ECL shower
  }
}
