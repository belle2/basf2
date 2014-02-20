/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackingEvaluation/TrackingEvaluationModule.h>

#include <framework/dataobjects/EventMetaData.h>

//datastore types
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreEntry.h>

#include <framework/gearbox/Const.h>

#include <mdst/dataobjects/MCParticle.h>

//#include <genfit/TrackCandHit.h>

#include <cmath>

//#include <Eigen/Dense>

using namespace std;
using namespace Belle2;


// Helper functions in anonymous namespace
namespace {

  // This method should eventually become part of the DataStore
  // In addtion to the method defined there it also yields the weight of the relation
  // If no relation is found the weight is not changed
  template <class TO> static TO* getRelatedFromObj(const TObject* fromObject, double& weight, const string& toName = "")
  {

    if (!fromObject) return nullptr;
    DataStore::StoreEntry* storeEntry = nullptr;
    int index = -1;
    RelationEntry toEntry = DataStore::Instance().getRelationWith(DataStore::c_ToSide, fromObject, storeEntry, index, TO::Class(), toName);
    TO* toObject = static_cast<TO*>(toEntry.object);

    if (toObject) weight = toEntry.weight;
    return toObject;
  }


  // Looks for the object on the datastore and retrieves its index if found
  // returns a negative value if not found
  template <class T> int getArrayIndex(const T* object, const string& name = "")
  {
    int index = -1;
    StoreArray<T> storeArray(name);
    TClonesArray* clonesArray = storeArray.getPtr();
    index = clonesArray->IndexOf(object);
    return index;
  }

}


// Implementation of the module
REG_MODULE(TrackingEvaluation);

TrackingEvaluationModule::TrackingEvaluationModule() : Module()
{

  setDescription("Generates some trees about the reconstruction performance using the relations from MCTrackMatcherModule");

  // Parameter definition
  // Inputs
  addParam("PRGFTrackCandsColName",
           m_param_prGFTrackCandsColName,
           "Name of the collection containing the tracks as generate a patter recognition algorithm to be evaluated ",
           string(""));

  addParam("MCGFTrackCandsColName",
           m_param_mcGFTrackCandsColName,
           "Name of the collection containing the reference tracks as generate by a Monte-Carlo-Tracker (e.g. MCTrackFinder)",
           string("MCGFTrackCands"));

  addParam("OutputFileName",
           m_param_outputFileName,
           "Path to the output file for the generated ROOT Trees",
           string("MCTrackingEvaluationOutput.root"));

  // Set up the standard number degress of freedom a hit contributes by detector
  m_ndf_by_detId.clear();
  m_ndf_by_detId[Const::PXD] = 2;
  m_ndf_by_detId[Const::SVD] = 2;
  m_ndf_by_detId[Const::CDC] = 1;

}

TrackingEvaluationModule::~TrackingEvaluationModule()
{
}

void TrackingEvaluationModule::initialize()
{

  // Require both GFTrackCand arrays to be present in the DataStore
  StoreArray<genfit::TrackCand>::required(m_param_prGFTrackCandsColName);
  StoreArray<genfit::TrackCand>::required(m_param_mcGFTrackCandsColName);
  StoreArray<MCParticle>::required("");

  // Purity relation - for each PRTrack to will store the purest MCTrack
  // an is required would be more appropriate here - ask Christian Pulvermacher if fixed
  RelationArray::registerPersistent< genfit::TrackCand, genfit::TrackCand>(m_param_prGFTrackCandsColName, m_param_mcGFTrackCandsColName);

  // Efficiency relation - for each MCTrack to will store the most efficient PRTrack
  RelationArray::registerPersistent< genfit::TrackCand, genfit::TrackCand>(m_param_mcGFTrackCandsColName, m_param_prGFTrackCandsColName);

  // MC matching relation
  RelationArray::registerPersistent< genfit::TrackCand, MCParticle>(m_param_prGFTrackCandsColName, "");

  m_outputFile = new TFile(m_param_outputFileName.c_str(), "RECREATE");

  m_eventTree = new TTree("events", "Variables of the whole event");
  m_mcTrackTree = new TTree("mcTracks", "Variables of the Monte Carlo tracks");
  m_prTrackTree = new TTree("prTracks", "Variables of the pattern recogntion tracks");

  setupEventTree();
  setupMCTrackTree();
  setupPRTrackTree();
}

