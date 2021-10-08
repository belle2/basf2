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
#include <Eigen/Dense>
#include <iostream>

// ROOT
#include <TVectorD.h>
#include <TMatrixD.h>


using namespace Belle2;

REG_MODULE(Chi2McMatcher)



// Constructor
Chi2McMatcherModule::Chi2McMatcherModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(Monte Carlo matcher using the helix parameters for matching by chi2-method)DOC");

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
           param_linalg,
           "Parameter to switch between ROOT and Eigen, to invert the covariance matrix",
           std::string("ROOT"));
  /*
  bool save_helix_param_default = 0;
  addParam("save_helix_param",
           m_param_savehelix,
           "Debug variable to save helix parameters to csv",
     save_helix_param_default);
  addParam("filename",
     m_param_filename,
     "Debug variable to give filename for saveing helix parameters",
     std::string("savedhelixparameter"));
  */
}

void Chi2McMatcherModule::initialize()
{
// Check if there are MC Particles
  StoreArray<MCParticle> storeMCParticles;
  StoreArray<Track> storeTrack;
  storeMCParticles.isRequired();
  storeTrack.isRequired();
  storeTrack.registerRelationTo(storeMCParticles);

  /*
  // Variables to save data
  fileHeader = {"D0_track", "Phi0_track", "Omega_track", "Z0_track", "TanLambda_track",
                "D0_mc_chi2", "Phi0_mc_chi2", "Omega_mc_chi2", "Z0_mc_chi2", "TanLambda_mc_chi2",
                "chi2_value",
                "hitRelation", "chi2Relation", "bothRelation", "bothRelationAndSameMC", "notbothRelation", "noRelation" // (bool)
               };
  */
}

