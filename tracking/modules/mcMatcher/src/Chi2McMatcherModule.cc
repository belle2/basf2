/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2021 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Florian Schweiger                                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/modules/mcMatcher/Chi2McMatcherModule.h>

/* --------------- WARNING ---------------------------------------------- *
If you have more complex parameter types in your class then simple int,
double or std::vector of those you might need to uncomment the following
include directive to avoid an undefined reference on compilation.
* ---------------------------------------------------------------------- */
// #include <framework/core/ModuleParam.templateDetails.h>

// datastore types

#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <iostream>
#include <typeinfo>
#include <Eigen/Dense>


using namespace Belle2;

REG_MODULE(Chi2McMatcher)

Chi2McMatcherModule::Chi2McMatcherModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(Monte Carlo matcher using the helix parameters for matching by chi2-method)DOC");
  addParam("CutOffType",
           m_param_CutOffType,
           "Defines the Cut Off behavior",
           std::string("individual"));
  double default_GeneralCutOff = 128024.0;
  addParam("GeneralCutOff",
           m_param_GeneralCutOff,
           "Defines the Cut Off value for general CutOffType Default->electron 99 percent boarder",
           default_GeneralCutOff);

  std::vector<double> defaultIndividualCutOffs(6);
  defaultIndividualCutOffs[0] = 128024;
  defaultIndividualCutOffs[1] = 95;
  defaultIndividualCutOffs[2] = 173;
  defaultIndividualCutOffs[3] = 424;
  defaultIndividualCutOffs[4] = 90;
  defaultIndividualCutOffs[5] = 128024;//research what coresponing 99% border
  addParam("IndividualCutOffs",
           m_param_IndividualCutOffs,
           "Defines the Cut Off values for each charged particle. pdg order [11,13,211,2212,321,1000010020]",
           defaultIndividualCutOffs);
}

//Chi2McMatcherModule::~Chi2McMatcherModule()
//{
//}

void Chi2McMatcherModule::initialize()
{
// Check if there are MC Particles
  StoreArray<MCParticle> storeMCParticles;
  StoreArray<Track> storeTrack;
  storeMCParticles.isRequired();
  storeTrack.isRequired();
  storeTrack.registerRelationTo(storeMCParticles);
}

void Chi2McMatcherModule::event()
{
  StoreArray<MCParticle> MCParticles;
  StoreArray<Track> Tracks;

  // get StoreArray length
  int nTracks = Tracks.getEntries();
  int nMCParticles = MCParticles.getEntries();

  // check if there are Tracks and MCParticles to match to
  if (not nMCParticles or not nTracks) {
    // Cannot perfom matching
    return;
  }
  int count = 0;
  int match_count = 0;

  double chi2_min;
  int it_min;
  int ip_min;
//  for (auto track:Tracks)
//  {
//    for (auto particle:MCParticles)
//    {

  //B2DEBUG(1,"partilce = "<<particle->getPDG());
  //B2DEBUG(1,"track = "<<track);
//      continue;
//    }
//  }
  for (int it = 0; it < nTracks; it++) {
    for (int ip = 0; ip < nMCParticles; ip++) {
      auto mcparticle = MCParticles[ip];
      auto track = Tracks[it];
      const Const::ParticleType mc_particleType(std::abs(mcparticle->getPDG()));
      if (not Const::chargedStableSet.contains(mc_particleType)) {
        continue;
      }

      auto trackfit_wcm = track->getTrackFitResultWithClosestMass(mc_particleType);
      auto Covariance5 = trackfit_wcm->getCovariance5();

      Eigen::VectorXd m_TrackHelixParameters(5);
      m_TrackHelixParameters[0] = trackfit_wcm->getD0();    // D0 Helix Parameter
      m_TrackHelixParameters[1] = trackfit_wcm->getPhi0();  // Phi0 Helix Parameter
      m_TrackHelixParameters[2] = trackfit_wcm->getOmega(); // Omega Helix Parameter
      m_TrackHelixParameters[3] = trackfit_wcm->getZ0();    // Z0 Helix Parameter
      m_TrackHelixParameters[4] = trackfit_wcm->getTanLambda();// TanLambda Helix Parameter

      double charge_sign = 1;
      if (mcparticle->getCharge() < 0) { charge_sign = -1;}

      auto b_field = BFieldManager::getField(mcparticle->getVertex()).Z() / Belle2::Unit::T;
      auto m_McHelix = Helix(mcparticle->getVertex(), mcparticle->getMomentum(), charge_sign, b_field);

      Eigen::VectorXd m_McHelixParameters(5);
      m_McHelixParameters[0] = m_McHelix.getD0();
      m_McHelixParameters[1] = m_McHelix.getPhi0();
      m_McHelixParameters[2] = m_McHelix.getOmega();
      m_McHelixParameters[3] = m_McHelix.getZ0();
      m_McHelixParameters[4] = m_McHelix.getTanLambda();

      // Check if Matrix is invertable
      // alternative: add a very small number to every matrix entry to make it invertable
      double det = Covariance5.Determinant();
      double det0 = 0.0;
      if (det == det0) {
        continue;
      }

      // Invert the matrix
      auto covariance5_inv = Covariance5.InvertFast();
      //B2DEBUG(1,"cov_inv = "<<covariance5_inv);
      Eigen::MatrixXd Covariance5_eigen_inv(5, 5);
      for (int i = 0; i == 4; i++) {
        for (int j = 0; j == 4; j++) {
          Covariance5_eigen_inv(i, j) = covariance5_inv[i][j];
        }
      }

      Eigen::VectorXd delta(5);
      delta = m_TrackHelixParameters - m_McHelixParameters;

      double Chi2_cur = delta.transpose() * Covariance5_eigen_inv * delta;

      if (count == 0) {
        chi2_min = Chi2_cur;
        ++count;
        it_min = it;
        ip_min = ip;
        continue;
      }
      if (Chi2_cur < chi2_min) {
        chi2_min = Chi2_cur;
        it_min = it;
        ip_min = ip;
      }
      ++count;
    }
  }
  //int index_min;
  B2DEBUG(1, "chi2_min = " << chi2_min);

  // initialize Cut Off
  double CutOff = 0;
  if (m_param_CutOffType == std::string("general")) {
    CutOff = m_param_GeneralCutOff;
  } else if (m_param_CutOffType == std::string("individual")) {
    //int pdg = std::abs(MCParticles[CompareNumber(index_min,1)]->getPDG());
    int pdg = std::abs(MCParticles[ip_min]->getPDG());
    if (pdg == 11) {
      CutOff = m_param_IndividualCutOffs[0];
    } else if (pdg == 13) {
      CutOff = m_param_IndividualCutOffs[1];
    } else if (pdg == 211) {
      CutOff = m_param_IndividualCutOffs[2];
    } else if (pdg == 2212) {
      CutOff = m_param_IndividualCutOffs[3];
    } else if (pdg == 321) {
      CutOff = m_param_IndividualCutOffs[4];
    } else if (pdg == 1000010020) {
      CutOff = m_param_IndividualCutOffs[5];
    }
  }
  if (chi2_min < CutOff) {
    Tracks[it_min]->addRelationTo(MCParticles[ip_min]);
    ++match_count;
  }
  //B2DEBUG(1,"it_min= "<<it_min<<", ip_min= "<<ip_min);
  //B2DEBUG(1,"count = "<<count);
  //B2DEBUG(1,"match_count = "<<match_count);
  B2DEBUG(1, "Relation ratio: from " << count << " pairs in total " << match_count << " where matched");
}



