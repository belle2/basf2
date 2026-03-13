/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMEventT0Estimator/KLMEventT0EstimatorModule.h>

/* KLM headers — only those not already pulled in via the own header. */
#include <klm/bklm/geometry/Module.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>

/* Basf2 headers — only those not already pulled in via the own header. */
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>

using namespace Belle2;
using namespace Belle2::bklm;
using namespace Belle2::EKLM;

REG_MODULE(KLMEventT0Estimator);

/* Constructor and destructor. */

KLMEventT0EstimatorModule::KLMEventT0EstimatorModule() :
  HistoModule(),
  m_geoParB(nullptr),
  m_geoParE(nullptr),
  m_transformE(nullptr)
{
  setDescription("Estimate per-event T0 using KLM digits matched to extrapolated tracks (BKLM/EKLM scintillators and RPC) with per-event track averages, uncertainties, and final combined KLM value.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("MuonListName", m_MuonListName,
           "Muon (or generic) ParticleList name used to access tracks and KLM relations (e.g. 'mu+:forT0').",
           std::string("mu+:forT0"));
  addParam("useCDCTemporaryT0", m_useCDCTemporaryT0,
           "Read CDC temporary EventT0 as a seed/diagnostic (not applied to the mean).",
           true);
  addParam("IgnoreBackwardPropagation", m_ignoreBackward,
           "Ignore backward-propagated ExtHits when forming entry/exit pairs.",
           false);
  addParam("histogramDirectoryName", m_histDirName,
           "Top directory for KLMEventT0Estimator histograms inside the ROOT file.",
           std::string("KLMEventT0Estimator"));
  addParam("histogramSubdirUncorrected", m_histSubdirUncorr,
           "Subdirectory name for uncorrected timing histograms.",
           std::string("uncorrected"));

  // ADC cut parameters
  addParam("ADCCut_BKLM_Scint_Min", m_ADCCut_BKLM_Scint_Min,
           "Minimum ADC charge cut for BKLM scintillator. Set to 0 to disable lower cut.",
           30.0);
  addParam("ADCCut_BKLM_Scint_Max", m_ADCCut_BKLM_Scint_Max,
           "Maximum ADC charge cut for BKLM scintillator. Set to large value to disable upper cut.",
           320.0);
  addParam("ADCCut_EKLM_Scint_Min", m_ADCCut_EKLM_Scint_Min,
           "Minimum ADC charge cut for EKLM scintillator. Set to 0 to disable lower cut.",
           40.0);
  addParam("ADCCut_EKLM_Scint_Max", m_ADCCut_EKLM_Scint_Max,
           "Maximum ADC charge cut for EKLM scintillator. Set to large value to disable upper cut.",
           350.0);

}

KLMEventT0EstimatorModule::~KLMEventT0EstimatorModule()
{
  delete m_transformE;
}

/* Histogram definition. */

