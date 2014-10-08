#include <tracking/modules/V0Finder/V0FinderModule.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <geometry/GeometryManager.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <tracking/gfbfield/GFGeant4Field.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/V0.h>

#include <TMath.h>
#include <TGeoManager.h>
#include <TLorentzVector.h>
#include <TDecompChol.h>

#include "genfit/Track.h"
#include "genfit/RKTrackRep.h"
#include "genfit/FieldManager.h"
#include "genfit/SharedPlanePtr.h"
#include "genfit/TGeoMaterialInterface.h"
#include "genfit/MaterialEffects.h"
#include "genfit/TGeoMaterialInterface.h"
#include "genfit/GFRaveVertexFactory.h"
#include "genfit/GFRaveTrackParameters.h"

using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
//Note that the 'Module' part of the class name is missing, this is also the way it
//will be called in the module list.
REG_MODULE(V0Finder);

V0FinderModule::V0FinderModule() : Module()
{
  setDescription("This is a simple V0 finder which matches all positive tracks with all negative tracks.  The two main cuts are 'vertexChi2Cut' (which rejects really bad vertex fits) and 'distRfromIP' (which rejects vertices that happen too close to the IP in the perpendicular direction).");
  setPropertyFlags(c_ParallelProcessingCertified);

  //input tracks
  addParam("GFTrackColName",  m_GFTrackColName,  "genfit::Track collection name (input)", std::string(""));
  addParam("TFRColName",  m_TFRColName,  "Belle2::TrackFitResult collection name (input).  Note that the V0s use pointers indices into these arrays, so all hell may break loose if you change this.", std::string(""));
  addParam("TrackColName",  m_TrackColName,  "Belle2::Track collection name (input).  Note that the V0s use pointers indices into these arrays, so all hell may break loose if you change this.", std::string(""));

  // output: V0s
  addParam("V0ColName",  m_V0ColName,  "V0 collection name (output)", std::string(""));

  addParam("vertexChi2Cut", m_vertexChi2Cut, "Maximum chi² for the vertex fit (NDF = 1)", 5.);
  addParam("distRfromIP", m_distRfromIP, "Minimum separation of vertex from IP in R (logical or with distZfromIP cut)", 1.);
  addParam("distZfromIP", m_distZfromIP, "Minimum separation of vertex from IP in Z (logical or with distRfromIP cut)", 0.);

}

V0FinderModule::~V0FinderModule()
{
}

void V0FinderModule::initialize()
{
  StoreArray<Track>::required(m_TrackColName);
  StoreArray<genfit::Track>::required(m_GFTrackColName);
  StoreArray<TrackFitResult>::required(m_TFRColName);
  StoreArray<V0>::registerPersistent(m_V0ColName);
  RelationArray::registerPersistent<V0, MCParticle> (m_V0ColName, "");

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

  std::unique_ptr<genfit::GFRaveVertexFactory> p(new genfit::GFRaveVertexFactory());
  m_vertexFactory = std::move(p);
}


void V0FinderModule::beginRun()
{
}

