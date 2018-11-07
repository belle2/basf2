/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/modules/MillepedeCollector/MillepedeCollectorModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/core/FileCatalog.h>

#include <alignment/dataobjects/MilleData.h>

#include <genfit/Track.h>
#include <genfit/GblFitter.h>

#include <analysis/dataobjects/ParticleList.h>
#include <../analysis/utility/include/ReferenceFrame.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/BeamParameters.h>
#include <mdst/dataobjects/Track.h>
#include <genfit/Track.h>

#include <alignment/GlobalLabel.h>
#include <framework/core/FileCatalog.h>
#include <framework/dataobjects/FileMetaData.h>
#include <../framework/particledb/include/EvtGenDatabasePDG.h>

#include <TMath.h>
#include <TH1F.h>
#include <TTree.h>

#include <genfit/FullMeasurement.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>
#include <tracking/trackFitting/measurementCreator/adder/MeasurementAdder.h>

#include <alignment/reconstruction/AlignableSVDRecoHit2D.h>
#include <alignment/reconstruction/AlignableSVDRecoHit.h>

#include <genfit/PlanarMeasurement.h>

#include <alignment/reconstruction/AlignableCDCRecoHit.h>
#include <alignment/reconstruction/AlignablePXDRecoHit.h>
#include <alignment/reconstruction/AlignableSVDRecoHit.h>
#include <alignment/reconstruction/AlignableSVDRecoHit2D.h>
#include <alignment/reconstruction/BKLMRecoHit.h>
#include <alignment/reconstruction/AlignableEKLMRecoHit.h>

#include <alignment/Manager.h>
#include <alignment/Hierarchy.h>
#include <alignment/GlobalParam.h>
#include <alignment/GlobalDerivatives.h>

#include <alignment/dbobjects/VXDAlignment.h>

#include <genfit/KalmanFitterInfo.h>

//#include <alignment/reconstruction/GblMultipleScatteringController.h>

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
           "Specify which DB objects are calibrated, like ['BeamParameters', 'CDCTimeWalks'] or leave empty to use all components available.",
           m_components);
  addParam("calibrateVertex", m_calibrateVertex,
           "For primary vertices / two body decays, beam spot vertex calibration derivatives are added",
           bool(true));
  addParam("calibrateKinematics", m_calibrateKinematics,
           "For primary vertices / two body decays, beam spot kinematics calibration derivatives are added",
           bool(true));

  // Configure GBL fit of individual tracks
  //   addParam("externalIterations", m_externalIterations, "Number of external iterations of GBL fitter",
  //            int(0));
  //   addParam("internalIterations", m_internalIterations, "String defining internal GBL iterations for outlier down-weighting",
  //            string(""));
  //   addParam("recalcJacobians", m_recalcJacobians, "Up to which external iteration propagation Jacobians should be re-calculated",
  //            int(0));

  addParam("minPValue", m_minPValue, "Minimum p-value to write out a (combined) trajectory. Set <0 to write out all.",
           double(-1.));

  // Configure CDC specific options
  addParam("fitEventT0", m_fitEventT0, "Add local parameter for event T0 fit in GBL",
           bool(true));
  addParam("updateCDCWeights", m_updateCDCWeights, "Update L/R weights from previous DAF fit result",
           bool(true));
  addParam("minCDCHitWeight", m_minCDCHitWeight, "Minimum (DAF) CDC hit weight for usage by GBL",
           double(1.0E-6));
  addParam("minUsedCDCHitFraction", m_minUsedCDCHitFraction, "Minimum used CDC hit fraction to write out a trajectory",
           double(0.85));

}

