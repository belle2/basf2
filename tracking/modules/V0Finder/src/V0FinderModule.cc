/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/V0Finder/V0FinderModule.h>

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/core/ModuleParam.templateDetails.h> // needed for complicated parameter types 

#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;

REG_MODULE(V0Finder);

V0FinderModule::V0FinderModule() : Module()
{
  setDescription("This is a simple V0 finder which matches all positive "
                 "tracks with all negative tracks, fitting a vertex for each "
                 "pair.  Depending on the outcome of each fit, a corresponding "
                 "Belle2::V0 is stored or not.\n\n"

                 "No V0s with vertex inside the beam pipe "
                 "are saved. They are recovered in a following step.\n\n"

                 "Outside the beam pipe only a chi^2 cut is applied "
                 "('vertexChi2CutOutside').\n"
                 "The value used as beam pipe radius is a parameter and "
                 "can be changed.");

  setPropertyFlags(c_ParallelProcessingCertified);

  //input tracks
  addParam("RecoTracks", m_arrayNameRecoTrack,
           "RecoTrack StoreArray name (input)", std::string(""));
  addParam("CopiedRecoTracks", m_arrayNameCopiedRecoTrack,
           "RecoTrack StoreArray name (used for track refitting)", std::string("CopiedRecoTracks"));
  addParam("TrackFitResults", m_arrayNameTFResult,
           "Belle2::TrackFitResult StoreArray name (in- and output).\n"
           "Note that the V0s use pointers indices into these arrays, so all hell may break loose, "
           "if you change this.", std::string(""));
  addParam("Tracks", m_arrayNameTrack,
           "Belle2::Track StoreArray name (input).\n"
           "Note that the V0s use pointers indices into these arrays, so all hell may break loose, "
           "if you change this.", std::string(""));

  // output: V0s
  addParam("V0s", m_arrayNameV0, "V0 StoreArry name (output).", std::string(""));
  addParam("Validation", m_validation, "Create output for validation.", bool(false));
  addParam("V0ValidationVertices", m_arrayNameV0ValidationVertex, "V0ValidationVertex StoreArray name (optional output)",
           std::string(""));

  addParam("beamPipeRadius", m_beamPipeRadius,
           "Radius at which we switch between the two classes of cuts. "
           "The default is a little inside the beam pipe to allow some tolerance.",
           1.);

  addParam("vertexChi2CutOutside", m_vertexChi2CutOutside,
           "Maximum chi² for the vertex fit (NDF = 1)", 10000.);

  addParam("invMassRangeKshort", m_invMassRangeKshort,
           "mass range in GeV for reconstructed Kshort after removing material effects and inner hits", m_invMassRangeKshort);

  addParam("invMassRangeLambda", m_invMassRangeLambda,
           "mass range in GeV for reconstructed Lambda after removing material effects and inner hits", m_invMassRangeLambda);

  addParam("invMassRangePhoton", m_invMassRangePhoton,
           "mass range in GeV for reconstructed Photon after removing material effects and inner hits", m_invMassRangePhoton);

  addParam("v0FitterMode", m_v0FitterMode,
           "designate which fitAndStore function is called in V0Fitter.\n"
           "    0: store V0 at the first vertex fit, regardless of inner hits \n"
           "    1: remove hits inside the V0 vertex position\n"
           "    2: mode 2 +  don't use SVD hits if there is only one available SVD hit-pair (default)",
           1);

  addParam("massRangeKshort", m_preFilterMassRangeKshort,
           "mass range in GeV for reconstructed Kshort used for pre-selection of candidates"
           " (to be chosen loosely as used momenta ignore material effects)", m_preFilterMassRangeKshort);
  addParam("massRangeLambda", m_preFilterMassRangeLambda,
           "mass range in GeV for reconstructed Lambda used for pre-selection of candidates"
           " (to be chosen loosely as used momenta ignore material effects)", m_preFilterMassRangeLambda);
}


