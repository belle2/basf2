/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Simon Kurz, Nils Braun                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/general/findlets/ECLTrackCreator.h>
#include <tracking/ckf/general/utilities/SearchDirection.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;

ECLTrackCreator::ECLTrackCreator() : Super()
{

}

void ECLTrackCreator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
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

void ECLTrackCreator::initialize()
{
  Super::initialize();

  m_inputECLshowers.isRequired(m_param_inputEclShowerStoreArrayName);

  StoreArray<RecoTrack> relationRecoTracks(m_param_eclSeedRecoTrackStoreArrayName);
  relationRecoTracks.registerRelationTo(m_eclSeedRecoTracks);
}

void ECLTrackCreator::apply(std::vector<RecoTrack*>& seeds)
{
  // only reserve for photon hypothesis (/2)
  seeds.reserve(seeds.size() + m_inputECLshowers.getEntries() / 2);

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

    seeds.push_back(eclSeedEle);
    seeds.push_back(eclSeedPos);

    // TODO: add relation to ECL shower
  }
}