void MillepedeCollectorModule::prepare()
{
  StoreObjPtr<EventMetaData> emd;
  emd.isRequired();

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

  Belle2::alignment::GlobalCalibrationManager::getInstance().initialize(m_components);
  Belle2::alignment::GlobalCalibrationManager::getInstance().writeConstraints("constraints.txt");

  AlignableCDCRecoHit::s_enableEventT0LocalDerivative = m_fitEventT0;

  /*
  if (m_useVXDHierarchy) {
    // Set-up hierarchy
    DBObjPtr<VXDAlignment> vxdAlignments;

    So the hierarchy is as follows:
                Belle 2
              / |     | \
          Ying  Yang Pat  Mat ... other sub-detectors
          / |   / |  |  \  | \
        ......  ladders ......
        / / |   / |  |  \  | \ \
      ......... sensors ........


    for (auto& halfShellPlacement : geo.getHalfShellPlacements()) {
      TGeoHMatrix trafoHalfShell = halfShellPlacement.second;
      trafoHalfShell *= geo.getTGeoFromRigidBodyParams(
                          vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dU),
                          vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dV),
                          vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dW),
                          vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dAlpha),
                          vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dBeta),
                          vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dGamma)
                        );
      hierarchy.insertTGeoTransform<VXDAlignment, alignment::EmptyGlobaParamSet>(halfShellPlacement.first, 0, trafoHalfShell);

      for (auto& ladderPlacement : geo.getLadderPlacements(halfShellPlacement.first)) {
        // Updated trafo
        TGeoHMatrix trafoLadder = ladderPlacement.second;
        trafoLadder *= geo.getTGeoFromRigidBodyParams(
                         vxdAlignments->get(ladderPlacement.first, VXDAlignment::dU),
                         vxdAlignments->get(ladderPlacement.first, VXDAlignment::dV),
                         vxdAlignments->get(ladderPlacement.first, VXDAlignment::dW),
                         vxdAlignments->get(ladderPlacement.first, VXDAlignment::dAlpha),
                         vxdAlignments->get(ladderPlacement.first, VXDAlignment::dBeta),
                         vxdAlignments->get(ladderPlacement.first, VXDAlignment::dGamma)
                       );
        hierarchy.insertTGeoTransform<VXDAlignment, VXDAlignment>(ladderPlacement.first, halfShellPlacement.first, trafoLadder);

        for (auto& sensorPlacement : geo.getSensorPlacements(ladderPlacement.first)) {
          // Updated trafo
          TGeoHMatrix trafoSensor = sensorPlacement.second;
          trafoSensor *= geo.getTGeoFromRigidBodyParams(
                           vxdAlignments->get(sensorPlacement.first, VXDAlignment::dU),
                           vxdAlignments->get(sensorPlacement.first, VXDAlignment::dV),
                           vxdAlignments->get(sensorPlacement.first, VXDAlignment::dW),
                           vxdAlignments->get(sensorPlacement.first, VXDAlignment::dAlpha),
                           vxdAlignments->get(sensorPlacement.first, VXDAlignment::dBeta),
                           vxdAlignments->get(sensorPlacement.first, VXDAlignment::dGamma)
                         );
          hierarchy.insertTGeoTransform<VXDAlignment, VXDAlignment>(sensorPlacement.first, ladderPlacement.first, trafoSensor);


        }
      }
    }
  }
  */
}

