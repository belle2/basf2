/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jincheng Mei                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMTimeCalibration/KLMTimeCalibrationCollectorModule.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMHit1d.h>
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMMuidLikelihood.h>

/* Belle 2 headers. */
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <framework/dataobjects/EventT0.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>

/* ROOT headers. */
#include <TTree.h>

/* C++ headers. */
#include  <utility>

using namespace Belle2;
using namespace Belle2::bklm;
using namespace Belle2::EKLM;

REG_MODULE(KLMTimeCalibrationCollector)

KLMTimeCalibrationCollectorModule::KLMTimeCalibrationCollectorModule() :
  CalibrationCollectorModule(),
  m_geoParB(nullptr),
  m_geoParE(nullptr),
  m_TransformData(nullptr),
  m_HeventT0_0(nullptr),
  m_HeventT0_1(nullptr),
  m_HnumTrack(nullptr),
  m_HnBHit2dOfTrack(nullptr),
  m_HnEHit2dOfTrack(nullptr),
  m_HpositionXDiff(nullptr),
  m_HpositionYDiff(nullptr),
  m_HpositionZDiff(nullptr),
  m_HflyTimeB(nullptr),
  m_HflyTimeE(nullptr),
  m_HnumDigit_scint_end(nullptr),
  m_HnumDigit_scint(nullptr),
  m_HnumDigit_rpc(nullptr),
  m_outTree(nullptr)
{
  setDescription("Module for KLM time calibration (data collection).");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("debug", m_Debug, "debug mode.", false);
  addParam("inputParticleList", m_inputListName, "input particle list.", std::string("mu:cali"));
  addParam("useEventT0", m_useEvtT0, "Use event T0 or not.", true);

  m_elementNum = &(KLMElementNumbers::Instance());
}

KLMTimeCalibrationCollectorModule::~KLMTimeCalibrationCollectorModule()
{
}