void V0FinderModule::initialize()
{
  m_tracks.isRequired(m_arrayNameTrack);
  StoreArray<RecoTrack> recoTracks(m_arrayNameRecoTrack);
  m_tracks.requireRelationTo(recoTracks);
  //All the other required StoreArrays are checked in the Construtor of the V0Fitter.
  m_v0Fitter = std::make_unique<V0Fitter>(m_arrayNameTFResult, m_arrayNameV0,
                                          m_arrayNameV0ValidationVertex, m_arrayNameRecoTrack,
                                          m_arrayNameCopiedRecoTrack, m_validation);

  m_v0Fitter->initializeCuts(m_beamPipeRadius,  m_vertexChi2CutOutside,
                             m_invMassRangeKshort, m_invMassRangeLambda, m_invMassRangePhoton);
  m_v0Fitter->setFitterMode(m_v0FitterMode);

  // safeguard for users that try to break the code
  if (std::get<0>(m_preFilterMassRangeKshort) > std::get<1>(m_preFilterMassRangeKshort)) {
    B2FATAL("The minimum has to be smaller than the maximum of the Kshort mass range! min = " <<  std::get<0>
            (m_preFilterMassRangeKshort) << " max = " << std::get<1>(m_preFilterMassRangeKshort));
  }
  if (std::get<0>(m_preFilterMassRangeLambda) > std::get<1>(m_preFilterMassRangeLambda)) {
    B2FATAL("The minimum has to be smaller than the maximum of the Lambda mass range! min = " <<  std::get<0>
            (m_preFilterMassRangeLambda) << " max = " << std::get<1>(m_preFilterMassRangeLambda));
  }

  // precalculate the mass range squared
  m_mKshortMin2 = std::get<0>(m_preFilterMassRangeKshort) < 0 ? -std::get<0>(m_preFilterMassRangeKshort) * std::get<0>
                  (m_preFilterMassRangeKshort) : std::get<0>
                  (m_preFilterMassRangeKshort) * std::get<0>(m_preFilterMassRangeKshort);
  m_mKshortMax2 = std::get<1>(m_preFilterMassRangeKshort) < 0 ? -std::get<1>(m_preFilterMassRangeKshort) * std::get<1>
                  (m_preFilterMassRangeKshort) : std::get<1>
                  (m_preFilterMassRangeKshort) * std::get<1>(m_preFilterMassRangeKshort);
  m_mLambdaMin2 = std::get<0>(m_preFilterMassRangeLambda) < 0 ? -std::get<0>(m_preFilterMassRangeLambda) * std::get<0>
                  (m_preFilterMassRangeLambda) : std::get<0>
                  (m_preFilterMassRangeLambda) * std::get<0>(m_preFilterMassRangeLambda);
  m_mLambdaMax2 = std::get<1>(m_preFilterMassRangeLambda) < 0 ? -std::get<1>(m_preFilterMassRangeLambda) * std::get<1>
                  (m_preFilterMassRangeLambda) : std::get<1>
                  (m_preFilterMassRangeLambda) * std::get<1>(m_preFilterMassRangeLambda);

}