void TrackingEvaluationModule::beginRun()
{

}

void TrackingEvaluationModule::event()
{
  B2DEBUG(100, "########## TrackingEvaluationModule ############");

  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  m_eventNumber = eventMetaDataPtr->getEvent();

  assignEventVars();
  m_eventTree->Fill();
  B2DEBUG(100, "EventTree content about to be filled" << endl << (m_eventTree->Show(), ""));

  {
    StoreArray<genfit::TrackCand> mcGFTrackCands(m_param_mcGFTrackCandsColName);

    for (const genfit::TrackCand & mcTrackCand : mcGFTrackCands) {
      assignMCTrackVars(mcTrackCand);
      B2DEBUG(100, "MCTrackTree content about to be filled" << endl << (m_mcTrackTree->Show(), ""));
      m_mcTrackTree->Fill();
    }
  }

  {
    StoreArray<genfit::TrackCand> prGFTrackCands(m_param_prGFTrackCandsColName);

    for (const genfit::TrackCand & prTrackCand : prGFTrackCands) {
      assignPRTrackVars(prTrackCand);
      B2DEBUG(100, "PRTrackTree content about to be filled" << endl << (m_prTrackTree->Show(), ""));
      m_prTrackTree->Fill();
    }
  }

  B2DEBUG(100, "########## TrackingEvaluationModule ############");

} //end event()






void TrackingEvaluationModule::endRun()
{
}

void TrackingEvaluationModule::terminate()
{
  if (m_eventTree != NULL)  m_eventTree->Write();
  if (m_mcTrackTree != NULL)  m_mcTrackTree->Write();
  if (m_prTrackTree != NULL)  m_prTrackTree->Write();

  if (m_outputFile != NULL) m_outputFile->Close();
}


void TrackingEvaluationModule::assignEventVars()
{

  //Fetch store array
  StoreArray<genfit::TrackCand> prGFTrackCands(m_param_prGFTrackCandsColName);
  StoreArray<genfit::TrackCand> mcGFTrackCands(m_param_mcGFTrackCandsColName);
  StoreArray<MCParticle> mcParticles("");

  m_eventVars.nMCParticles = mcParticles.getEntries();
  m_eventVars.nMCTracks = mcGFTrackCands.getEntries();
  m_eventVars.nPRTracks = prGFTrackCands.getEntries();

  B2DEBUG(200, "Number patter recognition tracks is " << m_eventVars.nPRTracks);
  B2DEBUG(200, "Number Monte-Carlo tracks is " << m_eventVars.nMCTracks);

  m_eventVars.nMatched = 0;
  m_eventVars.nGhost = 0;
  m_eventVars.nBackground = 0;
  m_eventVars.nClone = 0;

  for (const genfit::TrackCand & prTrackCand : prGFTrackCands) {

    float purity = NAN;

    const genfit::TrackCand* mcTrackCand = getRelatedMCTrackCand(prTrackCand, purity);
    PRToMCMatchInfo matchInfo = getPRToMCMatchInfo(prTrackCand, mcTrackCand, purity);

    if (matchInfo == PRToMCMatchInfo::MATCHED) ++m_eventVars.nMatched;
    else if (matchInfo == PRToMCMatchInfo::CLONE) ++m_eventVars.nClone;
    else if (matchInfo == PRToMCMatchInfo::BACKGROUND) ++m_eventVars.nBackground;
    else if (matchInfo == PRToMCMatchInfo::GHOST) ++m_eventVars.nGhost;
    else B2WARNING("Invalid matching information from pattern recognition track to Monte Carlo tracks");

  }

  B2DEBUG(200, "Number matched pattern recognition tracks is " << m_eventVars.nMatched);
  B2DEBUG(200, "Number clone pattern recognition tracks is " << m_eventVars.nClone);
  B2DEBUG(200, "Number background pattern recognition tracks is " << m_eventVars.nBackground);
  B2DEBUG(200, "Number ghost pattern recognition tracks is " << m_eventVars.nGhost);

  Int_t nMatchedCheck = 0;

  m_eventVars.nMissing = 0;
  m_eventVars.nMerged = 0;

  for (const genfit::TrackCand & mcTrackCand : mcGFTrackCands) {

    float efficiency = NAN;

    const genfit::TrackCand* prTrackCand = getRelatedPRTrackCand(mcTrackCand, efficiency);
    MCToPRMatchInfo matchInfo = getMCToPRMatchInfo(prTrackCand, efficiency);

    if (matchInfo == MCToPRMatchInfo::MATCHED) ++nMatchedCheck;
    else if (matchInfo == MCToPRMatchInfo::MERGED) ++m_eventVars.nMerged;
    else if (matchInfo == MCToPRMatchInfo::MISSING) ++m_eventVars.nMissing;
    else B2WARNING("Invalid matching information from Monte Carlo track to pattern recognition tracks");

  }

  B2DEBUG(200, "Number matched Monte Carlo tracks is " << nMatchedCheck);
  B2DEBUG(200, "Number merged Monte Carlo tracks is " << m_eventVars.nMerged);
  B2DEBUG(200, "Number missing Monte Carlo tracks is " << m_eventVars.nMissing);

  if (nMatchedCheck != m_eventVars.nMatched) {
    B2WARNING("Number of matches from PR to MC disagrees with number of matches from MC to PR");
  }

}