void Chi2McMatcherModule::event()
{

  // suppress the ROOT "matrix is singular" error message
  auto default_gErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = 5000;

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

  // compare all tracks with all mcParticles in event
  for (int it = 0 ; it < nTracks; ++it) {
    auto track = Tracks[it];

    /*
    // Varibles to save matching information to csv file
    int hitMatch = 0;
    int chi2Match = 0;
    double track_helix[5];// D0,Phi0,Omega,Z0,TanLambda
    double mc_helix[5];
    double classifiers[6] = {0, 0, 0, 0, 0, 0}; //"hitRelation","chi2Relation","bothRelation","bothRelationAndSameMC","notbothRelation","noRelation"
    auto hitMCParticle = track->getRelated<MCParticle>();
    */

    // test for existing relations
    if (track->getRelated<MCParticle>()) {
      B2DEBUG(100, "Relation already set continue with next track");
      continue;

      /*
      // Variables to save matching information to csv file
      ++hitRelationCounter;
      classifiers[0] = 1; //hitrelation
      hitMatch = 1;
      */
    }
    // initialize minimal chi2 variables
    int ip_min = -1;
    double chi2Min = std::numeric_limits<double>::infinity();

    // loop over MCParticles and calculate Chi2 for each track mcparticle pair, to fine minimal chi2
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
        B2DEBUG(100, "Covariance5 matrix is not invertable. Continue!");
        continue;
      }

      // generate helix for current mc particle
      double charge_sign = 1;
      if (mcParticle->getCharge() < 0) { charge_sign = -1;}
      auto b_field = BFieldManager::getField(mcParticle->getVertex()).Z() / Belle2::Unit::T;
      auto mcParticleHelix = Helix(mcParticle->getVertex(), mcParticle->getMomentum(), charge_sign, b_field);

      // initialize variable for current chi2
      double chi2Cur = std::numeric_limits<double>::infinity();

      // Check which linear algebra system should be used and calculate chi2Cur
      if (param_linalg == "Eigen") {
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
      } else if (param_linalg == "ROOT") {
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

        /*
        // Variables to save matching information to file
        if (m_param_savehelix){
          track_helix[0] = trackFitResult->getD0();
          track_helix[1] = trackFitResult->getPhi0();
          track_helix[2] = trackFitResult->getOmega();
          track_helix[3] = trackFitResult->getZ0();
          track_helix[4] = trackFitResult->getTanLambda();
          mc_helix[0] = mcParticleHelix.getD0();
          mc_helix[1] = mcParticleHelix.getPhi0();
          mc_helix[2] = mcParticleHelix.getOmega();
          mc_helix[3] = mcParticleHelix.getZ0();
          mc_helix[4] = mcParticleHelix.getTanLambda();
        }
        */
      }
    }
    // check if any matching candidate was found
    if (ip_min == -1) {
      continue;
    }
    // initialize Cut Off
    double cutOff = 0;
    // fill cut off with value
    // find PDG for mcParticle with minimal chi2, because this determines individual cutOff
    int mcMinPDG = abs(MCParticles[ip_min]->getPDG());

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
      Tracks[it]->addRelationTo(MCParticles[ip_min]);
      /*
      // Variables to save matching information to file
      ++chi2RelationCounter;
      chi2Match = 1;
      classifiers[1] = 1;
      */
    }
    /*
    // Variables to save matching information to file
    if ((chi2Match == 1) and (hitMatch == 1)) {
      ++bothRelationCounter;
      classifiers[2] = 1;
      if (hitMCParticle == MCParticles[ip_min]) {
        ++bothRelationAndSameMCCounter;
        classifiers[3] = 1;
      }
    } else if ((chi2Match == 1) or (hitMatch == 1)) {
      ++notBothRelationCounter;
      classifiers[4] = 1;
    } else {
      ++noRelationCounter;
      classifiers[5] = 1;
    }


    if (m_param_savehelix) {

      for (int i = 0; i < 5; i++) {
        fileContent.push_back(track_helix[i]);
      }
      for (int i = 0; i < 5; i++) {
        fileContent.push_back(mc_helix[i]);
      }
      fileContent.push_back(chi2Min);
      for (int i = 0; i < 5; i++) {
        fileContent.push_back(classifiers[i]);
      }
      fileContent.push_back(bothRelationCounter/hitRelationCounter);
    }
    */
    /*
    B2DEBUG(1,"totalCount = "<<totalCount);
    B2DEBUG(1,"hitRelationCounter = "<<hitRelationCounter);
    B2DEBUG(1,"chi2RelationCounter = "<<chi2RelationCounter);
    B2DEBUG(1,"bothRelationCounter = "<<bothRelationCounter);
    B2DEBUG(1,"bothRelationAndSameMCCounter = "<<bothRelationAndSameMCCounter);
    B2DEBUG(1,"notbothRelationCounter = "<<notBothRelationCounter);
    B2DEBUG(1,"noRelationCounter = "<<noRelationCounter);
    */
  }
  // reset ROOT Error Level to default
  gErrorIgnoreLevel = default_gErrorIgnoreLevel;
}


/*
// Variables to save matching information to file
void Chi2McMatcherModule::terminate()
{
  B2DEBUG(1,m_param_CutOffs);
  if (m_param_savehelix) {
  std::ofstream outfile;
  outfile.open(m_param_filename);
  if (!outfile) {  // file couldn't be opened
    std::cerr << "Error: file could not be opened" << std::endl;
    exit(1);
  }
  int fileHeaderSize = fileHeader.size();
  for (int i = 0; i < fileHeaderSize; i++) {
    outfile << fileHeader[i];
    if (i != (fileHeaderSize - 1)) {
      outfile << ",";
    }
  }
  outfile << std::endl;
  int count = 0;
  int fileContentSize = fileContent.size();
  for (int index = 0; index < (fileContentSize); index++) {
    outfile << fileContent[index];
    count++;
    if (count == 17) {
      count = 0;
      outfile << std::endl;
      continue;
    }
    outfile << ",";
  }
  outfile.close();
  }
}
*/