void KLMTimeCalibrationCollectorModule::prepare()
{
  setDescription("Preparation for BKLM TimeCalibration Collector Module.");

  /* Initialize geometry. */
  m_geoParB = bklm::GeometryPar::instance();
  m_geoParE = &(EKLM::GeometryData::Instance());
  m_TransformData = new EKLM::TransformData(true, EKLM::TransformData::c_None);

  /* Require input dataobjects. */
  if (m_useEvtT0) m_eventT0.isRequired("EventT0");
  m_tracks.isRequired();

  m_outTree = new TTree("time_calibration_data", "");
  m_outTree->Branch("t0",        &m_ev.t0,        "t0/D");
  m_outTree->Branch("flyTime",   &m_ev.flyTime,   "flyTime/D");
  m_outTree->Branch("recTime",   &m_ev.recTime,   "recTime/D");
  m_outTree->Branch("dist",      &m_ev.dist,      "dist/D");
  m_outTree->Branch("diffDistX", &m_ev.diffDistX, "diffDistX/D");
  m_outTree->Branch("diffDistY", &m_ev.diffDistY, "diffDistY/D");
  m_outTree->Branch("diffDistZ", &m_ev.diffDistZ, "diffDistZ/D");
  m_outTree->Branch("eDep",      &m_ev.eDep,      "eDep/D");
  m_outTree->Branch("nPE",       &m_ev.nPE,       "nPE/D");
  m_outTree->Branch("channelId", &m_ev.channelId, "channelId/I");
  m_outTree->Branch("inRPC",     &m_ev.inRPC,     "inRPC/O");
  m_outTree->Branch("isFlipped", &m_ev.isFlipped, "isFlipped/O");

  registerObject<TTree>("time_calibration_data", m_outTree);

  m_HeventT0_0 = new TH1D("m_HeventT0_0", "collision time before track number request;t0[ns]", 200, -100, 100);
  m_HeventT0_1 = new TH1D("m_HeventT0_1", "collision time after track number request;t0[ns]", 200, -100, 100);
  m_HnumTrack = new TH1I("m_HnnumTrack", "Number of Track;nTrack", 30, 0, 30);

  m_HnBHit2dOfTrack = new TH1I("m_HnBKLMHit2dOfTrack", "Number of BKLMHit2d belong to recTrack;num of BKLMHit2d", 20, 0, 20);
  m_HnEHit2dOfTrack = new TH1I("m_HnEKLMHit2dOfTrack", "Number of EKLMHit2d belong to recTrack;num of EKLMHit2d", 15, 0, 15);

  m_HpositionDiff = new TH1D("m_HpositionDiff", "Dist between extHit and KLMHit2d;dist", 160, 0, 8);
  m_HpositionXDiff = new TH1D("m_HpositionXDiff", "DistX between extHit and KLMHit2d;distX", 100, 0, 5);
  m_HpositionYDiff = new TH1D("m_HpositionYDiff", "DistY between extHit and KLMHit2d;distY", 100, 0, 5);
  m_HpositionZDiff = new TH1D("m_HpositionZDiff", "DistZ between extHit and KLMHit2d;distZ", 100, 0, 5);

  m_HflyTimeB = new TH2D("m_HflyTimeB", "flyTime;flyTime(wet)/ns;Layer", 32, 0, 16, 20, 0, 20);
  m_HflyTimeE = new TH2D("m_HflyTimeE", "flyTime;flyTime(wet)/ns;Layer", 32, 0, 16, 20, 0, 20);

  m_HnumDigit_rpc = new TH1I("m_HnumDigit_rpc", "Number of digit per bklmHit1d (RPC);number of digit", 15, 0, 15);
  m_HnumDigit_scint = new TH1I("m_HnumDigit_scint", "Number of digit per bklmHit1d (scint);number of digit", 15, 0, 15);
  m_HnumDigit_scint_end = new TH1I("m_HnumDigit_scint_end", "Number of eklmDigit per eklmHit1d (scint);number of eklmDigit", 15, 0,
                                   15);

  registerObject<TH1D>("m_HevtT0_0", m_HeventT0_0);
  registerObject<TH1D>("m_HevtT0_1", m_HeventT0_1);
  registerObject<TH1I>("m_HnTrack", m_HnumTrack);
  registerObject<TH1I>("m_HnBHit2d", m_HnBHit2dOfTrack);
  registerObject<TH1I>("m_HnEHit2d", m_HnEHit2dOfTrack);

  registerObject<TH1D>("m_HposiDiff", m_HpositionDiff);
  registerObject<TH1D>("m_HposiXDiff", m_HpositionXDiff);
  registerObject<TH1D>("m_HposiYDiff", m_HpositionYDiff);
  registerObject<TH1D>("m_HposiZDiff", m_HpositionZDiff);

  registerObject<TH2D>("m_HfTimeB", m_HflyTimeB);
  registerObject<TH2D>("m_HfTimeE", m_HflyTimeE);

  registerObject<TH1I>("m_HnDigit_rpc", m_HnumDigit_rpc);
  registerObject<TH1I>("m_HnDigit_scint", m_HnumDigit_scint);
  registerObject<TH1I>("m_HnDigit_scint_end", m_HnumDigit_scint_end);
}

