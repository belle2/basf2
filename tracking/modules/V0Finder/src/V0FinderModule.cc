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
           "Maximum chi² for the vertex fit (NDF = 1)", 50.);


  addParam("massRangeKshort", m_MassRangeKshort, "mass range in GeV for reconstructed Kshort used for pre-selection of candidates"
           " (to be chosen loosely as used momenta are ignore material effects)", m_MassRangeKshort);
  addParam("massRangeLambda", m_MassRangeLambda, "mass range in GeV for reconstructed Lambda used for pre-selection of candidates"
           " (to be chosen loosely as used momenta are ignore material effects)", m_MassRangeLambda);
  addParam("massRangeGamma", m_MassRangeGamma, "mass range in GeV for reconstructed photon mass used for pre-selection of candidates"
           " (to be chosen loosely as used momenta are ignore material effects)", m_MassRangeGamma);
}


void V0FinderModule::initialize()
{
  m_tracks.isRequired(m_arrayNameTrack);
  StoreArray<RecoTrack> recoTracks(m_arrayNameRecoTrack);
  m_tracks.requireRelationTo(recoTracks);
  //All the other required StoreArrays are checked in the Construtor of the V0Fitter.
  m_v0Fitter = std::make_unique<V0Fitter>(m_arrayNameTFResult, m_arrayNameV0,
                                          m_arrayNameV0ValidationVertex, m_arrayNameRecoTrack, m_validation);

  m_v0Fitter->initializeCuts(m_beamPipeRadius,  m_vertexChi2CutOutside);

  // precalculate the mass range squared
  m_mGammaMin2 = std::get<0>(m_MassRangeGamma) < 0 ? -std::get<0>(m_MassRangeGamma) * std::get<0>(m_MassRangeGamma) : std::get<0>
                 (m_MassRangeGamma) * std::get<0>(m_MassRangeGamma);
  m_mGammaMax2 = std::get<1>(m_MassRangeGamma) < 0 ? -std::get<1>(m_MassRangeGamma) * std::get<1>(m_MassRangeGamma) : std::get<1>
                 (m_MassRangeGamma) * std::get<1>(m_MassRangeGamma);
  m_mKshortMin2 = std::get<0>(m_MassRangeKshort) < 0 ? -std::get<0>(m_MassRangeKshort) * std::get<0>(m_MassRangeKshort) : std::get<0>
                  (m_MassRangeKshort) * std::get<0>(m_MassRangeKshort);
  m_mKshortMax2 = std::get<1>(m_MassRangeKshort) < 0 ? -std::get<1>(m_MassRangeKshort) * std::get<1>(m_MassRangeKshort) : std::get<1>
                  (m_MassRangeKshort) * std::get<1>(m_MassRangeKshort);
  m_mLambdaMin2 = std::get<0>(m_MassRangeLambda) < 0 ? -std::get<0>(m_MassRangeLambda) * std::get<0>(m_MassRangeLambda) : std::get<0>
                  (m_MassRangeLambda) * std::get<0>(m_MassRangeLambda);
  m_mLambdaMax2 = std::get<1>(m_MassRangeLambda) < 0 ? -std::get<1>(m_MassRangeLambda) * std::get<1>(m_MassRangeLambda) : std::get<1>
                  (m_MassRangeLambda) * std::get<1>(m_MassRangeLambda);

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
    B2ASSERT(recoTrack, "No RecoTrack available for given Track.");

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
      // TODO: this will throw away all hypotheses if one of them fails! Not sure if that is a problem or how frequent it is?
      try {
        if (preFilterTracks(trackPlus, trackMinus, Const::Kshort)) m_v0Fitter->fitAndStore(trackPlus, trackMinus, Const::Kshort);
        //else std::cout << "rejected" << std::endl;
        if (preFilterTracks(trackPlus, trackMinus, Const::photon)) m_v0Fitter->fitAndStore(trackPlus, trackMinus, Const::photon);
        //else std::cout << "rejected" << std::endl;
        if (preFilterTracks(trackPlus, trackMinus, Const::Lambda))  m_v0Fitter->fitAndStore(trackPlus, trackMinus, Const::Lambda);
        //else std::cout << "rejected" << std::endl;
        if (preFilterTracks(trackPlus, trackMinus, Const::antiLambda)) m_v0Fitter->fitAndStore(trackPlus, trackMinus, Const::antiLambda);
        //else std::cout << "rejected" << std::endl;

      } catch (const genfit::Exception& e) {
        // genfit exception raised, skip this track pair
        B2DEBUG(27, "Genfit exception caught: " << e.what() << "skip the track pair and continue");
        continue;
      }
    }
  }
}


bool
V0FinderModule::preFilterTracks(const Track* trackPlus, const Track* trackMinus, const Const::ParticleType& v0Hypothesis)
{
  // the used method is not able to reject photons
  if (v0Hypothesis == Const::photon) {
    //std::cout << "I am a gamma, so no filtering" << std::endl;
    //std::cout << "accepted 1" << std::endl;
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


  const double* range_m2_min = nullptr;
  const double* range_m2_max = nullptr;
  if (v0Hypothesis == Const::Kshort) {
    range_m2_min = &m_mKshortMin2;
    range_m2_max = &m_mKshortMax2;
  } else if (v0Hypothesis == Const::Lambda or v0Hypothesis == Const::antiLambda) {
    range_m2_min = &m_mLambdaMin2;
    range_m2_max = &m_mLambdaMax2;
  } else if (v0Hypothesis == Const::photon) {
    range_m2_min = &m_mGammaMin2;
    range_m2_max = &m_mGammaMax2;
  } else {
    B2WARNING("This should not happen!");
  }

  // if true possible candiate mass overlaps with the user specified range
  bool in_range = candmass_max2 > *range_m2_min and candmass_min2 < *range_m2_max;

  // debugging output
  //std::cout << std::endl;
  //std::cout << v0Hypothesis.__repr__() << std::endl;
  //std::cout << "cand plus m " << m_plus << " p " << p_plus << " E " << E_plus << std::endl;
  //std::cout << "cand plus m " << m_minus << " p " << p_minus << " E " << E_minus << std::endl;
  //std::cout << "cand mass2 min " << candmass_min2 << " max " << candmass_max2 << std::endl;
  //std::cout << "user range " << *range_m2_min << " " << *range_m2_max << std::endl;
  //std::cout << "accepted " << in_range << std::endl;

  return in_range;
}
