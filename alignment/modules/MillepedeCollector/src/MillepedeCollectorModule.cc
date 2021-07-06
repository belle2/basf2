/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/modules/MillepedeCollector/MillepedeCollectorModule.h>

#include <alignment/dataobjects/MilleData.h>
#include <alignment/GblMultipleScatteringController.h>
#include <alignment/GlobalDerivatives.h>
#include <alignment/GlobalLabel.h>
#include <alignment/GlobalParam.h>
#include <alignment/GlobalTimeLine.h>
#include <alignment/Manager.h>
#include <alignment/reconstruction/AlignableCDCRecoHit.h>
#include <alignment/reconstruction/AlignablePXDRecoHit.h>
#include <alignment/reconstruction/AlignableSVDRecoHit.h>
#include <alignment/reconstruction/AlignableSVDRecoHit2D.h>
#include <alignment/reconstruction/AlignableBKLMRecoHit.h>
#include <alignment/reconstruction/AlignableEKLMRecoHit.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/utility/ReferenceFrame.h>
#include <framework/core/FileCatalog.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dataobjects/EventT0.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dbobjects/BeamParameters.h>
#include <framework/particledb/EvtGenDatabasePDG.h>
#include <framework/pcore/ProcHandler.h>
#include <mdst/dbobjects/BeamSpot.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>
#include <tracking/trackFitting/measurementCreator/adder/MeasurementAdder.h>

#include <genfit/FullMeasurement.h>
#include <genfit/GblFitter.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/PlanarMeasurement.h>
#include <genfit/Track.h>

#include <TMath.h>
#include <TH1F.h>
#include <TTree.h>
#include <TDecompSVD.h>

using namespace std;
using namespace Belle2;
using namespace alignment;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MillepedeCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MillepedeCollectorModule::MillepedeCollectorModule() : CalibrationCollectorModule()
{
  setPropertyFlags(c_ParallelProcessingCertified);
  setDescription("Calibration data collector for Millepede Algorithm");

  // Configure input sample types
  addParam("tracks", m_tracks, "Names of collections of RecoTracks (already fitted with DAF) for calibration", vector<string>({""}));
  addParam("particles", m_particles, "Names of particle list of single particles", vector<string>());
  addParam("vertices", m_vertices,
           "Name of particle list of (mother) particles with daughters for calibration using vertex constraint", vector<string>());
  addParam("primaryVertices", m_primaryVertices,
           "Name of particle list of (mother) particles with daughters for calibration using vertex + IP profile constraint",
           vector<string>());
  addParam("twoBodyDecays", m_twoBodyDecays,
           "Name of particle list of (mother) particles with daughters for calibration using vertex + mass constraint",
           vector<string>());
  addParam("primaryTwoBodyDecays", m_primaryTwoBodyDecays,
           "Name of particle list of (mother) particles with daughters for calibration using vertex + IP profile + kinematics constraint",
           vector<string>());
  addParam("primaryMassTwoBodyDecays", m_primaryMassTwoBodyDecays,
           "Name of particle list of (mother) particles with daughters for calibration using vertex + mass constraint",
           vector<string>());
  addParam("primaryMassVertexTwoBodyDecays", m_primaryMassVertexTwoBodyDecays,
           "Name of particle list of (mother) particles with daughters for calibration using vertex + IP profile + mass constraint",
           vector<string>());

  addParam("stableParticleWidth", m_stableParticleWidth,
           "Width (in GeV/c/c) to use for invariant mass constraint for 'stable' particles (like K short). Temporary until proper solution is found.",
           double(0.002));
  // Configure output
  addParam("doublePrecision", m_doublePrecision, "Use double (=true) or single/float (=false) precision for writing binary files",
           bool(false));
  addParam("useGblTree", m_useGblTree, "Store GBL trajectories in a tree instead of output to binary files",
           bool(true));
  addParam("absFilePaths", m_absFilePaths, "Use absolute paths to remember binary files. Only applies if useGblTree=False",
           bool(false));

  // Configure global parameters
  addParam("components", m_components,
           "Specify which DB objects are calibrated, like ['BeamSpot', 'CDCTimeWalks'] or leave empty to use all components available.",
           m_components);
  addParam("calibrateVertex", m_calibrateVertex,
           "For primary vertices / two body decays, beam spot vertex calibration derivatives are added",
           bool(true));
  addParam("calibrateKinematics", m_calibrateKinematics,
           "For primary two body decays, beam spot kinematics calibration derivatives are added",
           bool(true));

  //Configure GBL fit of individual tracks
  addParam("externalIterations", m_externalIterations, "Number of external iterations of GBL fitter",
           int(0));
  addParam("internalIterations", m_internalIterations, "String defining internal GBL iterations for outlier down-weighting",
           string(""));
  addParam("recalcJacobians", m_recalcJacobians, "Up to which external iteration propagation Jacobians should be re-calculated",
           int(0));

  addParam("minPValue", m_minPValue, "Minimum p-value to write out a (combined) trajectory. Set <0 to write out all.",
           double(-1.));

  // Configure CDC specific options
  addParam("fitTrackT0", m_fitTrackT0, "Add local parameter for track T0 fit in GBL",
           bool(true));
  addParam("updateCDCWeights", m_updateCDCWeights, "Update L/R weights from previous DAF fit result",
           bool(true));
  addParam("minCDCHitWeight", m_minCDCHitWeight, "Minimum (DAF) CDC hit weight for usage by GBL",
           double(1.0E-6));
  addParam("minUsedCDCHitFraction", m_minUsedCDCHitFraction, "Minimum used CDC hit fraction to write out a trajectory",
           double(0.85));

  addParam("hierarchyType", m_hierarchyType, "Type of (VXD only now) hierarchy: 0 = None, 1 = Flat, 2 = Half-Shells, 3 = Full",
           int(3));
  addParam("enablePXDHierarchy", m_enablePXDHierarchy, "Enable PXD in hierarchy (flat or full)",
           bool(true));
  addParam("enableSVDHierarchy", m_enableSVDHierarchy, "Enable SVD in hierarchy (flat or full)",
           bool(true));

  addParam("enableWireByWireAlignment", m_enableWireByWireAlignment, "Enable global derivatives for wire-by-wire alignment",
           bool(false));
  addParam("enableWireSagging", m_enableWireSagging, "Enable global derivatives for wire sagging",
           bool(false));

  // Time dependence
  addParam("events", m_eventNumbers,
           "List of (event, run, exp) with event numbers at which payloads can change for timedep calibration.",
           m_eventNumbers);
  // Time dependence config
  addParam("timedepConfig", m_timedepConfig,
           "list{ {list{param1, param2, ...}, list{(ev1, run1, exp1), ...}}, ... }.",
           m_timedepConfig);

  // Custom mass+width config
  addParam("customMassConfig", m_customMassConfig,
           "dict{ list_name: (mass, width), ... } with custom mass and width to use as external measurement.",
           m_customMassConfig);
}

void MillepedeCollectorModule::prepare()
{
  StoreObjPtr<EventMetaData> emd;
  emd.isRequired();

  StoreObjPtr<EventT0> eventT0;
  //eventT0.isRequired();

  if (m_tracks.empty() &&
      m_particles.empty() &&
      m_vertices.empty() &&
      m_primaryVertices.empty() &&
      m_twoBodyDecays.empty() &&
      m_primaryTwoBodyDecays.empty() &&
      m_primaryMassTwoBodyDecays.empty() &&
      m_primaryMassVertexTwoBodyDecays.empty())
    B2ERROR("You have to specify either arrays of single tracks or particle lists of single single particles or mothers with vertex constrained daughters.");

  if (!m_tracks.empty()) {
    for (auto arrayName : m_tracks)
      continue;
    // StoreArray<RecoTrack>::required(arrayName);
  }

  if (!m_particles.empty() || !m_vertices.empty() || !m_primaryVertices.empty()) {
    StoreArray<RecoTrack> recoTracks;
    StoreArray<Track> tracks;
    StoreArray<TrackFitResult> trackFitResults;

    //recoTracks.isRequired();
    //tracks.isRequired();
    //trackFitResults.isRequired();
  }

  for (auto listName : m_particles) {
    StoreObjPtr<ParticleList> list(listName);
    //list.isRequired();
  }

  for (auto listName : m_vertices) {
    StoreObjPtr<ParticleList> list(listName);
    //list.isRequired();
  }

  for (auto listName : m_primaryVertices) {
    StoreObjPtr<ParticleList> list(listName);
    //list.isRequired();
  }

  // Register Mille output
  registerObject<MilleData>("mille", new MilleData(m_doublePrecision, m_absFilePaths));

  auto gblDataTree = new TTree("GblDataTree", "GblDataTree");
  gblDataTree->Branch<std::vector<gbl::GblData>>("GblData", &m_currentGblData, 32000, 99);
  registerObject<TTree>("GblDataTree", gblDataTree);

  registerObject<TH1I>("ndf", new TH1I("ndf", "ndf", 200, 0, 200));
  registerObject<TH1F>("chi2_per_ndf", new TH1F("chi2_per_ndf", "chi2 divided by ndf", 200, 0., 50.));
  registerObject<TH1F>("pval", new TH1F("pval", "pval", 100, 0., 1.));

  registerObject<TH1F>("cdc_hit_fraction", new TH1F("cdc_hit_fraction", "cdc_hit_fraction", 100, 0., 1.));
  registerObject<TH1F>("evt0", new TH1F("evt0", "evt0", 400, -100., 100.));

  // Configure the (VXD) hierarchy before being built
  if (m_hierarchyType == 0)
    Belle2::alignment::VXDGlobalParamInterface::s_hierarchyType = VXDGlobalParamInterface::c_None;
  else if (m_hierarchyType == 1)
    Belle2::alignment::VXDGlobalParamInterface::s_hierarchyType = VXDGlobalParamInterface::c_Flat;
  else if (m_hierarchyType == 2)
    Belle2::alignment::VXDGlobalParamInterface::s_hierarchyType = VXDGlobalParamInterface::c_HalfShells;
  else if (m_hierarchyType == 3)
    Belle2::alignment::VXDGlobalParamInterface::s_hierarchyType = VXDGlobalParamInterface::c_Full;

  Belle2::alignment::VXDGlobalParamInterface::s_enablePXD = m_enablePXDHierarchy;
  Belle2::alignment::VXDGlobalParamInterface::s_enableSVD = m_enableSVDHierarchy;

  std::vector<EventMetaData> events;
  for (auto& ev_run_exp : m_eventNumbers) {
    events.push_back(EventMetaData(std::get<0>(ev_run_exp), std::get<1>(ev_run_exp), std::get<2>(ev_run_exp)));
  }

  // This will also build the hierarchy for the first time:
  if (!m_timedepConfig.empty() && m_eventNumbers.empty()) {
    auto autoEvents = Belle2::alignment::timeline::setupTimedepGlobalLabels(m_timedepConfig);
    Belle2::alignment::GlobalCalibrationManager::getInstance().initialize(m_components, autoEvents);
  } else if (m_timedepConfig.empty() && !m_eventNumbers.empty()) {
    Belle2::alignment::GlobalCalibrationManager::getInstance().initialize(m_components, events);
  } else if (m_timedepConfig.empty() && m_eventNumbers.empty()) {
    Belle2::alignment::GlobalCalibrationManager::getInstance().initialize(m_components);
  } else {
    B2ERROR("Cannot set both, event list and timedep config.");
  }

//   Belle2::alignment::GlobalCalibrationManager::getInstance().writeConstraints("constraints.txt");

  AlignableCDCRecoHit::s_enableTrackT0LocalDerivative = m_fitTrackT0;
  AlignableCDCRecoHit::s_enableWireSaggingGlobalDerivative = m_enableWireSagging;
  AlignableCDCRecoHit::s_enableWireByWireAlignmentGlobalDerivatives = m_enableWireByWireAlignment;
}

