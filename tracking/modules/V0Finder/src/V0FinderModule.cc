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
  setDescription("This is a simple V0 finder for X = Ks, Lambda and converted fotons "
                 "which matches all positive tracks with all negative tracks, "
                 "fitting a vertex for each pair. "
                 "Depending on the outcome of each fit, a corresponding "
                 "Belle2::V0 is stored or not.\n\n"
                 "A loose cut on the invariant mass (``massRangeX``) is applied before the fit (not considering material effects), "
                 "then a vertex fit is performed and only pairs passing a chi^2 (``vertexChi2CutOutside``) "
                 "and a second cut on the invariant mass (``invMassRangeX``) are stored as Belle2::V0. \n\n"
                 "No V0s with vertex inside the beam pipe "
                 "are saved as they can be recovered at analysis level. ");

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
  addParam("Validation", m_useValidation, "Create output for validation.", bool(false));
  addParam("V0ValidationVertices", m_arrayNameV0ValidationVertex, "V0ValidationVertex StoreArray name (optional output)",
           std::string(""));

  addParam("beamPipeRadius", m_beamPipeRadius,
           "Radius at which we switch between the two classes of cuts. "
           "The default is a little inside the beam pipe to allow some tolerance.",
           1.);

  addParam("vertexChi2CutOutside", m_vertexChi2CutOutside,
           "Maximum chi^2 for the vertex fit (NDF = 1)", 10000.);

  addParam("invMassRangeKshort", m_invMassRangeKshort,
           "mass range in GeV for reconstructed Kshort after removing material effects and inner hits", m_invMassRangeKshort);

  addParam("invMassRangeLambda", m_invMassRangeLambda,
           "mass range in GeV for reconstructed Lambda after removing material effects and inner hits", m_invMassRangeLambda);

  addParam("invMassRangePhoton", m_invMassRangePhoton,
           "mass range in GeV for reconstructed Photon after removing material effects and inner hits", m_invMassRangePhoton);

  addParam("v0FitterMode", m_v0FitterMode,
           "designate which fitAndStore function is called in V0Fitter.\n"
           "0: store V0 at the first vertex fit, regardless of inner hits; \n"
           "1: remove hits inside the V0 vertex position;\n"
           "2: mode 1 + don't use SVD hits if there is only one available SVD hit-pair",
           1);

  addParam("massRangeKshort", m_preFilterMassRangeKshort,
           "mass range in GeV for reconstructed Kshort used for pre-selection of candidates"
           " (to be chosen loosely as used momenta ignore material effects)", m_preFilterMassRangeKshort);
  addParam("massRangeLambda", m_preFilterMassRangeLambda,
           "mass range in GeV for reconstructed Lambda used for pre-selection of candidates"
           " (to be chosen loosely as used momenta ignore material effects)", m_preFilterMassRangeLambda);
  addParam("precutRho", m_precutRho, "preselection cut on the transverse radius of the point-of-closest-approach of two tracks. "
           "Set value to 0 to accept all.", 0.5);
  addParam("precutCosAlpha", m_precutCosAlpha, "preselection cut on the cosine of opening angle between two tracks. "
           "Those above this cut are always accepted.", 0.9);
  addParam("useNewV0Fitter", m_useNewV0Fitter, "on true use new V0 fitter, othewise use the old one", false);
}


void V0FinderModule::initialize()
{
  m_tracks.isRequired(m_arrayNameTrack);
  StoreArray<RecoTrack> recoTracks(m_arrayNameRecoTrack);
  m_tracks.requireRelationTo(recoTracks);
  //All the other required StoreArrays are checked in the Construtor of the V0Fitter.

  if (m_useNewV0Fitter) {
    m_newV0Fitter = std::make_unique<NewV0Fitter>(m_arrayNameTFResult, m_arrayNameV0,
                                                  m_arrayNameV0ValidationVertex, m_arrayNameRecoTrack,
                                                  m_arrayNameCopiedRecoTrack, m_useValidation);
    m_newV0Fitter->initializeCuts(m_beamPipeRadius,  m_vertexChi2CutOutside,
                                  m_invMassRangeKshort, m_invMassRangeLambda, m_invMassRangePhoton);
    m_newV0Fitter->setFitterMode(m_v0FitterMode);
  } else {
    m_v0Fitter = std::make_unique<V0Fitter>(m_arrayNameTFResult, m_arrayNameV0,
                                            m_arrayNameV0ValidationVertex, m_arrayNameRecoTrack,
                                            m_arrayNameCopiedRecoTrack, m_useValidation);
    m_v0Fitter->initializeCuts(m_beamPipeRadius,  m_vertexChi2CutOutside,
                               m_invMassRangeKshort, m_invMassRangeLambda, m_invMassRangePhoton);
    m_v0Fitter->setFitterMode(m_v0FitterMode);
  }

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
  B2DEBUG(29, m_tracks.getEntries() << " tracks in event.");

  // Group tracks into positive and negative tracks.
  std::vector<const Track*> tracksPlus;
  tracksPlus.reserve(m_tracks.getEntries());

  std::vector<const Track*> tracksMinus;
  tracksMinus.reserve(m_tracks.getEntries());

  for (const auto& track : m_tracks) {
    RecoTrack const* const  recoTrack = track.getRelated<RecoTrack>(m_arrayNameRecoTrack);
    B2ASSERT("No RecoTrack available for given Track.", recoTrack);

    if (recoTrack->getChargeSeed() > 0) {
      tracksPlus.push_back(&track);
    }
    if (recoTrack->getChargeSeed() < 0) {
      tracksMinus.push_back(&track);
    }
  }

  // Reject boring events.
  if (tracksPlus.empty() or tracksMinus.empty()) {
    B2DEBUG(29, "No interesting track pairs. tracksPlus " << tracksPlus.size() << ", tracksMinus " << tracksMinus.size());
    return;
  }

  // Pair up each positive track with each negative track.
  for (auto& trackPlus : tracksPlus) {
    for (auto& trackMinus : tracksMinus) {
      if (not isTrackPairSelected(trackPlus, trackMinus)) continue;

      if (preFilterTracks(trackPlus, trackMinus, Const::Kshort)) fitAndStore(trackPlus, trackMinus, Const::Kshort);
      if (preFilterTracks(trackPlus, trackMinus, Const::Lambda)) fitAndStore(trackPlus, trackMinus, Const::Lambda);
      if (preFilterTracks(trackPlus, trackMinus, Const::antiLambda)) fitAndStore(trackPlus, trackMinus, Const::antiLambda);
      // the pre-filter is not able to reject photons, so no need to apply pre filter for photons
      fitAndStore(trackPlus, trackMinus, Const::photon);
    }
  }

}


