/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2011 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Cate MacQueen (UniMelb)                                  *
* Last Updated: December 2017                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <list>
#include <iostream>
#include <ecl/modules/eclChargedPID/ECLChargedPIDModule.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationVector.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLPidLikelihood.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLLocalMaximum.h>

//#include <ecl/dataobjects/ECLTrig.h>

using namespace std;
using namespace Belle2;
//using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(ECLChargedPID)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLChargedPIDModule::ECLChargedPIDModule()
  : Module(),
    m_rootFilePtr(0),
    m_writeToRoot(1),
    m_eclShowers(eclShowerArrayName()),

    //N1 Hypothesis
    n1_tree(0),
    n1_iExperiment(0),
    n1_iRun(0),
    n1_iEvent(0),

    //Shower
    n1_eclShowerMultip(0),
    n1_eclShowerEnergy(0),
    n1_eclShowerTheta(0),
    n1_eclShowerPhi(0),
    n1_eclShowerR(0),
    n1_eclShowerHypothesisId(0),

    n1_eclShowerAbsZernike11(0),
    n1_eclShowerAbsZernike20(0),
    n1_eclShowerAbsZernike22(0),
    n1_eclShowerAbsZernike31(0),
    n1_eclShowerAbsZernike33(0),
    n1_eclShowerAbsZernike40(0),
    n1_eclShowerAbsZernike42(0),
    n1_eclShowerAbsZernike44(0),
    n1_eclShowerAbsZernike51(0),
    n1_eclShowerAbsZernike53(0),
    n1_eclShowerAbsZernike55(0),

    //MC
    n1_mcMultip(0),
    n1_mcPdg(0),
    n1_mcMothPdg(0),
    n1_mcEnergy(0),
    n1_mcP(0),
    n1_mcTheta(0),
    n1_mcPhi(0),

    //Tracks
    n1_trkMultip(0),
    n1_trkPdg(0),
    n1_trkCharge(0),
    n1_trkP(0),
    n1_trkTheta(0),
    n1_trkPhi(0),

    n1_eclEoP(0),


    //N2 Hypothesis
    n2_tree(0),
    n2_iExperiment(0),
    n2_iRun(0),
    n2_iEvent(0),

    //Shower
    n2_eclShowerMultip(0),
    n2_eclShowerEnergy(0),
    n2_eclShowerTheta(0),
    n2_eclShowerPhi(0),
    n2_eclShowerR(0),
    n2_eclShowerHypothesisId(0),

    n2_eclShowerAbsZernike11(0),
    n2_eclShowerAbsZernike20(0),
    n2_eclShowerAbsZernike22(0),
    n2_eclShowerAbsZernike31(0),
    n2_eclShowerAbsZernike33(0),
    n2_eclShowerAbsZernike40(0),
    n2_eclShowerAbsZernike42(0),
    n2_eclShowerAbsZernike44(0),
    n2_eclShowerAbsZernike51(0),
    n2_eclShowerAbsZernike53(0),
    n2_eclShowerAbsZernike55(0),

    //MC
    n2_mcMultip(0),
    n2_mcPdg(0),
    n2_mcMothPdg(0),
    n2_mcEnergy(0),
    n2_mcP(0),
    n2_mcTheta(0),
    n2_mcPhi(0),

    //Tracks
    n2_trkMultip(0),
    n2_trkPdg(0),
    n2_trkCharge(0),
    n2_trkP(0),
    n2_trkTheta(0),
    n2_trkPhi(0),

    n2_eclEoP(0)


{
  //Set module properties

  setDescription("This module produces an ntuple with ECL-related quantities starting from mdst");
  addParam("writeToRoot", m_writeToRoot,
           "set true if you want to save the informations in a root file named by parameter 'rootFileName'", bool(true));
  addParam("rootFileName", m_rootFileName,
           "fileName used for root file where info are saved. Will be ignored if parameter 'writeToRoot' is false (standard)",
           string("eclChargedPID"));
  //addParam("usingHypoID", m_myHypoID, "Hypo ID = 5 for photons AND Hypo ID = 6 for hadrons", int(5));

}

