/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jincheng Mei                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* ROOT headers. */
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH1I.h>
#include <TH2D.h>

/* C++ containers. */
#include <vector>
#include <map>

/* Belle2 headers. */
#include <klm/modules/KLMTimeCalibration/KLMTimeCalibrationCollectorModule.h>
#include <klm/dataobjects/KLMElementNumbers.h>

#include <klm/bklm/dataobjects/BKLMDigit.h>
#include <klm/bklm/dataobjects/BKLMHit1d.h>
#include <klm/bklm/dataobjects/BKLMHit2d.h>
#include <klm/eklm/dataobjects/EKLMDigit.h>
#include <klm/eklm/dataobjects/EKLMHit2d.h>

#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/eklm/geometry/GeometryData.h>
#include <klm/eklm/geometry/TransformData.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternCDC.h>

#include <tracking/dataobjects/Muid.h>
#include <tracking/dataobjects/MuidHit.h>
#include <tracking/dataobjects/ExtHit.h>

#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/logging/LogSystem.h>
#include <framework/dataobjects/EventT0.h>



using namespace Belle2;
using namespace Belle2::bklm;
using namespace Belle2::EKLM;

REG_MODULE(KLMTimeCalibrationCollector)

KLMTimeCalibrationCollectorModule::KLMTimeCalibrationCollectorModule() :
  CalibrationCollectorModule()
{
  setDescription("Module for BKLM time calibration (data collection).");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("deBug", m_debug, "debug modle.", false);

  m_geoParB = nullptr;
  m_geoParE = nullptr;
  m_TransformData = nullptr;
  m_elementNum = nullptr;
  m_outTree = nullptr;
  m_debug = false;
}

KLMTimeCalibrationCollectorModule::~KLMTimeCalibrationCollectorModule()
{
}

void KLMTimeCalibrationCollectorModule::prepare()
{
  setDescription("Preparation for BKLM TimeCalibration Collector Module.");
  m_geoParB = GeometryPar::instance();
  m_geoParE = &(EKLM::GeometryData::Instance());
  m_TransformData = new EKLM::TransformData(true, EKLM::TransformData::c_None);
  m_elementNum = &(KLMElementNumbers::Instance());

  m_tracks.isRequired();

  m_eventT0.isRequired("EventT0");

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
  //m_outTree->Branch("forward",   &m_ev.forward,   "forward/I");
  //m_outTree->Branch("sector",    &m_ev.sector,    "sector/I");
  //m_outTree->Branch("layer",     &m_ev.layer,     "layer/I");
  //m_outTree->Branch("plane",     &m_ev.plane,     "plane/I");
  //m_outTree->Branch("strip",     &m_ev.strip,     "strip/I");
  m_outTree->Branch("inRPC",     &m_ev.inRPC,     "inRPC/O");
  m_outTree->Branch("isFlipped", &m_ev.isFlipped, "isFlipped/O");

  registerObject<TTree>("time_calibration_data", m_outTree);

  h_eventT0_0 = new TH1D("h_eventT0_0", "collision time before track number request;t0[ns]", 200, -100, 100);
  h_eventT0_1 = new TH1D("h_eventT0_1", "collision time after track number request;t0[ns]", 200, -100, 100);
  h_numTrack = new TH1I("h_nnumTrack", "Number of Track;nTrack", 30, 0, 30);

  h_nBHit2dOfTrack = new TH1I("h_nBKLMHit2dOfTrack", "Number of BKLMHit2d belong to recTrack;num of BKLMHit2d", 20, 0, 20);
  h_nEHit2dOfTrack = new TH1I("h_nEKLMHit2dOfTrack", "Number of EKLMHit2d belong to recTrack;num of EKLMHit2d", 15, 0, 15);

  h_positionDiff = new TH1D("h_positionDiff", "Dist between extHit and KLMHit2d;dist", 160, 0, 8);
  h_positionXDiff = new TH1D("h_positionXDiff", "DistX between extHit and KLMHit2d;distX", 100, 0, 5);
  h_positionYDiff = new TH1D("h_positionYDiff", "DistY between extHit and KLMHit2d;distY", 100, 0, 5);
  h_positionZDiff = new TH1D("h_positionZDiff", "DistZ between extHit and KLMHit2d;distZ", 100, 0, 5);

  h_flyTimeB = new TH2D("h_flyTimeB", "flyTime;flyTime(wet)/ns;Layer", 32, 0, 16, 20, 0, 20);
  h_flyTimeE = new TH2D("h_flyTimeE", "flyTime;flyTime(wet)/ns;Layer", 32, 0, 16, 20, 0, 20);

  h_numDigit_rpc = new TH1I("h_numDigit_rpc", "Number of digit per bklmHit1d (RPC);number of digit", 15, 0, 15);
  h_numDigit_scint = new TH1I("h_numDigit_scint", "Number of digit per bklmHit1d (scint);number of digit", 15, 0, 15);
  h_numDigit_scint_end = new TH1I("h_numDigit_scint_end", "Number of eklmDigit per eklmHit1d (scint);number of eklmDigit", 15, 0, 15);

  registerObject<TH1D>("h_evtT0_0", h_eventT0_0);
  registerObject<TH1D>("h_evtT0_1", h_eventT0_1);
  registerObject<TH1I>("h_nTrack", h_numTrack);
  registerObject<TH1I>("h_nBHit2d", h_nBHit2dOfTrack);
  registerObject<TH1I>("h_nEHit2d", h_nEHit2dOfTrack);

  registerObject<TH1D>("h_posiDiff", h_positionDiff);
  registerObject<TH1D>("h_posiXDiff", h_positionXDiff);
  registerObject<TH1D>("h_posiYDiff", h_positionYDiff);
  registerObject<TH1D>("h_posiZDiff", h_positionZDiff);

  registerObject<TH2D>("h_fTimeB", h_flyTimeB);
  registerObject<TH2D>("h_fTimeE", h_flyTimeE);

  registerObject<TH1I>("h_nDigit_rpc", h_numDigit_rpc);
  registerObject<TH1I>("h_nDigit_scint", h_numDigit_scint);
  registerObject<TH1I>("h_nDigit_scint_end", h_numDigit_scint_end);
}