void KLMEventT0EstimatorModule::defineHisto()
{
  // Parent directory for this module
  TDirectory* topdir = gDirectory->mkdir(m_histDirName.c_str());
  TDirectory::TContext ctxTop{gDirectory, topdir};

  // Subdirectory for uncorrected timing histograms (always created)
  TDirectory* d_unc = topdir->mkdir(m_histSubdirUncorr.c_str());

  auto H1 = [](const char* n, const char* t, int nb, double lo, double hi) {
    return new TH1D(n, t, nb, lo, hi);
  };

  /* Uncorrected timing histograms. */
  {
    TDirectory::TContext ctxUnc{gDirectory, d_unc};

    // --- per_track/ subdirectory ---
    TDirectory* d_per_track = d_unc->mkdir("per_track");
    {
      TDirectory::TContext ctxTrk{gDirectory, d_per_track};
      m_hT0Trk_BKLM_Scint = H1("h_t0trk_bklm_scint", "Per-track T0 (BKLM Scint);T0 [ns]", 800, -100, 100);
      m_hT0Trk_BKLM_RPC   = H1("h_t0trk_bklm_rpc",   "Per-track T0 (BKLM RPC);T0 [ns]",   800, -100, 100);
      m_hT0Trk_EKLM_Scint = H1("h_t0trk_eklm_scint", "Per-track T0 (EKLM Scint);T0 [ns]", 800, -100, 100);
    }

    // --- per_event/ subdirectory ---
    TDirectory* d_per_event = d_unc->mkdir("per_event");
    {
      TDirectory::TContext ctxEvt{gDirectory, d_per_event};

      // Track-average
      m_hT0Evt_TrkAvg_BKLM_Scint     = H1("h_t0evt_trkavg_bklm_scint",     "Per-event T0 (track-avg, BKLM Scint);T0 [ns]", 800, -100,
                                          100);
      m_hT0Evt_TrkAvg_BKLM_RPC       = H1("h_t0evt_trkavg_bklm_rpc",       "Per-event T0 (track-avg, BKLM RPC);T0 [ns]",   800, -100,
                                          100);
      m_hT0Evt_TrkAvg_EKLM_Scint     = H1("h_t0evt_trkavg_eklm_scint",     "Per-event T0 (track-avg, EKLM Scint);T0 [ns]", 800, -100,
                                          100);
      m_hT0Evt_TrkAvg_All            = H1("h_t0evt_trkavg_all",            "Per-event T0 (track-avg, all categories);T0 [ns]", 800, -100,
                                          100);
      m_hT0Evt_TrkAvg_BKLM_Scint_SEM = H1("h_t0evt_trkavg_bklm_scint_sem", "SEM (track-avg, BKLM Scint);SEM [ns]", 800, 0.0, 20.0);
      m_hT0Evt_TrkAvg_BKLM_RPC_SEM   = H1("h_t0evt_trkavg_bklm_rpc_sem",   "SEM (track-avg, BKLM RPC);SEM [ns]",   800, 0.0, 20.0);
      m_hT0Evt_TrkAvg_EKLM_Scint_SEM = H1("h_t0evt_trkavg_eklm_scint_sem", "SEM (track-avg, EKLM Scint);SEM [ns]", 800, 0.0, 20.0);
      m_hT0Evt_TrkAvg_All_SEM        = H1("h_t0evt_trkavg_all_sem",        "SEM (track-avg, all categories);SEM [ns]", 800, 0.0, 20.0);

      // Final-source audit: all possible combinations of B(KLM Scint), E(KLM Scint), R(PC)
      m_hFinalSource = new TH1I("h_final_source", "Final KLM source;;events", 7, 0.5, 7.5);
      m_hFinalSource->GetXaxis()->SetBinLabel(1, "B only");
      m_hFinalSource->GetXaxis()->SetBinLabel(2, "E only");
      m_hFinalSource->GetXaxis()->SetBinLabel(3, "R only");
      m_hFinalSource->GetXaxis()->SetBinLabel(4, "B+E");
      m_hFinalSource->GetXaxis()->SetBinLabel(5, "B+R");
      m_hFinalSource->GetXaxis()->SetBinLabel(6, "E+R");
      m_hFinalSource->GetXaxis()->SetBinLabel(7, "B+E+R");
    }
  } // end uncorrected/ directory

}

/* Lifecycle. */

void KLMEventT0EstimatorModule::initialize()
{
  // Register that we define histograms
  REG_HISTOGRAM;

  // Inputs
  m_MuonList.isRequired(m_MuonListName);
  m_tracks.isRequired();

  // Geometry
  m_geoParB = bklm::GeometryPar::instance();
  m_geoParE = &(EKLM::GeometryData::Instance());
  m_transformE = new EKLM::TransformData(true, EKLM::TransformData::c_None);

  // Log ADC cut settings
  B2DEBUG(20, "KLMEventT0Estimator: ADC cuts configured:"
          << LogVar("BKLM Scint min", m_ADCCut_BKLM_Scint_Min)
          << LogVar("BKLM Scint max", m_ADCCut_BKLM_Scint_Max)
          << LogVar("EKLM Scint min", m_ADCCut_EKLM_Scint_Min)
          << LogVar("EKLM Scint max", m_ADCCut_EKLM_Scint_Max));
}

void KLMEventT0EstimatorModule::beginRun()
{
  if (!m_eventT0HitResolution.isValid())
    B2FATAL("KLMEventT0Estimator: KLM EventT0 hit resolution data are not available.");

  B2DEBUG(20, "KLMEventT0Estimator: Using calibrated per-hit resolution."
          << LogVar("sigma_RPC (ns)", m_eventT0HitResolution->getSigmaRPC())
          << LogVar("sigma_BKLM_Scint (ns)", m_eventT0HitResolution->getSigmaBKLMScint())
          << LogVar("sigma_EKLM_Scint (ns)", m_eventT0HitResolution->getSigmaEKLMScint()));
}

void KLMEventT0EstimatorModule::endRun()
{
}

void KLMEventT0EstimatorModule::terminate() {}