void KLMTimeCalibrationCollectorModule::collect()
{
  setDescription("Time Calibration Collector. Main Collect Function of Collector Module Begins.");
  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);

  m_ev.t0 = 0.0;
  /* Require event T0 determined from CDC */
  if (m_useEvtT0) {
    if (!m_eventT0.isValid()) return;
    if (!m_eventT0->hasTemporaryEventT0(Const::EDetector::CDC)) return;
    const std::vector<EventT0::EventT0Component> evtT0C = m_eventT0->getTemporaryEventT0s(Const::EDetector::CDC);
    m_ev.t0 = evtT0C.back().eventT0;
  }

  /* Read data meta infor */
  int runId = eventMetaData->getRun();
  int evtId = eventMetaData->getEvent();

  getObjectPtr<TH1D>("m_HevtT0_0")->Fill(m_ev.t0);

  const StoreObjPtr<ParticleList> inputList(m_inputListName);
  unsigned n_track =  inputList->getListSize();
  if (n_track < 1) {
    B2DEBUG(20, "No necessary tracks in" << LogVar("run", runId) << LogVar("event", evtId));
    return;
  }  // track existence

  B2DEBUG(20, "debug infor for" << LogVar("run", runId) << LogVar("event", evtId) << LogVar("number of rec tracks", n_track));

  getObjectPtr<TH1D>("m_HevtT0_1")->Fill(m_ev.t0);
  getObjectPtr<TH1I>("m_HnTrack")->Fill(n_track);

  /* Here begins the ext track sequence */
  B2DEBUG(20, "Track loop begins");

  /* Main loop */
  for (unsigned iT = 0; iT < n_track; ++iT) {
    // Good track selection
    const Particle* particle = inputList->getParticle(iT);
    const Track* track = particle->getTrack();

    // Find data objects related to track
    RelationVector<ExtHit> extHits = track->getRelationsTo<ExtHit>();
    RelationVector<BKLMHit2d> bklmHit2ds = track->getRelationsTo<BKLMHit2d>();
    RelationVector<EKLMHit2d> eklmHit2ds = track->getRelationsTo<EKLMHit2d>();

    getObjectPtr<TH1I>("m_HnHit2d_bklm")->Fill(int(bklmHit2ds.size()));
    getObjectPtr<TH1I>("m_HnHit2d_eklm")->Fill(int(eklmHit2ds.size()));

    B2DEBUG(20, "Track" << LogVar("exthits", extHits.size())
            << LogVar("BKLMHit2d", bklmHit2ds.size()) << LogVar("EKLMHit2d", eklmHit2ds.size()));
    if (eklmHit2ds.size() < 2 && bklmHit2ds.size() < 2) continue;
    if (extHits.size() < 2) continue;

    // Loop for extroplate hits
    m_mapExtHits.clear();
    for (unsigned iE = 0; iE < extHits.size(); ++iE) {
      ExtHit* extHit = extHits[iE];
      // Select extHits in the KLM range
      bool bklmCover = (extHit->getDetectorID() == Const::EDetector::BKLM);
      bool eklmCover = (extHit->getDetectorID() == Const::EDetector::EKLM);
      if (!bklmCover && !eklmCover) continue;

      int copyId = extHit->getCopyID();
      int tSub, tFor, tSec, tLay;
      m_elementNum->moduleNumberToElementNumbers(copyId, &tSub, &tFor, &tSec, &tLay);
      bool crossed = false; // should be only once ?
      KLMMuidLikelihood* muidLikelihood = track->getRelatedTo<KLMMuidLikelihood>();
      if (tSub == KLMElementNumbers::c_BKLM)
        crossed = muidLikelihood->isExtrapolatedBarrelLayerCrossed(tLay - 1);
      else
        crossed = muidLikelihood->isExtrapolatedEndcapLayerCrossed(tLay);
      if (crossed)
        m_mapExtHits.insert(std::pair<int, ExtHit*>(copyId, extHit));
    }

    B2DEBUG(20, "In KLM coverage: " << LogVar("exthits", m_mapExtHits.size())
            << LogVar("BKLMHit2d", bklmHit2ds.size()) << LogVar("EKLMHit2d", eklmHit2ds.size()));
    if (m_mapExtHits.size() < 2) continue;

    collectRPC(bklmHit2ds);
    collectScint(bklmHit2ds);
    collectScintEnd(eklmHit2ds);
  }
}

void KLMTimeCalibrationCollectorModule::collectScintEnd(RelationVector<EKLMHit2d> hit2ds)
{
  setDescription("Time Calibration Collector. Collect Function for EKLM parts of Collector Module Begins.");

  const HepGeom::Transform3D* tr;
  HepGeom::Point3D<double> hitGlobal_extHit, hitLocal_extHit;
  double l;

  for (unsigned i = 0; i < hit2ds.size(); ++i) {
    EKLMHit2d* hit2d = hit2ds[i];

    RelationVector<KLMDigit> digits = hit2d->getRelationsTo<KLMDigit>();
    unsigned nDigit = digits.size();
    if (nDigit != 2)
      B2FATAL("Wrong number of related KLMDigits.");
    if (digits[0]->getNPhotoelectrons() == 0 || digits[1]->getNPhotoelectrons() == 0) continue;
    if (!digits[0]->isGood() || !digits[1]->isGood()) continue;

    for (unsigned j = 0; j < nDigit; ++j) {
      KLMDigit* digitHit = digits[j];
      unsigned int localID = digitHit->getUniqueChannelID();
      m_ev.channelId = m_elementNum->channelNumberEKLM(localID);
      m_ev.inRPC = 0;

      std::pair<ExtHit*, ExtHit*> pair_extHits = matchExt(m_ev.channelId);
      ExtHit* entryHit = pair_extHits.first;
      ExtHit* exitHit = pair_extHits.second;
      if (entryHit == nullptr || exitHit == nullptr) continue;

      m_ev.flyTime = 0.5 * (entryHit->getTOF() + exitHit->getTOF());

      TVector3 positionGlobal_extHit = 0.5 * (entryHit->getPosition() + exitHit->getPosition());
      TVector3 positionGlobal_digit = hit2d->getPosition();
      TVector3 positionGlobal_diff = positionGlobal_extHit - positionGlobal_digit;

      storeDistDiff(positionGlobal_diff);

      l = m_geoParE->getStripLength(digitHit->getStrip()) / CLHEP::mm * Unit::mm;
      hitGlobal_extHit.setX(positionGlobal_extHit.X() / Unit::mm * CLHEP::mm);
      hitGlobal_extHit.setY(positionGlobal_extHit.Y() / Unit::mm * CLHEP::mm);
      hitGlobal_extHit.setZ(positionGlobal_extHit.Z() / Unit::mm * CLHEP::mm);
      tr = m_TransformData->getStripGlobalToLocal(digitHit);
      hitLocal_extHit = (*tr) * hitGlobal_extHit;

      m_ev.dist = 0.5 * l - hitLocal_extHit.x() / CLHEP::mm * Unit::mm;
      m_ev.recTime = digitHit->getTime();
      m_ev.eDep = digitHit->getCharge();
      m_ev.nPE = digitHit->getNPhotoelectrons();

      getObjectPtr<TH2D>("m_HfTime_end")->Fill(m_ev.flyTime, digitHit->getLayer());
      getObjectPtr<TTree>("time_calibration_data")->Fill();
    }
  }
}