ECLChargedPIDModule::~ECLChargedPIDModule()
{
}

void ECLChargedPIDModule::initialize()
{

  B2INFO("[ECLChargedPID Module]: Starting initialization of ECLChargedPID Module.");

  if (m_writeToRoot == true) {
    m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  } else
    m_rootFilePtr = NULL;

  // initialize N1 tree
  n1_tree     = new TTree("n1_tree", "ECL Charged PID tree: N1 Hypothesis");

  n1_tree->Branch("expNo", &n1_iExperiment, "expNo/I");
  n1_tree->Branch("runNo", &n1_iRun, "runNo/I");
  n1_tree->Branch("evtNo", &n1_iEvent, "evtNo/I");

  //shower
  n1_tree->Branch("eclShowerMultip",     &n1_eclShowerMultip,     "eclShowerMultip/I");
  n1_tree->Branch("eclShowerEnergy",     "std::vector<double>",    &n1_eclShowerEnergy);
  n1_tree->Branch("eclShowerTheta",      "std::vector<double>",    &n1_eclShowerTheta);
  n1_tree->Branch("eclShowerPhi",        "std::vector<double>",    &n1_eclShowerPhi);
  n1_tree->Branch("eclShowerR",          "std::vector<double>",    &n1_eclShowerR);
  n1_tree->Branch("eclShowerHypothesisId",     "std::vector<int>",    &n1_eclShowerHypothesisId);

  n1_tree->Branch("eclShowerAbsZernike11",     "std::vector<double>",       &n1_eclShowerAbsZernike11);
  n1_tree->Branch("eclShowerAbsZernike20",     "std::vector<double>",       &n1_eclShowerAbsZernike20);
  n1_tree->Branch("eclShowerAbsZernike22",     "std::vector<double>",       &n1_eclShowerAbsZernike22);
  n1_tree->Branch("eclShowerAbsZernike31",     "std::vector<double>",       &n1_eclShowerAbsZernike31);
  n1_tree->Branch("eclShowerAbsZernike33",     "std::vector<double>",       &n1_eclShowerAbsZernike33);
  n1_tree->Branch("eclShowerAbsZernike40",     "std::vector<double>",       &n1_eclShowerAbsZernike40);
  n1_tree->Branch("eclShowerAbsZernike42",     "std::vector<double>",       &n1_eclShowerAbsZernike42);
  n1_tree->Branch("eclShowerAbsZernike44",     "std::vector<double>",       &n1_eclShowerAbsZernike44);
  n1_tree->Branch("eclShowerAbsZernike51",     "std::vector<double>",       &n1_eclShowerAbsZernike51);
  n1_tree->Branch("eclShowerAbsZernike53",     "std::vector<double>",       &n1_eclShowerAbsZernike53);
  n1_tree->Branch("eclShowerAbsZernike55",     "std::vector<double>",       &n1_eclShowerAbsZernike55);

  //MC
  n1_tree->Branch("mcMultip",     &n1_mcMultip,           "mcMultip/I");
  n1_tree->Branch("mcPdg",        "std::vector<int>",    &n1_mcPdg);
  n1_tree->Branch("mcMothPdg",    "std::vector<int>",    &n1_mcMothPdg);
  n1_tree->Branch("mcEnergy",     "std::vector<double>", &n1_mcEnergy);
  n1_tree->Branch("mcP",         "std::vector<double>", &n1_mcP);
  n1_tree->Branch("mcTheta",         "std::vector<double>", &n1_mcTheta);
  n1_tree->Branch("mcPhi",         "std::vector<double>", &n1_mcPhi);

  //tracks
  n1_tree->Branch("trkMultip",     &n1_trkMultip,          "trkMulti/I");
  n1_tree->Branch("trkPdg",        "std::vector<int>",    &n1_trkPdg);
  n1_tree->Branch("trkCharge",        "std::vector<int>",    &n1_trkCharge);
  n1_tree->Branch("trkP",         "std::vector<double>", &n1_trkP);
  n1_tree->Branch("trkTheta",         "std::vector<double>", &n1_trkTheta);
  n1_tree->Branch("trkPhi",         "std::vector<double>", &n1_trkPhi);

  n1_tree->Branch("eclEoP",         "std::vector<double>", &n1_eclEoP);


  // initialize N2 tree
  n2_tree     = new TTree("n2_tree", "ECL Charged PID tree: N2 Hypothesis");

  n2_tree->Branch("expNo", &n2_iExperiment, "expNo/I");
  n2_tree->Branch("runNo", &n2_iRun, "runNo/I");
  n2_tree->Branch("evtNo", &n2_iEvent, "evtNo/I");

  //shower
  n2_tree->Branch("eclShowerMultip",     &n2_eclShowerMultip,     "eclShowerMultip/I");
  n2_tree->Branch("eclShowerEnergy",     "std::vector<double>",    &n2_eclShowerEnergy);
  n2_tree->Branch("eclShowerTheta",      "std::vector<double>",    &n2_eclShowerTheta);
  n2_tree->Branch("eclShowerPhi",        "std::vector<double>",    &n2_eclShowerPhi);
  n2_tree->Branch("eclShowerR",          "std::vector<double>",    &n2_eclShowerR);
  n2_tree->Branch("eclShowerHypothesisId",     "std::vector<int>",    &n2_eclShowerHypothesisId);

  n2_tree->Branch("eclShowerAbsZernike11",     "std::vector<double>",       &n2_eclShowerAbsZernike11);
  n2_tree->Branch("eclShowerAbsZernike20",     "std::vector<double>",       &n2_eclShowerAbsZernike20);
  n2_tree->Branch("eclShowerAbsZernike22",     "std::vector<double>",       &n2_eclShowerAbsZernike22);
  n2_tree->Branch("eclShowerAbsZernike31",     "std::vector<double>",       &n2_eclShowerAbsZernike31);
  n2_tree->Branch("eclShowerAbsZernike33",     "std::vector<double>",       &n2_eclShowerAbsZernike33);
  n2_tree->Branch("eclShowerAbsZernike40",     "std::vector<double>",       &n2_eclShowerAbsZernike40);
  n2_tree->Branch("eclShowerAbsZernike42",     "std::vector<double>",       &n2_eclShowerAbsZernike42);
  n2_tree->Branch("eclShowerAbsZernike44",     "std::vector<double>",       &n2_eclShowerAbsZernike44);
  n2_tree->Branch("eclShowerAbsZernike51",     "std::vector<double>",       &n2_eclShowerAbsZernike51);
  n2_tree->Branch("eclShowerAbsZernike53",     "std::vector<double>",       &n2_eclShowerAbsZernike53);
  n2_tree->Branch("eclShowerAbsZernike55",     "std::vector<double>",       &n2_eclShowerAbsZernike55);

  //MC particle
  n2_tree->Branch("mcMultip",     &n2_mcMultip,           "mcMultip/I");
  n2_tree->Branch("mcPdg",        "std::vector<int>",    &n2_mcPdg);
  n2_tree->Branch("mcMothPdg",    "std::vector<int>",    &n2_mcMothPdg);
  n2_tree->Branch("mcEnergy",     "std::vector<double>", &n2_mcEnergy);
  n2_tree->Branch("mcP",         "std::vector<double>", &n2_mcP);
  n2_tree->Branch("mcTheta",         "std::vector<double>", &n2_mcTheta);
  n2_tree->Branch("mcPhi",         "std::vector<double>", &n2_mcPhi);

  //tracks
  n2_tree->Branch("trkMultip",     &n2_trkMultip,          "trkMulti/I");
  n2_tree->Branch("trkPdg",        "std::vector<int>",    &n2_trkPdg);
  n2_tree->Branch("trkCharge",        "std::vector<int>",    &n2_trkCharge);
  n2_tree->Branch("trkP",         "std::vector<double>", &n2_trkP);
  n2_tree->Branch("trkTheta",         "std::vector<double>", &n2_trkTheta);
  n2_tree->Branch("trkPhi",         "std::vector<double>", &n2_trkPhi);

  n2_tree->Branch("eclEoP",         "std::vector<double>", &n2_eclEoP);


  B2INFO("[ECLChargedPID Module]: Initialization of ECLChargedPID Module completed.");

}