void MillepedeCollectorModule::collect()
{
  StoreObjPtr<EventMetaData> emd;
  alignment::GlobalCalibrationManager::getInstance().preCollect(*emd);
  StoreObjPtr<EventT0> eventT0;

  if (!m_useGblTree) {
    // Open new file on request (at start or after being closed)
    auto mille = getObjectPtr<MilleData>("mille");
    if (!mille->isOpen())
      mille->open(getUniqueMilleName());
  }

  std::shared_ptr<genfit::GblFitter> gbl(new genfit::GblFitter());
  double chi2 = -1.;
  double lostWeight = -1.;
  int ndf = -1;
  float evt0 = -9999.;

  for (auto arrayName : m_tracks) {
    StoreArray<RecoTrack> recoTracks(arrayName);
    if (!recoTracks.isValid())
      continue;

    for (auto& recoTrack : recoTracks) {

      if (!fitRecoTrack(recoTrack))
        continue;

      auto& track = RecoTrackGenfitAccess::getGenfitTrack(recoTrack);
      if (!track.hasFitStatus())
        continue;
      genfit::GblFitStatus* fs = dynamic_cast<genfit::GblFitStatus*>(track.getFitStatus());
      if (!fs)
        continue;

      if (!fs->isFittedWithReferenceTrack())
        continue;

      using namespace gbl;
      GblTrajectory trajectory(gbl->collectGblPoints(&track, track.getCardinalRep()), fs->hasCurvature());

      trajectory.fit(chi2, ndf, lostWeight);
      getObjectPtr<TH1I>("ndf")->Fill(ndf);
      getObjectPtr<TH1F>("chi2_per_ndf")->Fill(chi2 / double(ndf));
      getObjectPtr<TH1F>("pval")->Fill(TMath::Prob(chi2, ndf));
      if (eventT0.isValid() && eventT0->hasEventT0()) {
        evt0 =  eventT0->getEventT0();
        getObjectPtr<TH1F>("evt0")->Fill(evt0);
      }

      if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(trajectory);

    }

  }

  for (auto listName : m_particles) {
    StoreObjPtr<ParticleList> list(listName);
    if (!list.isValid())
      continue;

    for (unsigned int iParticle = 0; iParticle < list->getListSize(); ++iParticle) {
      for (auto& track : getParticlesTracks({list->getParticle(iParticle)}, false)) {
        auto gblfs = dynamic_cast<genfit::GblFitStatus*>(track->getFitStatus());

        gbl::GblTrajectory trajectory(gbl->collectGblPoints(track, track->getCardinalRep()), gblfs->hasCurvature());

        trajectory.fit(chi2, ndf, lostWeight);
        getObjectPtr<TH1I>("ndf")->Fill(ndf);
        getObjectPtr<TH1F>("chi2_per_ndf")->Fill(chi2 / double(ndf));
        getObjectPtr<TH1F>("pval")->Fill(TMath::Prob(chi2, ndf));
        if (eventT0.isValid() && eventT0->hasEventT0()) {
          evt0 =  eventT0->getEventT0();
          getObjectPtr<TH1F>("evt0")->Fill(evt0);
        }

        if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(trajectory);

      }
    }
  }

  for (auto listName : m_vertices) {
    StoreObjPtr<ParticleList> list(listName);
    if (!list.isValid())
      continue;

    for (unsigned int iParticle = 0; iParticle < list->getListSize(); ++iParticle) {
      auto mother = list->getParticle(iParticle);
      std::vector<std::pair<std::vector<gbl::GblPoint>, TMatrixD> > daughters;

      for (auto& track : getParticlesTracks(mother->getDaughters()))
        daughters.push_back({
        gbl->collectGblPoints(track, track->getCardinalRep()),
        getGlobalToLocalTransform(track->getFittedState()).GetSub(0, 4, 0, 2)
      });

      if (daughters.size() > 1) {
        gbl::GblTrajectory combined(daughters);

        combined.fit(chi2, ndf, lostWeight);
        getObjectPtr<TH1I>("ndf")->Fill(ndf);
        getObjectPtr<TH1F>("chi2_per_ndf")->Fill(chi2 / double(ndf));
        getObjectPtr<TH1F>("pval")->Fill(TMath::Prob(chi2, ndf));
        if (eventT0.isValid() && eventT0->hasEventT0()) {
          evt0 =  eventT0->getEventT0();
          getObjectPtr<TH1F>("evt0")->Fill(evt0);
        }


        if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);

        B2RESULT("Vertex-constrained fit NDF = " << ndf << " Chi2/NDF = " << chi2 / double(ndf));

      }
    }
  }

  for (auto listName : m_primaryVertices) {
    StoreObjPtr<ParticleList> list(listName);
    if (!list.isValid())
      continue;

    for (unsigned int iParticle = 0; iParticle < list->getListSize(); ++iParticle) {
      auto mother = list->getParticle(iParticle);
      std::vector<std::pair<std::vector<gbl::GblPoint>, TMatrixD> > daughters;

      TMatrixD extProjection(5, 3);
      TMatrixD locProjection(3, 5);

      bool first(true);
      for (auto& track : getParticlesTracks(mother->getDaughters())) {
        if (first) {
          // For first trajectory only
          extProjection = getGlobalToLocalTransform(track->getFittedState()).GetSub(0, 4, 0, 2);
          locProjection = getLocalToGlobalTransform(track->getFittedState()).GetSub(0, 2, 0, 4);
          first = false;
        }
        daughters.push_back({
          gbl->collectGblPoints(track, track->getCardinalRep()),
          getGlobalToLocalTransform(track->getFittedState()).GetSub(0, 4, 0, 2)
        });
      }

      if (daughters.size() > 1) {
        auto beam = getPrimaryVertexAndCov();

        TMatrixDSym vertexCov(get<TMatrixDSym>(beam));
        TMatrixDSym vertexPrec(get<TMatrixDSym>(beam).Invert());
        B2Vector3D vertexResidual = - (mother->getVertex() - get<B2Vector3D>(beam));

        TVectorD extMeasurements(3);
        extMeasurements[0] = vertexResidual[0];
        extMeasurements[1] = vertexResidual[1];
        extMeasurements[2] = vertexResidual[2];

        TMatrixD extDeriv(3, 3);
        extDeriv.Zero();
        // beam vertex constraint
        extDeriv(0, 0) = 1.;
        extDeriv(1, 1) = 1.;
        extDeriv(2, 2) = 1.;

        if (m_calibrateVertex) {
          TMatrixD derivatives(3, 3);
          derivatives.Zero();
          derivatives(0, 0) = 1.;
          derivatives(1, 1) = 1.;
          derivatives(2, 2) = 1.;

          std::vector<int> labels;
          GlobalLabel label = GlobalLabel::construct<BeamSpot>(0, 0);
          labels.push_back(label.setParameterId(1));
          labels.push_back(label.setParameterId(2));
          labels.push_back(label.setParameterId(3));

          // Allow to disable BeamSpot externally
          alignment::GlobalDerivatives globals(labels, derivatives);
          // Add derivatives for vertex calibration to first point of first trajectory
          // NOTE: use GlobalDerivatives operators vector<int> and TMatrixD which filter
          // the derivatives to not pass those with zero labels (usefull to get rid of some params)
          std::vector<int> lab(globals); TMatrixD der(globals);

          // Transformation from local system at (vertex) point to global (vx,vy,vz)
          // of the (decay) vertex
          //
          // d(q/p,u',v',u,v)/d(vy,vy,vz) = dLocal_dExt
          //
          //
          // Note its transpose is its "inverse" in the sense that
          //
          // dloc/dext * (dloc/dext)^T = diag(0, 0, 0, 0, 1, 1)
          //
          //
          // N.B. typical dLocal_dExt matrix (5x3):
          //
          //      |      0    |      1    |      2    |
          // --------------------------------------------
          //    0 |          0           0           0
          //    1 |          0           0           0
          //    2 |          0           0           0
          //    3 |   -0.02614     -0.9997           0
          //    4 |          0           0           1
          //
          // Therefore one can simplify things by only taking the last two rows/columns in vectors/matrices
          // and vertex measurement can be expressed as standard 2D measurement in GBL.
          //
          TMatrixD dLocal_dExt = extProjection;
          TMatrixD dExt_dLocal = locProjection;

          TVectorD locRes = dLocal_dExt * extMeasurements;
          // Do not use inverted covariance - seems to have issues with numeric precision
          TMatrixD locCov = dLocal_dExt * vertexCov * dExt_dLocal;
          // Invert here only the 2D sub-matrix (rest is zero due to the foŕm of dLocal_dExt)
          TMatrixD locPrec = locCov.GetSub(3, 4, 3, 4).Invert();
          TMatrixDSym locPrec2D(2); locPrec2D.Zero();
          for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 2; ++j)
              locPrec2D(i, j) = locPrec(i, j);

          // Take the 2 last components also for residuals and global derivatives
          // (in local system of vertex point - defined during fitRecoTrack(..., particle) and using
          // the (hopefully) updated momentum and position seed after vertex fit by modularAnalysis
          TVectorD locRes2D = locRes.GetSub(3, 4);
          TMatrixD locDerivs2D = (extProjection * der).GetSub(3, 4, 0, 2);

          // Attach the primary beamspot vertex position as a measurement at 1st point
          // of first trajectory (and optionaly also the global derivatives for beamspot alignment
          daughters[0].first[0].addMeasurement(locRes2D, locPrec2D);
          if (!lab.empty()) {
            daughters[0].first[0].addGlobals(lab, locDerivs2D);
          }

          gbl::GblTrajectory combined(daughters);
          //combined.printTrajectory(100);
          //combined.printPoints(100);

          combined.fit(chi2, ndf, lostWeight);
          getObjectPtr<TH1I>("ndf")->Fill(ndf);
          getObjectPtr<TH1F>("chi2_per_ndf")->Fill(chi2 / double(ndf));
          getObjectPtr<TH1F>("pval")->Fill(TMath::Prob(chi2, ndf));
          if (eventT0.isValid() && eventT0->hasEventT0()) {
            evt0 =  eventT0->getEventT0();
            getObjectPtr<TH1F>("evt0")->Fill(evt0);
          }

          if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);
          B2RESULT("Beam vertex constrained fit results NDF = " << ndf << " Chi2/NDF = " << chi2 / double(ndf));

        } else {

          gbl::GblTrajectory combined(daughters, extDeriv, extMeasurements, vertexPrec);

          combined.fit(chi2, ndf, lostWeight);
          getObjectPtr<TH1I>("ndf")->Fill(ndf);
          getObjectPtr<TH1F>("chi2_per_ndf")->Fill(chi2 / double(ndf));
          getObjectPtr<TH1F>("pval")->Fill(TMath::Prob(chi2, ndf));
          if (eventT0.isValid() && eventT0->hasEventT0()) {
            evt0 =  eventT0->getEventT0();
            getObjectPtr<TH1F>("evt0")->Fill(evt0);
          }

          if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);

          B2RESULT("Beam vertex constrained fit results NDF = " << ndf << " Chi2/NDF = " << chi2 / double(ndf));

        }
      }
    }
  }

  for (auto listName : m_twoBodyDecays) {
    StoreObjPtr<ParticleList> list(listName);
    if (!list.isValid())
      continue;

    for (unsigned int iParticle = 0; iParticle < list->getListSize(); ++iParticle) {

      auto mother = list->getParticle(iParticle);
      auto track12 = getParticlesTracks(mother->getDaughters());
      if (track12.size() != 2) {
        B2ERROR("Did not get 2 fitted tracks. Skipping this mother.");
        continue;
      }

      auto pdgdb = EvtGenDatabasePDG::Instance();
      double motherMass = mother->getPDGMass();
      double motherWidth = pdgdb->GetParticle(mother->getPDGCode())->Width();

      updateMassWidthIfSet(listName, motherMass, motherWidth);

      //TODO: what to take as width for "real" particles? -> make a param for default detector mass resolution??
      if (motherWidth == 0.) {
        motherWidth = m_stableParticleWidth * Unit::GeV;
        B2WARNING("Using artificial width for " << pdgdb->GetParticle(mother->getPDGCode())->GetName() << " : " << motherWidth << " GeV");
      }

      auto dfdextPlusMinus = getTwoBodyToLocalTransform(*mother, motherMass);
      std::vector<std::pair<std::vector<gbl::GblPoint>, TMatrixD> > daughters;

      daughters.push_back({gbl->collectGblPoints(track12[0], track12[0]->getCardinalRep()), dfdextPlusMinus.first});
      daughters.push_back({gbl->collectGblPoints(track12[1], track12[1]->getCardinalRep()), dfdextPlusMinus.second});

      TMatrixDSym massPrec(1); massPrec(0, 0) = 1. / motherWidth / motherWidth;
      TVectorD massResidual(1); massResidual = - (mother->getMass() - motherMass);

      TVectorD extMeasurements(1);
      extMeasurements[0] = massResidual[0];

      TMatrixD extDeriv(1, 9);
      extDeriv.Zero();
      extDeriv(0, 8) = 1.;

      gbl::GblTrajectory combined(daughters, extDeriv, extMeasurements, massPrec);

      combined.fit(chi2, ndf, lostWeight);
      //combined.printTrajectory(1000);
      //combined.printPoints(1000);
      getObjectPtr<TH1I>("ndf")->Fill(ndf);
      getObjectPtr<TH1F>("chi2_per_ndf")->Fill(chi2 / double(ndf));
      getObjectPtr<TH1F>("pval")->Fill(TMath::Prob(chi2, ndf));
      if (eventT0.isValid() && eventT0->hasEventT0()) {
        evt0 =  eventT0->getEventT0();
        getObjectPtr<TH1F>("evt0")->Fill(evt0);
      }


      B2RESULT("Mass(PDG) + vertex constrained fit results NDF = " << ndf << " Chi2/NDF = " << chi2 / double(ndf));

      if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);

    }
  }

  for (auto listName : m_primaryMassTwoBodyDecays) {
    StoreObjPtr<ParticleList> list(listName);
    if (!list.isValid())
      continue;

    DBObjPtr<BeamParameters> beam;

    double motherMass = beam->getMass();
    double motherWidth = sqrt((beam->getCovHER() + beam->getCovLER())(0, 0));

    updateMassWidthIfSet(listName, motherMass, motherWidth);

    for (unsigned int iParticle = 0; iParticle < list->getListSize(); ++iParticle) {

      auto mother = list->getParticle(iParticle);
      auto track12 = getParticlesTracks(mother->getDaughters());
      if (track12.size() != 2) {
        B2ERROR("Did not get 2 fitted tracks. Skipping this mother.");
        continue;
      }

      auto dfdextPlusMinus = getTwoBodyToLocalTransform(*mother, motherMass);
      std::vector<std::pair<std::vector<gbl::GblPoint>, TMatrixD> > daughters;

      daughters.push_back({gbl->collectGblPoints(track12[0], track12[0]->getCardinalRep()), dfdextPlusMinus.first});
      daughters.push_back({gbl->collectGblPoints(track12[1], track12[1]->getCardinalRep()), dfdextPlusMinus.second});

      TMatrixDSym massPrec(1); massPrec(0, 0) = 1. / motherWidth / motherWidth;
      TVectorD massResidual(1); massResidual = - (mother->getMass() - motherMass);

      TVectorD extMeasurements(1);
      extMeasurements[0] = massResidual[0];

      TMatrixD extDeriv(1, 9);
      extDeriv.Zero();
      extDeriv(0, 8) = 1.;

      gbl::GblTrajectory combined(daughters, extDeriv, extMeasurements, massPrec);

      combined.fit(chi2, ndf, lostWeight);
      getObjectPtr<TH1I>("ndf")->Fill(ndf);
      getObjectPtr<TH1F>("chi2_per_ndf")->Fill(chi2 / double(ndf));
      getObjectPtr<TH1F>("pval")->Fill(TMath::Prob(chi2, ndf));
      if (eventT0.isValid() && eventT0->hasEventT0()) {
        evt0 =  eventT0->getEventT0();
        getObjectPtr<TH1F>("evt0")->Fill(evt0);
      }


      B2RESULT("Mass constrained fit results NDF = " << ndf << " Chi2/NDF = " << chi2 / double(ndf));

      if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);

    }
  }

  for (auto listName : m_primaryMassVertexTwoBodyDecays) {
    StoreObjPtr<ParticleList> list(listName);
    if (!list.isValid())
      continue;

    DBObjPtr<BeamParameters> beam;

    double motherMass = beam->getMass();
    double motherWidth = sqrt((beam->getCovHER() + beam->getCovLER())(0, 0));

    updateMassWidthIfSet(listName, motherMass, motherWidth);

    for (unsigned int iParticle = 0; iParticle < list->getListSize(); ++iParticle) {

      auto mother = list->getParticle(iParticle);
      auto track12 = getParticlesTracks(mother->getDaughters());
      if (track12.size() != 2) {
        B2ERROR("Did not get 2 fitted tracks. Skipping this mother.");
        continue;
      }

      auto dfdextPlusMinus = getTwoBodyToLocalTransform(*mother, motherMass);
      std::vector<std::pair<std::vector<gbl::GblPoint>, TMatrixD> > daughters;

      daughters.push_back({gbl->collectGblPoints(track12[0], track12[0]->getCardinalRep()), dfdextPlusMinus.first});
      daughters.push_back({gbl->collectGblPoints(track12[1], track12[1]->getCardinalRep()), dfdextPlusMinus.second});

      TMatrixDSym vertexPrec(get<TMatrixDSym>(getPrimaryVertexAndCov()).Invert());
      B2Vector3D vertexResidual = - (mother->getVertex() - get<B2Vector3D>(getPrimaryVertexAndCov()));

      TMatrixDSym massPrec(1); massPrec(0, 0) = 1. / motherWidth / motherWidth;
      TVectorD massResidual(1); massResidual = - (mother->getMass() - motherMass);

      TMatrixDSym extPrec(4); extPrec.Zero();
      extPrec.SetSub(0, 0, vertexPrec);
      extPrec(3, 3) = massPrec(0, 0);

      TVectorD extMeasurements(4);
      extMeasurements[0] = vertexResidual[0];
      extMeasurements[1] = vertexResidual[1];
      extMeasurements[2] = vertexResidual[2];
      extMeasurements[3] = massResidual[0];

      TMatrixD extDeriv(4, 9);
      extDeriv.Zero();
      extDeriv(0, 0) = 1.;
      extDeriv(1, 1) = 1.;
      extDeriv(2, 2) = 1.;
      extDeriv(3, 8) = 1.;

      gbl::GblTrajectory combined(daughters, extDeriv, extMeasurements, extPrec);

      combined.fit(chi2, ndf, lostWeight);
      getObjectPtr<TH1I>("ndf")->Fill(ndf);
      getObjectPtr<TH1F>("chi2_per_ndf")->Fill(chi2 / double(ndf));
      getObjectPtr<TH1F>("pval")->Fill(TMath::Prob(chi2, ndf));
      if (eventT0.isValid() && eventT0->hasEventT0()) {
        evt0 =  eventT0->getEventT0();
        getObjectPtr<TH1F>("evt0")->Fill(evt0);
      }


      if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);

      B2RESULT("Mass + vertex constrained fit results NDF = " << ndf << " Chi2/NDF = " << chi2 / double(ndf));

    }
  }

  for (auto listName : m_primaryTwoBodyDecays) {
    B2WARNING("This should NOT be used for production of calibration constants for the real detector (yet)!");

    StoreObjPtr<ParticleList> list(listName);
    if (!list.isValid())
      continue;

    DBObjPtr<BeamParameters> beam;

    // For the error of invariant mass M = 2 * sqrt(E_HER * E_LER) (for m_e ~ 0)
    double M = beam->getMass();
    double E_HER = beam->getHER().E();
    double E_LER = beam->getLER().E();

    double pz = (beam->getHER().Vect() + beam->getLER().Vect())[2];
    double E  = (beam->getHER() + beam->getLER()).E();

    double motherMass = beam->getMass();
    double motherWidth = sqrt((E_HER / M) * (E_HER / M) * beam->getCovLER()(0, 0) + (E_LER / M) * (E_LER / M) * beam->getCovHER()(0,
                              0));

    updateMassWidthIfSet(listName, motherMass, motherWidth);

    for (unsigned int iParticle = 0; iParticle < list->getListSize(); ++iParticle) {

      B2WARNING("Two body decays with full kinematic constraint not yet correct - need to resolve strange covariance provided by BeamParameters!");

      auto mother = list->getParticle(iParticle);

      auto track12 = getParticlesTracks(mother->getDaughters());
      if (track12.size() != 2) {
        B2ERROR("Did not get exactly 2 fitted tracks. Skipping this mother in list " << listName);
        continue;
      }

      auto dfdextPlusMinus = getTwoBodyToLocalTransform(*mother, motherMass);
      std::vector<std::pair<std::vector<gbl::GblPoint>, TMatrixD> > daughters;

      daughters.push_back({gbl->collectGblPoints(track12[0], track12[0]->getCardinalRep()), dfdextPlusMinus.first});
      daughters.push_back({gbl->collectGblPoints(track12[1], track12[1]->getCardinalRep()), dfdextPlusMinus.second});

      TMatrixDSym extCov(7); extCov.Zero();

      // 3x3 IP vertex covariance
      extCov.SetSub(0, 0, get<TMatrixDSym>(getPrimaryVertexAndCov()));

      // 3x3 boost vector covariance
      //NOTE: BeamSpot return covarince in variables (E, theta_x, theta_y)
      // We need to transform it to our variables (px, py, pz)

      TMatrixD dBoost_dVect(3, 3);
      dBoost_dVect(0, 0) = 0.;     dBoost_dVect(0, 1) = 1. / pz; dBoost_dVect(0, 2) = 0.;
      dBoost_dVect(1, 0) = 0.;     dBoost_dVect(1, 1) = 0.;      dBoost_dVect(1, 2) = 1. / pz;
      dBoost_dVect(2, 0) = pz / E; dBoost_dVect(2, 1) = 0.;      dBoost_dVect(2, 2) = 0.;

      TMatrixD dVect_dBoost(3, 3);
      dVect_dBoost(0, 0) = 0.;     dVect_dBoost(0, 1) = 0.;      dVect_dBoost(0, 2) = E / pz;
      dVect_dBoost(1, 0) = pz;     dVect_dBoost(1, 1) = 0.;      dVect_dBoost(1, 2) = 0.;
      dVect_dBoost(2, 0) = 0.;     dVect_dBoost(2, 1) = pz;      dVect_dBoost(2, 2) = 0.;

      TMatrixD covBoost(3, 3);
      for (int i = 0; i < 3; ++i) {
        for (int j = i; j < 3; ++j) {
          covBoost(j, i) = covBoost(i, j) = (beam->getCovHER() + beam->getCovLER())(i, j);
        }
      }
      //TODO: Temporary fix: if theta_x, theta_y covariance is zero, use arbitrary 10mrad^2
//       if (covBoost(1, 1) == 0.) covBoost(1, 1) = 1.;
//       if (covBoost(2, 2) == 0.) covBoost(2, 2) = 1.;
      if (covBoost(1, 1) == 0.) covBoost(1, 1) = 1.e-4;
      if (covBoost(2, 2) == 0.) covBoost(2, 2) = 1.e-4;

      TMatrixD covVect =  dBoost_dVect * covBoost * dVect_dBoost;

      extCov.SetSub(3, 3, covVect);

      extCov(6, 6) = motherWidth * motherWidth;
      auto extPrec = extCov; extPrec.Invert();

      TVectorD extMeasurements(7);
      extMeasurements[0] = - (mother->getVertex() - get<B2Vector3D>(getPrimaryVertexAndCov()))[0];
      extMeasurements[1] = - (mother->getVertex() - get<B2Vector3D>(getPrimaryVertexAndCov()))[1];
      extMeasurements[2] = - (mother->getVertex() - get<B2Vector3D>(getPrimaryVertexAndCov()))[2];
      extMeasurements[3] = - (mother->getMomentum() - (beam->getHER().Vect() + beam->getLER().Vect()))[0];
      extMeasurements[4] = - (mother->getMomentum() - (beam->getHER().Vect() + beam->getLER().Vect()))[1];
      extMeasurements[5] = - (mother->getMomentum() - (beam->getHER().Vect() + beam->getLER().Vect()))[2];
      extMeasurements[6] = - (mother->getMass() - motherMass);

      B2INFO("mother mass = " << mother->getMass() << "  and beam mass = " << beam->getMass());

      TMatrixD extDeriv(7, 9);
      extDeriv.Zero();
      // beam vertex constraint
      extDeriv(0, 0) = 1.;
      extDeriv(1, 1) = 1.;
      extDeriv(2, 2) = 1.;
      // beam kinematics constraint
      extDeriv(3, 3) = 1.;
      extDeriv(4, 4) = 1.;
      extDeriv(5, 5) = 1.;
      // beam inv. mass constraint
      extDeriv(6, 8) = 1;

      if (m_calibrateVertex || m_calibrateKinematics) {
        B2WARNING("Primary vertex+kinematics calibration not (yet?) fully implemented!");
        B2WARNING("This code is highly experimental and has (un)known issues!");

        // up to d(x,y,z,px,py,pz,theta,phi,M)/d(vx,vy,vz,theta_x,theta_y,E)
        TMatrixD derivatives(9, 6);
        std::vector<int> labels;
        derivatives.Zero();

        if (m_calibrateVertex) {
          derivatives(0, 0) = 1.;
          derivatives(1, 1) = 1.;
          derivatives(2, 2) = 1.;
          GlobalLabel label = GlobalLabel::construct<BeamSpot>(0, 0);
          labels.push_back(label.setParameterId(1));
          labels.push_back(label.setParameterId(2));
          labels.push_back(label.setParameterId(3));
        } else {
          labels.push_back(0);
          labels.push_back(0);
          labels.push_back(0);
        }

        if (m_calibrateKinematics) {
          derivatives(3, 3) = mother->getMomentumMagnitude();
          derivatives(4, 4) = mother->getMomentumMagnitude();
          derivatives(8, 5) = (beam->getLER().E() + beam->getHER().E()) / beam->getMass();

          GlobalLabel label = GlobalLabel::construct<BeamSpot>(0, 0);
          labels.push_back(label.setParameterId(4)); //theta_x
          labels.push_back(label.setParameterId(5)); //theta_y
          labels.push_back(label.setParameterId(6)); //E

        } else {
          labels.push_back(0);
          labels.push_back(0);
          labels.push_back(0);
        }

        // Allow to disable BeamSpot externally
        alignment::GlobalDerivatives globals(labels, derivatives);

        // Add derivatives for vertex calibration to first point of first trajectory
        // NOTE: use GlobalDerivatives operators vector<int> and TMatrixD which filter
        // the derivatives to not pass those with zero labels (usefull to get rid of some params)
        std::vector<int> lab(globals); TMatrixD der(globals);

        // I want: dlocal/dext = dlocal/dtwobody * dtwobody/dext = dfdextPlusMinus * dtwobody/dext
        TMatrixD dTwoBody_dExt(9, 7);
        dTwoBody_dExt.Zero();
        // beam vertex constraint
        dTwoBody_dExt(0, 0) = 1.;
        dTwoBody_dExt(1, 1) = 1.;
        dTwoBody_dExt(2, 2) = 1.;
        // beam kinematics constraint
        dTwoBody_dExt(3, 3) = 1.;
        dTwoBody_dExt(4, 4) = 1.;
        dTwoBody_dExt(5, 5) = 1.;
        // beam inv. mass constraint
        dTwoBody_dExt(8, 6) = 1.;

        const TMatrixD dLocal_dExt = dfdextPlusMinus.first * dTwoBody_dExt;
        TMatrixD dLocal_dExt_T = dLocal_dExt; dLocal_dExt_T.T();

        // The 5x7 transformation matrix d(q/p,u',v',u,v)/d(vx,vy,vz,px,py,pz,M) needs to be "inverted"
        // to transform the covariance of the beamspot and boost vector of SuperKEKB into the local system
        // of one GBL point - such that Millepede can align the beamspot (or even beam kinematics) if requested.
        //
        // I tested also other methods, but only the Singular Value Decomposition gives nice-enough results,
        // with almost no code:
        //
        TDecompSVD svd(dLocal_dExt_T);
        TMatrixD dExt_dLocal  = svd.Invert().T();
        //
        // (dLocal_dExt * dExt_dLocal).Print(); // Check how close we are to unit matrix
        //
        // 5x5 matrix is as follows
        //
        //      |      0    |      1    |      2    |      3    |      4    |
        // ----------------------------------------------------------------------
        //    0 |          1   -2.58e-17   6.939e-18   1.571e-17  -1.649e-19
        //    1 |  1.787e-14           1   5.135e-16  -3.689e-16  -2.316e-18
        //    2 | -1.776e-15  -7.806e-17           1   5.636e-17   6.193e-18
        //    3 | -2.453e-15    7.26e-18   2.009e-16           1   -1.14e-16
        //    4 | -1.689e-14  -9.593e-17  -2.317e-15  -3.396e-17           1
        //
        // It took me half a day to find out how to do this with 2 lines of code (3 with the include).
        // Source: ROOT macro example - actually found at:
        // <https://root.cern.ch/root/html/tutorials/matrix/solveLinear.C.html>
        for (int i = 0; i < 7; ++i) {
          for (int j = 0; j < 5; ++j) {
            if (fabs(dExt_dLocal(i, j)) < 1.e-6)
              dExt_dLocal(i, j) = 0.;
          }
        }
        const TVectorD locRes = dLocal_dExt * extMeasurements;
        const TMatrixD locPrec =  dLocal_dExt * extPrec * dExt_dLocal;

        TMatrixDSym locPrecSym(5); locPrecSym.Zero();
        for (int i = 0; i < 5; ++i) {
          for (int j = i; j < 5; ++j) {
            //locPrecSym(j, i) = locPrecSym(i, j) = locPrec(i, j);
            locPrecSym(j, i) = locPrecSym(i, j) = (fabs(locPrec(i, j)) > 1.e-6) ? locPrec(i, j) : 0.;
          }
        }

        daughters[0].first[0].addMeasurement(locRes, locPrecSym);
        if (!lab.empty())
          daughters[0].first[0].addGlobals(lab, dfdextPlusMinus.first * der);

        //TODO: Understand this: either find a bug somewhere or improve the parametrization or .... ?
        // This should be enough, but the parametrization seems to fail for nearly horizontal pairs...
        //gbl::GblTrajectory combined(daughters);
        // This should not be needed, it actually seems to make worse Chi2/NDF, but GBL does not fail.
        // The measurement added just to be able to add the global derivatives (done just above) is redundant
        // to the external measurement added here:
        gbl::GblTrajectory combined(daughters, extDeriv, extMeasurements, extPrec);
        //combined.printTrajectory(1000);
        //combined.printPoints(1000);

        combined.fit(chi2, ndf, lostWeight);
        getObjectPtr<TH1I>("ndf")->Fill(ndf);
        getObjectPtr<TH1F>("chi2_per_ndf")->Fill(chi2 / double(ndf));
        getObjectPtr<TH1F>("pval")->Fill(TMath::Prob(chi2, ndf));
        if (eventT0.isValid() && eventT0->hasEventT0()) {
          evt0 =  eventT0->getEventT0();
          getObjectPtr<TH1F>("evt0")->Fill(evt0);
        }


        B2RESULT("Full kinematic-constrained fit (calibration version) results NDF = " << ndf << " Chi2/NDF = " << chi2 / double(ndf));

        if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);

      } else {

        gbl::GblTrajectory combined(daughters, extDeriv, extMeasurements, extPrec);
        //combined.printTrajectory(1000);
        //combined.printPoints(1000);

        combined.fit(chi2, ndf, lostWeight);
        getObjectPtr<TH1I>("ndf")->Fill(ndf);
        getObjectPtr<TH1F>("chi2_per_ndf")->Fill(chi2 / double(ndf));
        getObjectPtr<TH1F>("pval")->Fill(TMath::Prob(chi2, ndf));
        if (eventT0.isValid() && eventT0->hasEventT0()) {
          evt0 =  eventT0->getEventT0();
          getObjectPtr<TH1F>("evt0")->Fill(evt0);
        }


        B2RESULT("Full kinematic-constrained fit results NDF = " << ndf << " Chi2/NDF = " << chi2 / double(ndf));

        if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);
      }
    }
  }
}

