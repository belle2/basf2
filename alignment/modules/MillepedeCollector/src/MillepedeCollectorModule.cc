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
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/dataobjects/BeamParameters.h>
#include <mdst/dataobjects/Track.h>
#include <genfit/Track.h>

#include <TMath.h>

using namespace Belle2;

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

  addParam("tracks", m_tracks, "Names of collections of genfit::Tracks for calibration", vector<string>({""}));
  addParam("particles", m_particles, "Names of particle list of single particles", vector<string>());
  addParam("vertices", m_vertices,
           "Name of particle list of (mother) particles with daughters for calibration using vertex constraint", vector<string>());
  addParam("primaryVertices", m_primaryVertices,
           "Name of particle list of (mother) particles with daughters for calibration using vertex + IP profile constraint",
           vector<string>());
  addParam("doublePrecision", m_doublePrecision, "Use double (=true) or single/float (=false) precision for writing binary files",
           bool(false));
}

void MillepedeCollectorModule::prepare()
{
  StoreObjPtr<EventMetaData>::required();

  if (m_tracks.empty() && m_particles.empty() && m_vertices.empty() && m_primaryVertices.empty())
    B2ERROR("You have to specify either arrays of single tracks or particle lists single single particles or mothers with vertex constrained daughters.");

  if (!m_tracks.empty()) {
    for (auto arrayName : m_tracks)
      StoreArray<genfit::Track>::required(arrayName);
  }

  if (!m_particles.empty() || !m_vertices.empty() || !m_primaryVertices.empty()) {
    StoreArray<genfit::TrackCand> TrackCands;
    StoreArray<genfit::Track> GF2Tracks;
    RelationArray TrackCandsToGF2Tracks(TrackCands, GF2Tracks);
    TrackCands.isRequired();
    GF2Tracks.isRequired();
    TrackCandsToGF2Tracks.isRequired();
  }

  for (auto listName : m_particles) {
    StoreObjPtr<ParticleList> list(listName);
    list.isRequired();
  }

  for (auto listName : m_vertices) {
    StoreObjPtr<ParticleList> list(listName);
    list.isRequired();
  }

  for (auto listName : m_primaryVertices) {
    StoreObjPtr<ParticleList> list(listName);
    list.isRequired();
  }

  // Register Mille output
  registerObject<MilleData>("mille", new MilleData(m_doublePrecision));

  registerObject<TH1F>("chi2/ndf", new TH1F("chi2/ndf", "chi2/ndf", 200, 0., 50.));
  registerObject<TH1F>("pval", new TH1F("pval", "pval", 100, 0., 1.));
}