void V0FinderModule::event()
{
  B2DEBUG(200, m_tracks.getEntries() << " tracks in event.");

  // Group tracks into positive and negative tracks.
  std::vector<const Track*> tracksPlus;
  tracksPlus.reserve(m_tracks.getEntries());

  std::vector<const Track*> tracksMinus;
  tracksMinus.reserve(m_tracks.getEntries());

  for (const auto& track : m_tracks) {
    RecoTrack const* const  recoTrack = track.getRelated<RecoTrack>();
    B2ASSERT("No RecoTrack available for given Track.", recoTrack);

    if (recoTrack->getChargeSeed() > 0) {
      tracksPlus.push_back(&track);
    }
    if (recoTrack->getChargeSeed() < 0) {
      tracksMinus.push_back(&track);
    }
  }

  // Reject boring events.
  if (tracksPlus.empty() || tracksMinus.empty()) {
    B2DEBUG(200, "No interesting track pairs. tracksPlus " << tracksPlus.size() << ", tracksMinus " << tracksMinus.size());
    return;
  }


  // Pair up each positive track with each negative track.
  for (auto& trackPlus : tracksPlus) {
    for (auto& trackMinus : tracksMinus) {
      bool isForceStored, isHitRemoved;
      try {
        if (preFilterTracks(trackPlus, trackMinus, Const::Kshort)) {
          m_v0Fitter->fitAndStore(trackPlus, trackMinus, Const::Kshort, isForceStored, isHitRemoved);
          m_nForceStored += isForceStored;
          m_nHitRemoved += isHitRemoved;
        }
      } catch (const genfit::Exception& e) {
        // genfit exception raised, skip this track pair for this hypothesis
        B2WARNING("Genfit exception caught. Skipping this track pair for Kshort hypothesis. " << LogVar("Genfit exception:", e.what()));
      }

      try {
        // the pre-filter is not able to reject photons, so no need to apply pre filter for photons
        m_v0Fitter->fitAndStore(trackPlus, trackMinus, Const::photon, isForceStored, isHitRemoved);
        m_nForceStored += isForceStored;
        m_nHitRemoved += isHitRemoved;
      } catch (const genfit::Exception& e) {
        // genfit exception raised, skip this track pair for this hypothesis
        B2WARNING("Genfit exception caught. Skipping this track pair for photon hypothesis. " << LogVar("Genfit exception:", e.what()));
      }

      try {
        if (preFilterTracks(trackPlus, trackMinus, Const::Lambda)) {
          m_v0Fitter->fitAndStore(trackPlus, trackMinus, Const::Lambda, isForceStored, isHitRemoved);
          m_nForceStored += isForceStored;
          m_nHitRemoved += isHitRemoved;
        }
      } catch (const genfit::Exception& e) {
        // genfit exception raised, skip this track pair for this hypothesis
        B2WARNING("Genfit exception caught. Skipping this track pair for Lambda hypothesis. " << LogVar("Genfit exception:", e.what()));
      }

      try {
        if (preFilterTracks(trackPlus, trackMinus, Const::antiLambda)) {
          m_v0Fitter->fitAndStore(trackPlus, trackMinus, Const::antiLambda, isForceStored, isHitRemoved);
          m_nForceStored += isForceStored;
          m_nHitRemoved += isHitRemoved;
        }
      } catch (const genfit::Exception& e) {
        // genfit exception raised, skip this track pair for this hypothesis
        B2WARNING("Genfit exception caught. Skipping this track pair for anti-Lambda hypothesis. " << LogVar("Genfit exception:",
                  e.what()));
      }
    }
  }

}

void V0FinderModule::terminate()
{
  B2INFO("===V0Finder summary=============================================================");
  B2INFO("In total " << m_nHitRemoved + m_nForceStored << " saved V0s have inner hits.");
  B2INFO("- Inner hits successfully removed in " << m_nHitRemoved << " V0s.");
  B2INFO("- Failed in " << m_nForceStored << " V0s, instead V0s before removing inner hits saved.");
}

bool
V0FinderModule::preFilterTracks(const Track* trackPlus, const Track* trackMinus, const Const::ParticleType& v0Hypothesis)
{
  const double* range_m2_min = nullptr;
  const double* range_m2_max = nullptr;
  if (v0Hypothesis == Const::Kshort) {
    range_m2_min = &m_mKshortMin2;
    range_m2_max = &m_mKshortMax2;
  } else if (v0Hypothesis == Const::Lambda or v0Hypothesis == Const::antiLambda) {
    range_m2_min = &m_mLambdaMin2;
    range_m2_max = &m_mLambdaMax2;
  } else {
    // this case is not covered so accept everything
    return true;
  }

  const auto trackHypotheses = m_v0Fitter->getTrackHypotheses(v0Hypothesis);

  // first track should always be the positve one
  double m_plus = trackHypotheses.first.getMass();
  double p_plus = trackPlus->getTrackFitResultWithClosestMass(trackHypotheses.first)->getMomentum().Mag();
  double E_plus = sqrt(m_plus * m_plus + p_plus * p_plus);

  // second track is the negative
  double m_minus = trackHypotheses.second.getMass();
  double p_minus = trackMinus->getTrackFitResultWithClosestMass(trackHypotheses.second)->getMomentum().Mag();
  double E_minus = sqrt(m_minus * m_minus + p_minus * p_minus);

  // now do the adding of the 4momenta
  double sum_E2 = (E_minus + E_plus) * (E_minus + E_plus);

  // the minimal/maximal allowed mass for these 4momenta is given if the 3momenta are aligned ( cos(angle)= +/- 1 )
  double candmass_min2 = sum_E2 - (p_plus + p_minus) * (p_plus + p_minus);
  double candmass_max2 = sum_E2 - (p_plus - p_minus) * (p_plus - p_minus);

  // if true possible candiate mass overlaps with the user specified range
  bool in_range = candmass_max2 > *range_m2_min and candmass_min2 < *range_m2_max;

  return in_range;
}
