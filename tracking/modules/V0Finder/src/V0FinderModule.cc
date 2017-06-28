#include <tracking/modules/V0Finder/V0FinderModule.h>

#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/V0.h>
#include <tracking/dataobjects/V0ValidationVertex.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/v0Finding/dataobjects/VertexVector.h>
#include <tracking/v0Finding/fitter/V0Fitter.h>
#include <TMath.h>
#include <TLorentzVector.h>

// TODO: This dependency can be removed, because the initialization of genfit happens centralized.
#include "genfit/FieldManager.h"
#include "genfit/MaterialEffects.h"

using namespace Belle2;

REG_MODULE(V0Finder);

V0FinderModule::V0FinderModule() : Module()
{
  setDescription("This is a simple V0 finder which matches all positive "
                 "tracks with all negative tracks, fitting a vertex for each "
                 "pair.  Depending on the outcome of each fit, a corresponding "
                 "Belle2::V0 is stored or not.\n\n"

                 "Inside the beam pipe (cut determined by the option "
                 "'beamPipeRadius') a loose chi2 cut is applied "
                 "('vertexChi2CutInside') and, since all tracks coming from a "
                 "single B decay will actually fit into a vertex, an additional "
                 "mass cut restricting to Kshorts is applied "
                 "('massWindowKshortInside').\n\n"

                 "Outside the beam pipe only a chi^2 cut is applied "
                 "('vertexChi2CutOutside').");

  setPropertyFlags(c_ParallelProcessingCertified);

  //input tracks
  addParam("RecoTrackColName", m_RecoTrackColName,
           "RecoTrack collection name (input)", std::string(""));
  addParam("TFRColName", m_TFRColName,
           "Belle2::TrackFitResult collection name (input).  Note that the V0s "
           "use pointers indices into these arrays, so all hell may break loose "
           "if you change this.", std::string(""));
  addParam("TrackColName", m_TrackColName,
           "Belle2::Track collection name (input).  Note that the V0s use "
           "pointers indices into these arrays, so all hell may break loose "
           "if you change this.", std::string(""));

  // output: V0s
  addParam("V0ColName", m_V0ColName, "V0 collection name (output)", std::string(""));
  addParam("Validation", m_validation, "Create output for validation.", bool(false));
  addParam("V0ValidationVertexColName", m_V0ValidationVertexColName, "V0ValidationVertex collection name (optional output)",
           std::string(""));

  addParam("beamPipeRadius", m_beamPipeRadius,
           "Radius at which we switch between the two classes of cuts.  The "
           "default is a little inside the beam pipe to allow some tolerance.",
           1.);
  addParam("vertexChi2CutInside", m_vertexChi2CutInside,
           "Maximum chi² for the vertex fit (NDF = 1)", 50.);
  // The mass window was agreed upon at the 2015-03-06 software
  // developer meeting.  It matches the one used in Belle.
  addParam("massWindowKshortInside", m_massWindowKshortInside,
           "Half-width of symmetric mass window about the Kshort mass for "
           "which Kshort candidates inside the beam pipe are stored (in MeV)",
           30.);
  addParam("vertexChi2CutOutside", m_vertexChi2CutOutside,
           "Maximum chi² for the vertex fit (NDF = 1)", 50.);
}


V0FinderModule::~V0FinderModule()
{
}


void V0FinderModule::initialize()
{
  StoreArray<Track> tracks(m_TrackColName);
  tracks.isRequired();

  StoreArray<TrackFitResult> trackFitResults(m_TFRColName);
  trackFitResults.isRequired();

  StoreArray<RecoTrack> recoTracks(m_RecoTrackColName);
  recoTracks.isRequired();

  StoreArray<V0> v0s(m_V0ColName);
  v0s.registerInDataStore(DataStore::c_WriteOut | DataStore::c_ErrorIfAlreadyRegistered);

  if (m_validation) {
    B2DEBUG(300, "Register DataStore for validation.");
    StoreArray<V0ValidationVertex> validationV0s(m_V0ValidationVertexColName);
    validationV0s.registerInDataStore(DataStore::c_WriteOut);
  }

  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2FATAL("Material effects not set up.  Please use SetupGenfitExtrapolationModule.");
  }

  if (!genfit::FieldManager::getInstance()->isInitialized()) {
    B2FATAL("Magnetic field not set up.  Please use SetupGenfitExtrapolationModule.");
  }
}


void V0FinderModule::beginRun()
{
}

void V0FinderModule::event()
{
  StoreArray<Track> tracks(m_TrackColName);
  const int nTracks = tracks.getEntries();

  if (nTracks == 0)
    return;

  B2DEBUG(200, nTracks << " tracks in event.");

  // Group tracks into positive and negative tracks.
  std::vector<const Track*> tracksPlus;
  tracksPlus.reserve(nTracks);

  std::vector<const Track*> tracksMinus;
  tracksMinus.reserve(nTracks);

  for (const auto& track : tracks) {
    const RecoTrack* recoTrack = track.getRelated<RecoTrack>();

    if (!recoTrack) {
      B2WARNING("No RecoTrack for this track");
      continue;
    }

    const double charge = recoTrack->getChargeSeed();
    if (charge == +1) {
      tracksPlus.push_back(&track);
    } else {
      tracksMinus.push_back(&track);
    }
  }

  // Reject boring events.
  if (tracksPlus.size() == 0 || tracksMinus.size() == 0) {
    B2DEBUG(200, "No interesting tracks.");
    return;
  }

  V0Fitter v0Fitter(m_TFRColName, m_V0ColName, m_V0ValidationVertexColName, m_RecoTrackColName);
  v0Fitter.initializeCuts(m_beamPipeRadius, m_vertexChi2CutInside, m_massWindowKshortInside, m_vertexChi2CutOutside);
  if (m_validation) {
    v0Fitter.enableValidation();
  }

  // Pair up each positive track with each negative track.
  for (auto& trackPlus : tracksPlus) {
    for (auto& trackMinus : tracksMinus) {
      v0Fitter.fitAndStore(trackPlus, trackMinus, Const::Kshort);
      v0Fitter.fitAndStore(trackPlus, trackMinus, Const::photon);
    }
  }
}

void V0FinderModule::endRun()
{
}

void V0FinderModule::terminate()
{
}