/* Helper methods. */

bool KLMEventT0EstimatorModule::passesADCCut(double charge, int subdetector, int layer, bool inRPC) const
{
  // RPC: No ADC cut applied
  if (subdetector == KLMElementNumbers::c_BKLM && (inRPC || layer >= BKLMElementNumbers::c_FirstRPCLayer)) {
    return true;
  }

  // BKLM Scintillator
  if (subdetector == KLMElementNumbers::c_BKLM) {
    return (charge >= m_ADCCut_BKLM_Scint_Min && charge <= m_ADCCut_BKLM_Scint_Max);
  }

  // EKLM Scintillator
  if (subdetector == KLMElementNumbers::c_EKLM) {
    return (charge >= m_ADCCut_EKLM_Scint_Min && charge <= m_ADCCut_EKLM_Scint_Max);
  }

  return true;
}

double KLMEventT0EstimatorModule::getHitSigma(int subdetector, int layer, bool inRPC, int plane) const
{
  if (!m_eventT0HitResolution.isValid()) {
    B2ERROR("KLMEventT0Estimator: Calibrated hit resolution payload not available!");
    return 1.0; // Fallback
  }

  if (subdetector == KLMElementNumbers::c_BKLM) {
    if (inRPC || layer >= BKLMElementNumbers::c_FirstRPCLayer) {
      // Use direction-specific RPC resolution if available (version 2+)
      // Fall back to combined RPC resolution for backward compatibility
      if (plane == BKLMElementNumbers::c_ZPlane) {
        float sigmaZ = m_eventT0HitResolution->getSigmaRPCZ();
        return (sigmaZ > 0.0) ? sigmaZ : m_eventT0HitResolution->getSigmaRPC();
      } else {
        float sigmaPhi = m_eventT0HitResolution->getSigmaRPCPhi();
        return (sigmaPhi > 0.0) ? sigmaPhi : m_eventT0HitResolution->getSigmaRPC();
      }
    } else {
      return m_eventT0HitResolution->getSigmaBKLMScint();
    }
  } else { // EKLM
    return m_eventT0HitResolution->getSigmaEKLMScint();
  }
}

KLMEventT0EstimatorModule::ExtPair
KLMEventT0EstimatorModule::matchExt(unsigned int key, ExtMap& v_ExtHits)
{
  ExtHit* entryHit = nullptr;
  ExtHit* exitHit  = nullptr;
  auto itlow = v_ExtHits.lower_bound(key);
  auto itup  = v_ExtHits.upper_bound(key);
  for (auto it = itlow; it != itup; ++it) {
    if (!entryHit || it->second.getTOF() < entryHit->getTOF()) entryHit = &(it->second);
    if (!exitHit  || it->second.getTOF() > exitHit->getTOF())  exitHit  = &(it->second);
  }
  return std::make_pair(entryHit, exitHit);
}

void KLMEventT0EstimatorModule::collectExtrapolatedHits(const Track* track,
                                                        ExtMap& scintMap,
                                                        ExtMap& rpcMap)
{
  scintMap.clear();
  rpcMap.clear();

  RelationVector<ExtHit> extHits = track->getRelationsTo<ExtHit>();
  KLMMuidLikelihood* muidLikelihood = track->getRelatedTo<KLMMuidLikelihood>();

  for (const ExtHit& eHit : extHits) {
    if (eHit.getStatus() != EXT_EXIT) continue;
    if (m_ignoreBackward && eHit.isBackwardPropagated()) continue;

    const bool isB = (eHit.getDetectorID() == Const::EDetector::BKLM);
    const bool isE = (eHit.getDetectorID() == Const::EDetector::EKLM);
    if (!isB && !isE) continue;

    int copyId = eHit.getCopyID();
    int tFor, tSec, tLay, tPla, tStr;
    int tSub = -1;

    if (isE) {
      tSub = KLMElementNumbers::c_EKLM;
      EKLMElementNumbers::Instance().stripNumberToElementNumbers(copyId,
                                                                 &tFor, &tLay, &tSec, &tPla, &tStr);
    }
    if (isB) {
      tSub = KLMElementNumbers::c_BKLM;
      BKLMElementNumbers::channelNumberToElementNumbers(copyId,
                                                        &tFor, &tSec, &tLay, &tPla, &tStr);
    }
    if (tSub < 0) continue;

    bool crossed = false;

    if (isB) {
      crossed = muidLikelihood
                ? muidLikelihood->isExtrapolatedBarrelLayerCrossed(tLay - 1)
                : true;
      if (!crossed) continue;

      const bool isRPC = (tLay >= BKLMElementNumbers::c_FirstRPCLayer);

      if (isRPC) {
        // RPC: match by module
        unsigned int moduleKey =
          m_elementNum->moduleNumber(tSub, tFor, tSec, tLay);
        rpcMap.insert(std::make_pair(moduleKey, eHit));
      } else {
        // BKLM scintillator: match by channel
        unsigned int channelKey =
          m_elementNum->channelNumber(tSub, tFor, tSec, tLay, tPla, tStr);
        if (m_channelStatus.isValid() &&
            m_channelStatus->getChannelStatus(channelKey) != KLMChannelStatus::c_Normal)
          continue;
        scintMap.insert(std::make_pair(channelKey, eHit));
      }
    }

    if (isE) {
      crossed = muidLikelihood
                ? muidLikelihood->isExtrapolatedEndcapLayerCrossed(tLay - 1)
                : true;
      if (!crossed) continue;

      unsigned int channelKey =
        m_elementNum->channelNumber(tSub, tFor, tSec, tLay, tPla, tStr);
      if (m_channelStatus.isValid() &&
          m_channelStatus->getChannelStatus(channelKey) != KLMChannelStatus::c_Normal)
        continue;
      scintMap.insert(std::make_pair(channelKey, eHit));
    }
  }
}