void MillepedeCollectorModule::collect()
{
  StoreObjPtr<EventMetaData> emd;
  alignment::GlobalCalibrationManager::getInstance().preCollect(*emd);

  if (!m_useGblTree) {
    // Open new file on request (at start or after being closed)
    auto mille = getObjectPtr<MilleData>("mille");
    if (!mille->isOpen())
      mille->open(getUniqueMilleName());
  }

  std::shared_ptr<genfit::GblFitter> gbl(new genfit::GblFitter());
  //gbl->setTrackSegmentController(new GblMultipleScatteringController);
  double chi2 = -1.;
  double lostWeight = -1.;
  int ndf = -1;

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
        getGlobalToLocalTransform(track->getFittedState()).T().GetSub(0, 4, 0, 2)
      });

      if (daughters.size() > 1) {
        gbl::GblTrajectory combined(daughters);

        combined.fit(chi2, ndf, lostWeight);
        getObjectPtr<TH1I>("ndf")->Fill(ndf);
        getObjectPtr<TH1F>("chi2_per_ndf")->Fill(chi2 / double(ndf));
        getObjectPtr<TH1F>("pval")->Fill(TMath::Prob(chi2, ndf));

        if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);

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

      TMatrixD extProjection(3, 5);

      bool first(true);
      for (auto& track : getParticlesTracks(mother->getDaughters())) {
        if (first) {
          // For first trajectory only
          extProjection = getLocalToGlobalTransform(track->getFittedState()).T().GetSub(0, 2, 0, 4);
          first = false;
        }
        daughters.push_back({
          gbl->collectGblPoints(track, track->getCardinalRep()),
          getGlobalToLocalTransform(track->getFittedState()).GetSub(0, 4, 0, 2)
        });
      }

      if (daughters.size() > 1) {
        DBObjPtr<BeamParameters> beam;

        TMatrixDSym vertexPrec(beam->getCovVertex().Invert());
        TVector3 vertexResidual = - (mother->getVertex() - beam->getVertex());

        TVectorD extMeasurements(3);
        extMeasurements[0] = vertexResidual[0];
        extMeasurements[1] = vertexResidual[1];
        extMeasurements[2] = vertexResidual[2];

        // Attach the external measurement to first point of first trajectory
        daughters[0].first[0].addMeasurement(extProjection, extMeasurements, vertexPrec);

        if (m_calibrateVertex) {
          TMatrixD derivatives(3, 3);
          derivatives.UnitMatrix();
          std::vector<int> labels;
          GlobalLabel label = GlobalLabel::construct<BeamParameters>(0, 0);
          labels.push_back(label.setParameterId(1));
          labels.push_back(label.setParameterId(2));
          labels.push_back(label.setParameterId(3));

          // Allow to disable BeamParameters externally
          alignment::GlobalDerivatives globals(labels, derivatives);

          // Add derivatives for vertex calibration to first point of first trajectory
          // NOTE: use GlobalDerivatives operators vector<int> and TMatrixD which filter
          // the derivatives to not pass those with zero labels (usefull to get rid of some params)
          std::vector<int> lab(globals); TMatrixD der(globals);

          if (!lab.empty())
            daughters[0].first[0].addGlobals(lab, der);
        }

        gbl::GblTrajectory combined(daughters);

        combined.fit(chi2, ndf, lostWeight);
        getObjectPtr<TH1I>("ndf")->Fill(ndf);
        getObjectPtr<TH1F>("chi2_per_ndf")->Fill(chi2 / double(ndf));
        getObjectPtr<TH1F>("pval")->Fill(TMath::Prob(chi2, ndf));

        if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);

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

      auto dfdextPlusMinus = getLocalToCommonTwoBodyExtParametersTransform(*mother, mother->getPDGMass());
      std::vector<std::pair<std::vector<gbl::GblPoint>, TMatrixD> > daughters;

      daughters.push_back({gbl->collectGblPoints(track12[0], track12[0]->getCardinalRep()), dfdextPlusMinus.first});
      daughters.push_back({gbl->collectGblPoints(track12[1], track12[1]->getCardinalRep()), dfdextPlusMinus.second});

      auto pdgdb = EvtGenDatabasePDG::Instance();
      double motherWidth = pdgdb->GetParticle(mother->getPDGCode())->Width();
      //TODO: what to take as width for "real" particles? -> make a param for default detector mass resolution??
      if (motherWidth == 0.) {
        motherWidth = m_stableParticleWidth * Unit::GeV;
        B2WARNING("Using artificial width for " << pdgdb->GetParticle(mother->getPDGCode())->GetName() << " : " << motherWidth << " GeV");
      }


      TMatrixDSym massPrec(1); massPrec(0, 0) = 1. / motherWidth;
      TVectorD massResidual(1); massResidual = - (mother->getMass() - mother->getPDGMass());

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

      B2RESULT("Mass(PDG) + vertex constrained fit results NDF = " << ndf << " Chi2/NDF = " << chi2 / double(ndf));

      if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);

    }
  }

  for (auto listName : m_primaryMassTwoBodyDecays) {
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

      DBObjPtr<BeamParameters> beam;
      auto dfdextPlusMinus = getLocalToCommonTwoBodyExtParametersTransform(*mother, beam->getMass());
      std::vector<std::pair<std::vector<gbl::GblPoint>, TMatrixD> > daughters;

      daughters.push_back({gbl->collectGblPoints(track12[0], track12[0]->getCardinalRep()), dfdextPlusMinus.first});
      daughters.push_back({gbl->collectGblPoints(track12[1], track12[1]->getCardinalRep()), dfdextPlusMinus.second});

      TMatrixDSym massPrec(1); massPrec(0, 0) = 1. / (beam->getCovHER() + beam->getCovHER())(0, 0);
      TVectorD massResidual(1); massResidual = - (mother->getMass() - beam->getMass());

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

      B2RESULT("Mass constrained fit results NDF = " << ndf << " Chi2/NDF = " << chi2 / double(ndf));

      if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);

    }
  }

  for (auto listName : m_primaryMassVertexTwoBodyDecays) {
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

      DBObjPtr<BeamParameters> beam;
      auto dfdextPlusMinus = getLocalToCommonTwoBodyExtParametersTransform(*mother, beam->getMass());
      std::vector<std::pair<std::vector<gbl::GblPoint>, TMatrixD> > daughters;

      daughters.push_back({gbl->collectGblPoints(track12[0], track12[0]->getCardinalRep()), dfdextPlusMinus.first});
      daughters.push_back({gbl->collectGblPoints(track12[1], track12[1]->getCardinalRep()), dfdextPlusMinus.second});

      TMatrixDSym vertexPrec(beam->getCovVertex().Invert());
      TVector3 vertexResidual = - (mother->getVertex() - beam->getVertex());

      TMatrixDSym massPrec(1); massPrec(0, 0) = 1. / (beam->getCovHER() + beam->getCovHER())(0, 0);
      TVectorD massResidual(1); massResidual = - (mother->getMass() - beam->getMass());

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

      if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);

      B2RESULT("Mass + vertex constrained fit results NDF = " << ndf << " Chi2/NDF = " << chi2 / double(ndf));

    }
  }

  for (auto listName : m_primaryTwoBodyDecays) {
    StoreObjPtr<ParticleList> list(listName);
    if (!list.isValid())
      continue;

    DBObjPtr<BeamParameters> beam;

    for (unsigned int iParticle = 0; iParticle < list->getListSize(); ++iParticle) {

      auto mother = list->getParticle(iParticle);

      auto track12 = getParticlesTracks(mother->getDaughters());
      if (track12.size() != 2) {
        B2ERROR("Did not get 2 fitted tracks. Skipping this mother.");
        continue;
      }

      auto dfdextPlusMinus = getLocalToCommonTwoBodyExtParametersTransform(*mother, beam->getMass());
      std::vector<std::pair<std::vector<gbl::GblPoint>, TMatrixD> > daughters;

      daughters.push_back({gbl->collectGblPoints(track12[0], track12[0]->getCardinalRep()), dfdextPlusMinus.first});
      daughters.push_back({gbl->collectGblPoints(track12[1], track12[1]->getCardinalRep()), dfdextPlusMinus.second});

      TMatrixDSym extPrec(7); extPrec.Zero();
      extPrec.SetSub(0, 0, beam->getCovVertex().Invert());
      //TODO:
      //I cannot get all 3 entries non-zero using Y4S setting for add_beamparameters
      //extPrec.SetSub(3, 3, (beam->getCovLER() + beam->getCovHER()).Invert());
      extPrec(3, 3) = 1. / (beam->getCovLER() + beam->getCovHER())(0, 0);
      extPrec(4, 4) = 1. / (beam->getCovLER() + beam->getCovHER())(0, 0);
      extPrec(5, 5) = 1. / (beam->getCovLER() + beam->getCovHER())(0, 0);
      //
      extPrec(6, 6) = 1. / (beam->getCovLER()(0, 0) + beam->getCovHER()(0, 0));

      TVectorD extMeasurements(7);
      extMeasurements[0] = - (mother->getVertex() - beam->getVertex())[0];
      extMeasurements[1] = - (mother->getVertex() - beam->getVertex())[1];
      extMeasurements[2] = - (mother->getVertex() - beam->getVertex())[2];
      extMeasurements[3] = - (mother->getMomentum() - (beam->getHER().Vect() + beam->getLER().Vect()))[0];
      extMeasurements[4] = - (mother->getMomentum() - (beam->getHER().Vect() + beam->getLER().Vect()))[1];
      extMeasurements[5] = - (mother->getMomentum() - (beam->getHER().Vect() + beam->getLER().Vect()))[2];
      extMeasurements[6] = - (mother->getMass() - beam->getMass());

      B2ERROR("mother mass = " << mother->getMass() << "  and beam mass = " << beam->getMass());

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

        TMatrixD derivatives(9, 6); // up to d(x,y,z,px,py,pz,theta,phi,M)/d(vx,vy,vz,theta_x,theta_y,E)
        std::vector<int> labels;
        derivatives.Zero();
        if (m_calibrateVertex) {
          derivatives(0, 0) = 1.;
          derivatives(1, 1) = 1.;
          derivatives(2, 2) = 1.;
          GlobalLabel label = GlobalLabel::construct<BeamParameters>(0, 0);
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

          GlobalLabel label = GlobalLabel::construct<BeamParameters>(0, 0);
          labels.push_back(label.setParameterId(4)); //theta_x
          labels.push_back(label.setParameterId(5)); //theta_y
          labels.push_back(label.setParameterId(6)); //E

        } else {
          labels.push_back(0);
          labels.push_back(0);
          labels.push_back(0);
        }
        // Allow to disable BeamParameters externally
        alignment::GlobalDerivatives globals(labels, derivatives);

        // Add derivatives for vertex calibration to first point of first trajectory
        // NOTE: use GlobalDerivatives operators vector<int> and TMatrixD which filter
        // the derivatives to not pass those with zero labels (usefull to get rid of some params)
        std::vector<int> lab(globals); TMatrixD der(globals);

        // I want: dlocal/dext = dlocal/dtwobody * dtwobody/dext = dfdextPlusMinus * extDeriv^(-1)
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

        TMatrixD dLocal_dExt = dfdextPlusMinus.first * dTwoBody_dExt;
        TMatrixD dLocal_dExt_T = dLocal_dExt; dLocal_dExt_T.T();
        TVectorD locRes = dLocal_dExt * extMeasurements;
        TMatrixD locPrec =  dLocal_dExt * extPrec * dLocal_dExt_T;

        TMatrixDSym prec(5); prec.Zero();
        for (int i = 0; i < 5; ++i)
          for (int j = 0; j < 5; ++j)
            prec(i, j) = locPrec(i, j);

        daughters[0].first[0].addMeasurement(locRes, prec);

        // particle 2
        TMatrixD dLocal_dExt2 = dfdextPlusMinus.second * dTwoBody_dExt;
        TMatrixD dLocal_dExt2_T = dLocal_dExt2; dLocal_dExt2_T.T();
        TVectorD locRes2 = dLocal_dExt2 * extMeasurements;
        TMatrixD locPrec2 =  dLocal_dExt2 * extPrec * dLocal_dExt2_T;

        TMatrixDSym prec2(5); prec2.Zero();
        for (int i = 0; i < 5; ++i)
          for (int j = 0; j < 5; ++j)
            prec2(i, j) = locPrec2(i, j);

        //daughters[1].first[0].addMeasurement(locRes2, prec2);

        if (!lab.empty())
          daughters[0].first[0].addGlobals(lab, dfdextPlusMinus.first * der);

        //if (!lab.empty())
        //  daughters[1].first[0].addGlobals(lab, dfdextPlusMinus.second * der);

        gbl::GblTrajectory combined(daughters);//, extDeriv, extMeasurements, extPrec);
        //combined.printTrajectory(1000);
        //combined.printPoints(1000);

        combined.fit(chi2, ndf, lostWeight);
        getObjectPtr<TH1I>("ndf")->Fill(ndf);
        getObjectPtr<TH1F>("chi2_per_ndf")->Fill(chi2 / double(ndf));
        getObjectPtr<TH1F>("pval")->Fill(TMath::Prob(chi2, ndf));

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

        B2RESULT("Full kinematic-constrained fit results NDF = " << ndf << " Chi2/NDF = " << chi2 / double(ndf));

        if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);
      }
    }
  }



