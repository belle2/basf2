/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/modules/CosmicsAlignmentValidation/CosmicsAlignmentValidation.h>

#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/MCParticle.h>

#include <root/TFile.h>
#include <root/TTree.h>

#include <boost/foreach.hpp>

//avoid having to wrap everything in the namespace explicitly
//only permissible in .cc files!
using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
//Note that the 'Module' part of the class name is missing, this is also the way it
//will be called in the module list.

REG_MODULE(CosmicsAlignmentValidation)

CosmicsAlignmentValidationModule::CosmicsAlignmentValidationModule() :
  Module(),
  file(NULL),
  tree(NULL)
{
  setDescription("Alignment Validation with Cosmics");

  addParam("gfTrackColName", m_gfTrackColName,
           "Name of genfit::Track collection.", std::string(""));
  addParam("outputFileName", m_outputFileName, "Name of the output file.",
           std::string("cosmics.root"));
}

CosmicsAlignmentValidationModule::~CosmicsAlignmentValidationModule()
{
}

void CosmicsAlignmentValidationModule::initialize()
{
  B2INFO(
    "[CosmicsAlignmentValidation Module]: Starting initialization of CosmicsAlignmentValidation Module. Give me Cosmics!");
  m_GenfitTracks.isRequired(m_gfTrackColName);
  m_MCParticles.isRequired();
  file = new TFile(m_outputFileName.c_str(), "RECREATE");
  tree = new TTree("cosmics", "cosmics");
  tree->Branch("p1", &t_p1);
  tree->Branch("p2", &t_p2);
  tree->Branch("p1MC", &t_p1MC);
  tree->Branch("p2MC", &t_p2MC);
  tree->Branch("pt1", &t_pt1);
  tree->Branch("dz", &t_dz);
  tree->Branch("dR", &t_dR);

  tree->Branch("x1", &t_x1);
  tree->Branch("x2", &t_x2);
  tree->Branch("y1", &t_y1);
  tree->Branch("y2", &t_y2);
  tree->Branch("z1", &t_z1);
  tree->Branch("z2", &t_z2);

  tree->Branch("px1", &t_px1);
  tree->Branch("px2", &t_px2);
  tree->Branch("py1", &t_py1);
  tree->Branch("py2", &t_py2);
  tree->Branch("pz1", &t_pz1);
  tree->Branch("pz2", &t_pz2);

  tree->Branch("D01", &t_D01);
  tree->Branch("Phi1", &t_Phi1);
  tree->Branch("Omega1", &t_Omega1);
  tree->Branch("Z01", &t_Z01);
  tree->Branch("cotTheta1", &t_cotTheta1);

  tree->Branch("D02", &t_D02);
  tree->Branch("Phi2", &t_Phi2);
  tree->Branch("Omega2", &t_Omega2);
  tree->Branch("Z02", &t_Z02);
  tree->Branch("cotTheta2", &t_cotTheta2);

}

void CosmicsAlignmentValidationModule::beginRun()
{
}