/* Per-subdetector accumulators. */

namespace {
  // Weighted accumulation using per-hit sigma
  inline void acc_stat_weighted(double t, double sigma, double& sumW, double& sumWT)
  {
    if (sigma <= 0.0 || !std::isfinite(sigma)) return;
    const double w = 1.0 / (sigma * sigma);
    sumW += w;
    sumWT += w * t;
  }
}

/* EKLM scintillator */
void KLMEventT0EstimatorModule::accumulateEKLM(const RelationVector<KLMHit2d>& klmHit2ds,
                                               const ExtMap& scintMap,
                                               double& sumW, double& sumWT)
{
  DBObjPtr<KLMTimeConstants> timeConstants;
  DBObjPtr<KLMTimeCableDelay> timeCableDelay;

  const double delayScint = timeConstants.isValid()
                            ? timeConstants->getDelay(KLMTimeConstants::c_EKLM)
                            : 0.0;

  HepGeom::Point3D<double> hitGlobal_ext, hitLocal_ext;

  for (const KLMHit2d& hit2d : klmHit2ds) {
    if (hit2d.getSubdetector() != KLMElementNumbers::c_EKLM) continue;

    RelationVector<KLMDigit> digits = hit2d.getRelationsTo<KLMDigit>();
    if (digits.size() == 0) continue;

    for (const KLMDigit& d : digits) {
      if (!d.isGood()) continue;

      unsigned int cid = d.getUniqueChannelID();
      if (m_channelStatus.isValid() &&
          m_channelStatus->getChannelStatus(cid) != KLMChannelStatus::c_Normal) continue;

      // Apply ADC cut
      if (!passesADCCut(d.getCharge(), KLMElementNumbers::c_EKLM, d.getLayer(), false)) {
        continue;
      }

      // Match using channel ID
      ExtPair ex = const_cast<KLMEventT0EstimatorModule*>(this)->matchExt(cid, const_cast<ExtMap&>(scintMap));
      if (!ex.first || !ex.second) continue;
      const double flyTime = 0.5 * (ex.first->getTOF() + ex.second->getTOF());

      // Distance along strip to readout
      const ROOT::Math::XYZVector posGlobExt = 0.5 * (ex.first->getPosition() + ex.second->getPosition());
      hitGlobal_ext.setX(posGlobExt.X() / Unit::mm * CLHEP::mm);
      hitGlobal_ext.setY(posGlobExt.Y() / Unit::mm * CLHEP::mm);
      hitGlobal_ext.setZ(posGlobExt.Z() / Unit::mm * CLHEP::mm);

      const double Lmm = m_geoParE->getStripLength(d.getStrip()) / CLHEP::mm * Unit::mm;
      const HepGeom::Transform3D* tr = m_transformE->getStripGlobalToLocal(const_cast<KLMDigit*>(&d));
      hitLocal_ext = (*tr) * hitGlobal_ext;
      const double dist_mm = 0.5 * Lmm - hitLocal_ext.x() / CLHEP::mm * Unit::mm;

      // Components
      const double Trec = d.getTime();
      const double Tcable = timeCableDelay.isValid() ? timeCableDelay->getTimeDelay(cid) : 0.0;
      const double Tprop = dist_mm * delayScint;
      const double Tfly = flyTime;

      // Correct digit time
      double t = Trec;
      if (timeCableDelay.isValid()) t -= Tcable;
      t -= Tprop;

      const double t0_est = t - Tfly;
      if (!std::isfinite(t0_est)) continue;

      // Weighted accumulation using calibrated sigma
      const double sigma = getHitSigma(KLMElementNumbers::c_EKLM, d.getLayer(), false);
      acc_stat_weighted(t0_est, sigma, sumW, sumWT);
    }
  }
}