void KLMTimeCalibrationCollectorModule::collectScint(RelationVector<BKLMHit2d> bklmHit2ds)
{
  double stripWidtm_HZ, stripWidtm_HPhi;
  for (unsigned iH2 = 0; iH2 < bklmHit2ds.size(); ++iH2) {
    BKLMHit2d* hit2d = bklmHit2ds[iH2];
    if (hit2d->inRPC()) continue;
    if (hit2d->isOutOfTime()) continue;

    RelationVector<BKLMHit1d> bklmHit1ds = hit2d->getRelationsTo<BKLMHit1d>();
    if (bklmHit1ds.size() != 2) continue;

    TVector3 positionGlobal_hit2d = hit2d->getGlobalPosition();
    const bklm::Module* corMod = m_geoParB->findModule(hit2d->getSection(), hit2d->getSector(), hit2d->getLayer());
    stripWidtm_HZ = corMod->getZStripWidth();
    stripWidtm_HPhi = corMod->getPhiStripWidth();
    for (unsigned iH1 = 0; iH1 < bklmHit1ds.size(); ++iH1) {
      BKLMHit1d* hit1d = bklmHit1ds[iH1];
      RelationVector<KLMDigit> digits = hit1d->getRelationsTo<KLMDigit>();
      getObjectPtr<TH1I>("m_HnDigit_scint")->Fill(digits.size());
      if (digits.size() > 5) continue;

      for (unsigned iHd = 0; iHd < digits.size(); ++iHd) {
        KLMDigit* digitHit = digits[iHd];
        if (digitHit->inRPC() || !digitHit->isGood()) continue;

        uint16_t channelId_digit = m_elementNum->channelNumberBKLM(
                                     digitHit->getSection(), digitHit->getSector(), digitHit->getLayer(),
                                     digitHit->isPhiReadout(), digitHit->getStrip());
        std::pair<ExtHit*, ExtHit*> pair_extHits = matchExt(channelId_digit);
        ExtHit* entryHit = pair_extHits.first;
        ExtHit* exitHit = pair_extHits.second;
        if (entryHit == nullptr || exitHit == nullptr) continue;

        m_ev.inRPC = digitHit->inRPC();
        m_ev.flyTime = 0.5 * (entryHit->getTOF() + exitHit->getTOF());

        TVector3 positionGlobal_extHit = 0.5 * (entryHit->getPosition() + exitHit->getPosition());
        TVector3 positionGlobal_diff = positionGlobal_extHit - positionGlobal_hit2d;
        B2DEBUG(29, LogVar("Distance between digit and hit2d", positionGlobal_diff.Mag()));

        storeDistDiff(positionGlobal_diff);
        const CLHEP::Hep3Vector positionLocal_extHit = corMod->globalToLocal(CLHEP::Hep3Vector(
                                                         positionGlobal_extHit.X(), positionGlobal_extHit.Y(), positionGlobal_extHit.Z()), true);
        const CLHEP::Hep3Vector positionLocal_hit2d = corMod->globalToLocal(CLHEP::Hep3Vector(
                                                        positionGlobal_hit2d.X(), positionGlobal_hit2d.Y(), positionGlobal_hit2d.Z()), true);
        const CLHEP::Hep3Vector diffLocal = positionLocal_extHit - positionLocal_hit2d;
        if (fabs(diffLocal.z()) > stripWidtm_HZ  || fabs(diffLocal.y()) > stripWidtm_HPhi) continue;

        const CLHEP::Hep3Vector propaLengthV3 = corMod->getPropagationDistance(positionLocal_extHit);
        double propaLength = propaLengthV3[2 - digitHit->isPhiReadout()];

        m_ev.isFlipped = corMod->isFlipped();
        m_ev.recTime = digitHit->getTime();
        m_ev.dist = propaLength;
        m_ev.eDep = digitHit->getEnergyDeposit();
        m_ev.nPE = digitHit->getNPhotoelectrons();
        m_ev.channelId = channelId_digit;

        getObjectPtr<TH2D>("m_HfTime")->Fill(m_ev.flyTime, digitHit->getLayer());
        getObjectPtr<TTree>("time_calibration_data")->Fill();
      }
    }
  }
}

