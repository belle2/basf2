/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <display/modules/display/DisplayModule.h>

#include <framework/dataobjects/DisplayData.h>
#include <display/DisplayUI.h>
#include <display/EVEVisualization.h>
#include <display/EveGeometry.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <simulation/dataobjects/MCParticleTrajectory.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/core/Path.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <genfit/GFRaveVertex.h>

#include <TApplication.h>
#include <TEveManager.h>

using namespace Belle2;

REG_MODULE(Display);

DisplayModule::DisplayModule() : Module(), m_display(0), m_visualizer(0)
{
  setDescription("Interactive visualisation of Monte Carlo, intermediate and reconstructed objects, plus geometry. See https://confluence.desy.de/display/BI/Software+/EventDisplay for detailed documentation.");

  addParam("options", m_options,
           "Drawing options for RecoTracks, a combination of DHMP. See EVEVisualization::setOptions or the display.py example for an explanation.",
           std::string("MH"));
  addParam("showMCInfo", m_showMCInfo, "Show Monte Carlo information (MCParticles, SimHits)", true);
  addParam("assignHitsToPrimaries", m_assignToPrimaries,
           "If true, hits created by secondary particles (after scattering, decay-in-flight, ...) will be assigned to the original primary particle.",
           false);
  addParam("showAllPrimaries", m_showAllPrimaries,
           "If true, all primary MCParticles will be shown, regardless of wether hits are produced.", true);
  addParam("hideSecondaries", m_hideSecondaries, "If true, secondary MCParticles (and hits created by them) will not be shown.",
           false);
  addParam("showCharged", m_showCharged,
           "If true, all charged MCParticles will be shown, including secondaries (implies disabled assignHitsToPrimaries). May be slow.",
           true);
  addParam("showNeutrals", m_showNeutrals,
           "If true, all neutral MCParticles will be shown, including secondaries (implies disabled assignHitsToPrimaries). May be slow.",
           true);
  addParam("showTrackLevelObjects", m_showTrackLevelObjects,
           "If true, fitted Tracks, GFRave Vertices and ECLCluster objects will be shown in the display.", true);
  addParam("showRecoTracks", m_showRecoTracks,
           "If true, track candidates (RecoTracks) and reconstructed hits will be shown in the display.", false);
  addParam("showCDCHits", m_showCDCHits,
           "If true, CDCHit objects will be shown as drift cylinders (shortened, z position set to zero).", false);
  addParam("showTriggerObjects", m_showTriggerObjects,
           "If true, CDCHit objects will be assigned to trigger segments and trigger tracks will be shown.", false);
  addParam("showKLM2dHits", m_showKLM2dHits,
           "If true, KLMHit2d objects will be shown in the display.", true);
  addParam("showARICHHits", m_showARICHHits,
           "If true, ARICHHit objects will be shown.", false);
  addParam("automatic", m_automatic,
           "Non-interactively save visualisations for each event. Note that this still requires an X server, but you can use the 'Xvfb' dummy server by running basf2 using 'xvfb-run -s \"-screen 0 640x480x24\" basf2 ...' to run headless.",
           false);
  addParam("fullGeometry", m_fullGeometry,
           "Show full geometry instead of simplified shapes. Further details can be enabled by changing the VisLevel option for Eve -> Scenes -> Geometry Scene -> Top_1.",
           false);
  addParam("hideObjects", m_hideObjects,
           "Objects which are to be hidden (can be manually re-enabled in tree view). Names correspond to the object names in the 'Event'. (Note that this won't work for objects somewhere deep in the tree, only for those immediately below 'Event'.)", {});
  addParam("customGeometryExtractPath", m_customGeometryExtractPath, "Path to custom file with geometry extract.", std::string(""));
  addParam("hideVolumes", m_hideVolumes,
           "List of volumes to be hidden (can be re-enabled in Eve panel / Geometry scene. The volume and all its daughters will be hidden.", {});
  addParam("deleteVolumes", m_deleteVolumes,
           "List of volumes to be deleted. The volume and all its daughters will be deleted completely. Uses Regular Expressions (RE)! If the expression starts with '#', only daughters are removed (# is removed for RE)", {});
  addParam("playOnStartup", m_playOnStartup,
           "When launching the event display, immediately start advancing through events. Useful for control room uses etc.", false);


  //create gApplication so we can use graphics support. Needs to be done before ROOT has a chance to do it for us.
  if ((!gApplication) || (gApplication->TestBit(TApplication::kDefaultApplication))) {
    new TApplication("ROOT_application", 0, 0);
  }
}


