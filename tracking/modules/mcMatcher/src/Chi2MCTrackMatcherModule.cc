/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/mcMatcher/Chi2MCTrackMatcherModule.h>

// datastore types
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/gearbox/Const.h>
#include <Eigen/Dense>
#include <iostream>

// ROOT
#include <TVectorD.h>
#include <TMatrixD.h>


using namespace Belle2;

REG_MODULE(Chi2MCTrackMatcher)



// Constructor
Chi2MCTrackMatcherModule::Chi2MCTrackMatcherModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(Monte Carlo matcher using the helix parameters for matching by chi2-method)DOC");
  // setPropertyFlags(c_ParallelProcessingCertified);
  // set module parameters
  std::vector<double> defaultCutOffs(6);
  defaultCutOffs[0] = 128024;
  defaultCutOffs[1] = 95;
  defaultCutOffs[2] = 173;
  defaultCutOffs[3] = 424;
  defaultCutOffs[4] = 90;
  defaultCutOffs[5] = 424;//in first approximation take proton
  addParam("CutOffs",
           m_param_CutOffs,
           "Defines the Cut Off values for each charged particle. pdg order [11,13,211,2212,321,1000010020]",
           defaultCutOffs);
  addParam("linalg",
           m_param_linalg,
           "Parameter to switch between ROOT and Eigen, to invert the covariance matrix. false: ROOT is used; true: Eigen is used",
           false);
}

void Chi2MCTrackMatcherModule::initialize()
{
  // Check if there are MC Particles
  m_MCParticles.isRequired();
  m_Tracks.isRequired();
  m_Tracks.registerRelationTo(m_MCParticles);

  // Statistic variables
  m_trackCount = 0;
  m_notInvertableCount = 0;
  m_noMatchCount = 0;
  m_noMatchingCandidateCount = 0;
  m_trackCount = 0;
  m_covarianceMatrixCount = 0;
  m_event = 0;
}