void ECLChargedPIDModule::beginRun()
{
}


void ECLChargedPIDModule::event()
{

  B2DEBUG(1, "  ++++++++++++++ ECLChargedPIDModule");

  ///Showers
  n1_eclShowerMultip = 0;
  n1_eclShowerEnergy->clear();
  n1_eclShowerTheta->clear();
  n1_eclShowerPhi->clear();
  n1_eclShowerR->clear();
  n1_eclShowerHypothesisId->clear();

  n1_eclShowerAbsZernike11->clear();
  n1_eclShowerAbsZernike20->clear();
  n1_eclShowerAbsZernike22->clear();
  n1_eclShowerAbsZernike31->clear();
  n1_eclShowerAbsZernike33->clear();
  n1_eclShowerAbsZernike40->clear();
  n1_eclShowerAbsZernike42->clear();
  n1_eclShowerAbsZernike44->clear();
  n1_eclShowerAbsZernike51->clear();
  n1_eclShowerAbsZernike53->clear();
  n1_eclShowerAbsZernike55->clear();

  ///MC
  n1_mcMultip = 0;
  n1_mcPdg->clear();
  n1_mcMothPdg->clear();
  n1_mcEnergy->clear();
  n1_mcP->clear();
  n1_mcTheta->clear();
  n1_mcPhi->clear();

  ///Tracks
  n1_trkMultip = 0;
  n1_trkPdg->clear();
  n1_trkCharge->clear();
  n1_trkP->clear();
  n1_trkTheta->clear();
  n1_trkPhi->clear();

  n1_eclEoP->clear();

  //Showers
  n2_eclShowerMultip = 0;
  n2_eclShowerEnergy->clear();
  n2_eclShowerTheta->clear();
  n2_eclShowerPhi->clear();
  n2_eclShowerR->clear();
  n2_eclShowerHypothesisId->clear();

  n2_eclShowerAbsZernike11->clear();
  n2_eclShowerAbsZernike20->clear();
  n2_eclShowerAbsZernike22->clear();
  n2_eclShowerAbsZernike31->clear();
  n2_eclShowerAbsZernike33->clear();
  n2_eclShowerAbsZernike40->clear();
  n2_eclShowerAbsZernike42->clear();
  n2_eclShowerAbsZernike44->clear();
  n2_eclShowerAbsZernike51->clear();
  n2_eclShowerAbsZernike53->clear();
  n2_eclShowerAbsZernike55->clear();

  //MC
  n2_mcMultip = 0;
  n2_mcPdg->clear();
  n2_mcMothPdg->clear();
  n2_mcEnergy->clear();
  n2_mcP->clear();
  n2_mcTheta->clear();
  n2_mcPhi->clear();

  //Tracks
  n2_trkMultip = 0;
  n2_trkPdg->clear();
  n2_trkCharge->clear();
  n2_trkP->clear();
  n2_trkTheta->clear();
  n2_trkPhi->clear();

  n2_eclEoP->clear();


  StoreObjPtr<EventMetaData> eventmetadata;
  if (eventmetadata) {
    n1_iExperiment = eventmetadata->getExperiment();
    n1_iRun = eventmetadata->getRun();
    n1_iEvent = eventmetadata->getEvent();
    n2_iExperiment = eventmetadata->getExperiment();
    n2_iRun = eventmetadata->getRun();
    n2_iEvent = eventmetadata->getEvent();
  } else {
    n1_iExperiment = -1;
    n1_iRun = -1;
    n1_iEvent = -1;
    n2_iExperiment = -1;
    n2_iRun = -1;
    n2_iEvent = -1;
  }






  // get the matched MC particle
  StoreArray<MCParticle> m_mcpart;
  n1_mcMultip = 0;
  n2_mcMultip = 0;
  for (const MCParticle& imcpart : m_mcpart) {
    if (!imcpart.hasStatus(MCParticle::c_PrimaryParticle)) continue; // only check primaries
    if (imcpart.hasStatus(MCParticle::c_Initial)) continue; // only check primaries
    if (imcpart.hasStatus(MCParticle::c_IsVirtual)) continue; // only check primaries

    // get mc particle kinematics
    n1_mcPdg->push_back(imcpart.getPDG());
    if (imcpart.getMother() != NULL) n1_mcMothPdg->push_back(imcpart.getMother()->getPDG());
    else n1_mcMothPdg->push_back(-999);
    n1_mcEnergy->push_back(imcpart.getEnergy());
    n1_mcP->push_back(imcpart.getMomentum().Mag());
    n1_mcTheta->push_back(imcpart.getMomentum().Theta());
    n1_mcPhi->push_back(imcpart.getMomentum().Phi());

    n2_mcPdg->push_back(imcpart.getPDG());
    if (imcpart.getMother() != NULL) n2_mcMothPdg->push_back(imcpart.getMother()->getPDG());
    else n2_mcMothPdg->push_back(-999);
    n2_mcEnergy->push_back(imcpart.getEnergy());
    n2_mcP->push_back(imcpart.getMomentum().Mag());
    n2_mcTheta->push_back(imcpart.getMomentum().Theta());
    n2_mcPhi->push_back(imcpart.getMomentum().Phi());

    // loop over all tracks to find index of max momentum
    n1_trkMultip = 0;
    n2_trkMultip = 0;
    int index = 0;
    int index_max_mom = -1;
    double max_mom = -1;
    for (auto& itrk : imcpart.getRelationsFrom<Track>()) {
      // get the track fit results
      const TrackFitResult* atrkF = itrk.getTrackFitResult(Const::pion);
      if (atrkF == nullptr) continue; //go to next track if no fit result

      if (atrkF->getMomentum().Mag() > max_mom) {
        max_mom = atrkF->getMomentum().Mag();
        index_max_mom = index;
      }
      index++;
    }

    // loop over all tracks to fill vector for event
    index = -1;
    for (auto& itrk : imcpart.getRelationsFrom<Track>()) {
      ++index;
      if (index != index_max_mom) continue;

      // get the track fit results
      const TrackFitResult* atrkF = itrk.getTrackFitResult(Const::pion);
      if (atrkF == nullptr) continue; //go to next track if no fit result

      // get trk kinematics
      n1_trkPdg->push_back(atrkF->getParticleType().getPDGCode());
      n1_trkCharge->push_back(atrkF->getChargeSign());
      n1_trkP->push_back(atrkF->getMomentum().Mag());
      n1_trkTheta->push_back(atrkF->getMomentum().Theta());
      n1_trkPhi->push_back(atrkF->getMomentum().Phi());

      n2_trkPdg->push_back(atrkF->getParticleType().getPDGCode());
      n2_trkCharge->push_back(atrkF->getChargeSign());
      n2_trkP->push_back(atrkF->getMomentum().Mag());
      n2_trkTheta->push_back(atrkF->getMomentum().Theta());
      n2_trkPhi->push_back(atrkF->getMomentum().Phi());

      // loop over all  matched ECLShowers to find index of max energy
      n1_eclShowerMultip = 0;
      int jndex1 = -1;
      int jndex1_max_e = -1;
      double max_e1 = -1;
      for (auto& i1shower : itrk.getRelationsTo<ECLShower>()) {
        ++jndex1;
        //use input Hypo ID (5 is default)
        if (i1shower.getHypothesisId() != 5) continue;
        if (i1shower.getEnergy() > max_e1) {
          max_e1 = i1shower.getEnergy();
          jndex1_max_e = jndex1;
        }
      }

      n2_eclShowerMultip = 0;
      int jndex2 = -1;
      int jndex2_max_e = -1;
      double max_e2 = -1;
      for (auto& i2shower : itrk.getRelationsTo<ECLShower>()) {
        ++jndex2;
        //use input Hypo ID (5 is default)
        if (i2shower.getHypothesisId() != 6) continue;
        if (i2shower.getEnergy() > max_e2) {
          max_e2 = i2shower.getEnergy();
          jndex2_max_e = jndex2;
        }
      }

      // loop over all matched ECLShowers to fill vector for event
      jndex1 = -1;
      for (auto& i1shower : itrk.getRelationsTo<ECLShower>()) {
        ++jndex1;
        if (jndex1 != jndex1_max_e) continue;

//  //use input Hypo ID (5 is default)
//  if(ishower.getHypothesisId()!=5) continue;

        // get shower kinematics
        n1_eclShowerEnergy->push_back(i1shower.getEnergy());
        n1_eclShowerTheta->push_back(i1shower.getTheta());
        n1_eclShowerPhi->push_back(i1shower.getPhi());
        n1_eclShowerR->push_back(i1shower.getR());
        n1_eclShowerHypothesisId->push_back(i1shower.getHypothesisId());

        //get shower Zernike moments
        n1_eclShowerAbsZernike11->push_back(i1shower.getAbsZernike11());
        n1_eclShowerAbsZernike20->push_back(i1shower.getAbsZernike20());
        n1_eclShowerAbsZernike22->push_back(i1shower.getAbsZernike22());
        n1_eclShowerAbsZernike31->push_back(i1shower.getAbsZernike31());
        n1_eclShowerAbsZernike33->push_back(i1shower.getAbsZernike33());
        n1_eclShowerAbsZernike40->push_back(i1shower.getAbsZernike40());
        n1_eclShowerAbsZernike42->push_back(i1shower.getAbsZernike42());
        n1_eclShowerAbsZernike44->push_back(i1shower.getAbsZernike44());
        n1_eclShowerAbsZernike51->push_back(i1shower.getAbsZernike51());
        n1_eclShowerAbsZernike53->push_back(i1shower.getAbsZernike53());
        n1_eclShowerAbsZernike55->push_back(i1shower.getAbsZernike55());

        n1_eclEoP->push_back((i1shower.getEnergy()) / (atrkF->getMomentum().Mag()));

        n1_eclShowerMultip++;
      }

      jndex2 = -1;
      for (auto& i2shower : itrk.getRelationsTo<ECLShower>()) {
        ++jndex2;
        if (jndex2 != jndex2_max_e) continue;

        // get shower kinematics
        n2_eclShowerEnergy->push_back(i2shower.getEnergy());
        n2_eclShowerTheta->push_back(i2shower.getTheta());
        n2_eclShowerPhi->push_back(i2shower.getPhi());
        n2_eclShowerR->push_back(i2shower.getR());
        n2_eclShowerHypothesisId->push_back(i2shower.getHypothesisId());

        // get shower Zernike moments
        n2_eclShowerAbsZernike11->push_back(i2shower.getAbsZernike11());
        n2_eclShowerAbsZernike20->push_back(i2shower.getAbsZernike20());
        n2_eclShowerAbsZernike22->push_back(i2shower.getAbsZernike22());
        n2_eclShowerAbsZernike31->push_back(i2shower.getAbsZernike31());
        n2_eclShowerAbsZernike33->push_back(i2shower.getAbsZernike33());
        n2_eclShowerAbsZernike40->push_back(i2shower.getAbsZernike40());
        n2_eclShowerAbsZernike42->push_back(i2shower.getAbsZernike42());
        n2_eclShowerAbsZernike44->push_back(i2shower.getAbsZernike44());
        n2_eclShowerAbsZernike51->push_back(i2shower.getAbsZernike51());
        n2_eclShowerAbsZernike53->push_back(i2shower.getAbsZernike53());
        n2_eclShowerAbsZernike55->push_back(i2shower.getAbsZernike55());

        n2_eclEoP->push_back((i2shower.getEnergy()) / (atrkF->getMomentum().Mag()));

        n2_eclShowerMultip++;
      }

      n1_trkMultip++;
      n2_trkMultip++;
    }

    n1_mcMultip++;
    n2_mcMultip++;
  }

  // must add separate loop to fill vectors after this loop
  // as this loop must ensure m_XXXMultip != 0 for trk or Shower


  n1_tree->Fill();
  n2_tree->Fill();

}


void ECLChargedPIDModule::endRun()
{
}


void ECLChargedPIDModule::terminate()
{

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();
    n1_tree->Write();
    n2_tree->Write();
  }

}