void DisplayModule::initialize()
{
  //optional inputs
  StoreArray<MCParticle> MCParticles; MCParticles.isOptional();
  StoreArray<MCParticleTrajectory> MCParticleTrajectorys; MCParticleTrajectorys.isOptional();
  StoreArray<CDCSimHit> CDCSimHits; CDCSimHits.isOptional();
  StoreArray<PXDSimHit> PXDSimHits; PXDSimHits.isOptional();
  StoreArray<SVDSimHit> SVDSimHits; SVDSimHits.isOptional();
  StoreArray<KLMSimHit> KLMSimHits; KLMSimHits.isOptional();
  StoreArray<ECLCluster> ECLClusters; ECLClusters.isOptional();
  StoreArray<KLMCluster> KLMClusters; KLMClusters.isOptional();
  StoreArray<KLMHit2d> KLMHit2ds; KLMHit2ds.isOptional();
  StoreArray<Track> Tracks; Tracks.isOptional();
  StoreArray<TrackFitResult> TrackFitResults; TrackFitResults.isOptional();
  StoreArray<RecoTrack> RecoTracks; RecoTracks.isOptional();
  StoreArray<genfit::GFRaveVertex> GFRaveVertexs; GFRaveVertexs.isOptional();
  StoreObjPtr<DisplayData> DisplayDatas; DisplayDatas.isOptional();
  StoreArray<PXDCluster> PXDClusters; PXDClusters.isOptional();
  StoreArray<SVDCluster> SVDClusters; SVDClusters.isOptional();
  StoreArray<CDCHit> CDCHits; CDCHits.isOptional();
  StoreArray<CDCTriggerSegmentHit> CDCTriggerSegmentHits; CDCTriggerSegmentHits.isOptional();
  StoreArray<ARICHHit> ARICHHits; ARICHHits.isOptional();
  StoreArray<TOPDigit> TOPDigits; TOPDigits.isOptional();
  StoreArray<ROIid> ROIids; ROIids.isOptional();
  StoreArray<RecoHitInformation::UsedPXDHit> UsedPXDHits; UsedPXDHits.isOptional();
  StoreArray<RecoHitInformation::UsedSVDHit> UsedSVDHits; UsedSVDHits.isOptional();
  StoreArray<RecoHitInformation::UsedCDCHit> UsedCDCHits; UsedCDCHits.isOptional();

  m_display = new DisplayUI(m_automatic, m_playOnStartup);
  if (hasCondition())
    m_display->allowFlaggingEvents(getCondition()->getPath()->getPathString());
  m_display->addParameter("Show MC info", getParam<bool>("showMCInfo"), 0);
  {
    //MC-specific parameters
    m_display->addParameter("Assign hits to primary particles", getParam<bool>("assignHitsToPrimaries"), 1);
    m_display->addParameter("Show all primaries", getParam<bool>("showAllPrimaries"), 1);
    m_display->addParameter("Show all charged particles", getParam<bool>("showCharged"), 1);
    m_display->addParameter("Show all neutral particles", getParam<bool>("showNeutrals"), 1);
    m_display->addParameter("Hide secondaries", getParam<bool>("hideSecondaries"), 1);
  }
  m_display->addParameter("Show candidates and rec. hits", getParam<bool>("showRecoTracks"), 0);
  m_display->addParameter("Show tracks, vertices, gammas", getParam<bool>("showTrackLevelObjects"), 0);

  if (!m_fullGeometry and Gearbox::getInstance().exists("Detector/Name")) {
    std::string detectorName = Gearbox::getInstance().getString("Detector/Name");
    if (detectorName != "Belle2Detector") {
      B2INFO("Non-standard detector '" << detectorName << "' used, switching to full geometry.");
      m_fullGeometry = true;
    }
  }
  if (m_fullGeometry) {
    //pass some parameters to DisplayUI to be able to change them at run time
    m_display->addParameter("Show full geometry", getParam<bool>("fullGeometry"), 0);
  }

  if (!m_customGeometryExtractPath.empty()) EveGeometry::setCustomExtractPath(m_customGeometryExtractPath);

  EveGeometry::setHideVolumes(m_hideVolumes);
  EveGeometry::setDeleteVolumes(m_deleteVolumes);

  EveGeometry::addGeometry(m_fullGeometry ? EveGeometry::c_Full : EveGeometry::c_Simplified);
  m_visualizer = new EVEVisualization();
  m_visualizer->setOptions(m_options);
  m_display->hideObjects(m_hideObjects);
}