void TrackingEvaluationModule::assignMCTrackVars(const genfit::TrackCand& mcTrackCand)
{

  m_mcTrackVars.mcTrackCandId = getArrayIndex(&mcTrackCand, m_param_mcGFTrackCandsColName);

  // Assign hit content
  m_mcTrackVars.nPXDHits = 0;
  m_mcTrackVars.nSVDHits = 0;
  m_mcTrackVars.nCDCHits = 0;
  m_mcTrackVars.ndf = 0;

  int detId = 0;
  int hitId = 0;

  const int nHits = mcTrackCand.getNHits();
  for (int iHit = 0; iHit < nHits; ++iHit) {
    mcTrackCand.getHit(iHit, detId, hitId);

    switch (detId) {
      case Const::PXD:  ++m_mcTrackVars.nPXDHits; break;
      case Const::SVD:  ++m_mcTrackVars.nSVDHits; break;
      case Const::CDC:  ++m_mcTrackVars.nCDCHits; break;
    }

    m_mcTrackVars.ndf += m_ndf_by_detId[detId];

  }


  // Assign matching information to pattern recognition
  m_mcTrackVars.relatedPRTrackCandId = -1;
  m_mcTrackVars.efficiency = NAN;
  m_mcTrackVars.purity = NAN;

  m_mcTrackVars.isMatched = false;
  m_mcTrackVars.isMerged = false;
  m_mcTrackVars.isMissing = false;

  // Assigns efficieny as side effect
  const genfit::TrackCand* prTrackCand = getRelatedPRTrackCand(mcTrackCand, m_mcTrackVars.efficiency);
  MCToPRMatchInfo matchInfo = getMCToPRMatchInfo(prTrackCand, m_mcTrackVars.efficiency);

  if (prTrackCand) {
    m_mcTrackVars.relatedPRTrackCandId = getArrayIndex(prTrackCand, m_param_prGFTrackCandsColName);
  }


  if (matchInfo == MCToPRMatchInfo::MATCHED) {
    if (prTrackCand) {
      // Assigns purity as side effect
      const genfit::TrackCand* mcTrackCand = getRelatedMCTrackCand(*prTrackCand, m_mcTrackVars.purity);
      if (!mcTrackCand) B2WARNING("Matched Monte Carlo track has related pattern recognition track, which is not related to this Monte Carlo track. (RelationArrays are not correctly built.)");
    } else {
      B2WARNING("Matched Monte Carlo track has no related pattern recognition track");
    }
    m_mcTrackVars.isMatched = true;
  } else if (matchInfo == MCToPRMatchInfo::MERGED) {
    m_mcTrackVars.purity = NAN;
    m_mcTrackVars.isMerged = true;
  } else if (matchInfo == MCToPRMatchInfo::MISSING) {
    m_mcTrackVars.purity = NAN;
    m_mcTrackVars.isMissing = true;
  } else B2WARNING("Invalid matching information from Monte Carlo track to pattern recognition tracks");


  // Assign physics information
  const MCParticle* mcParticle = DataStore::getRelatedFromObj<MCParticle>(&mcTrackCand);

  m_mcTrackVars.truePDGCode = -999;
  m_mcTrackVars.trueCharge = NAN;
  m_mcTrackVars.truePx = NAN;
  m_mcTrackVars.truePy = NAN;
  m_mcTrackVars.truePz = NAN;

  if (mcParticle) {

    m_mcTrackVars.truePDGCode = mcParticle->getPDG();
    m_mcTrackVars.trueCharge = mcParticle->getCharge();

    const TVector3 trueP = mcParticle->getMomentum();

    m_mcTrackVars.truePx = trueP.X();
    m_mcTrackVars.truePy = trueP.Y();
    m_mcTrackVars.truePz = trueP.Z();

  } else {
    B2WARNING("Monte Carlo track has no underlying MCParticle");
  }


}