void V0FinderModule::terminate()
{
  B2INFO("===V0Finder summary=============================================================");
  B2INFO("In total " << m_nHitRemoved + m_nForceStored << " of " << m_allStored << " saved V0s have inner hits.");
  B2INFO("- Inner hits successfully removed in " << m_nHitRemoved << " V0s.");
  B2INFO("- The hit removal failed in " << m_nForceStored << " V0s, instead V0s before removing inner hits saved.");
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

  const auto trackHypotheses = m_newV0Fitter ? m_newV0Fitter->getTrackHypotheses(v0Hypothesis) : m_v0Fitter->getTrackHypotheses(
                                 v0Hypothesis);

  // first track should always be the positve one
  double m_plus = trackHypotheses.first.getMass();
  double p_plus = trackPlus->getTrackFitResultWithClosestMass(trackHypotheses.first)->getMomentum().R();
  double E_plus = sqrt(m_plus * m_plus + p_plus * p_plus);

  // second track is the negative
  double m_minus = trackHypotheses.second.getMass();
  double p_minus = trackMinus->getTrackFitResultWithClosestMass(trackHypotheses.second)->getMomentum().R();
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


bool V0FinderModule::isTrackPairSelected(const Track* track1, const Track* track2)
{
  if (m_precutRho <= 0) return true;

  auto* fit1 = track1->getTrackFitResultWithClosestMass(Belle2::Const::pion);
  if (not fit1) return false;
  auto r1 = fit1->getPosition();        // point on the straight line
  auto k1 = fit1->getMomentum().Unit(); // direction of the line

  auto* fit2 = track2->getTrackFitResultWithClosestMass(Belle2::Const::pion);
  if (not fit2) return false;
  auto r2 = fit2->getPosition();        // point on the straight line
  auto k2 = fit2->getMomentum().Unit(); // direction of the line

  double cosAlpha = k1.Dot(k2);  // cosine of opening angle between two tracks
  if (cosAlpha > m_precutCosAlpha) return true;

  // Find points, p1 and p2, on the straight lines that are closest to each other, i.e.
  //   (p2 - p1).Dot(k1) = 0 and (p2 - p1).Dot(k2) = 0,
  // where p1 = r1 + k1 * lam1 and p2 = r2 + k2 * lam2,
  // and lam1 and lam2 are running parameters - the unknowns of the equations.
  //
  // After rearangement the system of equations reads:
  //
  //   lam1 - cosAlpha * lam2 = b1, b1 = (r2 - r1).Dot(k1),
  //   cosAlpha * lam1 - lam2 = b2, b2 = (r2 - r1).Dot(k2)

  double D = cosAlpha * cosAlpha - 1; // determinant of the system of two equations
  if (D == 0) return true; // tracks are parallel

  auto dr = r2 - r1;
  double b1 = dr.Dot(k1);
  double b2 = dr.Dot(k2);
  double lam1 = (-b1 + b2 * cosAlpha) / D; // solution for the first straight line
  double lam2 = (b2 - b1 * cosAlpha) / D;  // solution for the second staright line
  auto p1 = r1 + k1 * lam1;  // point on the first line closest to the second line
  auto p2 = r2 + k2 * lam2;  // point on the second line closest to the first line
  auto poca = (p1 + p2) / 2; // POCA of two straight lines, an approximation for the vertex

  return poca.Rho() > m_precutRho;
}


void V0FinderModule::fitAndStore(const Track* trackPlus, const Track* trackMinus, const Const::ParticleType& v0Hypothesis)
{
  try {
    bool isForceStored = false, isHitRemoved = false;
    if (m_newV0Fitter) {
      bool ok = m_newV0Fitter->fitAndStore(trackPlus, trackMinus, v0Hypothesis, isForceStored, isHitRemoved);
      if (ok) m_allStored++;
    } else {
      bool ok = m_v0Fitter->fitAndStore(trackPlus, trackMinus, v0Hypothesis, isForceStored, isHitRemoved);
      if (ok) m_allStored++;
    }
    m_nForceStored += isForceStored;
    m_nHitRemoved += isHitRemoved;
  } catch (const genfit::Exception& e) {
    // genfit exception raised, skip this track pair for this hypothesis
    B2WARNING("Genfit exception caught. Skipping this track pair"
              << LogVar("V0 hypothesis PDG", v0Hypothesis.getPDGCode())
              << LogVar("Genfit exception:", e.what()));
  }
}