void MillepedeCollectorModule::collect()
{
  // Open new file on request (at start or after being closed)
  auto& mille = getObject<MilleData>("mille");
  if (!mille.isOpen())
    mille.open(getUniqueMilleName());


  std::shared_ptr<genfit::GblFitter> gbl(new genfit::GblFitter());


  for (auto arrayName : m_tracks) {
    // Input tracks (have to be fitted by GBL)
    StoreArray<genfit::Track> tracks(arrayName);

    for (auto track : tracks) {
      if (!track.hasFitStatus())
        continue;
      genfit::GblFitStatus* fs = dynamic_cast<genfit::GblFitStatus*>(track.getFitStatus());
      if (!fs)
        continue;

      if (!fs->isFittedWithReferenceTrack())
        continue;

      getObject<TH1F>("chi2/ndf").Fill(fs->getChi2() / fs->getNdf());
      getObject<TH1F>("pval").Fill(fs->getPVal());

      using namespace gbl;
      GblTrajectory trajectory(gbl->collectGblPoints(&track, track.getCardinalRep()), fs->hasCurvature());
      mille.fill(trajectory);

    }

  }

  for (auto listName : m_particles) {
    StoreObjPtr<ParticleList> list(listName);
    for (unsigned int iParticle = 0; iParticle < list->getListSize(); ++iParticle) {
      for (auto track : getParticlesTracks({list->getParticle(iParticle)})) {
        auto gblfs = dynamic_cast<genfit::GblFitStatus*>(track->getFitStatus());

        getObject<TH1F>("chi2/ndf").Fill(gblfs->getChi2() / gblfs->getNdf());
        getObject<TH1F>("pval").Fill(gblfs->getPVal());

        gbl::GblTrajectory trajectory(gbl->collectGblPoints(track, track->getCardinalRep()), gblfs->hasCurvature());
        mille.fill(trajectory);

      }
    }
  }

  for (auto listName : m_vertices) {
    StoreObjPtr<ParticleList> list(listName);

    for (unsigned int iParticle = 0; iParticle < list->getListSize(); ++iParticle) {
      auto mother = list->getParticle(iParticle);
      std::vector<std::pair<std::vector<gbl::GblPoint>, TMatrixD> > daughters;

      for (auto track : getParticlesTracks(mother->getDaughters()))
        daughters.push_back({
        gbl->collectGblPoints(track, track->getCardinalRep()),
        getGlobalToLocalTransform(track->getFittedState()).GetSub(0, 4, 0, 2)
      });

      if (daughters.size() > 1) {
        gbl::GblTrajectory combined(daughters);

        double chi2 = -1.;
        double lostWeight = -1.;
        int ndf = -1;

        combined.fit(chi2, ndf, lostWeight);
        B2INFO("Combined GBL fit with vertex constraint: NDF = " << ndf << " Chi2/NDF = " << chi2 / ndf);

        mille.fill(combined);
        getObject<TH1F>("chi2/ndf").Fill(chi2 / ndf);
        getObject<TH1F>("pval").Fill(TMath::Prob(chi2, ndf));
      }
    }
  }


  for (auto listName : m_primaryVertices) {
    StoreObjPtr<ParticleList> list(listName);

    for (unsigned int iParticle = 0; iParticle < list->getListSize(); ++iParticle) {
      auto mother = list->getParticle(iParticle);
      std::vector<std::pair<std::vector<gbl::GblPoint>, TMatrixD> > daughters;

      for (auto track : getParticlesTracks(mother->getDaughters()))
        daughters.push_back({
        gbl->collectGblPoints(track, track->getCardinalRep()),
        getGlobalToLocalTransform(track->getFittedState()).GetSub(0, 4, 0, 2)
      });

      if (daughters.size() > 1) {
        TMatrixD extDerivatives(3, 3);
        TVectorD extMeasurements(3);
        TVectorD extPrecisions(3);

        extDerivatives.UnitMatrix();

        StoreObjPtr<BeamParameters> beam("", DataStore::c_Persistent);
        TVector3 vertexResidual = mother->getVertex() - beam->getVertex();


        TMatrixDSymEigen vertexCov(beam->getCovVertex());
        extPrecisions = vertexCov.GetEigenValues();
        extPrecisions[0] = 1. / extPrecisions[0];
        extPrecisions[1] = 1. / extPrecisions[1];
        extPrecisions[2] = 1. / extPrecisions[2];

        extDerivatives = vertexCov.GetEigenVectors();
        extDerivatives.T();

        extMeasurements[0] = vertexResidual[0];
        extMeasurements[1] = vertexResidual[1];
        extMeasurements[2] = vertexResidual[2];

        gbl::GblTrajectory combined(daughters, extDerivatives, extMeasurements, extPrecisions);

        double chi2 = -1.;
        double lostWeight = -1.;
        int ndf = -1;

        combined.fit(chi2, ndf, lostWeight);
        B2INFO("Combined GBL fit with vertex + ip profile constraint: NDF = " << ndf << " Chi2/NDF = " << chi2 / ndf);

        mille.fill(combined);
        getObject<TH1F>("chi2/ndf").Fill(chi2 / ndf);
        getObject<TH1F>("pval").Fill(TMath::Prob(chi2, ndf));
      }
    }
  }

  // Split binary files each 10000 events
  //if (recordCounter % 10000 == 0)
  //  mille.close();

}

void MillepedeCollectorModule::endRun()
{
  // We close the file at end of run, producing
  // one file per run (and process id) which is more
  // convenient than one large binary block.
  auto& mille = getObject<MilleData>("mille");
  if (mille.isOpen())
    mille.close();
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

std::vector< genfit::Track* > MillepedeCollectorModule::getParticlesTracks(std::vector<Particle*> particles)
{
  std::vector< genfit::Track* > tracks;
  for (auto particle : particles) {
    auto belle2Track = particle->getTrack();
    if (!belle2Track) {
      B2INFO("No Belle2::Track for particle");
      continue;
    }
    auto trackFitResult = belle2Track->getTrackFitResult(Const::chargedStableSet.find(abs(particle->getPDGCode())));
    if (!trackFitResult) {
      B2INFO("No track fit result for track");
      continue;
    }
    auto trackCand = trackFitResult->getRelatedFrom<genfit::TrackCand>();
    if (!trackCand) {
      B2INFO("No related track candidate for track fit result");
      continue;
    }
    auto track = DataStore::getRelated<genfit::Track>(trackCand);
    if (!track) {
      B2INFO("No related track for track candidate");
      continue;
    }
    if (!track->hasFitStatus()) {
      B2INFO("Track has no fit status");
      continue;
    }
    genfit::GblFitStatus* fs = dynamic_cast<genfit::GblFitStatus*>(track->getFitStatus());
    if (!fs) {
      B2INFO("Fit status is not GblFitStatus. You need tracks fitted by GBLfit module.")
      continue;
    }
    if (!fs->isFittedWithReferenceTrack()) {
      B2INFO("Track is not fitted with reference track.")
      continue;
    }

    tracks.push_back(track);
  }

  return tracks;
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

  return J_Mp_6x5;
}