void KLMTimeCalibrationCollectorModule::collectRPC(RelationVector<BKLMHit2d> bklmHit2ds)
{
  //int iSub, iFor, iSec, iLay, iPla, iStr;
  for (unsigned iH2 = 0; iH2 < bklmHit2ds.size(); ++iH2) {
    B2DEBUG(20, "BKLMHit2d related Track loop begins");

    BKLMHit2d* hit2d = bklmHit2ds[iH2];
    if (!hit2d->inRPC()) continue;
    if (hit2d->isOutOfTime()) continue;
    RelationVector<BKLMHit1d> bklmHit1ds = hit2d->getRelationsTo<BKLMHit1d>();
    if (bklmHit1ds.size() != 2) continue;

    TVector3 positionGlobal_hit2d = hit2d->getGlobalPosition();
    const bklm::Module* corMod = m_geoParB->findModule(hit2d->getSection(), hit2d->getSector(), hit2d->getLayer());
    double stripWidtm_HZ = corMod->getZStripWidth();
    double stripWidtm_HPhi = corMod->getPhiStripWidth();
    for (unsigned iH1 = 0; iH1 < bklmHit1ds.size(); ++iH1) {
      BKLMHit1d* hit1d = bklmHit1ds[iH1];
      RelationVector<KLMDigit> digits = hit1d->getRelationsTo<KLMDigit>();
      getObjectPtr<TH1I>("m_HnDigit_rpc")->Fill(digits.size());
      if (digits.size() > 5) continue;

      for (unsigned iHd = 0; iHd < digits.size(); ++iHd) {
        KLMDigit* digitHit = digits[iHd];

        m_ev.inRPC = digitHit->inRPC();
        uint16_t channelId_digit = m_elementNum->channelNumberBKLM(
                                     digitHit->getSection(), digitHit->getSector(), digitHit->getLayer(),
                                     digitHit->isPhiReadout(), digitHit->getStrip());

        /*
        B2DEBUG(20, "BKLM element numbers check. RPC Part.\n" <<
                "ExtHit Forward :: Digit Forward   " << iFor   << "  " << digitHit->getSection()   << std::endl <<
                "ExtHit Sector  :: Digit Sector    " << iSec   << "  " << digitHit->getSector()    << std::endl <<
                "ExtHit Layer   :: Digit Layer     " << iLay   << "  " << digitHit->getLayer());
        */

        std::pair<ExtHit*, ExtHit*> pair_extHits = matchExt(channelId_digit);
        ExtHit* entryHit = pair_extHits.first;
        ExtHit* exitHit = pair_extHits.second;
        if (entryHit == nullptr || exitHit == nullptr) continue;

        m_ev.flyTime = 0.5 * (entryHit->getTOF() + exitHit->getTOF());
        TVector3 positionGlobal_extHit = 0.5 * (entryHit->getPosition() + exitHit->getPosition());
        TVector3 positionGlobal_diff = positionGlobal_extHit - positionGlobal_hit2d;
        B2DEBUG(29, LogVar("Distance between digit and hit2d", positionGlobal_diff.Mag()));

        storeDistDiff(positionGlobal_diff);
        const CLHEP::Hep3Vector positionLocal_extHit = corMod->globalToLocal(CLHEP::Hep3Vector(
                                                         positionGlobal_extHit.X(), positionGlobal_extHit.Y(), positionGlobal_extHit.Z()), true);
        const CLHEP::Hep3Vector positionLocal_hit2d = corMod->globalToLocal(CLHEP::Hep3Vector(
                                                        positionGlobal_hit2d.X(), positionGlobal_hit2d.Y(), positionGlobal_hit2d.Z()), true);
        const CLHEP::Hep3Vector diffLocal = positionLocal_extHit - positionLocal_hit2d;
        if (fabs(diffLocal.z()) > stripWidtm_HZ  || fabs(diffLocal.y()) > stripWidtm_HPhi) continue;

        const CLHEP::Hep3Vector propaLengthV3 = corMod->getPropagationDistance(positionLocal_extHit);
        double propaLength = propaLengthV3[2 - int(hit1d->isPhiReadout())];

        m_ev.isFlipped = corMod->isFlipped();
        m_ev.recTime = digitHit->getTime();
        m_ev.dist = propaLength;
        m_ev.eDep = digitHit->getEnergyDeposit();
        m_ev.nPE = digitHit->getNPhotoelectrons();
        m_ev.channelId = channelId_digit;

        getObjectPtr<TH2D>("m_HfTime")->Fill(m_ev.flyTime, digitHit->getLayer());
        getObjectPtr<TTree>("time_calibration_data")->Fill();
      }
    }
  }
}