void KLMTimeCalibrationCollectorModule::collect()
{
  B2INFO("Time Calibration Collector: collect begins");

  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);

  if (!m_eventT0.isValid()) return;
  if (!m_eventT0->hasTemporaryEventT0(Const::EDetector::CDC)) return;

  const std::vector<EventT0::EventT0Component> evtT0C = m_eventT0->getTemporaryEventT0s(Const::EDetector::CDC);
  int runId = eventMetaData->getRun();
  int evtId = eventMetaData->getEvent();

  m_ev.t0 = evtT0C.back().eventT0;
  getObjectPtr<TH1D>("h_evtT0_0")->Fill(m_ev.t0);
  unsigned n_track =  m_tracks.getEntries();
  if (n_track < 1) {
    B2DEBUG(20, "No necessary tracks in" << LogVar("run", runId) << LogVar("event", evtId));
    return;
  }  // track existence

  B2DEBUG(20, "debug infor for" << LogVar("run", runId) << LogVar("event", evtId) << LogVar("number of rec tracks", n_track));

  getObjectPtr<TH1D>("h_evtT0_1")->Fill(m_ev.t0);
  getObjectPtr<TH1I>("h_nTrack")->Fill(n_track);

  /** Here begins the ext track sequence */
  B2DEBUG(20, "Track loop begins");

  for (unsigned iT = 0; iT < n_track; ++iT) {
    // Good track selection
    Track* track = m_tracks[iT];
    const TrackFitResult* tFtr = track->getTrackFitResultWithClosestMass(Const::muon);
    double d_zero = tFtr->getD0();
    double z_zero = tFtr->getZ0();
    int nCDChits = tFtr->getHitPatternCDC().getNHits() ;
    if (fabs(d_zero) > 0.5 || fabs(z_zero) > 2.0) continue;
    if (tFtr->getMomentum().Mag() < 0.55) continue;
    if (nCDChits < 20) continue;

    // Find data objects related to track
    RelationVector<ExtHit> extHits = track->getRelationsTo<ExtHit>();
    RelationVector<BKLMHit2d> bklmHit2ds = track->getRelationsTo<BKLMHit2d>();
    RelationVector<EKLMHit2d> eklmHit2ds = track->getRelationsTo<EKLMHit2d>();
    RelationVector<Muid> Muids = track->getRelationsTo<Muid>();

    getObjectPtr<TH1I>("h_nHit2d_bklm")->Fill(int(bklmHit2ds.size()));
    getObjectPtr<TH1I>("h_nHit2d_eklm")->Fill(int(eklmHit2ds.size()));

    if (iT < 10) {
      B2INFO("Track" << LogVar("exthits", extHits.size())
             << LogVar("BKLMHit2d", bklmHit2ds.size()) << LogVar("EKLMHit2d", eklmHit2ds.size()));
    }
    if (eklmHit2ds.size() < 2 && bklmHit2ds.size() < 2) continue;

    map_extHits.clear();
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
      for (unsigned int mu = 0; mu < Muids.size(); mu++) {
        Muid* muid =  Muids[mu];
        int extPattern = muid->getExtLayerPattern();
        if ((extPattern & (1 << (tLay - 1))) != 0)  crossed = true;
        if (crossed) break;
      }
      if (!crossed) continue;

      map_extHits.insert(std::pair<int, ExtHit*>(copyId, extHit));
    }

    B2DEBUG(20, "In KLM coverage: " << LogVar("exthits", map_extHits.size())
            << LogVar("BKLMHit2d", bklmHit2ds.size()) << LogVar("EKLMHit2d", eklmHit2ds.size()));
    if (map_extHits.size() < 1) continue;

    collect_RPC(bklmHit2ds);
    collect_scint(bklmHit2ds);
    collect_scint_end(eklmHit2ds);
  }
}

