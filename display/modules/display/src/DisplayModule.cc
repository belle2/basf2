#include <display/modules/display/DisplayModule.h>

#include <display/dataobjects/DisplayData.h>
#include <display/DisplayUI.h>
#include <display/EVEVisualization.h>
#include <geometry/GeometryManager.h>
#include <vxd/geometry/GeoCache.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/dataobjects/ECLGamma.h>
#include <tracking/gfbfield/GFGeant4Field.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <genfit/Track.h>
#include <genfit/GFRaveVertex.h>
#include <genfit/FieldManager.h>
#include <genfit/MaterialEffects.h>
#include <genfit/TGeoMaterialInterface.h>

#include <TApplication.h>
#include <TGeoManager.h>
#include <TEveManager.h>
#include <TSystem.h>

using namespace Belle2;

REG_MODULE(Display)

DisplayModule::DisplayModule() : Module(), m_display(0), m_visualizer(0)
{
  setDescription("Interactive visualisation of MCParticles, genfit::Tracks and various SimHits (plus geometry). See https://belle2.cc.kek.jp/~twiki/bin/view/Computing/EventDisplay for detailed documentation.");

  addParam("options", m_options, "Drawing options for genfit::Tracks, a combination of DHMPS. See EVEVisualization::setOptions or the display.py example for an explanation.", std::string("MH"));
  addParam("showMCInfo", m_showMCInfo, "Show Monte Carlo information (MCParticles, SimHits)", true);
  addParam("assignHitsToPrimaries", m_assignToPrimaries, "If true, hits created by secondary particles (after scattering, decay-in-flight, ...) will be assigned to the original primary particle.", false);
  addParam("showAllPrimaries", m_showAllPrimaries, "If true, all primary MCParticles will be shown, regardless of wether hits are produced.", false);
  addParam("hideSecondaries", m_hideSecondaries, "If true, secondary MCParticles (and hits created by them) will not be shown.", false);
  addParam("showCharged", m_showCharged, "If true, all charged MCParticles will be shown, including secondaries (implies disabled assignHitsToPrimaries). May be slow.", false);
  addParam("showNeutrals", m_showNeutrals, "If true, all neutral MCParticles will be shown, including secondaries (implies disabled assignHitsToPrimaries). May be slow.", false);
  addParam("showTrackLevelObjects", m_showTrackLevelObjects, "If true, fitted genfit::Tracks, GFRave Vertices and ECLGamma objects will be shown in the display.", true);
  addParam("showTrackCandidates", m_showTrackCandidates, "If true, track candidates (genfit::TrackCand) and RecoHits will be shown in the display.", false);
  addParam("useClusters", m_useClusters, "Use PXD/SVD clusters for track candidate/recohit visualisation (instead of TrueHits).", true);
  addParam("automatic", m_automatic, "Non-interactively save visualisations for each event.", false);
  addParam("fullGeometry", m_fullGeometry, "Show full geometry instead of simplified shapes. Further details can be enabled by changing the VisLevel option for Eve -> Scenes -> Geometry Scene -> Top_1.", false);

  //make sure dictionaries for PXDrecohits are loaded
  //needs to be done here to have dictionaries available during RootInput::initialize()
  gSystem->Load("libpxd");

  //create gApplication so we can use graphics support. Needs to be done before ROOT has a chance to do it for us.
  if ((!gApplication) || (gApplication && gApplication->TestBit(TApplication::kDefaultApplication))) {
    new TApplication("ROOT_application", 0, 0);
  }
}


DisplayModule::~DisplayModule()
{
}


void DisplayModule::initialize()
{
  //optional inputs
  StoreArray<MCParticle>::optional();
  StoreArray<MCParticleTrajectory>::optional();
  StoreArray<CDCSimHit>::optional();
  StoreArray<PXDSimHit>::optional();
  StoreArray<SVDSimHit>::optional();
  StoreArray<BKLMSimHit>::optional();
  StoreArray<EKLMSimHit>::optional();
  StoreArray<ECLHit>::optional();
  StoreArray<ECLGamma>::optional();
  StoreArray<genfit::Track>::optional();
  StoreArray<genfit::TrackCand>::optional();
  StoreArray<genfit::GFRaveVertex>::optional();
  StoreObjPtr<DisplayData>::optional();
  StoreArray<PXDCluster>::optional();
  StoreArray<SVDCluster>::optional();
  StoreArray<CDCHit>::optional();

  if (!gGeoManager) { //TGeo geometry not initialized, do it ourselves
    //convert geant4 geometry to TGeo geometry
    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
    geoManager.createTGeoRepresentation();

    //initialize some things for genfit
    genfit::FieldManager::getInstance()->init(new GFGeant4Field());
    genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());

  }
  if (!gGeoManager) {
    B2ERROR("Couldn't create TGeo geometry!");
    return;
  }

  m_display = new DisplayUI(m_automatic);
  //pass some parameters to DisplayUI to be able to change them at run time
  m_display->addParameter("Show MC info", getParam<bool>("showMCInfo"), 0);
  {
    //MC-specific parameters
    m_display->addParameter("Assign hits to primary particles", getParam<bool>("assignHitsToPrimaries"), 1);
    m_display->addParameter("Show all primaries", getParam<bool>("showAllPrimaries"), 1);
    m_display->addParameter("Show all charged particles", getParam<bool>("showCharged"), 1);
    m_display->addParameter("Show all neutral particles", getParam<bool>("showNeutrals"), 1);
    m_display->addParameter("Hide secondaries", getParam<bool>("hideSecondaries"), 1);
  }
  m_display->addParameter("Show candidates and RecoHits", getParam<bool>("showTrackCandidates"), 0);
  m_display->addParameter("Show tracks, vertices, gammas", getParam<bool>("showTrackLevelObjects"), 0);


  m_visualizer = new EVEVisualization();
  m_visualizer->setOptions(m_options);
  m_visualizer->showFullGeo(m_fullGeometry);
  m_visualizer->addGeometry();
}