void MillepedeCollectorModule::closeRun()
{
  // We close the file at end of run, producing
  // one file per run (and process id) which is more
  // convenient than one large binary block.
  auto mille = getObjectPtr<MilleData>("mille");
  if (mille->isOpen())
    mille->close();
}

void MillepedeCollectorModule::finish()
{
  Belle2::alignment::GlobalCalibrationManager::getInstance().writeConstraints("constraints.txt");

  StoreObjPtr<FileMetaData> fileMetaData("", DataStore::c_Persistent);
  if (!fileMetaData.isValid()) {
    B2ERROR("Cannot register binaries in FileCatalog.");
    return;
  }


  const std::vector<string> parents = {fileMetaData->getLfn()};
  for (auto binary : getObjectPtr<MilleData>("mille")->getFiles()) {
    FileMetaData milleMetaData(*fileMetaData);
    // We reset filename to be set directly by the registerFile procedure
    milleMetaData.setLfn("");
    milleMetaData.setParents(parents);
    FileCatalog::Instance().registerFile(binary, milleMetaData);
  }

}

void MillepedeCollectorModule::storeTrajectory(gbl::GblTrajectory& trajectory)
{
  if (m_useGblTree) {
    if (trajectory.isValid())
      m_currentGblData = trajectory.getData();
    else
      m_currentGblData.clear();

    if (!m_currentGblData.empty())
      getObjectPtr<TTree>("GblDataTree")->Fill();
  } else {
    getObjectPtr<MilleData>("mille")->fill(trajectory);
  }
}