void Chi2MCTrackMatcherModule::event()
{
  m_event = m_event + 1;
  B2DEBUG(1, "m_event = " << m_event);
  // suppress the ROOT "matrix is singular" error message
  auto default_gErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = 5000;

  // get StoreArray length
  int nTracks = m_Tracks.getEntries();
  int nMCParticles = m_MCParticles.getEntries();
  // tracks number of tracks for Debug statistics
  m_trackCount = m_trackCount + nTracks;
  // check if there are m_Tracks and m_MCParticles to match to
  if (not nMCParticles or not nTracks) {
    // Cannot perfom matching
    return;
  }

  // compare all tracks with all mcParticles in event
  for (int it = 0 ; it < nTracks; ++it) {
    auto track = m_Tracks[it];

    // test for existing relations
    if (track->getRelated<MCParticle>()) {
      B2DEBUG(27, "Relation already set continue with next track");
      continue;
    }
    // initialize minimal chi2 variables
    int ip_min = -1;
    double chi2Min = std::numeric_limits<double>::infinity();

    // loop over m_MCParticles and calculate Chi2 for each track mcparticle pair, to fine minimal chi2
    for (int ip = 0; ip < nMCParticles; ++ip) {
      auto mcParticle = m_MCParticles[ip];
      // Check if current particle is a charged stable particle
      const Const::ParticleType mcParticleType(std::abs(mcParticle->getPDG()));
      if (not Const::chargedStableSet.contains(mcParticleType)) {
        continue;
      }

      // get trackfitresult of current track with clossest mass to current mcparticle Type
      auto trackFitResult = track->getTrackFitResultWithClosestMass(mcParticleType);
      TMatrixD Covariance5 = trackFitResult->getCovariance5();
      // statistic variable counting number of covariance matricies
      m_covarianceMatrixCount = m_covarianceMatrixCount + 1;
      // check if matrix is invertable
      double det = Covariance5.Determinant();
      if (det == 0.0) {
        // statistic variable counting number of not invertable covariance matricies
        m_notInvertableCount = m_notInvertableCount + 1;
        //Covariance5.Print();
        continue;
      }

      // generate helix for current mc particle
      double charge_sign = 1;
      if (mcParticle->getCharge() < 0) { charge_sign = -1;}
      UncertainHelix helix = trackFitResult->getUncertainHelix();
      auto b_field = BFieldManager::getField(helix.getPerigee()).Z() / Belle2::Unit::T;
      auto mcParticleHelix = Helix(mcParticle->getVertex(), mcParticle->getMomentum(), charge_sign, b_field);

      // initialize the curent chi2
      double chi2Cur;

      // Check which linear algebra system should be used and calculate chi2Cur
      if (not m_param_linalg) {
        // Eigen
        // build difference vector
        Eigen::VectorXd delta(5);
        delta(0) = trackFitResult->getD0()        - mcParticleHelix.getD0();
        delta(1) = trackFitResult->getPhi0()      - mcParticleHelix.getPhi0();
        delta(2) = trackFitResult->getOmega()     - mcParticleHelix.getOmega();
        delta(3) = trackFitResult->getZ0()        - mcParticleHelix.getZ0();
        delta(4) = trackFitResult->getTanLambda() - mcParticleHelix.getTanLambda();
        // build convariance5 Eigen matrix
        Eigen::MatrixXd covariance5Eigen(5, 5);
        for (int i = 0; i < 5; i++) {
          for (int j = 0; j < 5; j++) {
            covariance5Eigen(i, j) = Covariance5[i][j];
          }
        }
        //calculate chi2Cur
        chi2Cur = ((delta.transpose()) * (covariance5Eigen.inverse() * delta))(0, 0);
      } else {
        // ROOT
        // calculate difference vector
        TMatrixD delta(5, 1);
        delta[0][0] = trackFitResult->getD0()        - mcParticleHelix.getD0();
        delta[1][0] = trackFitResult->getPhi0()      - mcParticleHelix.getPhi0();
        delta[2][0] = trackFitResult->getOmega()     - mcParticleHelix.getOmega();
        delta[3][0] = trackFitResult->getZ0()        - mcParticleHelix.getZ0();
        delta[4][0] = trackFitResult->getTanLambda() - mcParticleHelix.getTanLambda();
        // invert Covariance5 matrix
        Covariance5.InvertFast();
        // transpose difference vector
        TMatrixD deltaT = delta;
        deltaT.T();
        // calculate chi2Cur
        chi2Cur = ((deltaT) * (Covariance5 * delta))[0][0];
      }
      // check if chi2Cur is smaller than the so far found minimal chi2Min
      if (chi2Min > chi2Cur) {
        chi2Min = chi2Cur;
        ip_min = ip;
      }
    }
    // check if any matching candidate was found
    if (ip_min == -1) {
      m_noMatchingCandidateCount = m_noMatchingCandidateCount + 1;
      m_noMatchCount = m_noMatchCount + 1;
      continue;
    }
    // initialize Cut Off
    double cutOff = 0;
    // fill cut off with value
    // find PDG for mcParticle with minimal chi2, because this determines individual cutOff
    int mcMinPDG = abs(m_MCParticles[ip_min]->getPDG());

    if (mcMinPDG == Belle2::Const::electron.getPDGCode()) {
      cutOff = m_param_CutOffs[0];
    } else if (mcMinPDG == Const::muon.getPDGCode()) {
      cutOff = m_param_CutOffs[1];
    } else if (mcMinPDG == Const::pion.getPDGCode()) {
      cutOff = m_param_CutOffs[2];
    } else if (mcMinPDG == Const::proton.getPDGCode()) {
      cutOff = m_param_CutOffs[3];
    } else if (mcMinPDG == Const::kaon.getPDGCode()) {
      cutOff = m_param_CutOffs[4];
    } else if (mcMinPDG == Const::deuteron.getPDGCode()) {
      cutOff = m_param_CutOffs[5];
    } else {
      B2WARNING("The pdg for minimal chi2 was not in chargedstable particle list: MinPDG = " << mcMinPDG);
      continue;
    }
    if (chi2Min < cutOff) {
      m_Tracks[it]->addRelationTo(m_MCParticles[ip_min]);
    } else {
      m_noMatchCount = m_noMatchCount + 1;
    }
  }
  // reset ROOT Error Level to default
  gErrorIgnoreLevel = default_gErrorIgnoreLevel;
}
void Chi2MCTrackMatcherModule::terminate()
{
  //if (m_noMatchCount>0){
  B2DEBUG(1, "For " << m_noMatchCount << "/" << m_trackCount << " tracks no relation was found.");
  //}
  //if (m_noMatchingCandidateCount>0){
  B2DEBUG(1, "For " << m_noMatchingCandidateCount << "/" << m_trackCount << " tracks got no matching candidate");
  //}
  //if (m_notInvertableCount>0){
  B2DEBUG(1, "" << m_notInvertableCount << "/" << m_covarianceMatrixCount << " covariance matrices where not invertable.");
  //}
}