void TrackingEvaluationModule::assignPRTrackVars(const genfit::TrackCand& prTrackCand)
{

  m_prTrackVars.prTrackCandId = getArrayIndex(&prTrackCand, m_param_prGFTrackCandsColName);

  // Assign hit content
  m_prTrackVars.nPXDHits = 0;
  m_prTrackVars.nSVDHits = 0;
  m_prTrackVars.nCDCHits = 0;
  m_prTrackVars.ndf = 0;

  int detId = 0;
  int hitId = 0;

  const int nHits = prTrackCand.getNHits();
  for (int iHit = 0; iHit < nHits; ++iHit) {
    prTrackCand.getHit(iHit, detId, hitId);

    switch (detId) {
      case Const::PXD:  ++m_prTrackVars.nPXDHits; break;
      case Const::SVD:  ++m_prTrackVars.nSVDHits; break;
      case Const::CDC:  ++m_prTrackVars.nCDCHits; break;
    }

    m_prTrackVars.ndf += m_ndf_by_detId[detId];

  }


  // Assign matching information to pattern recognition
  m_prTrackVars.relatedMCTrackCandId = -1;
  m_prTrackVars.efficiency = NAN;
  m_prTrackVars.purity = NAN;

  m_prTrackVars.isMatched = false;
  m_prTrackVars.isClone = false;
  m_prTrackVars.isBackground = false;
  m_prTrackVars.isGhost = false;

  // Assigns efficieny as side effect
  const genfit::TrackCand* mcTrackCand = getRelatedMCTrackCand(prTrackCand, m_prTrackVars.efficiency);
  PRToMCMatchInfo matchInfo = getPRToMCMatchInfo(prTrackCand, mcTrackCand, m_prTrackVars.efficiency);

  if (mcTrackCand) {
    m_prTrackVars.relatedMCTrackCandId = getArrayIndex(mcTrackCand, m_param_mcGFTrackCandsColName);
  }


  if (matchInfo == PRToMCMatchInfo::MATCHED) {
    if (mcTrackCand) {
      // Assigns purity as side effect
      const genfit::TrackCand* prTrackCand = getRelatedPRTrackCand(*mcTrackCand, m_prTrackVars.purity);
      if (!prTrackCand) B2WARNING("Matched pattern recognition track has related Monte Carlo track, which is not related to this pattern recognition track. (RelationArrays are not correctly built.)");
    } else {
      B2WARNING("Matched pattern recognition track has no related Monte Carlo track");
    }
    m_prTrackVars.isMatched = true;
  } else if (matchInfo == PRToMCMatchInfo::CLONE) {
    m_prTrackVars.purity = NAN;
    m_prTrackVars.isClone = true;
  } else if (matchInfo == PRToMCMatchInfo::BACKGROUND) {
    m_prTrackVars.purity = NAN;
    m_prTrackVars.isBackground = true;
  } else if (matchInfo == PRToMCMatchInfo::GHOST) {
    m_prTrackVars.purity = NAN;
    m_prTrackVars.isGhost = true;
  } else B2WARNING("Invalid matching information from Monte Carlo track to pattern recognition tracks");


  // Assign physics information
  m_prTrackVars.truePDGCode = -999;
  m_prTrackVars.trueCharge = NAN;
  m_prTrackVars.truePx = NAN;
  m_prTrackVars.truePy = NAN;
  m_prTrackVars.truePz = NAN;

  if (mcTrackCand) {
    const MCParticle* mcParticle = DataStore::getRelatedFromObj<MCParticle>(mcTrackCand);
    if (mcParticle) {

      m_prTrackVars.truePDGCode = mcParticle->getPDG();
      m_prTrackVars.trueCharge = mcParticle->getCharge();

      const TVector3 trueP = mcParticle->getMomentum();

      m_prTrackVars.truePx = trueP.X();
      m_prTrackVars.truePy = trueP.Y();
      m_prTrackVars.truePz = trueP.Z();

    } else {
      B2WARNING("Monte Carlo track has no underlying MCParticle");
    }
  }

}