std::string MillepedeCollectorModule::getUniqueMilleName()
{
  StoreObjPtr<EventMetaData> emd;
  string name = getName();

  name += "-e"   + to_string(emd->getExperiment());
  name += "-r"   + to_string(emd->getRun());
  name += "-ev"  + to_string(emd->getEvent());

  if (ProcHandler::parallelProcessingUsed())
    name += "-pid" + to_string(ProcHandler::EvtProcID());

  name += ".mille";

  return name;
}

bool MillepedeCollectorModule::fitRecoTrack(RecoTrack& recoTrack, Particle* particle)
{
  try {
    // For already fitted tracks, try to get fitted (DAF) weights for CDC
    if (m_updateCDCWeights && recoTrack.getNumberOfCDCHits() && recoTrack.getTrackFitStatus()
        && recoTrack.getTrackFitStatus()->isFitted()) {
      double sumCDCWeights = recoTrack.getNumberOfCDCHits(); // start with full weights
      // Do the hits synchronisation
      auto relatedRecoHitInformation =
        recoTrack.getRelationsTo<RecoHitInformation>(recoTrack.getStoreArrayNameOfRecoHitInformation());

      for (RecoHitInformation& recoHitInformation : relatedRecoHitInformation) {

        if (recoHitInformation.getFlag() == RecoHitInformation::c_pruned) {
          B2FATAL("Found pruned point in RecoTrack. Pruned tracks cannot be used in MillepedeCollector.");
        }

        if (recoHitInformation.getTrackingDetector() != RecoHitInformation::c_CDC) continue;

        const genfit::TrackPoint* trackPoint = recoTrack.getCreatedTrackPoint(&recoHitInformation);
        if (trackPoint) {
          if (not trackPoint->hasFitterInfo(recoTrack.getCardinalRepresentation()))
            continue;
          auto kalmanFitterInfo = dynamic_cast<genfit::KalmanFitterInfo*>(trackPoint->getFitterInfo());
          if (not kalmanFitterInfo) {
            continue;
          } else {
            std::vector<double> weights = kalmanFitterInfo->getWeights();
            if (weights.size() == 2) {
              if (weights.at(0) > weights.at(1))
                recoHitInformation.setRightLeftInformation(RecoHitInformation::c_left);
              else if (weights.at(0) < weights.at(1))
                recoHitInformation.setRightLeftInformation(RecoHitInformation::c_right);

              double weightLR = weights.at(0) + weights.at(1);
              if (weightLR < m_minCDCHitWeight)  recoHitInformation.setUseInFit(false);
              sumCDCWeights += weightLR - 1.; // reduce weight sum if weightLR<1
            }
          }
        }
      }

      double usedCDCHitFraction = sumCDCWeights / double(recoTrack.getNumberOfCDCHits());
      getObjectPtr<TH1F>("cdc_hit_fraction")->Fill(usedCDCHitFraction);
      if (usedCDCHitFraction < m_minUsedCDCHitFraction)
        return false;
    }
  } catch (...) {
    B2ERROR("Error in checking DAF weights from previous fit to resolve hit ambiguity. Why? Failed fit points in DAF? Skip track to be sure.");
    return false;
  }

  std::shared_ptr<genfit::GblFitter> gbl(new genfit::GblFitter());
  gbl->setOptions(m_internalIterations, true, true, m_externalIterations, m_recalcJacobians);
  gbl->setTrackSegmentController(new GblMultipleScatteringController);

  MeasurementAdder factory("", "", "", "", "");

  // We need the store arrays
  StoreArray<RecoHitInformation::UsedCDCHit> cdcHits("");
  StoreArray<RecoHitInformation::UsedPXDHit> pxdHits("");
  StoreArray<RecoHitInformation::UsedSVDHit> svdHits("");
  StoreArray<RecoHitInformation::UsedBKLMHit> bklmHits("");
  StoreArray<RecoHitInformation::UsedEKLMHit> eklmHits("");

  // Create the genfit::MeasurementFactory
  genfit::MeasurementFactory<genfit::AbsMeasurement> genfitMeasurementFactory;

  // Add producer for alignable RecoHits to factory
  if (pxdHits.isOptional()) {
    genfit::MeasurementProducer <RecoHitInformation::UsedPXDHit, AlignablePXDRecoHit>* PXDProducer =  new genfit::MeasurementProducer
    <RecoHitInformation::UsedPXDHit, AlignablePXDRecoHit> (pxdHits.getPtr());
    genfitMeasurementFactory.addProducer(Const::PXD, PXDProducer);
  }

  if (svdHits.isOptional())  {
    genfit::MeasurementProducer <RecoHitInformation::UsedSVDHit, AlignableSVDRecoHit>* SVDProducer =  new genfit::MeasurementProducer
    <RecoHitInformation::UsedSVDHit, AlignableSVDRecoHit> (svdHits.getPtr());
    genfitMeasurementFactory.addProducer(Const::SVD, SVDProducer);
  }

  if (cdcHits.isOptional()) {
    genfit::MeasurementProducer <RecoHitInformation::UsedCDCHit, AlignableCDCRecoHit>* CDCProducer =  new genfit::MeasurementProducer
    <RecoHitInformation::UsedCDCHit, AlignableCDCRecoHit> (cdcHits.getPtr());
    genfitMeasurementFactory.addProducer(Const::CDC, CDCProducer);
  }

  if (bklmHits.isOptional()) {
    genfit::MeasurementProducer <RecoHitInformation::UsedBKLMHit, AlignableBKLMRecoHit>* BKLMProducer =  new genfit::MeasurementProducer
    <RecoHitInformation::UsedBKLMHit, AlignableBKLMRecoHit> (bklmHits.getPtr());
    genfitMeasurementFactory.addProducer(Const::BKLM, BKLMProducer);
  }

  if (eklmHits.isOptional()) {
    genfit::MeasurementProducer <RecoHitInformation::UsedEKLMHit, AlignableEKLMRecoHit>* EKLMProducer =  new genfit::MeasurementProducer
    <RecoHitInformation::UsedEKLMHit, AlignableEKLMRecoHit> (eklmHits.getPtr());
    genfitMeasurementFactory.addProducer(Const::EKLM, EKLMProducer);
  }


  // Create the measurement creators
  std::vector<std::shared_ptr<PXDBaseMeasurementCreator>> pxdMeasurementCreators = { std::shared_ptr<PXDBaseMeasurementCreator>(new PXDCoordinateMeasurementCreator(genfitMeasurementFactory)) };
  std::vector<std::shared_ptr<SVDBaseMeasurementCreator>> svdMeasurementCreators = { std::shared_ptr<SVDBaseMeasurementCreator>(new SVDCoordinateMeasurementCreator(genfitMeasurementFactory)) };
  // TODO: Create a new MeasurementCreator based on SVDBaseMeasurementCreator (or on SVDCoordinateMeasurementCreator), which does the combination on the fly.

  std::vector<std::shared_ptr<CDCBaseMeasurementCreator>> cdcMeasurementCreators = { std::shared_ptr<CDCBaseMeasurementCreator>(new CDCCoordinateMeasurementCreator(genfitMeasurementFactory)) };
  std::vector<std::shared_ptr<BKLMBaseMeasurementCreator>> bklmMeasurementCreators = { std::shared_ptr<BKLMBaseMeasurementCreator>(new BKLMCoordinateMeasurementCreator(genfitMeasurementFactory)) };
  std::vector<std::shared_ptr<EKLMBaseMeasurementCreator>> eklmMeasurementCreators = { std::shared_ptr<EKLMBaseMeasurementCreator>(new EKLMCoordinateMeasurementCreator(genfitMeasurementFactory)) };

  // TODO: Or put it in here and leave the svdMeasurementCreators empty.
  std::vector<std::shared_ptr<BaseMeasurementCreator>> additionalMeasurementCreators = {};
  factory.resetMeasurementCreators(pxdMeasurementCreators, svdMeasurementCreators, cdcMeasurementCreators, bklmMeasurementCreators,
                                   eklmMeasurementCreators, additionalMeasurementCreators);
  factory.addMeasurements(recoTrack);

  auto& gfTrack = RecoTrackGenfitAccess::getGenfitTrack(recoTrack);

  int currentPdgCode = TrackFitter::createCorrectPDGCodeForChargedStable(Const::muon, recoTrack);
  if (particle)
    currentPdgCode = particle->getPDGCode();

  genfit::AbsTrackRep* trackRep = RecoTrackGenfitAccess::createOrReturnRKTrackRep(recoTrack, currentPdgCode);
  gfTrack.setCardinalRep(gfTrack.getIdForRep(trackRep));

  if (particle) {
    B2Vector3D vertexPos = particle->getVertex();
    B2Vector3D vertexMom = particle->getMomentum();
    gfTrack.setStateSeed(vertexPos, vertexMom);

    genfit::StateOnPlane vertexSOP(gfTrack.getCardinalRep());
    B2Vector3D vertexRPhiDir(vertexPos[0], vertexPos[1], 0);
    B2Vector3D vertexZDir(0, 0, vertexPos[2]);
    //FIXME: This causes problem to current GBL version in genfit -> needs update of GBL to re-enable
    // genfit::SharedPlanePtr vertexPlane(new genfit::DetPlane(vertexPos, vertexRPhiDir, vertexZDir));
    //This works instead fine:
    genfit::SharedPlanePtr vertexPlane(new genfit::DetPlane(vertexPos, vertexMom));

    vertexSOP.setPlane(vertexPlane);
    vertexSOP.setPosMom(vertexPos, vertexMom);
    TMatrixDSym vertexCov(5);
    vertexCov.UnitMatrix();
    // By using negative covariance no measurement is added to GBL. But this first point
    // is then used as additional point in trajectory at the assumed point of its fitted vertex
    vertexCov *= -1.;
    genfit::MeasuredStateOnPlane mop(vertexSOP, vertexCov);
    genfit::FullMeasurement* vertex = new genfit::FullMeasurement(mop, Const::IR);
    gfTrack.insertMeasurement(vertex, 0);
  }

  try {
    for (unsigned int i = 0; i < gfTrack.getNumPoints() - 1; ++i) {
      //if (gfTrack.getPointWithMeasurement(i)->getNumRawMeasurements() != 1)
      //  continue;
      genfit::PlanarMeasurement* planarMeas1 = dynamic_cast<genfit::PlanarMeasurement*>(gfTrack.getPointWithMeasurement(
                                                 i)->getRawMeasurement(0));
      genfit::PlanarMeasurement* planarMeas2 = dynamic_cast<genfit::PlanarMeasurement*>(gfTrack.getPointWithMeasurement(
                                                 i + 1)->getRawMeasurement(0));

      if (planarMeas1 != NULL && planarMeas2 != NULL &&
          planarMeas1->getDetId() == planarMeas2->getDetId() &&
          planarMeas1->getPlaneId() != -1 &&   // -1 is default plane id
          planarMeas1->getPlaneId() == planarMeas2->getPlaneId()) {
        Belle2::AlignableSVDRecoHit* hit1 = dynamic_cast<Belle2::AlignableSVDRecoHit*>(planarMeas1);
        Belle2::AlignableSVDRecoHit* hit2 = dynamic_cast<Belle2::AlignableSVDRecoHit*>(planarMeas2);
        if (hit1 && hit2) {
          Belle2::AlignableSVDRecoHit* hitU(NULL);
          Belle2::AlignableSVDRecoHit* hitV(NULL);
          // We have to decide U/V now (else AlignableSVDRecoHit2D could throw FATAL)
          if (hit1->isU() && !hit2->isU()) {
            hitU = hit1;
            hitV = hit2;
          } else if (!hit1->isU() && hit2->isU()) {
            hitU = hit2;
            hitV = hit1;
          } else {
            continue;
          }
          Belle2::AlignableSVDRecoHit2D* hit = new Belle2::AlignableSVDRecoHit2D(*hitU, *hitV);
          // insert measurement before point i (increases number of currect point to i+1)
          gfTrack.insertMeasurement(hit, i);
          // now delete current point (at its original place, we have the new 2D recohit)
          gfTrack.deletePoint(i + 1);
          gfTrack.deletePoint(i + 1);
        }
      }
    }
  } catch (std::exception& e) {
    B2ERROR(e.what());
    B2ERROR("SVD Cluster combination failed. This is symptomatic of pruned tracks. MillepedeCollector cannot process pruned tracks.");
    return false;
  }

  try {
    gbl->processTrackWithRep(&gfTrack, gfTrack.getCardinalRep(), true);
  } catch (genfit::Exception& e) {
    B2ERROR(e.what());
    return false;
  } catch (...) {
    B2ERROR("GBL fit failed.");
    return false;
  }

  return true;
}

