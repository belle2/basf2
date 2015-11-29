#include <tracking/modules/V0Finder/V0FinderModule.h>

#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/V0.h>

#include <TMath.h>
#include <TLorentzVector.h>

#include "genfit/Track.h"
#include "genfit/GFRaveVertexFactory.h"
#include "genfit/GFRaveTrackParameters.h"

// The following includes can be retired once we disable the TGeo
// stuff, genfit initialization should hopefully be centralized at
// that time.
#include "genfit/FieldManager.h"
#include "genfit/MaterialEffects.h"
#include "genfit/TGeoMaterialInterface.h"
#include <TGeoManager.h>
#include <geometry/GeometryManager.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <tracking/gfbfield/GFGeant4Field.h>


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
  addParam("GFTrackColName", m_GFTrackColName,
           "genfit::Track collection name (input)", std::string(""));
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

  StoreArray<genfit::Track> gfTracks(m_GFTrackColName);
  gfTracks.isRequired();
  gfTracks.registerRelationTo(trackFitResults);

  StoreArray<V0> v0s(m_V0ColName);
  v0s.registerInDataStore(DataStore::c_WriteOut | DataStore::c_ErrorIfAlreadyRegistered);

  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2WARNING("Material effects not set up, doing this myself with default values.  Please use SetupGenfitExtrapolationModule.");

    if (gGeoManager == NULL) { //setup geometry and B-field for Genfit if not already there
      geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
      geoManager.createTGeoRepresentation();
    }
    genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());

    // activate / deactivate material effects in genfit
    genfit::MaterialEffects::getInstance()->setEnergyLossBetheBloch(true);
    genfit::MaterialEffects::getInstance()->setNoiseBetheBloch(true);
    genfit::MaterialEffects::getInstance()->setNoiseCoulomb(true);
    genfit::MaterialEffects::getInstance()->setEnergyLossBrems(true);
    genfit::MaterialEffects::getInstance()->setNoiseBrems(true);

    genfit::MaterialEffects::getInstance()->setMscModel("Highland");
  }

  if (!genfit::FieldManager::getInstance()->isInitialized()) {
    B2WARNING("Magnetic field not set up, doing this myself.");

    //pass the magnetic field to genfit
    genfit::FieldManager::getInstance()->init(new GFGeant4Field());
    genfit::FieldManager::getInstance()->useCache();
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

  StoreArray<genfit::Track> gfTracks(m_GFTrackColName);
  StoreArray<TrackFitResult> trackFitResults(m_TFRColName);
  StoreArray<V0> v0s(m_V0ColName);


  // Group tracks into positive and negative tracks.
  typedef std::pair<Track*, genfit::Track*> trackPair;
  std::vector<trackPair> tracksPlus;
  tracksPlus.reserve(nTracks);
  std::vector<trackPair> tracksMinus;
  tracksMinus.reserve(nTracks);

  for (auto& track : tracks) {
    const TrackFitResult* tfr = track.getTrackFitResult(Const::pion);

    if (!tfr) {
      B2WARNING("No TrackFitResult for track");
      continue;
    }
    if (tfr->getParticleType() != Const::pion) {
      B2DEBUG(99, "Requested TrackFitResult for pion, got something else");
      continue;
    }

    genfit::Track* gfTrack = tfr->getRelatedFrom<genfit::Track>(m_GFTrackColName);

    if (!gfTrack) {
      B2ERROR("No genfit::Track for TrackFitResult");
      continue;
    }

    const double charge = tfr->getChargeSign();
    if (charge == +1) {
      tracksPlus.push_back({&track, gfTrack});
    } else {
      tracksMinus.push_back({&track, gfTrack});
    }
  }

  // Reject boring events.
  if (tracksPlus.size() == 0 || tracksMinus.size() == 0) {
    B2DEBUG(200, "No interesting tracks.");
    return;
  }

  std::vector< std::pair< genfit::Track*, genfit::Track*> > vertices;

  // Pair up each positive track with each negative track.
  for (auto& trackPlusPair : tracksPlus) {
    genfit::Track* gfTrackPlus = trackPlusPair.second;

    for (auto& trackMinusPair : tracksMinus) {
      genfit::Track* gfTrackMinus = trackMinusPair.second;

      genfit::MeasuredStateOnPlane stPlus = gfTrackPlus->getFittedState();
      genfit::MeasuredStateOnPlane stMinus = gfTrackMinus->getFittedState();
      const TVector3& posPlus = stPlus.getPos();
      const TVector3& posMinus = stMinus.getPos();

      // Starting point: point closest to axis where either track is defined
      //
      // This is intended to reject tracks that curl away before
      // meeting, there are corner cases where this could throw away
      // legitimate candidates, namely where one track makes a full
      // circle through the detector without hitting any detectors
      // then making it past Rstart without hitting the detector there
      // -- while still being part of the V0.  Unlikely, I didn't find
      // a single example in MC.  On the other hand it rejects
      // impossible candidates.
      const double Rstart = std::min(posPlus.Perp(), posMinus.Perp());
      try {
        stPlus.extrapolateToCylinder(Rstart);
        stMinus.extrapolateToCylinder(Rstart);
      } catch (...) {
        B2DEBUG(200, "Extrapolation to cylinder failed.");
        continue;
      }

      class vertVect {
        // Need this container for exception-safe cleanup, GFRave's
        // interface isn't exception-safe as is.  I guess this could
        // fail if delete throws an exception ...
      public:
        ~vertVect() noexcept
        {
          for (size_t i = 0; i < v.size(); ++i)
            delete v[i];
        }
        size_t size() const noexcept { return v.size(); }
        genfit::GFRaveVertex*& operator[](int idx) { return v[idx]; }
        std::vector<genfit::GFRaveVertex*> v;
      } vertexVector;
      std::vector<genfit::Track*> trackPair {gfTrackPlus, gfTrackMinus};

      try {
        genfit::GFRaveVertexFactory vertexFactory;
        vertexFactory.findVertices(&vertexVector.v, trackPair);
      } catch (...) {
        B2ERROR("Exception during vertex fit.");
        continue;
      }
      if (vertexVector.size() != 1) {
        B2INFO("Vertex fit failed. Size of vertexVector not 1, but: " << vertexVector.size());
        continue;
      }

      const genfit::GFRaveVertex& vert = *vertexVector[0];
      const TVector3& posVert(vert.getPos());

      // Apply cuts.  We have one set of cuts inside the beam pipe,
      // the other outside.
      if (posVert.Perp() < m_beamPipeRadius) {
        if (vert.getChi2() > m_vertexChi2CutInside) {
          B2DEBUG(200, "Vertex inside beam pipe, chi^2 too large.");
          continue;
        }

        // Reconstruct invariant mass.
        //
        // FIXME we assume that only the pion hypothesis is employed.
        // Non-pion tracks are ignored at this point.
        if (vert.getNTracks() != 2) {
          B2ERROR("Wrong number of tracks in vertex.");
          continue;
        }

        const genfit::GFRaveTrackParameters* tr0 = vert.getParameters(0);
        const genfit::GFRaveTrackParameters* tr1 = vert.getParameters(1);

        if (fabs(tr0->getPdg()) != Const::pion.getPDGCode()
            || fabs(tr1->getPdg()) != Const::pion.getPDGCode()) {
          B2ERROR("Unsupported particle hypothesis in V0.");
          continue;
        }

        TLorentzVector lv0, lv1;
        lv0.SetVectM(tr0->getMom(), Const::pionMass);
        lv1.SetVectM(tr1->getMom(), Const::pionMass);

        const double mReco = (lv0 + lv1).M();
        if (fabs(mReco - Const::K0Mass) > m_massWindowKshortInside * Unit::MeV) {
          B2DEBUG(200, "Vertex inside beam pipe, outside Kshort mass window.");
          continue;
        }
      } else {
        if (vert.getChi2() > m_vertexChi2CutOutside) {
          B2DEBUG(200, "Vertex outside beam pipe, chi^2 too large.");
          continue;
        }
      }

      B2DEBUG(200, "Vertex accepted.");

      vertices.push_back({gfTrackPlus, gfTrackMinus});

      // Assemble V0s.
      try {
        stPlus.extrapolateToPoint(posVert);
        stMinus.extrapolateToPoint(posVert);
      } catch (...) {
        // This shouldn't ever happen, but I can see the extrapolation
        // code trying several windings before giving up, so this
        // happens occasionally.  Something more stable would perhaps
        // be desirable.
        B2ERROR("Could not extrapolate track to vertex.");
        continue;
      }

      double Bx, By, Bz;
      genfit::FieldManager::getInstance()->getFieldVal(posVert.X(), posVert.Y(), posVert.Z(),
                                                       Bx, By, Bz);

      TrackFitResult* tfrPlusVtx
        = trackFitResults.appendNew(stPlus.getPos(), stPlus.getMom(),
                                    stPlus.get6DCov(), stPlus.getCharge(),
                                    Const::pion,
                                    gfTrackPlus->getFitStatus()->getPVal(),
                                    Bz / 10., 0, 0);
      TrackFitResult* tfrMinusVtx
        = trackFitResults.appendNew(stMinus.getPos(), stMinus.getMom(),
                                    stMinus.get6DCov(), stMinus.getCharge(),
                                    Const::pion,
                                    gfTrackMinus->getFitStatus()->getPVal(),
                                    Bz / 10., 0, 0);

      DataStore::addRelationFromTo(gfTrackPlus, tfrPlusVtx);
      DataStore::addRelationFromTo(gfTrackMinus, tfrMinusVtx);

      B2DEBUG(100, "Creating new V0.");
      v0s.appendNew(std::make_pair(trackPlusPair.first, tfrPlusVtx),
                    std::make_pair(trackMinusPair.first, tfrMinusVtx));
    }
  }
}

void V0FinderModule::endRun()
{
}

void V0FinderModule::terminate()
{
}
