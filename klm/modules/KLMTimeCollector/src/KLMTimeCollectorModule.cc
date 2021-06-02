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
#include <klm/modules/KLMTimeCollector/KLMTimeCollectorModule.h>

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
#include <utility>

using namespace Belle2;
using namespace Belle2::bklm;
using namespace Belle2::EKLM;

REG_MODULE(KLMTimeCollector)

KLMTimeCollectorModule::KLMTimeCollectorModule() :
  CalibrationCollectorModule(),
  m_geoParB(nullptr),
  m_geoParE(nullptr),
  m_TransformData(nullptr),
  m_HeventT0_0(nullptr),
  m_HeventT0_1(nullptr),
  m_HnumTrack(nullptr),
  m_HnBHit2dOfTrack(nullptr),
  m_HnEHit2dOfTrack(nullptr),
  m_HpositionDiff{nullptr},
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
  addParam("inputParticleList", m_inputListName, "input particle list.", std::string("mu+:cali"));
  addParam("useEventT0", m_useEvtT0, "Use event T0 or not.", true);
  addParam("IgnoreBackwardPropagation", m_IgnoreBackwardPropagation,
           "Whether to ignore ExtHits with backward propagation.", false);

  m_elementNum = &(KLMElementNumbers::Instance());
}

KLMTimeCollectorModule::~KLMTimeCollectorModule()
{
  B2INFO("Destructor done..");
}