std::vector< genfit::Track* > MillepedeCollectorModule::getParticlesTracks(std::vector<Particle*> particles, bool addVertexPoint)
{
  std::vector< genfit::Track* > tracks;
  for (auto particle : particles) {
    auto belle2Track = particle->getTrack();
    if (!belle2Track) {
      B2WARNING("No Belle2::Track for particle (particle->X");
      continue;
    }
//     auto trackFitResult = belle2Track->getTrackFitResult(Const::chargedStableSet.find(abs(particle->getPDGCode())));
//     if (!trackFitResult) {
//       B2INFO("No track fit result for track");
//       continue;
//     }
//    auto recoTrack = trackFitResult->getRelatedFrom<RecoTrack>();
    auto recoTrack = belle2Track->getRelatedTo<RecoTrack>();

    if (!recoTrack) {
      B2WARNING("No related RecoTrack for Belle2::Track (particle->Track->X)");
      continue;
    }

    // If any track fails, fail completely
    if (!fitRecoTrack(*recoTrack, (addVertexPoint) ? particle : nullptr))
      return {};

    auto& track = RecoTrackGenfitAccess::getGenfitTrack(*recoTrack);

    if (!track.hasFitStatus()) {
      B2WARNING("Track has no fit status");
      continue;
    }
    genfit::GblFitStatus* fs = dynamic_cast<genfit::GblFitStatus*>(track.getFitStatus());
    if (!fs) {
      B2WARNING("Track FitStatus is not GblFitStatus.");
      continue;
    }
    if (!fs->isFittedWithReferenceTrack()) {
      B2WARNING("Track is not fitted with reference track.");
      continue;
    }

    tracks.push_back(&track);
  }

  return tracks;
}