/* BKLM scintillator */
void KLMEventT0EstimatorModule::accumulateBKLMScint(RelationVector<KLMHit2d>& klmHit2ds,
                                                    const ExtMap& scintMap,
                                                    double& sumW, double& sumWT)
{
  DBObjPtr<KLMTimeConstants> timeConstants;
  DBObjPtr<KLMTimeCableDelay> timeCableDelay;

  const double delayScint = timeConstants.isValid()
                            ? timeConstants->getDelay(KLMTimeConstants::c_BKLM)
                            : 0.0;

  for (KLMHit2d& hit2d : klmHit2ds) {
    if (hit2d.getSubdetector() != KLMElementNumbers::c_BKLM) continue;
    if (hit2d.inRPC()) continue;
    if (hit2d.isOutOfTime()) continue;

    RelationVector<BKLMHit1d> b1ds = hit2d.getRelationsTo<BKLMHit1d>();
    if (b1ds.size() == 0) continue;

    const bklm::Module* mod = m_geoParB->findModule(hit2d.getSection(), hit2d.getSector(), hit2d.getLayer());
    const ROOT::Math::XYZVector posG2d = hit2d.getPosition();

    for (const BKLMHit1d& h1d : b1ds) {
      RelationVector<KLMDigit> digits = h1d.getRelationsTo<KLMDigit>();

      for (const KLMDigit& d : digits) {
        if (d.inRPC() || !d.isGood()) continue;

        unsigned int cid = d.getUniqueChannelID();
        if (m_channelStatus.isValid() &&
            m_channelStatus->getChannelStatus(cid) != KLMChannelStatus::c_Normal) continue;

        // Apply ADC cut
        if (!passesADCCut(d.getCharge(), KLMElementNumbers::c_BKLM, d.getLayer(), false)) {
          continue;
        }

        // Match using channel ID
        ExtPair p = const_cast<KLMEventT0EstimatorModule*>(this)->matchExt(cid, const_cast<ExtMap&>(scintMap));
        if (!p.first || !p.second) continue;

        const double flyTime = 0.5 * (p.first->getTOF() + p.second->getTOF());
        const ROOT::Math::XYZVector posGext = 0.5 * (p.first->getPosition() + p.second->getPosition());

        // Gate in local
        const CLHEP::Hep3Vector locExt  = mod->globalToLocal(CLHEP::Hep3Vector(posGext.X(), posGext.Y(), posGext.Z()), true);
        const CLHEP::Hep3Vector locHit2 = mod->globalToLocal(CLHEP::Hep3Vector(posG2d.X(),  posG2d.Y(),  posG2d.Z()),  true);
        const CLHEP::Hep3Vector diff    = locExt - locHit2;
        if (std::fabs(diff.z()) > mod->getZStripWidth() || std::fabs(diff.y()) > mod->getPhiStripWidth()) continue;

        // Prop distance
        const bool isPhiReadout = h1d.isPhiReadout();
        double propaLen = mod->getPropagationDistance(locExt, d.getStrip(), isPhiReadout);

        // Components
        const double Trec = d.getTime();
        const double Tcable = timeCableDelay.isValid() ? timeCableDelay->getTimeDelay(cid) : 0.0;
        const double Tprop = propaLen * delayScint;
        const double Tfly = flyTime;

        double t = Trec;
        if (timeCableDelay.isValid()) t -= Tcable;
        t -= Tprop;

        const double t0_est = t - Tfly;
        if (!std::isfinite(t0_est)) continue;

        // Weighted accumulation using calibrated sigma
        const double sigma = getHitSigma(KLMElementNumbers::c_BKLM, d.getLayer(), false);
        acc_stat_weighted(t0_est, sigma, sumW, sumWT);
      }
    }
  }
}