void KLMTimeCollectorModule::prepare()
{
  /* Initialize geometry. */
  //B2INFO("prepare :: Initialize geometry..");
  m_geoParB = bklm::GeometryPar::instance();
  m_geoParE = &(EKLM::GeometryData::Instance());
  m_TransformData = new EKLM::TransformData(true, EKLM::TransformData::c_None);

  /* Require input dataobjects. */
  //B2INFO("prepare :: Require input dataobjects..");
  if (m_useEvtT0)
    m_eventT0.isRequired("EventT0");
  m_tracks.isRequired();

  //B2INFO("prepare :: Initialize outTree..");
  m_outTree = new TTree("time_calibration_data", "");
  m_outTree->Branch("t0",        &m_Event.t0,        "t0/D");
  m_outTree->Branch("flyTime",   &m_Event.flyTime,   "flyTime/D");
  m_outTree->Branch("recTime",   &m_Event.recTime,   "recTime/D");
  m_outTree->Branch("dist",      &m_Event.dist,      "dist/D");
  m_outTree->Branch("diffDistX", &m_Event.diffDistX, "diffDistX/D");
  m_outTree->Branch("diffDistY", &m_Event.diffDistY, "diffDistY/D");
  m_outTree->Branch("diffDistZ", &m_Event.diffDistZ, "diffDistZ/D");
  m_outTree->Branch("eDep",      &m_Event.eDep,      "eDep/D");
  m_outTree->Branch("nPE",       &m_Event.nPE,       "nPE/D");
  m_outTree->Branch("channelId", &m_Event.channelId, "channelId/I");
  m_outTree->Branch("inRPC",     &m_Event.inRPC,     "inRPC/O");
  m_outTree->Branch("isFlipped", &m_Event.isFlipped, "isFlipped/O");

  registerObject<TTree>("time_calibration_data", m_outTree);

  B2DEBUG(20, "prepare :: Initialize histgrams..");
  m_HeventT0_0 = new TH1D("m_HeventT0_0", "collision time before track number request;t0[ns]", 200, -100, 100);
  m_HeventT0_1 = new TH1D("m_HeventT0_1", "collision time after track number request;t0[ns]", 200, -100, 100);
  m_HnumTrack = new TH1I("m_HnnumTrack", "Number of Track;nTrack", 30, 0, 30);

  m_HnBHit2dOfTrack = new TH1I("m_HnBKLMHit2dOfTrack", "Number of BKLMHit2d belong to recTrack;num of BKLMHit2d", 20, 0, 20);
  m_HnEHit2dOfTrack = new TH1I("m_HnEKLMHit2dOfTrack", "Number of EKLMHit2d belong to recTrack;num of EKLMHit2d", 15, 0, 15);

  m_HpositionDiff = new TH1D("m_HpositionDiff", "Dist between extHit and KLMHit2d;dist", 160, 0, 8);
  m_HpositionXDiff = new TH1D("m_HpositionXDiff", "DistX between extHit and KLMHit2d;distX", 100, 0, 5);
  m_HpositionYDiff = new TH1D("m_HpositionYDiff", "DistY between extHit and KLMHit2d;distY", 100, 0, 5);
  m_HpositionZDiff = new TH1D("m_HpositionZDiff", "DistZ between extHit and KLMHit2d;distZ", 100, 0, 5);

  m_HflyTimeB = new TH2D("m_HflyTimeB", "flyTime;flyTime(wet)/ns;Layer", 40, 0, 20, 20, 0, 20);
  m_HflyTimeE = new TH2D("m_HflyTimeE", "flyTime;flyTime(wet)/ns;Layer", 40, 0, 20, 20, 0, 20);

  m_HnumDigit_rpc = new TH1I("m_HnumDigit_rpc", "Number of digit per bklmHit1d (RPC);number of digit", 15, 0, 15);
  m_HnumDigit_scint = new TH1I("m_HnumDigit_scint", "Number of digit per bklmHit1d (scint);number of digit", 15, 0, 15);
  m_HnumDigit_scint_end = new TH1I("m_HnumDigit_scint_end", "Number of eklmDigit per eklmHit2d (scint);number of eklmDigit", 15, 0,
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

void KLMTimeCollectorModule::collect()
{
  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);

  m_Event.t0 = 0.0;
  /* Require event T0 determined from CDC */
  if (m_useEvtT0) {
    if (!m_eventT0.isValid())
      return;
    if (!m_eventT0->hasTemporaryEventT0(Const::EDetector::CDC))
      return;
    const std::vector<EventT0::EventT0Component> evtT0C = m_eventT0->getTemporaryEventT0s(Const::EDetector::CDC);
    m_Event.t0 = evtT0C.back().eventT0;
  }

  /* Read data meta infor */
  int runId = eventMetaData->getRun();
  int evtId = eventMetaData->getEvent();

  getObjectPtr<TH1D>("m_HevtT0_0")->Fill(m_Event.t0);

  const StoreObjPtr<ParticleList> inputList(m_inputListName);
  unsigned n_track =  inputList->getListSize();
  if (n_track < 1) {
    B2DEBUG(20, "No necessary tracks in" << LogVar("run", runId) << LogVar("event", evtId));
    return;
  }  // track existence

  B2DEBUG(20, "debug infor for" << LogVar("run", runId) << LogVar("event", evtId) << LogVar("number of rec tracks", n_track));

  getObjectPtr<TH1D>("m_HevtT0_1")->Fill(m_Event.t0);
  getObjectPtr<TH1I>("m_HnTrack")->Fill(n_track);

  /* Here begins the ext track sequence */
  B2DEBUG(20, "Collect :: Track loop begins.");

  /* Main loop */
  for (unsigned iT = 0; iT < n_track; ++iT) {
    // Good track selection
    const Particle* particle = inputList->getParticle(iT);
    const Track* track = particle->getTrack();

    // Find data objects related to track
    RelationVector<ExtHit> extHits = track->getRelationsTo<ExtHit>();
    RelationVector<BKLMHit2d> bklmHit2ds = track->getRelationsTo<BKLMHit2d>();
    RelationVector<EKLMHit2d> eklmHit2ds = track->getRelationsTo<EKLMHit2d>();

    getObjectPtr<TH1I>("m_HnBHit2d")->Fill(int(bklmHit2ds.size()));
    getObjectPtr<TH1I>("m_HnEHit2d")->Fill(int(eklmHit2ds.size()));

    B2DEBUG(20, "Track" << LogVar("exthits", extHits.size())
            << LogVar("BKLMHit2d", bklmHit2ds.size()) << LogVar("EKLMHit2d", eklmHit2ds.size()));
    if (eklmHit2ds.size() < 2 && bklmHit2ds.size() < 2)
      continue;

    // Loop for extroplate hits
    m_vExtHits.clear();
    m_vExtHits_RPC.clear();
    B2DEBUG(20, "Collect :: extHits loop begins.");
    for (const ExtHit& eHit : extHits) {
      if (eHit.getStatus() != EXT_EXIT)
        continue;

      if (m_IgnoreBackwardPropagation) {
        if (eHit.isBackwardPropagated())
          continue;
      }

      bool bklmCover = (eHit.getDetectorID() == Const::EDetector::BKLM);
      bool eklmCover = (eHit.getDetectorID() == Const::EDetector::EKLM);
      if (!bklmCover && !eklmCover)
        continue;

      int copyId = eHit.getCopyID();
      int tFor, tSec, tLay, tPla, tStr;
      int tSub = -1;

      if (eklmCover) {
        tSub = KLMElementNumbers::c_EKLM;
        EKLMElementNumbers::Instance().stripNumberToElementNumbers(copyId, &tFor, &tLay, &tSec, &tPla, &tStr);
      }
      if (bklmCover) {
        tSub = KLMElementNumbers::c_BKLM;
        BKLMElementNumbers::channelNumberToElementNumbers(copyId, &tFor, &tSec, &tLay, &tPla, &tStr);
      }
      if (tSub < 0)
        continue;
      B2DEBUG(20, "Collect :: Assign elementNum based on copyId for extHits." << LogVar("Sub from elementNumber",
              tSub) << LogVar("bklmCover", bklmCover) << LogVar("eklmCover", eklmCover));

      bool crossed = false; // should be only once ?
      KLMMuidLikelihood* muidLikelihood = track->getRelatedTo<KLMMuidLikelihood>();
      if (bklmCover) {
        crossed = muidLikelihood->isExtrapolatedBarrelLayerCrossed(tLay - 1);
        if (crossed) {
          if (tLay > 2) {
            unsigned int tModule = m_elementNum->moduleNumber(tSub, tFor, tSec, tLay);
            m_vExtHits_RPC.insert(std::pair<unsigned int, ExtHit>(tModule, eHit));
          } else {
            unsigned int tChannel = m_elementNum->channelNumber(tSub, tFor, tSec, tLay, tPla, tStr);
            if (m_channelStatus->getChannelStatus(tChannel) != KLMChannelStatus::c_Normal)
              continue;
            m_vExtHits.insert(std::pair<unsigned int, ExtHit>(tChannel, eHit));
          }
        }
      }
      if (eklmCover) {
        crossed = muidLikelihood->isExtrapolatedEndcapLayerCrossed(tLay - 1);
        if (crossed) {
          unsigned int tChannel = m_elementNum->channelNumber(tSub, tFor, tSec, tLay, tPla, tStr);
          if (m_channelStatus->getChannelStatus(tChannel) != KLMChannelStatus::c_Normal)
            continue;
          m_vExtHits.insert(std::pair<unsigned int, ExtHit>(tChannel, eHit));
        }
      }
    }

    //B2INFO("Hits Collection Done."
    //        << LogVar("exthits in BKLM", m_vExtHitsB.size()) << LogVar("BKLMHit2d", bklmHit2ds.size())
    //        << LogVar("exthits in EKLM", m_vExtHitsE.size()) << LogVar("EKLMHit2d", eklmHit2ds.size()));

    if (m_vExtHits.size() > 0) {
      collectScintEnd(eklmHit2ds);
      collectScint(bklmHit2ds);
    }
    if (m_vExtHits_RPC.size() > 0) {
      collectRPC(bklmHit2ds);
    }
  }
}

void KLMTimeCollectorModule::collectScintEnd(RelationVector<EKLMHit2d>& eklmHit2ds)
{
  const HepGeom::Transform3D* tr;
  HepGeom::Point3D<double> hitGlobal_extHit, hitLocal_extHit;
  double l;

  for (const EKLMHit2d& hit2d : eklmHit2ds) {
    RelationVector<KLMDigit> digits = hit2d.getRelationsTo<KLMDigit>();
    unsigned nDigit = digits.size();
    getObjectPtr<TH1I>("m_HnDigit_scint_end")->Fill(nDigit);
    if (nDigit != 2)
      B2FATAL("Wrong number of related KLMDigits.");

    if (!digits[0]->isGood() || !digits[1]->isGood())
      continue;

    for (KLMDigit& digitHit : digits) {
      m_Event.channelId = digitHit.getUniqueChannelID();
      m_Event.inRPC = 0;
      if (m_channelStatus->getChannelStatus(m_Event.channelId) != KLMChannelStatus::c_Normal)
        continue;

      std::pair<ExtHit*, ExtHit*> pair_extHits = matchExt(m_Event.channelId, m_vExtHits);
      if (pair_extHits.first == nullptr || pair_extHits.second == nullptr)
        continue;
      ExtHit& entryHit = *(pair_extHits.first);
      ExtHit& exitHit = *(pair_extHits.second);

      m_Event.flyTime = 0.5 * (entryHit.getTOF() + exitHit.getTOF());

      TVector3 positionGlobal_extHit = 0.5 * (entryHit.getPosition() + exitHit.getPosition());
      TVector3 positionGlobal_digit = hit2d.getPosition();
      TVector3 positionGlobal_diff = positionGlobal_extHit - positionGlobal_digit;

      storeDistDiff(positionGlobal_diff);

      l = m_geoParE->getStripLength(digitHit.getStrip()) / CLHEP::mm * Unit::mm;
      hitGlobal_extHit.setX(positionGlobal_extHit.X() / Unit::mm * CLHEP::mm);
      hitGlobal_extHit.setY(positionGlobal_extHit.Y() / Unit::mm * CLHEP::mm);
      hitGlobal_extHit.setZ(positionGlobal_extHit.Z() / Unit::mm * CLHEP::mm);
      tr = m_TransformData->getStripGlobalToLocal(&digitHit);
      hitLocal_extHit = (*tr) * hitGlobal_extHit;

      m_Event.dist = 0.5 * l - hitLocal_extHit.x() / CLHEP::mm * Unit::mm;
      m_Event.recTime = digitHit.getTime();
      m_Event.eDep = digitHit.getEnergyDeposit();
      m_Event.nPE = digitHit.getNPhotoelectrons();

      getObjectPtr<TH2D>("m_HfTimeE")->Fill(m_Event.flyTime, digitHit.getLayer());
      getObjectPtr<TTree>("time_calibration_data")->Fill();
    }
  }
}

void KLMTimeCollectorModule::collectScint(RelationVector<BKLMHit2d>& bklmHit2ds)
{
  double stripWidtm_HZ, stripWidtm_HPhi;
  for (BKLMHit2d& hit2d : bklmHit2ds) {
    if (hit2d.inRPC())
      continue;
    if (hit2d.isOutOfTime())
      continue;

    RelationVector<BKLMHit1d> bklmHit1ds = hit2d.getRelationsTo<BKLMHit1d>();
    if (bklmHit1ds.size() != 2)
      continue;

    TVector3 positionGlobal_hit2d = hit2d.getGlobalPosition();
    const bklm::Module* corMod = m_geoParB->findModule(hit2d.getSection(), hit2d.getSector(), hit2d.getLayer());
    stripWidtm_HZ = corMod->getZStripWidth();
    stripWidtm_HPhi = corMod->getPhiStripWidth();

    for (const BKLMHit1d& hit1d : bklmHit1ds) {
      RelationVector<KLMDigit> digits = hit1d.getRelationsTo<KLMDigit>();
      getObjectPtr<TH1I>("m_HnDigit_scint")->Fill(digits.size());
      if (digits.size() > 3)
        continue;

      for (const KLMDigit& digitHit : digits) {
        if (digitHit.inRPC() || !digitHit.isGood())
          continue;

        unsigned int channelId_digit = digitHit.getUniqueChannelID();
        if (m_channelStatus->getChannelStatus(channelId_digit) != KLMChannelStatus::c_Normal)
          continue;

        std::pair<ExtHit*, ExtHit*> pair_extHits = matchExt(channelId_digit, m_vExtHits);
        if (pair_extHits.first == nullptr || pair_extHits.second == nullptr)
          continue;
        ExtHit& entryHit = *(pair_extHits.first);
        ExtHit& exitHit = *(pair_extHits.second);

        m_Event.inRPC = digitHit.inRPC();
        m_Event.flyTime = 0.5 * (entryHit.getTOF() + exitHit.getTOF());

        TVector3 positionGlobal_extHit = 0.5 * (entryHit.getPosition() + exitHit.getPosition());
        TVector3 positionGlobal_diff = positionGlobal_extHit - positionGlobal_hit2d;
        B2DEBUG(20, LogVar("Distance between digit and hit2d", positionGlobal_diff.Mag()));

        storeDistDiff(positionGlobal_diff);
        const CLHEP::Hep3Vector positionLocal_extHit = corMod->globalToLocal(CLHEP::Hep3Vector(
                                                         positionGlobal_extHit.X(), positionGlobal_extHit.Y(), positionGlobal_extHit.Z()), true);
        const CLHEP::Hep3Vector positionLocal_hit2d = corMod->globalToLocal(CLHEP::Hep3Vector(
                                                        positionGlobal_hit2d.X(), positionGlobal_hit2d.Y(), positionGlobal_hit2d.Z()), true);
        const CLHEP::Hep3Vector diffLocal = positionLocal_extHit - positionLocal_hit2d;
        if (fabs(diffLocal.z()) > stripWidtm_HZ  || fabs(diffLocal.y()) > stripWidtm_HPhi)
          continue;

        double propaLength = corMod->getPropagationDistance(positionLocal_extHit, digitHit.getStrip(), digitHit.isPhiReadout());

        m_Event.isFlipped = corMod->isFlipped();
        m_Event.recTime = digitHit.getTime();
        m_Event.dist = propaLength;
        m_Event.eDep = digitHit.getEnergyDeposit();
        m_Event.nPE = digitHit.getNPhotoelectrons();
        m_Event.channelId = channelId_digit;

        getObjectPtr<TH2D>("m_HfTimeB")->Fill(m_Event.flyTime, digitHit.getLayer());
        getObjectPtr<TTree>("time_calibration_data")->Fill();
      }
    }
  }
}

void KLMTimeCollectorModule::collectRPC(RelationVector<BKLMHit2d>& bklmHit2ds)
{
  for (BKLMHit2d& hit2d : bklmHit2ds) {
    if (!hit2d.inRPC())
      continue;
    if (hit2d.isOutOfTime())
      continue;
    RelationVector<BKLMHit1d> bklmHit1ds = hit2d.getRelationsTo<BKLMHit1d>();
    if (bklmHit1ds.size() != 2)
      continue;

    TVector3 positionGlobal_hit2d = hit2d.getGlobalPosition();
    const bklm::Module* corMod = m_geoParB->findModule(hit2d.getSection(), hit2d.getSector(), hit2d.getLayer());
    double stripWidtm_HZ = corMod->getZStripWidth();
    double stripWidtm_HPhi = corMod->getPhiStripWidth();

    for (const BKLMHit1d& hit1d : bklmHit1ds) {
      RelationVector<KLMDigit> digits = hit1d.getRelationsTo<KLMDigit>();
      getObjectPtr<TH1I>("m_HnDigit_rpc")->Fill(digits.size());
      if (digits.size() > 5)
        continue;

      for (const KLMDigit& digitHit : digits) {
        unsigned int channelId_digit = digitHit.getUniqueChannelID();
        m_Event.inRPC = digitHit.inRPC();
        if (m_channelStatus->getChannelStatus(channelId_digit) != KLMChannelStatus::c_Normal)
          continue;
        unsigned int tModule = m_elementNum->moduleNumber(digitHit.getSubdetector(), digitHit.getSection(), digitHit.getSector(),
                                                          digitHit.getLayer());

        std::pair<ExtHit*, ExtHit*> pair_extHits = matchExt(tModule, m_vExtHits_RPC);
        if (pair_extHits.first == nullptr || pair_extHits.second == nullptr)
          continue;
        ExtHit& entryHit = *(pair_extHits.first);
        ExtHit& exitHit = *(pair_extHits.second);

        m_Event.flyTime = 0.5 * (entryHit.getTOF() + exitHit.getTOF());
        TVector3 positionGlobal_extHit = 0.5 * (entryHit.getPosition() + exitHit.getPosition());
        TVector3 positionGlobal_diff = positionGlobal_extHit - positionGlobal_hit2d;
        B2DEBUG(20, LogVar("Distance between digit and hit2d", positionGlobal_diff.Mag()));

        storeDistDiff(positionGlobal_diff);
        const CLHEP::Hep3Vector positionLocal_extHit = corMod->globalToLocal(CLHEP::Hep3Vector(
                                                         positionGlobal_extHit.X(), positionGlobal_extHit.Y(), positionGlobal_extHit.Z()), true);
        const CLHEP::Hep3Vector positionLocal_hit2d = corMod->globalToLocal(CLHEP::Hep3Vector(
                                                        positionGlobal_hit2d.X(), positionGlobal_hit2d.Y(), positionGlobal_hit2d.Z()), true);
        const CLHEP::Hep3Vector diffLocal = positionLocal_extHit - positionLocal_hit2d;
        if (fabs(diffLocal.z()) > stripWidtm_HZ  || fabs(diffLocal.y()) > stripWidtm_HPhi)
          continue;

        const CLHEP::Hep3Vector propaLengthV3 = corMod->getPropagationDistance(positionLocal_extHit);
        double propaLength = propaLengthV3[2 - int(hit1d.isPhiReadout())];

        m_Event.isFlipped = corMod->isFlipped();
        m_Event.recTime = digitHit.getTime();
        m_Event.dist = propaLength;
        m_Event.eDep = digitHit.getEnergyDeposit();
        m_Event.nPE = digitHit.getNPhotoelectrons();
        m_Event.channelId = channelId_digit;

        getObjectPtr<TH2D>("m_HfTimeB")->Fill(m_Event.flyTime, digitHit.getLayer());
        getObjectPtr<TTree>("time_calibration_data")->Fill();
      }
    }
  }
}

std::pair<ExtHit*, ExtHit*> KLMTimeCollectorModule::matchExt(uint16_t kID,
    std::multimap<unsigned int, ExtHit>& v_ExtHits)
{
  ExtHit* entryHit = nullptr;
  ExtHit* exitHit = nullptr;
  std::multimap<unsigned int, ExtHit>::iterator it, itlow, itup;
  itlow = v_ExtHits.lower_bound(kID);
  itup = v_ExtHits.upper_bound(kID);

  for (it = itlow; it != itup; ++it) {
    if (entryHit == nullptr) {
      entryHit = &(it->second);
    } else if ((it->second).getTOF() < entryHit->getTOF()) {
      entryHit = &(it->second);
    }
    if (exitHit == nullptr) {
      exitHit = &(it->second);
    } else if ((it->second).getTOF() > exitHit->getTOF()) {
      exitHit = &(it->second);
    }
  }

  // switch turn on when entry status avaiable
  /*
  switch (eHit.getStatus()) {
    case EXT_ENTER:
      if (entryHit == NULL) {
        entryHit = eHit;
      } else if (eHit->getTOF() < entryHit.getTOF()) {
        entryHit = eHit;
      }
      break;
    case EXT_EXIT:
      if (exitHit == NULL) {
        exitHit = eHit;
      } else if (eHit.getTOF() > exitHit.getTOF()) {
        exitHit = eHit;
      }
      break;
    default:
      break;
  }
  */
  std::pair<ExtHit*, ExtHit*> p_extHits = std::make_pair(entryHit, exitHit);
  return p_extHits;
}

void KLMTimeCollectorModule::storeDistDiff(TVector3& pDiff)
{
  double diffM = pDiff.Mag();
  double diffX = pDiff.X();
  double diffY = pDiff.Y();
  double diffZ = pDiff.Z();
  getObjectPtr<TH1D>("m_HposiDiff")->Fill(diffM);
  getObjectPtr<TH1D>("m_HposiXDiff")->Fill(diffX);
  getObjectPtr<TH1D>("m_HposiYDiff")->Fill(diffY);
  getObjectPtr<TH1D>("m_HposiZDiff")->Fill(diffZ);
  m_Event.diffDistX = diffX;
  m_Event.diffDistY = diffY;
  m_Event.diffDistZ = diffZ;
}

void KLMTimeCollectorModule::finish()
{
  B2INFO("Data Collection Done.");
}