void V0FinderModule::event()
{
  //get tracks
  StoreArray<Track> Tracks(m_TrackColName);
  int nTracks = Tracks.getEntries();

  if (nTracks == 0)
    return;

  StoreArray<genfit::Track> GFTracks(m_GFTrackColName);
  if (GFTracks.getEntries() != nTracks)
    B2WARNING("No one-to-one correspondence between genfit::Tracks and Belle2::Tracks");
  StoreArray<TrackFitResult> trackFitResults(m_TFRColName);
  RelationIndex<genfit::Track, TrackFitResult> gftracktotfr(GFTracks, trackFitResults);

  StoreArray<V0> V0s(m_V0ColName);

  // Group tracks into positive and negative tracks.
  typedef std::pair<Track*, genfit::Track*> trackPair;
  std::vector<trackPair> tracksPlus;
  tracksPlus.reserve(nTracks);
  std::vector<trackPair> tracksMinus;
  tracksMinus.reserve(nTracks);

  for (int iTr = 0; iTr < nTracks; iTr++) {
    const TrackFitResult* tfr = Tracks[iTr]->getTrackFitResult(Const::pion);
    if (!tfr) {
      B2WARNING("No TrackFitResult for track");
      continue;
    }

    genfit::Track* gfTrack = tfr->getRelatedFrom<genfit::Track>(m_GFTrackColName);
    if (!gfTrack) {
      B2ERROR("No genfit::Track for TrackFitResult");
      continue;
    }

    double charge = tfr->getChargeSign();
    if (charge == +1)
      tracksPlus.push_back(std::make_pair(Tracks[iTr], gfTrack));
    else
      tracksMinus.push_back(std::make_pair(Tracks[iTr], gfTrack));
  }

  // Reject boring events.
  if (tracksPlus.size() == 0 || tracksMinus.size() == 0) {
    B2DEBUG(200, "No interesting tracks.");
    return;
  }

  std::vector< std::pair< genfit::Track*, genfit::Track*> > vertices;

  // Pair up each positive track with each negative track.
  for (size_t iTrPlus = 0; iTrPlus < tracksPlus.size(); ++iTrPlus) {
    genfit::Track* gfTrackPlus = tracksPlus[iTrPlus].second;

    const TrackFitResult* tfrPlus = 0;
    if (gftracktotfr.getFirstElementFrom(gfTrackPlus))
      tfrPlus = gftracktotfr.getFirstElementFrom(gfTrackPlus)->to;
    assert(tfrPlus);  // ensured above.

    for (size_t iTrMinus = 0; iTrMinus < tracksMinus.size(); ++iTrMinus) {
      genfit::Track* gfTrackMinus = tracksMinus[iTrMinus].second;

      const TrackFitResult* tfrMinus = 0;
      if (gftracktotfr.getFirstElementFrom(gfTrackMinus))
        tfrMinus = gftracktotfr.getFirstElementFrom(gfTrackMinus)->to;
      assert(tfrMinus);  // ensured above.

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
      // an example in MC.  On the other hand, it rejects impossible
      // candidates.
      double Rstart = std::min(posPlus.Perp(), posMinus.Perp());
      try {
        stPlus.extrapolateToCylinder(Rstart);
        stMinus.extrapolateToCylinder(Rstart);
      } catch (...) {
        B2DEBUG(200, "Extrapolation to cylinder failed.");
        continue;
      }

      class vertVect {
        // Need this container for exception-safe cleanup, GFRave's
        // interface isn't exception-safe as is.
      public:
        ~vertVect() {
          for (size_t i = 0; i < v.size(); ++i)
            delete v[i];
        }
        size_t size() { return v.size(); }
        genfit::GFRaveVertex*& operator[](int idx) { return v[idx]; }
        std::vector<genfit::GFRaveVertex*> v;
      } vertexVector;
      std::vector<genfit::Track*> trackPair({gfTrackPlus, gfTrackMinus});

      try {
        m_vertexFactory->findVertices(&vertexVector.v, trackPair);
      } catch (...) {
        B2ERROR("Exception during vertex fit.");
        continue;
      }
      if (vertexVector.size() != 1)
        continue;

      const genfit::GFRaveVertex& vert = *vertexVector[0];
      if (vert.getChi2() > m_vertexChi2Cut)
        continue;

      const TVector3& posVert(vert.getPos());

      // One separation cut fulfilled is good enough, reject only if
      // close in both directions.
      if (posVert.Perp() < m_distRfromIP)
        continue;

      vertices.push_back(std::make_pair(gfTrackPlus, gfTrackMinus));

      // Assemble V0s.
      try {
        stPlus.extrapolateToPoint(posVert);
        stMinus.extrapolateToPoint(posVert);
      } catch (...) {
        // This shouldn't ever happen, but I can see the extrapolation
        // code trying several windings before giving up, so this
        // happens occasionally.  Something more stable would perhaps
        // be desirable.
        B2ERROR("couldn't extrapolate track to vertex.");
        continue;
      }

      double Bx, By, Bz;
      genfit::FieldManager::getInstance()->getFieldVal(posVert.X(), posVert.Y(), posVert.Z(),
                                                       Bx, By, Bz);

      TrackFitResult* tfrPlusVtx = trackFitResults.appendNew(stPlus.getPos(), stPlus.getMom(),
                                                             stPlus.get6DCov(), stPlus.getCharge(),
                                                             Const::pion,
                                                             gfTrackPlus->getFitStatus()->getPVal(),
                                                             Bz / 10., 0, 0);
      TrackFitResult* tfrMinusVtx = trackFitResults.appendNew(stMinus.getPos(), stMinus.getMom(),
                                                              stMinus.get6DCov(), stMinus.getCharge(),
                                                              Const::pion,
                                                              gfTrackMinus->getFitStatus()->getPVal(),
                                                              Bz / 10., 0, 0);

      V0s.appendNew(std::make_pair(tracksPlus[iTrPlus].first, tfrPlusVtx),
                    std::make_pair(tracksMinus[iTrMinus].first, tfrMinusVtx));

    }
  }

  // MC matching
  StoreArray < MCParticle > mcParticles;
  unsigned int nMCpart = mcParticles.getEntries();

  if (nMCpart == 0)
    return;

  RelationArray gfTrToMCpart(GFTracks, mcParticles);
  RelationArray TracksToMCpart(Tracks, mcParticles);

  for (size_t i = 0; i < nMCpart; ++i) {
    const MCParticle* MCpart = mcParticles[i];
    if (!MCpart) {
      B2ERROR("MC particle is NULL.");
      continue;
    }

    const std::vector<Belle2::MCParticle*>& daughters = MCpart->getDaughters();
    if (daughters.size() != 2)
      // We match V0 candidates by looking for particles decaying into
      // two oppositely charged particles instead of listing the possible PDG ids
      // and then looking whether the correct kind of decay took
      // place.  This approach is more general but it will miss decays
      // with an additional (soft) neutral which would also show as
      // V0.  In particular Kshort can go to pi- pi+ gamma (BR = 1.8 x
      // 10^-3).  A quick perusal of the PDG shows no other
      // significant displaced decays to two charged particles +
      // neutrals.
      continue;
    const MCParticle* daughterPlus = 0;
    const MCParticle* daughterMinus = 0;
    if (daughters[0]->getCharge() == +1)
      daughterPlus = daughters[0];
    else if (daughters[0]->getCharge() == -1)
      daughterMinus = daughters[0];
    if (daughters[1]->getCharge() == +1)
      daughterPlus = daughters[1];
    else if (daughters[1]->getCharge() == -1)
      daughterMinus = daughters[1];

    if (!daughterPlus || !daughterMinus) {
      // Not a V0, need both positively charged and negatively charged
      // daughter.
      continue;
    }

    // MC particle is a V0, see if we found it.
    for (int j = 0; j < V0s.getEntries(); ++j) {
      // Try to match the tracks of each V0 with the MC V0.
      auto trackPtrs = V0s[j]->getTrackPtrs();
      const Track* trPlus = trackPtrs.first;
      const Track* trMinus = trackPtrs.second;

      const MCParticle* V0PartPlus = trPlus->getRelatedTo<MCParticle>();
      const MCParticle* V0PartMinus = trMinus->getRelatedTo<MCParticle>();

      if (!V0PartPlus || !V0PartMinus) {
        B2WARNING("No MCParticles for V0 tracks.");
        continue;
      }

      if (daughterPlus == V0PartPlus && daughterMinus == V0PartMinus) {
        // Establish relation.
        V0s[j]->addRelationTo(MCpart);
      }
    }
  }
}

void V0FinderModule::endRun()
{
}

void V0FinderModule::terminate()
{
}
