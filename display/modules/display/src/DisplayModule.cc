#include <display/modules/display/DisplayModule.h>

#include <display/modules/display/DisplayUI.h>
#include <display/modules/display/EVEVisualization.h>
#include <geometry/GeometryManager.h>
#include <vxd/geometry/GeoCache.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <tracking/gfbfield/GFGeant4Field.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>

#include <GFTrack.h>
#include <GFFieldManager.h>
#include <GFMaterialEffects.h>
#include <GFTGeoMaterialInterface.h>

#include "TGeoManager.h"
#include "TEveManager.h"
#include "TSystem.h"

#include "GFRaveVertex.h"
#include "ecl/dataobjects/ECLGamma.h"

using namespace Belle2;

REG_MODULE(Display)

DisplayModule::DisplayModule() : Module(), m_display(0), m_visualizer(0)
{
  setDescription("Interactive visualisation of MCParticles, GFTracks and various SimHits (plus geometry).");

  addParam("Options", m_options, "Drawing options for GFTracks, a combination of ADHMPST. See EVEVisualization::setOptions or the display.py example for an explanation.", std::string("AMHT"));
  addParam("AssignHitsToPrimaries", m_assignToPrimaries, "If true, hits created by secondary particles (e.g. delta electrons) will be assigned to the original primary particle.", true);
  addParam("ShowAllPrimaries", m_showAllPrimaries, "If true, all primary MCParticles will be shown, regardless of wether hits are produced.", false);
  addParam("ShowCharged", m_showCharged, "If true, all charged MCParticles will be shown, including secondaries (implies disabled AssignHitsToPrimaries). May be slow.", false);
  addParam("ShowNeutrals", m_showNeutrals, "If true, all neutral MCParticles will be shown, including secondaries (implies disabled AssignHitsToPrimaries). May be slow.", false);
  addParam("ShowGFTracks", m_showGFTracks, "If true, fitted GFTracks will be shown in the display.", true);
  addParam("ShowGFTrackCands", m_showGFTrackCands, "If true, track candidates (GFTrackCands array) will be shown in the display.", false);
  addParam("UseClusters", m_useClusters, "Use PXD/SVD clusters for GFTrackCands visualisation", false);
  addParam("Automatic", m_automatic, "Non-interactively save visualisations for each event.", false);

  //make sure dictionaries for PXD/SVDrecohits are loaded
  //needs to be done here to have dictionaries available during RootInput::initialize()
  gSystem->Load("libpxd");
  gSystem->Load("libsvd");
  gSystem->Load("libgenfitRK");
}


DisplayModule::~DisplayModule()
{
}


void DisplayModule::initialize()
{
  //optional inputs
  StoreArray<MCParticle>::optional();
  StoreArray<CDCSimHit>::optional();
  StoreArray<PXDSimHit>::optional();
  StoreArray<SVDSimHit>::optional();
  StoreArray<BKLMSimHit>::optional();
  StoreArray<EKLMStepHit>::optional();
  StoreArray<ECLHit>::optional();
  StoreArray<ECLGamma>::optional();
  StoreArray<GFTrack>::optional();
  StoreArray<GFTrackCand>::optional();
  StoreArray<GFRaveVertex>::optional();

  if (!gGeoManager) { //TGeo geometry not initialized, do it ourselves
    //convert geant4 geometry to TGeo geometry
    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
    geoManager.createTGeoRepresentation();

    //initialize some things for genfit
    GFFieldManager::getInstance()->init(new GFGeant4Field());
    GFMaterialEffects::getInstance()->init(new GFTGeoMaterialInterface());
  }

  m_display = new DisplayUI(m_automatic);
  //pass some parameters to DisplayUI to be able to change them at run time
  m_display->addParameter("Assign hits to primary particles", getParam<bool>("AssignHitsToPrimaries"));
  m_display->addParameter("Show all primaries", getParam<bool>("ShowAllPrimaries"));
  m_display->addParameter("Show all charged particles", getParam<bool>("ShowCharged"));
  m_display->addParameter("Show all neutral particles", getParam<bool>("ShowNeutrals"));
  m_display->addParameter("Show GFTracks", getParam<bool>("ShowGFTracks"));
  m_display->addParameter("Show GFTrackCandidates", getParam<bool>("ShowGFTrackCands"));


  m_visualizer = new EVEVisualization();
  m_visualizer->setOptions(m_options);
  m_visualizer->addGeometry();
}