void CosmicsAlignmentValidationModule::event()
{
  B2DEBUG(99, "[CosmicsAlignmentValidationModule] begin event");

  //genfit stuff
  //StoreArray<genfit::Track> fittedTracks(""); // the results of the track fit
  const int nFittedTracks = m_GenfitTracks.getEntries();

  //RelationArray gfTracksToMCPart(fittedTracks, mcParticles);

  t_p1 = t_p2 = t_pt1 = t_pt2 = t_p1MC = t_p2MC = -999;
  t_x1 = t_x2 = t_y1 = t_y2 = t_z1 = t_z2 = -999;
  t_px1 = t_px2 = t_py1 = t_py2 = t_pz1 = t_pz2 = -999;
  t_D01 = t_Phi1 = t_Omega1 = t_Z01 = t_cotTheta1 - 999;
  t_D02 = t_Phi2 = t_Omega2 = t_Z02 = t_cotTheta2 - 999;

  if (nFittedTracks != 2) {
    B2INFO(
      "[CosmicsAlignmentValidationModule] no two tracks reconstructed, but "
      << nFittedTracks);
    return;
  }

  else {

    genfit::Track* tr1 = m_GenfitTracks[0];
    genfit::Track* tr2 = m_GenfitTracks[1];

    const TrackFitResult* fitResult1 = findRelatedTrackFitResult(tr1);
    const TrackFitResult* fitResult2 = findRelatedTrackFitResult(tr2);

    if (fitResult1 == fitResult2)
      B2WARNING(
        "[CosmicsAlignmentValidationModule] Fit Results are from the same track!");

    if (fitResult1 != NULL) { // valid TrackFitResult found
      t_p1 = fitResult1->getMomentum().Mag();
      t_pt1 = fitResult1->getMomentum().Pt();
      t_x1 = fitResult1->getPosition().X();
      t_y1 = fitResult1->getPosition().Y();
      t_z1 = fitResult1->getPosition().Z();
      t_px1 = fitResult1->getMomentum().X();
      t_py1 = fitResult1->getMomentum().Y();
      t_pz1 = fitResult1->getMomentum().Z();

      t_D01 = fitResult1->getD0();
      t_Phi1 = fitResult1->getPhi();
      t_Omega1 = fitResult1->getOmega();
      t_Z01 = fitResult1->getZ0();
      t_cotTheta1 = fitResult1->getCotTheta();
    }
    if (fitResult2 != NULL) { // valid TrackFitResult found
      t_p2 = fitResult2->getMomentum().Mag();
      t_pt2 = fitResult2->getMomentum().Pt();
      t_x2 = fitResult2->getPosition().X();
      t_y2 = fitResult2->getPosition().Y();
      t_z2 = fitResult2->getPosition().Z();
      t_px2 = fitResult2->getMomentum().X();
      t_py2 = fitResult2->getMomentum().Y();
      t_pz2 = fitResult2->getMomentum().Z();

      t_D02 = fitResult2->getD0();
      t_Phi2 = fitResult2->getPhi();
      t_Omega2 = fitResult2->getOmega();
      t_Z02 = fitResult2->getZ0();
      t_cotTheta2 = fitResult2->getCotTheta();
    }

    t_p1MC = m_MCParticles[0]->getMomentum().Mag();

    if (fitResult2 != NULL && fitResult1 != NULL) {
      t_dz = fitResult1->getPosition().Z()
             - fitResult2->getPosition().Z();
      t_dR = sqrt(
               fitResult1->getPosition().X()
               * fitResult1->getPosition().X()
               + fitResult1->getPosition().Y()
               * fitResult1->getPosition().Y())
             - sqrt(
               fitResult2->getPosition().X()
               * fitResult2->getPosition().X()
               + fitResult2->getPosition().Y()
               * fitResult2->getPosition().Y());

    }
    tree->Fill();
  }

}

void CosmicsAlignmentValidationModule::endRun()
{
}

void CosmicsAlignmentValidationModule::terminate()
{
  B2INFO("[CosmicsAlignmentValidationModule] Saving tree.");
  B2INFO(
    "[CosmicsAlignmentValidationModule] Tree has " << tree->GetEntries()
    << " entries");
  file->cd();
  tree->Write("cosmics");
  file->Close();
}

const TrackFitResult* CosmicsAlignmentValidationModule::findRelatedTrackFitResult(
  const genfit::Track* gfTrack)
{
  // search for a related TrackFitResult
  RelationIndex < genfit::Track, TrackFitResult > relGfTracksToTrackFitResults;

  typedef RelationIndex<genfit::Track, TrackFitResult>::Element relElement_t;
  std::vector<const TrackFitResult*> fitResults;

  BOOST_FOREACH(const relElement_t& relGfTrackToTrackFitResult, relGfTracksToTrackFitResults.getElementsFrom(gfTrack)) {
    B2DEBUG(99, "----> Related TrackFitResult found!!!");
    fitResults.push_back(relGfTrackToTrackFitResult.to);
  }

  int numberTrackFitResults = fitResults.size();

  if (numberTrackFitResults == 1) {
    return fitResults[0];
  }
  if (numberTrackFitResults == 0) {
    return NULL;
  }
  if (numberTrackFitResults > 1) {
    B2DEBUG(99,
            "[CosmicsAlignmentValidationModule] genfit::Track has "
            << numberTrackFitResults
            << " related TrackFitResults. No TrackFitResult is returned.");
  }

  return NULL;
}
