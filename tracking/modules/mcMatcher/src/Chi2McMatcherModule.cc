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

// datastore types
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/gearbox/Const.h>

#include <iostream>

// ROOT
#include <TVectorD.h>
#include <TMatrixD.h>

using namespace Belle2;

REG_MODULE(Chi2McMatcher)

Chi2McMatcherModule::Chi2McMatcherModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(Monte Carlo matcher using the helix parameters for matching by chi2-method)DOC");

  // set module parameters
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
  //B2DEBUG(100,"=======================EVENT BEGIN==================");
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

  // debug variables
  int det_not_valid = 0;
  int det0_count = 0;
  int match_count = 0;
  int minus_count = 0;
  int count = 0;

  // compare all tracks with all mcParticles in event
  //
  for (int it = 0 ; it < nTracks; ++it) {
    auto track = Tracks[it];
    // test for existing relations
    if (track->getRelated<MCParticle>()) {
      B2DEBUG(100, "relations = " << track->getRelated<MCParticle>());
      continue;
    }
    // initialize minimal chi2 variables
    int ip_min = -1;
    double chi2Min = std::numeric_limits<double>::infinity();

    for (int ip = 0; ip < nMCParticles; ++ip) {
      auto mcParticle = MCParticles[ip];
      // Check if current particle is a charged stable particle
      const Const::ParticleType mcParticleType(std::abs(mcParticle->getPDG()));
      if (not Const::chargedStableSet.contains(mcParticleType)) {
        continue;
      }

      // get trackfitresult of current track with clossest mass to current mcparticle Type
      auto trackFitResult = track->getTrackFitResultWithClosestMass(mcParticleType);
      TMatrixD Covariance5 = trackFitResult->getCovariance5();

      // check if matrix is invertable
      double det = Covariance5.Determinant();
      if (det == 0.0) {
        det0_count += 1;
        //Covariance5.Print();
        continue;
      }

      // generate helix for current mc particle
      double charge_sign = 1;
      if (mcParticle->getCharge() < 0) { charge_sign = -1;}
      auto b_field = BFieldManager::getField(mcParticle->getVertex()).Z() / Belle2::Unit::T;
      auto mcParticleHelix = Helix(mcParticle->getVertex(), mcParticle->getMomentum(), charge_sign, b_field);

      TMatrixD delta(5, 1);
      delta[0][0] = trackFitResult->getD0()        - mcParticleHelix.getD0();
      delta[1][0] = trackFitResult->getPhi0()      - mcParticleHelix.getPhi0();
      delta[2][0] = trackFitResult->getOmega()     - mcParticleHelix.getOmega();
      delta[3][0] = trackFitResult->getZ0()        - mcParticleHelix.getZ0();
      delta[4][0] = trackFitResult->getTanLambda() - mcParticleHelix.getTanLambda();

      TMatrixD covariance5_inv(TMatrixD::kInverted, Covariance5);

      TMatrixD deltaT = delta;
      deltaT.T();

      double chi2Cur = ((deltaT) * (covariance5_inv * delta))[0][0];
      B2DEBUG(110, "chi2_cur = " << chi2Cur);

      if (chi2Min > chi2Cur) {
        chi2Min = chi2Cur;
        ip_min = ip;
      }
      ++count;
    }


    B2DEBUG(100, "chi2_min = " << chi2Min);
    if (det0_count > 0) {
      B2DEBUG(100, "det0_count = " << det0_count);
    }
    if (det_not_valid > 0) {
      B2DEBUG(100, "det_not_valid = " << det_not_valid);
    }

    if (ip_min == -1) {
      ++minus_count;
      continue;
    }
    // initialize Cut Off
    double cutOff = 0;

    // fill cut off with value
    if (m_param_CutOffType == std::string("general")) {
      cutOff = m_param_GeneralCutOff;
    } else if (m_param_CutOffType == std::string("individual")) {
      int mcMinPDG = abs(MCParticles[ip_min]->getPDG());

      if (mcMinPDG == Belle2::Const::electron.getPDGCode()) {
        cutOff = m_param_IndividualCutOffs[0];
      } else if (mcMinPDG == Const::muon.getPDGCode()) {

        cutOff = m_param_IndividualCutOffs[1];
      } else if (mcMinPDG == Const::pion.getPDGCode()) {
        cutOff = m_param_IndividualCutOffs[2];
      } else if (mcMinPDG == Const::proton.getPDGCode()) {
        cutOff = m_param_IndividualCutOffs[3];
      } else if (mcMinPDG == Const::kaon.getPDGCode()) {
        cutOff = m_param_IndividualCutOffs[4];
      } else if (mcMinPDG == Const::deuteron.getPDGCode()) {
        cutOff = m_param_IndividualCutOffs[5];
      } else {
        B2DEBUG(1, "pdg_nochargedstalbe = " << mcMinPDG);
        continue;
        cutOff = 128024;
      }
    }
    B2DEBUG(100, "cutoff = " << cutOff);
    if (chi2Min < cutOff) {
      Tracks[it]->addRelationTo(MCParticles[ip_min]);
      ++match_count;
    }

  }
  B2DEBUG(100, "matchcount = " << match_count);
  B2DEBUG(100, "Relation ratio: from " << count << " pairs in total " << match_count << " where matched");
}