std::pair<TMatrixD, TMatrixD> MillepedeCollectorModule::getTwoBodyToLocalTransform(Particle& mother,
    double motherMass)
{
  std::vector<TMatrixD> result;

  double px = mother.getMomentum()[0];
  double py = mother.getMomentum()[1];
  double pz = mother.getMomentum()[2];
  double pt = sqrt(px * px + py * py);
  double p  = mother.getMomentumMagnitude();
  double M  = motherMass;
  double m  = mother.getDaughter(0)->getPDGMass();

  if (mother.getNDaughters() != 2
      || m != mother.getDaughter(1)->getPDGMass()) B2FATAL("Only two same-mass daughters (V0->f+f- decays) allowed.");

  // Rotation matrix from mother reference system to lab system
  TMatrixD mother2lab(3, 3);
  mother2lab(0, 0) = px * pz / pt / p; mother2lab(0, 1) = - py / pt; mother2lab(0, 2) = px / p;
  mother2lab(1, 0) = py * pz / pt / p; mother2lab(1, 1) =   px / pt; mother2lab(1, 2) = py / p;
  mother2lab(2, 0) = - pt / p;         mother2lab(2, 1) =   0;       mother2lab(2, 2) = pz / p;
  auto lab2mother = mother2lab; lab2mother.Invert();

  // Need to rotate and boost daughters' momenta to know which goes forward (+sign in decay model)
  // and to get the angles theta, phi of the decaying daughter system in mothers' reference frame
  RestFrame boostedFrame(&mother);
  TLorentzVector fourVector1 = mother.getDaughter(0)->get4Vector();
  TLorentzVector fourVector2 = mother.getDaughter(1)->get4Vector();

  auto mom1 = lab2mother * boostedFrame.getMomentum(fourVector1).Vect();
  auto mom2 = lab2mother * boostedFrame.getMomentum(fourVector2).Vect();
  // One momentum has opposite direction (otherwise should be same in CMS of mother), but which?
  double sign = 1.;
  auto avgMom = 0.5 * (mom1 - mom2);
  if (avgMom[2] < 0.) {
    avgMom *= -1.;
    // switch meaning of plus/minus trajectories
    sign = -1.;
  }

  double theta = atan2(avgMom.Perp(), avgMom[2]);
  double phi = atan2(avgMom[1], avgMom[0]);
  if (phi < 0.) phi += 2. * TMath::Pi();

  double alpha = M / 2. / m;
  double c1 = m * sqrt(alpha * alpha - 1.);
  double c2 = 0.5 * sqrt((alpha * alpha - 1.) / alpha / alpha * (p * p + M * M));

  double p3 = p * p * p;
  double pt3 = pt * pt * pt;


  for (auto& track : getParticlesTracks(mother.getDaughters())) {


    TMatrixD R = mother2lab;
    B2Vector3D P(sign * c1 * sin(theta) * cos(phi),
                 sign * c1 * sin(theta) * sin(phi),
                 p / 2. + sign * c2 * cos(theta));

    TMatrixD dRdpx(3, 3);
    dRdpx(0, 0) = - pz * (pow(px, 4.) - pow(py, 4.) - py * py * pz * pz) / pt3 / p3;
    dRdpx(0, 1) = px * py / pt3;
    dRdpx(0, 2) = (py * py + pz * pz) / p3;

    dRdpx(1, 0) = - px * py * pz * (2. * px * px + 2. * py * py + pz * pz) / pt3 / p3;
    dRdpx(1, 1) = - py * py / pt3;
    dRdpx(1, 2) = px * py / p3;

    dRdpx(2, 0) = - px * pz * pz / pt / p3;
    dRdpx(2, 1) = 0.;
    dRdpx(2, 2) = - px * pz / p3;

    TMatrixD dRdpy(3, 3);
    dRdpy(0, 0) = - px * py * pz * (2. * px * px + 2. * py * py + pz * pz) / pt3 / p3;
    dRdpy(0, 1) = - px * px / pt3;
    dRdpy(0, 2) = px * pz / p3;

    dRdpy(1, 0) = - pz * (- pow(px, 4.) - px * px * pz * pz + pow(py, 4.)) / pt3 / p3;
    dRdpy(1, 1) = px * py / pt3;
    dRdpy(1, 2) = (px * px + pz * pz) / p3;

    dRdpy(2, 0) = - py * pz * pz / pt / p3;
    dRdpy(2, 1) = 0.;
    dRdpy(2, 2) = - py * pz / p3;

    TMatrixD dRdpz(3, 3);
    dRdpz(0, 0) = px * pt / p3;
    dRdpz(0, 1) = 0.;
    dRdpz(0, 2) = - px * pz / p3;

    dRdpz(1, 0) = py * pt / p3;
    dRdpz(1, 1) = 0.;
    dRdpz(1, 2) = py * pz / p3;

    dRdpz(2, 0) = pz * pt / p3;
    dRdpz(2, 1) = 0.;
    dRdpz(2, 2) = (px * px + py * py) / p3;

    auto K = 1. / 2. / p + sign * cos(theta) * m * m * (M * M / 4. / m / m - 1.) / M / M / sqrt(m * m * (M * M / 4. / m / m - 1.) *
             (M * M + p * p) / M / M);

    B2Vector3D dpdpx = dRdpx * P + R * K * px * B2Vector3D(0., 0., 1.);
    B2Vector3D dpdpy = dRdpy * P + R * K * py * B2Vector3D(0., 0., 1.);
    B2Vector3D dpdpz = dRdpz * P + R * K * pz * B2Vector3D(0., 0., 1.);

    B2Vector3D dpdtheta = R * B2Vector3D(sign * c1 * cos(theta) * cos(phi),
                                         sign * c1 * cos(theta) * sin(phi),
                                         sign * c2 * (- sin(theta)));


    B2Vector3D dpdphi = R * B2Vector3D(sign * c1 * sin(theta) * (- sin(phi)),
                                       sign * c1 * sin(theta) * cos(phi),
                                       0.);

    double dc1dM = m * M / (2. * sqrt(M * M - 4. * m * m));
    double dc2dM = M * (4. * m * m * p * p + pow(M, 4)) / (2 * M * M * M * sqrt((M * M - 4. * m * m) * (p * p + M * M)));

    B2Vector3D dpdM = R * B2Vector3D(sign * sin(theta) * cos(phi) * dc1dM,
                                     sign * sin(theta) * sin(phi) * dc1dM,
                                     sign * cos(theta) * dc2dM);

    TMatrixD dpdz(3, 6);
    dpdz(0, 0) = dpdpx(0); dpdz(0, 1) = dpdpy(0); dpdz(0, 2) = dpdpz(0); dpdz(0, 3) = dpdtheta(0); dpdz(0, 4) = dpdphi(0);
    dpdz(0, 5) = dpdM(0);
    dpdz(1, 0) = dpdpx(1); dpdz(1, 1) = dpdpy(1); dpdz(1, 2) = dpdpz(1); dpdz(1, 3) = dpdtheta(1); dpdz(1, 4) = dpdphi(1);
    dpdz(1, 5) = dpdM(1);
    dpdz(2, 0) = dpdpx(2); dpdz(2, 1) = dpdpy(2); dpdz(2, 2) = dpdpz(2); dpdz(2, 3) = dpdtheta(2); dpdz(2, 4) = dpdphi(2);
    dpdz(2, 5) = dpdM(2);

    TMatrixD dqdv = getGlobalToLocalTransform(track->getFittedState()).GetSub(0, 4, 0, 2);
    TMatrixD dqdp = getGlobalToLocalTransform(track->getFittedState()).GetSub(0, 4, 3, 5);
    TMatrixD dfdvz(5, 9);
    dfdvz.SetSub(0, 0, dqdv);
    dfdvz.SetSub(0, 3, dqdp * dpdz);

    result.push_back(dfdvz);

    // switch sign for second trajectory
    sign *= -1.;
  }

  return {result[0], result[1]};
}