void KLMTimeCalibrationCollectorModule::collect_scint_end(RelationVector<EKLMHit2d> hit2ds)
{

  const HepGeom::Transform3D* tr;
  HepGeom::Point3D<double> hitGlobal_extHit, hitLocal_extHit;
  double l;
  int iSub, iFor, iSec, iLay, iPla, iStr;

  for (unsigned i = 0; i < hit2ds.size(); ++i) {
    EKLMHit2d* hit2d = hit2ds[i];

    RelationVector<EKLMDigit> digits = hit2d->getRelationsTo<EKLMDigit>();
    unsigned nDigit = digits.size();
    if (nDigit != 2)
      B2FATAL("Wrong number of related EKLMDigits.");
    if (digits[0]->getNPE() == 0 || digits[1]->getNPE() == 0) continue;
    if (!digits[0]->isGood() || !digits[1]->isGood()) continue;

    for (unsigned j = 0; j < nDigit; ++j) {
      EKLMDigit* digitHit = digits[j];
      unsigned int localID = digitHit->getUniqueChannelID();
      m_ev.channelId = m_elementNum->channelNumberEKLM(localID);
      m_ev.inRPC = 0;

      std::multimap<int, ExtHit*>::iterator it;
      ExtHit* extHit   = nullptr;
      ExtHit* entryHit = nullptr;
      ExtHit* exitHit  = nullptr;
      for (it = map_extHits.begin(); it != map_extHits.end(); ++it) {
        extHit = it->second;
        int copyid = extHit->getCopyID();
        m_elementNum->channelNumberToElementNumbers(copyid, &iSub, &iFor, &iSec, &iLay, &iPla, &iStr);
        uint16_t globalID_extHit = m_elementNum->channelNumberEKLM(iFor, iSec, iLay, iPla, iStr);

        B2INFO("EKLM element numbers check.\n" <<
               "ExtHit copyID :: Digit channelID " << copyid << "  " << m_ev.channelId);

        if (m_ev.channelId != globalID_extHit) continue;

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
      if (entryHit == nullptr || exitHit == nullptr) continue;

      m_ev.flyTime = 0.5 * (entryHit->getTOF() + exitHit->getTOF());

      TVector3 positionGlobal_extHit = 0.5 * (entryHit->getPosition() + exitHit->getPosition());
      TVector3 positionGlobal_digit = digitHit->getPosition();
      TVector3 positionGlobal_diff = positionGlobal_extHit - positionGlobal_digit;

      double diffM = positionGlobal_diff.Mag();
      double diffX = positionGlobal_diff.X();
      double diffY = positionGlobal_diff.Y();
      double diffZ = positionGlobal_diff.Z();
      getObjectPtr<TH1D>("h_posiDiff")->Fill(diffM);
      getObjectPtr<TH1D>("h_posiXDiff")->Fill(diffX);
      getObjectPtr<TH1D>("h_posiYDiff")->Fill(diffY);
      getObjectPtr<TH1D>("h_posiZDiff")->Fill(diffZ);
      //if (diffM > 10.0) continue;

      l = m_geoParE->getStripLength(digitHit->getStrip()) / CLHEP::mm * Unit::mm;
      hitGlobal_extHit.setX(positionGlobal_extHit.X() / Unit::mm * CLHEP::mm);
      hitGlobal_extHit.setY(positionGlobal_extHit.Y() / Unit::mm * CLHEP::mm);
      hitGlobal_extHit.setZ(positionGlobal_extHit.Z() / Unit::mm * CLHEP::mm);
      tr = m_TransformData->getStripGlobalToLocal(digitHit);
      hitLocal_extHit = (*tr) * hitGlobal_extHit;

      m_ev.dist = 0.5 * l - hitLocal_extHit.x() / CLHEP::mm * Unit::mm;

      m_ev.recTime = digitHit->getTime();
      m_ev.eDep = digitHit->getCharge();
      m_ev.nPE = digitHit->getNPE();

      getObjectPtr<TH2D>("h_fTime_end")->Fill(m_ev.flyTime, iLay);
      getObjectPtr<TTree>("time_calibration_data")->Fill();
    }
  }
}

void KLMTimeCalibrationCollectorModule::collect_scint(RelationVector<BKLMHit2d> bklmHit2ds)
{
  int iSub, iFor, iSec, iLay, iPla, iStr;
  double stripWidth_Z, stripWidth_Phi;

  for (unsigned iH2 = 0; iH2 < bklmHit2ds.size(); ++iH2) {
    BKLMHit2d* hit2d = bklmHit2ds[iH2];
    if (hit2d->inRPC()) continue;
    if (hit2d->isOutOfTime()) continue;

    RelationVector<BKLMHit1d> bklmHit1ds = hit2d->getRelationsTo<BKLMHit1d>();
    if (bklmHit1ds.size() != 2) continue;
    TVector3 positionGlobal_hit2d = hit2d->getGlobalPosition();

    const bklm::Module* corMod = m_geoParB->findModule(hit2d->getSection(), hit2d->getSector(), hit2d->getLayer());
    stripWidth_Z = corMod->getZStripWidth();
    stripWidth_Phi = corMod->getPhiStripWidth();
    for (unsigned iH1 = 0; iH1 < bklmHit1ds.size(); ++iH1) {
      BKLMHit1d* hit1d = bklmHit1ds[iH1];
      RelationVector<BKLMDigit> digits = hit1d->getRelationsTo<BKLMDigit>();
      getObjectPtr<TH1I>("h_nDigit_scint")->Fill(digits.size());
      if (digits.size() > 5) continue;

      for (unsigned iHd = 0; iHd < digits.size(); ++iHd) {
        BKLMDigit* digitHit = digits[iHd];
        if (digitHit->inRPC() || !digitHit->isAboveThreshold()) continue;

        m_ev.inRPC = digitHit->inRPC();

        uint16_t channelId_digit = m_elementNum->channelNumberBKLM(digitHit->getSection(), digitHit->getSector(), digitHit->getLayer(),
                                                                   digitHit->isPhiReadout(), digitHit->getStrip());

        std::multimap<int, ExtHit*>::iterator it;
        ExtHit* extHit   = nullptr;
        ExtHit* entryHit = nullptr;
        ExtHit* exitHit  = nullptr;
        for (it = map_extHits.begin(); it != map_extHits.end(); ++it) {
          extHit = it->second;
          int copyid = extHit->getCopyID();
          m_elementNum->channelNumberToElementNumbers(copyid, &iSub, &iFor, &iSec, &iLay, &iPla, &iStr);
          uint16_t globalID_extHit = m_elementNum->channelNumberBKLM(iFor, iSec, iLay, iPla, iStr);

          B2INFO("BKLM element numbers check (Scintillators).\n" <<
                 "ExtHit copyID :: Digit channelID " << copyid << "  " << m_ev.channelId);

          if (channelId_digit != globalID_extHit) continue;

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
        if (entryHit == nullptr || exitHit == nullptr) continue;

        m_ev.flyTime = 0.5 * (entryHit->getTOF() + exitHit->getTOF());

        TVector3 positionGlobal_extHit = 0.5 * (entryHit->getPosition() + exitHit->getPosition());
        TVector3 positionGlobal_diff = positionGlobal_extHit - positionGlobal_hit2d;

        B2DEBUG(29, LogVar("Distance between digit and hit2d", positionGlobal_diff.Mag()));

        double diffM = positionGlobal_diff.Mag();
        double diffX = positionGlobal_diff.X();
        double diffY = positionGlobal_diff.Y();
        double diffZ = positionGlobal_diff.Z();
        getObjectPtr<TH1D>("h_posiDiff")->Fill(diffM);
        getObjectPtr<TH1D>("h_posiXDiff")->Fill(diffX);
        getObjectPtr<TH1D>("h_posiYDiff")->Fill(diffY);
        getObjectPtr<TH1D>("h_posiZDiff")->Fill(diffZ);
        m_ev.diffDistX = diffX;
        m_ev.diffDistY = diffY;
        m_ev.diffDistZ = diffZ;
        //if (diffM > 10.0) continue;

        // If the Layer is filpped?
        m_ev.isFlipped = corMod->isFlipped();
        const CLHEP::Hep3Vector positionLocal_extHit = corMod->globalToLocal(CLHEP::Hep3Vector(
                                                         positionGlobal_extHit.X(), positionGlobal_extHit.Y(), positionGlobal_extHit.Z()), true);
        const CLHEP::Hep3Vector positionLocal_hit2d = corMod->globalToLocal(CLHEP::Hep3Vector(
                                                        positionGlobal_hit2d.X(), positionGlobal_hit2d.Y(), positionGlobal_hit2d.Z()), true);
        const CLHEP::Hep3Vector diffLocal = positionLocal_extHit - positionLocal_hit2d;
        if (fabs(diffLocal.z()) > stripWidth_Z  || fabs(diffLocal.y()) > stripWidth_Phi) continue;

        const CLHEP::Hep3Vector propaLengthV3 = corMod->getPropagationDistance(positionLocal_extHit);
        double propaLength = propaLengthV3[2 - digitHit->isPhiReadout()];

        m_ev.recTime = digitHit->getTime();
        m_ev.dist = propaLength;
        m_ev.eDep = digitHit->getEDep();
        m_ev.nPE = digitHit->getNPixel();
        m_ev.channelId = channelId_digit;

        getObjectPtr<TH2D>("h_fTime")->Fill(m_ev.flyTime, iLay);
        getObjectPtr<TTree>("time_calibration_data")->Fill();
      }
    }
  }
}

void KLMTimeCalibrationCollectorModule::collect_RPC(RelationVector<BKLMHit2d> bklmHit2ds)
{
  int iSub, iFor, iSec, iLay, iPla, iStr;

  for (unsigned iH2 = 0; iH2 < bklmHit2ds.size(); ++iH2) {
    B2DEBUG(20, "BKLMHit2d related Track loop begins");

    BKLMHit2d* hit2d = bklmHit2ds[iH2];
    if (!hit2d->inRPC()) continue;
    RelationVector<BKLMHit1d> bklmHit1ds = hit2d->getRelationsTo<BKLMHit1d>();
    if (bklmHit1ds.size() != 2) continue;
    if (hit2d->isOutOfTime()) continue;

    TVector3 positionGlobal_hit2d = hit2d->getGlobalPosition();

    std::multimap<int, ExtHit*>::iterator it;
    ExtHit* extHit   = nullptr;
    ExtHit* entryHit = nullptr;
    ExtHit* exitHit  = nullptr;
    for (it = map_extHits.begin(); it != map_extHits.end(); ++it) {
      extHit = it->second;
      int copyid = extHit->getCopyID();
      m_elementNum->channelNumberToElementNumbers(copyid, &iSub, &iFor, &iSec, &iLay, &iPla, &iStr);
      if (iFor != hit2d->getSection() || iSec != hit2d->getSector() || iLay != hit2d->getLayer()) continue;

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

    if (entryHit == nullptr || exitHit == nullptr) continue;

    m_ev.flyTime = 0.5 * (entryHit->getTOF() + exitHit->getTOF());

    TVector3 positionGlobal_extHit = 0.5 * (entryHit->getPosition() + exitHit->getPosition());
    TVector3 positionGlobal_diff = positionGlobal_extHit - positionGlobal_hit2d;

    B2DEBUG(29, LogVar("Distance between digit and hit2d", positionGlobal_diff.Mag()));

    double diffM = positionGlobal_diff.Mag();
    double diffX = positionGlobal_diff.X();
    double diffY = positionGlobal_diff.Y();
    double diffZ = positionGlobal_diff.Z();
    getObjectPtr<TH1D>("h_posiDiff")->Fill(diffM);
    getObjectPtr<TH1D>("h_posiXDiff")->Fill(diffX);
    getObjectPtr<TH1D>("h_posiYDiff")->Fill(diffY);
    getObjectPtr<TH1D>("h_posiZDiff")->Fill(diffZ);
    m_ev.diffDistX = diffX;
    m_ev.diffDistY = diffY;
    m_ev.diffDistZ = diffZ;
    //if (diffM > 10.0) continue;

    const bklm::Module* corMod = m_geoParB->findModule(hit2d->getSection(), hit2d->getSector(), hit2d->getLayer());
    double stripWidth_Z = corMod->getZStripWidth();
    double stripWidth_Phi = corMod->getPhiStripWidth();

    // If the Layer is filpped?
    m_ev.isFlipped = corMod->isFlipped();
    const CLHEP::Hep3Vector positionLocal_extHit = corMod->globalToLocal(CLHEP::Hep3Vector(
                                                     positionGlobal_extHit.X(), positionGlobal_extHit.Y(), positionGlobal_extHit.Z()), true);
    const CLHEP::Hep3Vector positionLocal_hit2d = corMod->globalToLocal(CLHEP::Hep3Vector(
                                                    positionGlobal_hit2d.X(), positionGlobal_hit2d.Y(), positionGlobal_hit2d.Z()), true);
    const CLHEP::Hep3Vector diffLocal = positionLocal_extHit - positionLocal_hit2d;
    if (fabs(diffLocal.z()) > stripWidth_Z  || fabs(diffLocal.y()) > stripWidth_Phi) continue;

    const CLHEP::Hep3Vector propaLengthV3 = corMod->getPropagationDistance(positionLocal_extHit);

    for (unsigned iH1 = 0; iH1 < bklmHit1ds.size(); ++iH1) {
      BKLMHit1d* hit1d = bklmHit1ds[iH1];
      RelationVector<BKLMDigit> digits = hit1d->getRelationsTo<BKLMDigit>();
      getObjectPtr<TH1I>("h_nDigit_rpc")->Fill(digits.size());
      if (digits.size() > 5) continue;

      double propaLength = propaLengthV3[2 - int(hit1d->isPhiReadout())];

      for (unsigned iHd = 0; iHd < digits.size(); ++iHd) {
        BKLMDigit* digitHit = digits[iHd];

        m_ev.inRPC = digitHit->inRPC();
        uint16_t channelId_digit = m_elementNum->channelNumberBKLM(digitHit->getSection(), digitHit->getSector(), digitHit->getLayer(),
                                                                   digitHit->isPhiReadout(), digitHit->getStrip());

        m_ev.recTime = digitHit->getTime();
        m_ev.dist = propaLength;
        m_ev.eDep = digitHit->getEDep();
        m_ev.nPE = digitHit->getNPixel();
        m_ev.channelId = channelId_digit;

        getObjectPtr<TH2D>("h_fTime")->Fill(m_ev.flyTime, iLay);
        getObjectPtr<TTree>("time_calibration_data")->Fill();
      }
    }
  }
}

void KLMTimeCalibrationCollectorModule::finish()
{
  B2INFO("Data Collection Done.");
}
