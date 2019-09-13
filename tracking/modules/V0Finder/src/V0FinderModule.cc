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


  addParam("massRangeKshort", m_MassRangeKshort, "mass range for reconstructed Kshort used for pre-selection of candidates"
           " (to be chosen loosely as used momenta are not very precise)", m_MassRangeKshort);
  addParam("massRangeLambda", m_MassRangeLambda, "mass range for reconstructed Lambda used for pre-selection of candidates"
           " (to be chosen loosely as used momenta are not very precise)", m_MassRangeLambda);
  addParam("massRangeGamma", m_MassRangeGamma, "mass range for reconstructed photon mass used for pre-selection of candidates"
           " (to be chosen loosely as used momenta are not very precise)", m_MassRangeGamma);
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
        if (preFilterTracks(trackPlus, trackMinus, Const::photon)) m_v0Fitter->fitAndStore(trackPlus, trackMinus, Const::photon);
        if (preFilterTracks(trackPlus, trackMinus, Const::Lambda))  m_v0Fitter->fitAndStore(trackPlus, trackMinus, Const::Lambda);
        if (preFilterTracks(trackPlus, trackMinus, Const::antiLambda)) m_v0Fitter->fitAndStore(trackPlus, trackMinus, Const::antiLambda);

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

  const auto trackHypotheses = m_v0Fitter->getTrackHypotheses(v0Hypothesis);

  // first track should always be the positve one
  TLorentzVector p4trackPlus;
  p4trackPlus.SetVectM(trackPlus->getTrackFitResultWithClosestMass(trackHypotheses.first)->getMomentum(),
                       trackHypotheses.first.getMass());

  // second track should always be the negative  one
  TLorentzVector p4trackMinus;
  p4trackMinus.SetVectM(trackMinus->getTrackFitResultWithClosestMass(trackHypotheses.second)->getMomentum(),
                        trackHypotheses.second.getMass());

  float mass = (p4trackPlus + p4trackMinus).Mag();

  std::tuple<float, float>  massrange = {0., 0.};
  if (v0Hypothesis == Const::Kshort) {
    massrange = m_MassRangeKshort;
  } else if (v0Hypothesis == Const::photon) {
    massrange = m_MassRangeGamma;
  } else if (v0Hypothesis == Const::Lambda or v0Hypothesis == Const::antiLambda) {
    massrange = m_MassRangeKshort;
  } else {
    B2WARNING("This should not happen!");
  }
  return (mass > std::get<0>(massrange) and mass < std::get<1>(massrange));
}