/* BKLM RPC */
void KLMEventT0EstimatorModule::accumulateBKLMRPC(RelationVector<KLMHit2d>& klmHit2ds,
                                                  const ExtMap& rpcMap,
                                                  double& sumW, double& sumWT)
{
  DBObjPtr<KLMTimeConstants> timeConstants;
  DBObjPtr<KLMTimeCableDelay> timeCableDelay;

  const double delayPhi = timeConstants.isValid()
                          ? timeConstants->getDelay(KLMTimeConstants::c_RPCPhi)
                          : 0.0;
  const double delayZ   = timeConstants.isValid()
                          ? timeConstants->getDelay(KLMTimeConstants::c_RPCZ)
                          : 0.0;

  for (KLMHit2d& hit2d : klmHit2ds) {
    if (hit2d.getSubdetector() != KLMElementNumbers::c_BKLM) continue;
    if (!hit2d.inRPC()) continue;
    if (hit2d.isOutOfTime()) continue;

    RelationVector<BKLMHit1d> b1ds = hit2d.getRelationsTo<BKLMHit1d>();
    if (b1ds.size() == 0) continue;

    const bklm::Module* mod = m_geoParB->findModule(hit2d.getSection(), hit2d.getSector(), hit2d.getLayer());
    const ROOT::Math::XYZVector posG2d = hit2d.getPosition();

    for (const BKLMHit1d& h1d : b1ds) {
      const bool isPhi = h1d.isPhiReadout();
      RelationVector<KLMDigit> digits = h1d.getRelationsTo<KLMDigit>();

      for (const KLMDigit& d : digits) {
        if (!d.inRPC()) continue;

        unsigned int cid = d.getUniqueChannelID();
        if (m_channelStatus.isValid() &&
            m_channelStatus->getChannelStatus(cid) != KLMChannelStatus::c_Normal) continue;

        if (!d.isGood()) continue;

        // RPC matched by module key
        unsigned int moduleKey = m_elementNum->moduleNumber(d.getSubdetector(), d.getSection(), d.getSector(), d.getLayer());
        ExtPair p = const_cast<KLMEventT0EstimatorModule*>(this)->matchExt(moduleKey, const_cast<ExtMap&>(rpcMap));
        if (!p.first || !p.second) continue;

        const double flyTime = 0.5 * (p.first->getTOF() + p.second->getTOF());
        const ROOT::Math::XYZVector posGext = 0.5 * (p.first->getPosition() + p.second->getPosition());

        const CLHEP::Hep3Vector locExt  = mod->globalToLocal(CLHEP::Hep3Vector(posGext.X(), posGext.Y(), posGext.Z()), true);
        const CLHEP::Hep3Vector locHit2 = mod->globalToLocal(CLHEP::Hep3Vector(posG2d.X(),  posG2d.Y(),  posG2d.Z()),  true);

        const CLHEP::Hep3Vector diff = locExt - locHit2;
        if (std::fabs(diff.z()) > mod->getZStripWidth() || std::fabs(diff.y()) > mod->getPhiStripWidth()) continue;

        const CLHEP::Hep3Vector propaV = mod->getPropagationDistance(locExt);
        const double propaDist = isPhi ? propaV.y() : propaV.z();

        // Components
        const double Trec = d.getTime();
        const double Tcable = timeCableDelay.isValid() ? timeCableDelay->getTimeDelay(cid) : 0.0;
        const double Tprop = propaDist * (isPhi ? delayPhi : delayZ);
        const double Tfly = flyTime;

        double t = Trec;
        if (timeCableDelay.isValid()) t -= Tcable;
        t -= Tprop;

        const double t0_est = t - Tfly;
        if (!std::isfinite(t0_est)) continue;

        // Weighted accumulation using calibrated sigma with direction-specific resolution
        const int plane = isPhi ? BKLMElementNumbers::c_PhiPlane : BKLMElementNumbers::c_ZPlane;
        const double sigma = getHitSigma(KLMElementNumbers::c_BKLM, d.getLayer(), true, plane);
        acc_stat_weighted(t0_est, sigma, sumW, sumWT);
      }
    }
  }
}

/* Event. */