void DisplayModule::event()
{
  if (!gEve) {
    //window closed?
    B2WARNING("Display window closed, continuing with next module. (hit Ctrl+C to exit)");
    return;
  }

  //secondaries cannot be shown if they are merged into primaries
  m_visualizer->setAssignToPrimaries(m_assignToPrimaries && !m_showNeutrals && !m_showCharged);
  if (m_assignToPrimaries && (m_showNeutrals || m_showCharged)) {
    B2WARNING("assignHitsToPrimaries and showCharged/showNeutrals can not be used together!");
  }

  m_visualizer->setHideSecondaries(m_hideSecondaries);

  if (m_showMCInfo) {
    //gather MC particles
    StoreArray<MCParticle> mcparticles;
    if (m_showAllPrimaries or m_showCharged or m_showNeutrals) {
      for (const MCParticle & part : mcparticles) {
        if ((m_showAllPrimaries and part.hasStatus(MCParticle::c_PrimaryParticle))
            or (m_showCharged and TMath::Nint(part.getCharge()) != 0)
            or (m_showNeutrals and TMath::Nint(part.getCharge()) == 0)) {
          switch (abs(part.getPDG())) {
            case 11:
            case 13:
            case 22:
            case 211:
            case 321:
            case 2212:
              m_visualizer->addMCParticle(&part);
              break;
            default:
              if (part.hasStatus(MCParticle::c_PrimaryParticle)) {
                //only show odd things if they're primary...
                m_visualizer->addMCParticle(&part);
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
    m_visualizer->addSimHits(StoreArray<ECLHit>()); //ECLHits are input to the digitizer, so this _is_ MC level
    m_visualizer->addSimHits(StoreArray<EKLMSimHit>());
    m_visualizer->addSimHits(StoreArray<BKLMSimHit>());
  }

  if (m_showTrackCandidates) {
    StoreArray<genfit::TrackCand> gftrackcands;
    const int nCands = gftrackcands.getEntries();
    for (int i = 0; i < nCands; i++) {
      if (m_useClusters) {
        m_visualizer->addTrackCandidate(gftrackcands[i], TString::Format("genfit::TrackCand %d", i),
                                        StoreArray<PXDCluster>(), StoreArray<SVDCluster>(), StoreArray<CDCHit>());
      } else {
        m_visualizer->addTrackCandidate(gftrackcands[i], TString::Format("genfit::TrackCand %d", i),
                                        StoreArray<PXDTrueHit>(), StoreArray<SVDTrueHit>(), StoreArray<CDCHit>());
      }
    }

    //add remaining recohits
    if (m_useClusters) {
      m_visualizer->addUnassignedRecoHits(StoreArray<PXDCluster>());
      m_visualizer->addUnassignedRecoHits(StoreArray<SVDCluster>());
    } else {
      m_visualizer->addUnassignedRecoHits(StoreArray<PXDTrueHit>());
      m_visualizer->addUnassignedRecoHits(StoreArray<SVDTrueHit>());
    }
    m_visualizer->addUnassignedRecoHits(StoreArray<CDCHit>());
  }

  if (m_showTrackLevelObjects) {
    //gather track-level objects
    StoreArray<genfit::Track> gftracks;
    const int nTracks = gftracks.getEntries();
    for (int i = 0; i < nTracks; i++)
      m_visualizer->addTrack(gftracks[i], TString::Format("genfit::Track %d", i));

    StoreArray<genfit::GFRaveVertex> vertices;
    const int nVertices = vertices.getEntries();
    for (int i = 0; i < nVertices; i++) {
      m_visualizer->addVertex(vertices[i], TString::Format("GFRaveVertex %d", i));
    }

    StoreArray<ECLGamma> gammas;
    const int nRecGammas = gammas.getEntries();
    for (int i = 0; i < nRecGammas; i++) {
      m_visualizer->addGamma(gammas[i], TString::Format("ECLGamma %d", i));
    }
  }

  //all hits/tracks are added, finish visual representations
  m_visualizer->makeTracks();

  StoreObjPtr<DisplayData> displayData;
  if (displayData) {
    m_visualizer->showUserData(*displayData);
    m_display->showUserData(*displayData);
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