const genfit::TrackCand*
TrackingEvaluationModule::getRelatedMCTrackCand(const genfit::TrackCand& prTrackCand, float& purity)
{

  double double_purity = 0; //help variable because DataStore expects double
  genfit::TrackCand* mcTrackCand = getRelatedFromObj<genfit::TrackCand >(&prTrackCand, double_purity, m_param_mcGFTrackCandsColName);
  if (mcTrackCand) purity = double_purity;
  return mcTrackCand;

}

const genfit::TrackCand*
TrackingEvaluationModule::getRelatedPRTrackCand(const genfit::TrackCand& mcTrackCand, float& efficiency)
{

  double double_efficiency = 0; //help variable because DataStore expects double
  genfit::TrackCand* prTrackCand = getRelatedFromObj<genfit::TrackCand >(&mcTrackCand, double_efficiency, m_param_prGFTrackCandsColName);
  if (prTrackCand) efficiency = double_efficiency;
  return prTrackCand;
}

//Helper functions to figure out the match of the PRParticle
TrackingEvaluationModule::MCToPRMatchInfo
TrackingEvaluationModule::getMCToPRMatchInfo(const genfit::TrackCand* prTrackCand, const float& efficiency)
{

  if (!prTrackCand) return MCToPRMatchInfo::MISSING;
  if (efficiency < 0) return MCToPRMatchInfo::MERGED;
  else if (efficiency > 0) return MCToPRMatchInfo::MATCHED;

  return MCToPRMatchInfo::INVALID;

}

TrackingEvaluationModule::PRToMCMatchInfo
TrackingEvaluationModule::getPRToMCMatchInfo(const genfit::TrackCand& prTrackCand, const genfit::TrackCand* mcTrackCand, const float& purity)
{

  if (!mcTrackCand) {

    // The patter recognition track has no associated Monte Carlo track.
    // Figure out of it is a clone or a match by the McTrackId property assigned to the track cand.
    // That is also why we need the pattern recogntion track in this method as well.
    int mcTrackIdProperty = prTrackCand.getMcTrackId();

    if (mcTrackIdProperty == -999) return PRToMCMatchInfo::GHOST;
    else if (mcTrackIdProperty == -99) return PRToMCMatchInfo::BACKGROUND;
    else if (mcTrackIdProperty == -9) return PRToMCMatchInfo::CLONE; // MCTrackMatcher is runing without RelateClonesToMcParticles
    else if (mcTrackIdProperty < 0) return PRToMCMatchInfo::INVALID;

  } else {

    // The patter recognition track has an associated Monte Carlo track.
    // Figure out of it is a clone or a match by the sign of the purity.
    if (purity < 0) return PRToMCMatchInfo::CLONE;
    else if (purity > 0) return PRToMCMatchInfo::MATCHED;
    else return PRToMCMatchInfo::INVALID;

  }

  return PRToMCMatchInfo ::INVALID;
}