//   for (auto listName : m_primaryTwoBodyDecays) {
//     StoreObjPtr<ParticleList> list(listName);
//     if (!list.isValid())
//       continue;
//
//     DBObjPtr<BeamParameters> beam;
//
//     for (unsigned int iParticle = 0; iParticle < list->getListSize(); ++iParticle) {
//
//       auto mother = list->getParticle(iParticle);
//
//       auto track12 = getParticlesTracks(mother->getDaughters());
//       if (track12.size() != 2) {
//         B2ERROR("Did not get 2 fitted tracks. Skipping this mother.");
//         continue;
//       }
//
//       auto dfdextPlusMinus = getLocalToCommonTwoBodyExtParametersTransform(*mother, beam->getMass());
//       std::vector<std::pair<std::vector<gbl::GblPoint>, TMatrixD> > daughters;
//
//       daughters.push_back({gbl->collectGblPoints(track12[0], track12[0]->getCardinalRep()), dfdextPlusMinus.first});
//       daughters.push_back({gbl->collectGblPoints(track12[1], track12[1]->getCardinalRep()), dfdextPlusMinus.second});
//
//       TMatrixDSym extPrec(7); extPrec.Zero();
//       extPrec.SetSub(0, 0, beam->getCovVertex().Invert());
//       //TODO:
//       //I cannot get all 3 entries non-zero using Y4S setting for add_beamparameters
//       //extPrec.SetSub(3, 3, (beam->getCovLER() + beam->getCovHER()).Invert());
//       extPrec(3, 3) = 1. / (beam->getCovLER() + beam->getCovHER())(0, 0);
//       extPrec(4, 4) = 1. / (beam->getCovLER() + beam->getCovHER())(0, 0);
//       extPrec(5, 5) = 1. / (beam->getCovLER() + beam->getCovHER())(0, 0);
//       //
//       extPrec(6, 6) = 1. / (beam->getCovLER()(0, 0) + beam->getCovHER()(0, 0));
//
//       TVectorD extMeasurements(7);
//       extMeasurements[0] = - (mother->getVertex() - beam->getVertex())[0];
//       extMeasurements[1] = - (mother->getVertex() - beam->getVertex())[1];
//       extMeasurements[2] = - (mother->getVertex() - beam->getVertex())[2];
//       extMeasurements[3] = - (mother->getMomentum() - (beam->getHER().Vect() + beam->getLER().Vect()))[0];
//       extMeasurements[4] = - (mother->getMomentum() - (beam->getHER().Vect() + beam->getLER().Vect()))[1];
//       extMeasurements[5] = - (mother->getMomentum() - (beam->getHER().Vect() + beam->getLER().Vect()))[2];
//       extMeasurements[6] = - (mother->getMass() - beam->getMass());
//
//       TMatrixD extDeriv(7, 9);
//       extDeriv.Zero();
//       // beam vertex constraint
//       extDeriv(0, 0) = 1.;
//       extDeriv(1, 1) = 1.;
//       extDeriv(2, 2) = 1.;
//       // beam kinematics constraint
//       extDeriv(3, 3) = 1.;
//       extDeriv(4, 4) = 1.;
//       extDeriv(5, 5) = 1.;
//       // beam inv. mass constraint
//       extDeriv(6, 8) = 1;
//
//       if (m_calibrateVertex || m_calibrateKinematics) {
//         B2WARNING("Primary vertex+kinematics calibration not (yet?) fully implemented!");
//
//         TMatrixD derivatives(9, 6); // up to d(x,y,z,px,py,pz,theta,phi,M)/d(vx,vy,vz,theta_x,theta_y,E)
//         std::vector<int> labels;
//         derivatives.Zero();
//         if (m_calibrateVertex) {
//           derivatives(0, 0) = 1.;
//           derivatives(1, 1) = 1.;
//           derivatives(2, 2) = 1.;
//           GlobalLabel label = GlobalLabel::construct<BeamParameters>(0, 0);
//           labels.push_back(label.setParameterId(1));
//           labels.push_back(label.setParameterId(2));
//           labels.push_back(label.setParameterId(3));
//         } else {
//           labels.push_back(0);
//           labels.push_back(0);
//           labels.push_back(0);
//         }
//
//         if (m_calibrateKinematics) {
//           derivatives(3, 3) = mother->getMomentumMagnitude();
//           derivatives(4, 4) = mother->getMomentumMagnitude();
//           derivatives(8, 5) = (beam->getLER().E() + beam->getHER().E()) / beam->getMass();
//
//           GlobalLabel label = GlobalLabel::construct<BeamParameters>(0, 0);
//           labels.push_back(label.setParameterId(4)); //theta_x
//           labels.push_back(label.setParameterId(5)); //theta_y
//           labels.push_back(label.setParameterId(6)); //E
//
//         } else {
//           labels.push_back(0);
//           labels.push_back(0);
//           labels.push_back(0);
//         }
//         // Allow to disable BeamParameters externally
//         alignment::GlobalDerivatives globals(labels, derivatives);
//
//         // Add derivatives for vertex calibration to first point of first trajectory
//         // NOTE: use GlobalDerivatives operators vector<int> and TMatrixD which filter
//         // the derivatives to not pass those with zero labels (usefull to get rid of some params)
//         std::vector<int> lab(globals); TMatrixD der(globals);
//
//         // I want: dlocal/dext = dlocal/dtwobody * dtwobody/dext = dfdextPlusMinus * extDeriv^(-1)
//         TMatrixD dTwoBody_dExt(9, 7);
//         dTwoBody_dExt.Zero();
//         // beam vertex constraint
//         dTwoBody_dExt(0, 0) = 1.;
//         dTwoBody_dExt(1, 1) = 1.;
//         dTwoBody_dExt(2, 2) = 1.;
//         // beam kinematics constraint
//         dTwoBody_dExt(3, 3) = 1.;
//         dTwoBody_dExt(4, 4) = 1.;
//         dTwoBody_dExt(5, 5) = 1.;
//         // beam inv. mass constraint
//         dTwoBody_dExt(8, 6) = 1.;
//
//         TMatrixD dLocal_dExt = dfdextPlusMinus.first * dTwoBody_dExt;
//         TMatrixD dLocal_dExt_T = dLocal_dExt; dLocal_dExt_T.T();
//         TVectorD locRes = dLocal_dExt * extMeasurements;
//         TMatrixD locPrec =  dLocal_dExt * extPrec * dLocal_dExt_T;
//
//         TMatrixDSym prec(5); prec.Zero();
//         for (int i = 0; i < 5; ++i)
//           for (int j = 0; j < 5; ++j)
//             prec(i, j) = locPrec(i, j);
//
//         daughters[0].first[0].addMeasurement(locRes, prec);
//
//         // particle 2
//         TMatrixD dLocal_dExt2 = dfdextPlusMinus.second * dTwoBody_dExt;
//         TMatrixD dLocal_dExt2_T = dLocal_dExt2; dLocal_dExt2_T.T();
//         TVectorD locRes2 = dLocal_dExt2 * extMeasurements;
//         TMatrixD locPrec2 =  dLocal_dExt2 * extPrec * dLocal_dExt2_T;
//
//         TMatrixDSym prec2(5); prec2.Zero();
//         for (int i = 0; i < 5; ++i)
//           for (int j = 0; j < 5; ++j)
//             prec2(i, j) = locPrec2(i, j);
//
//         daughters[1].first[0].addMeasurement(locRes2, prec2);
//
//         if (!lab.empty())
//           daughters[0].first[0].addGlobals(lab, dfdextPlusMinus.first * der);
//
//         if (!lab.empty())
//           daughters[1].first[0].addGlobals(lab, dfdextPlusMinus.second * der);
//
//         gbl::GblTrajectory combined(daughters, extDeriv, extMeasurements, extPrec);
//         //combined.printTrajectory(1000);
//         //combined.printPoints(1000);
//
//         combined.fit(chi2, ndf, lostWeight);
//         getObjectPtr<TH1I>("ndf")->Fill(ndf);
//         getObjectPtr<TH1F>("chi2_per_ndf")->Fill(chi2 / double(ndf));
//         getObjectPtr<TH1F>("pval")->Fill(TMath::Prob(chi2, ndf));
//
//         B2RESULT("Full kinematic-constrained fit (calibration version) results NDF = " << ndf << " Chi2/NDF = " << chi2 / double(ndf));
//
//         if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);
//
//       } else {
//
//         gbl::GblTrajectory combined(daughters, extDeriv, extMeasurements, extPrec);
//         //combined.printTrajectory(1000);
//         //combined.printPoints(1000);
//
//         combined.fit(chi2, ndf, lostWeight);
//         getObjectPtr<TH1I>("ndf")->Fill(ndf);
//         getObjectPtr<TH1F>("chi2_per_ndf")->Fill(chi2 / double(ndf));
//         getObjectPtr<TH1F>("pval")->Fill(TMath::Prob(chi2, ndf));
//
//         B2RESULT("Full kinematic-constrained fit results NDF = " << ndf << " Chi2/NDF = " << chi2 / double(ndf));
//
//         if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);
//       }
//     }
//   }
//


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
  //gbl->setOptions(m_internalIterations, true, true, m_externalIterations, m_recalcJacobians);
  gbl->setOptions("", true, true, 0, 0);
  //gbl->setTrackSegmentController(new GblMultipleScatteringController);

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
    genfit::MeasurementProducer <RecoHitInformation::UsedBKLMHit, BKLMRecoHit>* BKLMProducer =  new genfit::MeasurementProducer
    <RecoHitInformation::UsedBKLMHit, BKLMRecoHit> (bklmHits.getPtr());
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
    TVector3 vertexPos = particle->getVertex();
    TVector3 vertexMom = particle->getMomentum();
    gfTrack.setStateSeed(vertexPos, vertexMom);

    genfit::StateOnPlane vertexSOP(gfTrack.getCardinalRep());
    TVector3 vertexRPhiDir(vertexPos[0], vertexPos[1], 0);
    TVector3 vertexZDir(0, 0, vertexPos[2]);
    genfit::SharedPlanePtr vertexPlane(new genfit::DetPlane(vertexPos, vertexRPhiDir, vertexZDir));
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
  } catch (...) {
    B2ERROR("SVD Cluster combination failed. This is symptomatic of pruned tracks. MillepedeCollector cannot process pruned tracks.");
    return false;
  }

  try {
    gbl->processTrackWithRep(&gfTrack, gfTrack.getCardinalRep(), true);
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


std::pair<TMatrixD, TMatrixD> MillepedeCollectorModule::getLocalToCommonTwoBodyExtParametersTransform(Particle& mother,
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
  auto lab2mother = mother2lab.Invert();

  // Need to rotate and boost daughters' momenta to know which goes forward (+sign in decay model)
  // and to get the angles theta, phi of the decaying daughter system in mothers' reference frame
  RestFrame boostedFrame(&mother);
  TLorentzVector fourVector1(lab2mother * mother.getDaughter(0)->get4Vector().Vect(), mother.getDaughter(0)->get4Vector().T());
  TLorentzVector fourVector2(lab2mother * mother.getDaughter(1)->get4Vector().Vect(), mother.getDaughter(1)->get4Vector().T());

  auto mom1 = boostedFrame.getMomentum(fourVector1).Vect();
  auto mom2 = boostedFrame.getMomentum(fourVector2).Vect();
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

  std::vector<std::pair<std::vector<gbl::GblPoint>, TMatrixD> > daughters;

  double alpha = M / 2. / m;
  double c1 = m * sqrt(alpha * alpha - 1.);
  double c2 = 0.5 * sqrt((alpha * alpha - 1.) / alpha / alpha * (p * p + M * M));

  double p3 = p * p * p;
  double pt3 = pt * pt * pt;


  for (auto& track : getParticlesTracks(mother.getDaughters())) {


    TMatrixD R = mother2lab;
    TVector3 P(sign * c1 * sin(theta) * cos(phi),
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

    TVector3 dpdpx = dRdpx * P;
    TVector3 dpdpy = dRdpy * P;
    TVector3 dpdpz = dRdpz * P;

    TVector3 dpdtheta = R * TVector3(sign * c1 * cos(theta) * cos(phi),
                                     sign * c1 * cos(theta) * sin(phi),
                                     p / 2. + sign * c2 * (- sin(phi)));


    TVector3 dpdphi = R * TVector3(sign * c1 * sin(theta) * (- sin(phi)),
                                   sign * c1 * sin(theta) * cos(phi),
                                   0.);

    TVector3 dpdM = R * TVector3(0.,
                                 0.,
                                 0.5 * sign * M / (2. * c2) * cos(phi));

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

TMatrixD MillepedeCollectorModule::getGlobalToLocalTransform(genfit::MeasuredStateOnPlane msop)
{
  auto state = msop;
  const TVector3& U(state.getPlane()->getU());
  const TVector3& V(state.getPlane()->getV());
  const TVector3& O(state.getPlane()->getO());
  const TVector3& W(state.getPlane()->getNormal());

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

TMatrixD MillepedeCollectorModule::getLocalToGlobalTransform(genfit::MeasuredStateOnPlane msop)
{
  auto state = msop;
  // get vectors and aux variables
  const TVector3& U(state.getPlane()->getU());
  const TVector3& V(state.getPlane()->getV());
  const TVector3& W(state.getPlane()->getNormal());

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