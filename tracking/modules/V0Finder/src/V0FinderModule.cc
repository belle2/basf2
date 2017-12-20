#include <tracking/modules/V0Finder/V0FinderModule.h>
//Object with performing the actual algorithm:
#include <tracking/v0Finding/fitter/V0Fitter.h>
//Needed to perform assertion of presence of setup:
#include "genfit/FieldManager.h"
#include "genfit/MaterialEffects.h"

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

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
                 "The value used as beam pipe radius is a parameter and"
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
           "Radius at which we switch between the two classes of cuts."
           "The default is a little inside the beam pipe to allow some tolerance.",
           1.);

  addParam("vertexChi2CutOutside", m_vertexChi2CutOutside,
           "Maximum chi² for the vertex fit (NDF = 1)", 50.);
}


void V0FinderModule::initialize()
{
  m_recoTracks.isRequired(m_arrayNameRecoTrack);
  m_tfResults.isRequired(m_arrayNameTFResult);
  m_tracks.isRequired(m_arrayNameTrack);
  m_v0s.registerInDataStore(m_arrayNameV0, DataStore::c_WriteOut | DataStore::c_ErrorIfAlreadyRegistered);

  if (m_validation) {
    B2DEBUG(300, "Register DataStore for validation.");
    m_v0ValidationVertices.registerInDataStore(m_arrayNameV0ValidationVertex);
  }

  B2ASSERT(genfit::MaterialEffects::getInstance()->isInitialized(),
           "Material effects not set up.  Please use SetupGenfitExtrapolationModule.");

  B2ASSERT(genfit::FieldManager::getInstance()->isInitialized(),
           "Magnetic field not set up.  Please use SetupGenfitExtrapolationModule.");
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
    B2DEBUG(200, "No interesting track pairs.");
    return;
  }

  V0Fitter v0Fitter(m_arrayNameTFResult, m_arrayNameV0, m_arrayNameV0ValidationVertex, m_arrayNameRecoTrack);
  v0Fitter.initializeCuts(m_beamPipeRadius,  m_vertexChi2CutOutside);
  if (m_validation) {
    v0Fitter.enableValidation();
  }

  // Pair up each positive track with each negative track.
  for (auto& trackPlus : tracksPlus) {
    for (auto& trackMinus : tracksMinus) {
      v0Fitter.fitAndStore(trackPlus, trackMinus, Const::Kshort);
      v0Fitter.fitAndStore(trackPlus, trackMinus, Const::photon);
      v0Fitter.fitAndStore(trackPlus, trackMinus, Const::Lambda);
      v0Fitter.fitAndStore(trackPlus, trackMinus, Const::antiLambda);
    }
  }
}