std::pair<ExtHit*, ExtHit*> KLMTimeCalibrationCollectorModule::matchExt(uint16_t kID)
{
  setDescription("KLM time calibration collector. Used for KLM hit and ExtHit matching.");

  int iSub, iFor, iSec, iLay, iPla, iStr;
  int iSubK, iForK, iSecK, iLayK, iPlaK, iStrK;

  m_elementNum->channelNumberToElementNumbers(kID, &iSubK, &iForK, &iSecK, &iLayK, &iPlaK, &iStrK);

  std::multimap<int, ExtHit*>::iterator it;
  ExtHit* extHit = nullptr;
  ExtHit* entryHit = nullptr;
  ExtHit* exitHit = nullptr;

  for (it = m_mapExtHits.begin(); it != m_mapExtHits.end(); ++it) {
    extHit = it->second;
    int copyid = extHit->getCopyID();
    m_elementNum->channelNumberToElementNumbers(copyid, &iSub, &iFor, &iSec, &iLay, &iPla, &iStr);

    if (iSub != iSubK) continue;
    if (iFor != iForK) continue;
    if (iSec != iSecK) continue;
    if (iLay != iLayK) continue;
    if (iSub == KLMElementNumbers::c_EKLM || (iSub == KLMElementNumbers::c_BKLM && iLay > 2)) {
      if (iPla != iPlaK) continue;
      if (iStr != iStrK) continue;
    }

    switch (extHit->getStatus()) {
      case EXT_ENTER:
        if (entryHit == nullptr) {
          entryHit = extHit;
        } else if (extHit->getTOF() < entryHit->getTOF()) {
          entryHit = extHit;
        }
        break;
      case EXT_EXIT:
        if (exitHit == nullptr) {
          exitHit = extHit;
        } else if (extHit->getTOF() > exitHit->getTOF()) {
          exitHit = extHit;
        }
        break;
      default:
        break;
    }
  }
  std::pair<ExtHit*, ExtHit*> p_extHits = std::make_pair(entryHit, exitHit);
  return p_extHits;
}

void KLMTimeCalibrationCollectorModule::storeDistDiff(TVector3 pDiff)
{
  double diffM = pDiff.Mag();
  double diffX = pDiff.X();
  double diffY = pDiff.Y();
  double diffZ = pDiff.Z();
  getObjectPtr<TH1D>("m_HposiDiff")->Fill(diffM);
  getObjectPtr<TH1D>("m_HposiXDiff")->Fill(diffX);
  getObjectPtr<TH1D>("m_HposiYDiff")->Fill(diffY);
  getObjectPtr<TH1D>("m_HposiZDiff")->Fill(diffZ);
  m_ev.diffDistX = diffX;
  m_ev.diffDistY = diffY;
  m_ev.diffDistZ = diffZ;
}

void KLMTimeCalibrationCollectorModule::finish()
{
  B2INFO("Data Collection Done.");
}