void KLMEventT0EstimatorModule::event()
{
  // CDC seed for logging (not used to compute the means)
  m_seedT0 = 0.0;
  if (m_useCDCTemporaryT0) {
    StoreObjPtr<EventT0> evtT0("EventT0", DataStore::c_Event);
    if (evtT0.isValid() && evtT0->hasTemporaryEventT0(Const::EDetector::CDC)) {
      const auto best = evtT0->getBestCDCTemporaryEventT0();
      if (best) m_seedT0 = best->eventT0;
    }
  }

  if (!m_MuonList.isValid()) { B2WARNING("KLMEventT0Estimator: ParticleList '" << m_MuonListName << "' not found."); return; }
  const unsigned nTracks = m_MuonList->getListSize();
  if (nTracks == 0u) return;

  // Weighted mean and uncertainty from running sums
  auto weighted_result = [](double wsum, double wtsum) -> std::pair<double, double> {
    if (wsum <= 0.0) return {NAN, NAN};
    return {wtsum / wsum, std::sqrt(1.0 / wsum)};
  };

  // Weighted track averaging using inverse-variance (1/SEM²) weighting
  auto mean_sem_tracks = [](const std::vector<std::pair<double, double>>& v) -> std::pair<double, double> {
    if (v.empty()) return {NAN, NAN};
    if (v.size() == 1)
    {
      return {v[0].first, std::isfinite(v[0].second) ? v[0].second : 0.0};
    }

    bool allValid = true;
    for (const auto& [t0, sem] : v)
    {
      if (!std::isfinite(sem) || sem <= 0.0) { allValid = false; break; }
    }

    if (allValid)
    {
      double wsum = 0.0, wtsum = 0.0;
      for (const auto& [t0, sem] : v) {
        const double w = 1.0 / (sem * sem);
        wsum += w;
        wtsum += w * t0;
      }
      if (wsum > 0.0) {
        return {wtsum / wsum, std::sqrt(1.0 / wsum)};
      }
    }

    // Fallback to simple average if weights not valid
    double s = 0.0;
    for (const auto& [t0, sem] : v) s += t0;
    const double mu = s / v.size();
    double ss = 0.0;
    for (const auto& [t0, sem] : v) { const double d = t0 - mu; ss += d * d; }
    const double var = (v.size() > 1) ? ss / (v.size() - 1) : 0.0;
    return {mu, std::sqrt(var / v.size())};
  };

  // For per-event track-averages: pairs of (T0, SEM) for weighted averaging
  std::vector<std::pair<double, double>> vTrk_B, vTrk_R, vTrk_E, vTrk_All;

  for (unsigned i = 0; i < nTracks; ++i) {
    const Particle* particle = m_MuonList->getParticle(i);
    if (!particle) continue;
    const Track* track = particle->getTrack();
    if (!track) continue;

    RelationVector<KLMHit2d> hit2ds = track->getRelationsTo<KLMHit2d>();
    if (hit2ds.size() == 0) continue;

    // Build ExtHit maps for this track
    m_extScint.clear();
    m_extRPC.clear();
    collectExtrapolatedHits(track, m_extScint, m_extRPC);

    // Per-track digit sums per category (weighted)
    double wE = 0, wTE = 0;
    accumulateEKLM(hit2ds, m_extScint, wE, wTE);

    double wB = 0, wTB = 0;
    accumulateBKLMScint(hit2ds, m_extScint, wB, wTB);

    double wR = 0, wTR = 0;
    accumulateBKLMRPC(hit2ds, m_extRPC, wR, wTR);

    // Per-track means and SEMs by category
    if (wB > 0.0) {
      auto [muB, seB] = weighted_result(wB, wTB);
      if (m_hT0Trk_BKLM_Scint && std::isfinite(muB)) m_hT0Trk_BKLM_Scint->Fill(muB);
      if (std::isfinite(muB)) vTrk_B.push_back({muB, seB});
    }

    if (wR > 0.0) {
      auto [muR, seR] = weighted_result(wR, wTR);
      if (m_hT0Trk_BKLM_RPC && std::isfinite(muR)) m_hT0Trk_BKLM_RPC->Fill(muR);
      if (std::isfinite(muR)) vTrk_R.push_back({muR, seR});
    }

    if (wE > 0.0) {
      auto [muE, seE] = weighted_result(wE, wTE);
      if (m_hT0Trk_EKLM_Scint && std::isfinite(muE)) m_hT0Trk_EKLM_Scint->Fill(muE);
      if (std::isfinite(muE)) vTrk_E.push_back({muE, seE});
    }

    // Per-track overall (if any category present)
    {
      const double wAll = wB + wE + wR;
      const double wtAll = wTB + wTE + wTR;
      if (wAll > 0.0) {
        const double t0 = wtAll / wAll;
        const double se = std::sqrt(1.0 / wAll);
        vTrk_All.push_back({t0, se});
      }
    }
  }

  if (vTrk_All.empty()) {
    B2DEBUG(20, "KLMEventT0Estimator: no usable KLM timing residuals for this event.");
    return;
  }

  // Per-event track-averages using inverse-variance (1/SEM²) weighting
  const auto [muB_trk,   seB_trk]   = mean_sem_tracks(vTrk_B);
  const auto [muR_trk,   seR_trk]   = mean_sem_tracks(vTrk_R);
  const auto [muE_trk,   seE_trk]   = mean_sem_tracks(vTrk_E);
  const auto [muAll_trk, seAll_trk]  = mean_sem_tracks(vTrk_All);

  if (m_hT0Evt_TrkAvg_BKLM_Scint && std::isfinite(muB_trk))   m_hT0Evt_TrkAvg_BKLM_Scint->Fill(muB_trk);
  if (m_hT0Evt_TrkAvg_BKLM_RPC   && std::isfinite(muR_trk))   m_hT0Evt_TrkAvg_BKLM_RPC->Fill(muR_trk);
  if (m_hT0Evt_TrkAvg_EKLM_Scint && std::isfinite(muE_trk))   m_hT0Evt_TrkAvg_EKLM_Scint->Fill(muE_trk);
  if (m_hT0Evt_TrkAvg_All        && std::isfinite(muAll_trk)) m_hT0Evt_TrkAvg_All->Fill(muAll_trk);

  if (m_hT0Evt_TrkAvg_BKLM_Scint_SEM && std::isfinite(seB_trk))   m_hT0Evt_TrkAvg_BKLM_Scint_SEM->Fill(seB_trk);
  if (m_hT0Evt_TrkAvg_BKLM_RPC_SEM   && std::isfinite(seR_trk))   m_hT0Evt_TrkAvg_BKLM_RPC_SEM->Fill(seR_trk);
  if (m_hT0Evt_TrkAvg_EKLM_Scint_SEM && std::isfinite(seE_trk))   m_hT0Evt_TrkAvg_EKLM_Scint_SEM->Fill(seE_trk);
  if (m_hT0Evt_TrkAvg_All_SEM        && std::isfinite(seAll_trk)) m_hT0Evt_TrkAvg_All_SEM->Fill(seAll_trk);

  // ---------------- Final KLM combination (single saved component) ----------------
  const bool useB = std::isfinite(muB_trk);
  const bool useE = std::isfinite(muE_trk);
  const bool useR = std::isfinite(muR_trk);

  double finalT0 = NAN, finalSE = NAN;
  int sourceBin = -1;

  {
    std::vector<std::pair<double, double>> parts;
    if (useB) parts.emplace_back(muB_trk, seB_trk);
    if (useE) parts.emplace_back(muE_trk, seE_trk);
    if (useR) parts.emplace_back(muR_trk, seR_trk);

    auto [t0, se] = mean_sem_tracks(parts);
    finalT0 = t0;
    finalSE = se;

    if (useB && useE && useR) sourceBin = 7;
    else if (useB && useE) sourceBin = 4;
    else if (useB && useR) sourceBin = 5;
    else if (useE && useR) sourceBin = 6;
    else if (useB) sourceBin = 1;
    else if (useE) sourceBin = 2;
    else if (useR) sourceBin = 3;
  }

  B2DEBUG(20, "KLMEventT0Estimator: "
          << "T0_trkavg_all=" << muAll_trk << " ns  (seed CDC=" << m_seedT0 << " ns)"
          << " | E=" << muE_trk << " | Bsc=" << muB_trk << " | Brpc=" << muR_trk
          << (std::isfinite(finalT0) ? (std::string(" | FINAL KLM=") + std::to_string(finalT0) + " ns") : std::string("")));

  StoreObjPtr<EventT0> outT0("EventT0", DataStore::c_Event);
  if (!outT0.isValid()) outT0.construct();

  if (std::isfinite(finalT0)) {
    if (m_hFinalSource && sourceBin > 0) m_hFinalSource->Fill(sourceBin);
    const double quality = static_cast<double>((useB ? 1 : 0) + (useE ? 1 : 0) + (useR ? 1 : 0));
    EventT0::EventT0Component klmT0Component(finalT0, std::isfinite(finalSE) ? finalSE : 0.0,
                                             Const::KLM, "KLM", quality);
    outT0->addTemporaryEventT0(klmT0Component);
    outT0->setEventT0(klmT0Component);
  }
}