void DisplayModule::event()
{
  setReturnValue(false);
  if (!gEve) {
    //window closed?
    B2WARNING("Display window closed, continuing with next module. (hit Ctrl+C to exit)");
    return;
  }

  EveGeometry::setVisualisationMode(m_fullGeometry ? EveGeometry::c_Full : EveGeometry::c_Simplified);

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
      for (const MCParticle& part : mcparticles) {
        if ((m_showAllPrimaries and part.hasStatus(MCParticle::c_PrimaryParticle))
            or (m_showCharged and TMath::Nint(part.getCharge()) != 0)
            or (m_showNeutrals and TMath::Nint(part.getCharge()) == 0)) {
          m_visualizer->addMCParticle(&part);
        }
      }
    }

    //gather simhits
    m_visualizer->addSimHits(StoreArray<CDCSimHit>());
    m_visualizer->addSimHits(StoreArray<PXDSimHit>());
    m_visualizer->addSimHits(StoreArray<SVDSimHit>());
    m_visualizer->addSimHits(StoreArray<KLMSimHit>());
  }


  if (m_showRecoTracks) {
    //add all possible track candidate arrays
    const auto recoTrackArrays = StoreArray<RecoTrack>::getArrayList();
    for (std::string colName : recoTrackArrays) {
      StoreArray<RecoTrack> recoTracks(colName);
      for (const RecoTrack& recoTrack : recoTracks) {
        if (colName != "RecoTracksMpl") {
          m_visualizer->addTrackCandidate(colName, recoTrack);
        } else {
          m_visualizer->addTrackCandidateImproved(colName, recoTrack);
        }
      }
    }

    StoreArray<RecoHitInformation::UsedPXDHit> pxdStoreArray;
    StoreArray<RecoHitInformation::UsedSVDHit> svdStoreArray;
    StoreArray<RecoHitInformation::UsedCDCHit> cdcStoreArray;

    //add remaining recohits
    m_visualizer->addUnassignedRecoHits(pxdStoreArray);
    m_visualizer->addUnassignedRecoHits(svdStoreArray);
    m_visualizer->addUnassignedRecoHits(cdcStoreArray);

    StoreArray<ROIid> ROIs;
    for (int i = 0 ; i < ROIs.getEntries(); i++)
      m_visualizer->addROI(ROIs[i]);
    //well, non-standard names are used for testbeams?
    StoreArray<ROIid> testbeamROIs("ROIs");
    for (int i = 0 ; i < testbeamROIs.getEntries(); i++)
      m_visualizer->addROI(testbeamROIs[i]);
  }

  if (m_showCDCHits || m_showTriggerObjects) {
    StoreArray<CDCHit> cdchits;
    for (auto& hit : cdchits)
      m_visualizer->addCDCHit(&hit, m_showTriggerObjects);
  }

  if (m_showTriggerObjects) {
    const auto arrayList = StoreArray<CDCTriggerSegmentHit>::getArrayList();
    for (const auto& i : arrayList) {
      StoreArray<CDCTriggerSegmentHit> tshits(i);
      for (auto& hit : tshits)
        m_visualizer->addCDCTriggerSegmentHit(i, &hit);
    }

    //add all possible track candidate arrays
    const auto trgTrackArrays = StoreArray<CDCTriggerTrack>::getArrayList();
    for (std::string colName : trgTrackArrays) {
      StoreArray<CDCTriggerTrack> trgTracks(colName);
      for (const CDCTriggerTrack& trgTrack : trgTracks) {
        m_visualizer->addCDCTriggerTrack(colName, trgTrack);
      }
    }
  }

  if (m_showKLM2dHits) {
    StoreArray<KLMHit2d> klmHits;
    for (auto& hit : klmHits) {
      if (hit.getSubdetector() == KLMElementNumbers::c_BKLM)
        m_visualizer->addBKLMHit2d(&hit);
      else
        m_visualizer->addEKLMHit2d(&hit);
    }
  }

  if (m_showARICHHits) {
    StoreArray<ARICHHit> arichhits;
    for (auto& hit : arichhits)
      m_visualizer->addARICHHit(&hit);
  }

  if (m_showTrackLevelObjects) {
    //gather track-level objects
    StoreArray<Track> tracks;
    for (const Track& track : tracks)
      m_visualizer->addTrack(&track);

    StoreArray<genfit::GFRaveVertex> vertices;
    const int nVertices = vertices.getEntries();
    for (int i = 0; i < nVertices; i++) {
      m_visualizer->addVertex(vertices[i]);
    }

    StoreArray<ECLCluster> clusters;
    for (const ECLCluster& cluster : clusters) {
      if (cluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) {
        if (m_showMCInfo) {
          //make sure we add particles producing these
          const MCParticle* mcpart = cluster.getRelated<MCParticle>();
          if (mcpart)
            m_visualizer->addMCParticle(mcpart);
        }

        m_visualizer->addECLCluster(&cluster);
      }
    }

    StoreArray<KLMCluster> klmclusters;
    for (const KLMCluster& cluster : klmclusters) {
      if (m_showMCInfo) {
        //make sure we add particles producing these
        const MCParticle* mcpart = cluster.getRelated<MCParticle>();
        if (mcpart)
          m_visualizer->addMCParticle(mcpart);
      }

      m_visualizer->addKLMCluster(&cluster);
    }

    m_visualizer->addTOPDigits(StoreArray<TOPDigit>());
  }

  //all hits/tracks are added, finish visual representations
  m_visualizer->makeTracks();

  StoreObjPtr<DisplayData> displayData;
  if (displayData) {
    m_visualizer->showUserData(*displayData);
    m_display->showUserData(*displayData);
  }


  bool reshow = m_display->startDisplay();
  setReturnValue(m_display->getReturnValue());
  if (!m_display->cumulativeIsOn()) {
    m_visualizer->clearEvent(); //clean up internal state of visualiser
  }
  m_display->clearEvent(); //clean up event scene, incl. projections

  //reprocess current event (maybe some options changed)
  if (reshow)
    event();
}


void DisplayModule::terminate()
{
  if (gEve) {
    gEve->Terminate();
  }

  delete m_visualizer;
  delete m_display;
}