TMatrixD MillepedeCollectorModule::getGlobalToLocalTransform(const genfit::MeasuredStateOnPlane& msop)
{
  auto state = msop;
  const B2Vector3D& U(state.getPlane()->getU());
  const B2Vector3D& V(state.getPlane()->getV());
  const B2Vector3D& O(state.getPlane()->getO());
  const B2Vector3D& W(state.getPlane()->getNormal());

  const double* state5 = state.getState().GetMatrixArray();

  double spu = 1.;

  const TVectorD& auxInfo = state.getAuxInfo();
  if (auxInfo.GetNrows() == 2
      || auxInfo.GetNrows() == 1) // backwards compatibility with old RKTrackRep
    spu = state.getAuxInfo()(0);

  TVectorD state7(7);

  state7[0] = O.X() + state5[3] * U.X() + state5[4] * V.X(); // x
  state7[1] = O.Y() + state5[3] * U.Y() + state5[4] * V.Y(); // y
  state7[2] = O.Z() + state5[3] * U.Z() + state5[4] * V.Z(); // z

  state7[3] = spu * (W.X() + state5[1] * U.X() + state5[2] * V.X()); // a_x
  state7[4] = spu * (W.Y() + state5[1] * U.Y() + state5[2] * V.Y()); // a_y
  state7[5] = spu * (W.Z() + state5[1] * U.Z() + state5[2] * V.Z()); // a_z

  // normalize dir
  double norm = 1. / sqrt(state7[3] * state7[3] + state7[4] * state7[4] + state7[5] * state7[5]);
  for (unsigned int i = 3; i < 6; ++i) state7[i] *= norm;

  state7[6] = state5[0]; // q/p

  const double AtU = state7[3] * U.X() + state7[4] * U.Y() + state7[5] * U.Z();
  const double AtV = state7[3] * V.X() + state7[4] * V.Y() + state7[5] * V.Z();
  const double AtW = state7[3] * W.X() + state7[4] * W.Y() + state7[5] * W.Z();

  // J_Mp matrix is d(q/p,u',v',u,v) / d(x,y,z,px,py,pz)       (in is 6x6)

  const double qop = state7[6];
  const double p = state.getCharge() / qop; // momentum

  TMatrixD J_Mp_6x5(6, 5);
  J_Mp_6x5.Zero();

  //d(u)/d(x,y,z)
  J_Mp_6x5(0, 3)  = U.X(); // [0][3]
  J_Mp_6x5(1, 3)  = U.Y(); // [1][3]
  J_Mp_6x5(2, 3) = U.Z(); // [2][3]
  //d(v)/d(x,y,z)
  J_Mp_6x5(0, 4)  = V.X(); // [0][4]
  J_Mp_6x5(1, 4)  = V.Y(); // [1][4]
  J_Mp_6x5(2, 4) = V.Z(); // [2][4]

  // d(q/p)/d(px,py,pz)
  double fact = (-1.) * qop / p;
  J_Mp_6x5(3, 0) = fact * state7[3]; // [3][0]
  J_Mp_6x5(4, 0) = fact * state7[4]; // [4][0]
  J_Mp_6x5(5, 0) = fact * state7[5]; // [5][0]
  // d(u')/d(px,py,pz)
  fact = 1. / (p * AtW * AtW);
  J_Mp_6x5(3, 1) = fact * (U.X() * AtW - W.X() * AtU); // [3][1]
  J_Mp_6x5(4, 1) = fact * (U.Y() * AtW - W.Y() * AtU); // [4][1]
  J_Mp_6x5(5, 1) = fact * (U.Z() * AtW - W.Z() * AtU); // [5][1]
  // d(v')/d(px,py,pz)
  J_Mp_6x5(3, 2) = fact * (V.X() * AtW - W.X() * AtV); // [3][2]
  J_Mp_6x5(4, 2) = fact * (V.Y() * AtW - W.Y() * AtV); // [4][2]
  J_Mp_6x5(5, 2) = fact * (V.Z() * AtW - W.Z() * AtV); // [5][2]

  return J_Mp_6x5.T();
}