void DisplayModule::event()
{
  if (!gEve) {
    //window closed?
    B2WARNING("No TEveManager found, skipping display. (hit Ctrl+C to exit)");
    return;
  }

  //secondaries cannot be shown if they are merged into primaries
  m_visualizer->setAssignToPrimaries(m_assignToPrimaries && !m_showNeutrals && !m_showCharged);
  if (m_assignToPrimaries && (m_showNeutrals || m_showCharged)) {
    B2WARNING("AssignHitsToPrimaries and ShowCharged/ShowNeutrals can not be used together!");
  }


  //gather MC particles
  StoreArray<MCParticle> mcparticles;
  if (m_showAllPrimaries or m_showCharged or m_showNeutrals) {
    for (int i = 0; i < mcparticles.getEntries(); i++) {
      const MCParticle* part = mcparticles[i];
      if ((m_showAllPrimaries and part->hasStatus(MCParticle::c_PrimaryParticle))
          or(m_showCharged and TMath::Nint(part->getCharge()) != 0)
          or(m_showNeutrals and TMath::Nint(part->getCharge()) == 0)) {
        switch (abs(part->getPDG())) {
          case 11:
          case 13:
          case 22:
          case 211:
          case 321:
          case 2212:
            m_visualizer->addMCParticle(part);
            break;
          default:
            if (part->hasStatus(MCParticle::c_PrimaryParticle)) {
              //only show odd things if they're primary...
              m_visualizer->addMCParticle(part);
            }
            break;
        }
      }
    }
  }

  //gather simhits
  m_visualizer->addSimHits(StoreArray<CDCSimHit>());
  m_visualizer->addSimHits(StoreArray<PXDSimHit>());
  m_visualizer->addSimHits(StoreArray<SVDSimHit>());
  m_visualizer->addSimHits(StoreArray<ECLHit>());

  //aw, not derived from RelationsObject/SimHitBase yet
  //addSimHits(m_visualizer, StoreArray<BKLMSimHit>());
  //addSimHits(m_visualizer, StoreArray<EKLMStepHit>());

  StoreArray<BKLMSimHit> bklmhits;
  RelationIndex<MCParticle, BKLMSimHit> mcpart_to_bklmhits(mcparticles, bklmhits);
  const int nBKLMHits = bklmhits.getEntries();
  for (int i = 0; i < nBKLMHits; i++) {
    const RelationIndexContainer<MCParticle, BKLMSimHit>::Element* el = mcpart_to_bklmhits.getFirstElementTo(bklmhits[i]);
    if (!el) {
      B2WARNING("MCParticle not found for BKLMSimHit, skipping hit!");
      continue;
    }

    m_visualizer->addSimHit(bklmhits[i], el->from);
  }

  StoreArray<EKLMStepHit> eklmhits;
  RelationIndex<MCParticle, EKLMStepHit> mcpart_to_eklmhits(mcparticles, eklmhits);
  const int nEKLMHits = eklmhits.getEntries();
  for (int i = 0; i < nEKLMHits; i++) {
    const RelationIndexContainer<MCParticle, EKLMStepHit>::Element* el = mcpart_to_eklmhits.getFirstElementTo(eklmhits[i]);
    if (!el) {
      B2WARNING("MCParticle not found for EKLMStepHit, skipping hit!");
      continue;
    }

    m_visualizer->addSimHit(eklmhits[i], el->from);
  }


  if (m_showGFTracks) {
    //gather reconstructed tracks
    StoreArray<GFTrack> gftracks;
    const int nTracks = gftracks.getEntries();
    for (int i = 0; i < nTracks; i++)
      m_visualizer->addTrack(gftracks[i], TString::Format("GFTrack %d", i));
  }

  if (m_showGFTrackCands) {
    StoreArray<GFTrackCand> gftrackcands;
    const int nCands = gftrackcands.getEntries();
    for (int i = 0; i < nCands; i++) {
      if (m_useClusters) {
        m_visualizer->addTrackCandidate(gftrackcands[i], TString::Format("GFTrackCand %d", i),
                                        StoreArray<PXDCluster>(), StoreArray<SVDCluster>(), StoreArray<CDCHit>());
      } else {
        m_visualizer->addTrackCandidate(gftrackcands[i], TString::Format("GFTrackCand %d", i),
                                        StoreArray<PXDTrueHit>(), StoreArray<SVDTrueHit>(), StoreArray<CDCHit>());
      }
    }
  }

  m_visualizer->makeTracks();


  StoreArray<GFRaveVertex> Vertices;
  const int nVertices = Vertices.getEntries();
  for (int i = 0; i < nVertices; i++) {
    //B2INFO("Distance from center" <<  Vertices[i]->getPos().Mag());
    m_visualizer->AddVertexEllip(*(Vertices[i]), TString::Format("Vertex %d", i), TString::Format("VertexCovarianceEllipsoid %d", i));
  }


  StoreArray<ECLGamma> RecGammas;
  const int nRecGammas = RecGammas.getEntries();
  for (int i = 0; i < nRecGammas; i++) {
    m_visualizer->AddRecGammas(RecGammas[i], TString::Format("ECL_Gamma %d", i));  //RecGammas[i] points to each slot of ECLGamma object.
  }


  bool reshow = m_display->startDisplay();
  m_visualizer->clearEvent(); //clean up internal state of visualiser
  m_display->clearEvent(); //clean up event scene, incl. projections

  //reprocess current event (maybe some options changed)
  if (reshow)
    event();
}


void DisplayModule::terminate()
{
  if (gEve)
    gEve->Terminate();

  delete m_visualizer;
  delete m_display;
}