TMatrixD MillepedeCollectorModule::getLocalToGlobalTransform(const genfit::MeasuredStateOnPlane& msop)
{
  auto state = msop;
  // get vectors and aux variables
  const B2Vector3D& U(state.getPlane()->getU());
  const B2Vector3D& V(state.getPlane()->getV());
  const B2Vector3D& W(state.getPlane()->getNormal());

  const TVectorD& state5(state.getState());
  double spu = 1.;

  const TVectorD& auxInfo = state.getAuxInfo();
  if (auxInfo.GetNrows() == 2
      || auxInfo.GetNrows() == 1) // backwards compatibility with old RKTrackRep
    spu = state.getAuxInfo()(0);

  TVectorD pTilde(3);
  pTilde[0] = spu * (W.X() + state5(1) * U.X() + state5(2) * V.X()); // a_x
  pTilde[1] = spu * (W.Y() + state5(1) * U.Y() + state5(2) * V.Y()); // a_y
  pTilde[2] = spu * (W.Z() + state5(1) * U.Z() + state5(2) * V.Z()); // a_z

  const double pTildeMag = sqrt(pTilde[0] * pTilde[0] + pTilde[1] * pTilde[1] + pTilde[2] * pTilde[2]);
  const double pTildeMag2 = pTildeMag * pTildeMag;

  const double utpTildeOverpTildeMag2 = (U.X() * pTilde[0] + U.Y() * pTilde[1] + U.Z() * pTilde[2]) / pTildeMag2;
  const double vtpTildeOverpTildeMag2 = (V.X() * pTilde[0] + V.Y() * pTilde[1] + V.Z() * pTilde[2]) / pTildeMag2;

  //J_pM matrix is d(x,y,z,px,py,pz) / d(q/p,u',v',u,v)       (out is 6x6)

  const double qop = state5(0);
  const double p = state.getCharge() / qop; // momentum

  TMatrixD J_pM_5x6(5, 6);
  J_pM_5x6.Zero();

  // d(px,py,pz)/d(q/p)
  double fact = -1. * p / (pTildeMag * qop);
  J_pM_5x6(0, 3) = fact * pTilde[0]; // [0][3]
  J_pM_5x6(0, 4) = fact * pTilde[1]; // [0][4]
  J_pM_5x6(0, 5) = fact * pTilde[2]; // [0][5]
  // d(px,py,pz)/d(u')
  fact = p * spu / pTildeMag;
  J_pM_5x6(1, 3)  = fact * (U.X() - pTilde[0] * utpTildeOverpTildeMag2); // [1][3]
  J_pM_5x6(1, 4) = fact * (U.Y() - pTilde[1] * utpTildeOverpTildeMag2); // [1][4]
  J_pM_5x6(1, 5) = fact * (U.Z() - pTilde[2] * utpTildeOverpTildeMag2); // [1][5]
  // d(px,py,pz)/d(v')
  J_pM_5x6(2, 3) = fact * (V.X() - pTilde[0] * vtpTildeOverpTildeMag2); // [2][3]
  J_pM_5x6(2, 4) = fact * (V.Y() - pTilde[1] * vtpTildeOverpTildeMag2); // [2][4]
  J_pM_5x6(2, 5) = fact * (V.Z() - pTilde[2] * vtpTildeOverpTildeMag2); // [2][5]
  // d(x,y,z)/d(u)
  J_pM_5x6(3, 0) = U.X(); // [3][0]
  J_pM_5x6(3, 1) = U.Y(); // [3][1]
  J_pM_5x6(3, 2) = U.Z(); // [3][2]
  // d(x,y,z)/d(v)
  J_pM_5x6(4, 0) = V.X(); // [4][0]
  J_pM_5x6(4, 1) = V.Y(); // [4][1]
  J_pM_5x6(4, 2) = V.Z(); // [4][2]

  return J_pM_5x6.T();

}

tuple<B2Vector3D, TMatrixDSym> MillepedeCollectorModule::getPrimaryVertexAndCov() const
{
  DBObjPtr<BeamSpot> beam;
  return {beam->getIPPosition(), beam->getSizeCovMatrix()};
}

void MillepedeCollectorModule::updateMassWidthIfSet(string listName, double& mass, double& width)
{
  if (m_customMassConfig.find(listName) != m_customMassConfig.end()) {
    auto massWidth = m_customMassConfig.at(listName);
    mass = std::get<0>(massWidth);
    width = std::get<1>(massWidth);
  }
}

