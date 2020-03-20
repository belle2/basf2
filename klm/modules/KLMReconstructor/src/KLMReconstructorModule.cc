/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin, Giacomo De Pietro,                      *
 *               Leo Piilonen, Timofey Uglov                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMReconstructor/KLMReconstructorModule.h>

/* KLM headers. */
#include <klm/bklm/geometry/Module.h>
#include <klm/dataobjects/eklm/ElementNumbersSingleton.h>

/* Belle 2 headers. */
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

/* CLHEP headers. */
#include <CLHEP/Vector/ThreeVector.h>

using namespace Belle2;
using namespace Belle2::bklm;

REG_MODULE(KLMReconstructor)

static bool compareSector(EKLMDigit* d1, EKLMDigit* d2)
{
  int s1, s2;
  static const EKLM::ElementNumbersSingleton& elementNumbers =
    EKLM::ElementNumbersSingleton::Instance();
  s1 = elementNumbers.sectorNumber(d1->getSection(), d1->getLayer(),
                                   d1->getSector());
  s2 = elementNumbers.sectorNumber(d2->getSection(), d2->getLayer(),
                                   d2->getSector());
  return s1 < s2;
}

static bool comparePlane(EKLMDigit* d1, EKLMDigit* d2)
{
  return d1->getPlane() < d2->getPlane();
}

static bool compareStrip(EKLMDigit* d1, EKLMDigit* d2)
{
  return d1->getStrip() < d2->getStrip();
}

static bool compareTime(EKLMDigit* d1, EKLMDigit* d2)
{
  return d1->getTime() < d2->getTime();
}

static bool sameSector(EKLMDigit* d1, EKLMDigit* d2)
{
  return ((d1->getSection() == d2->getSection()) &&
          (d1->getLayer() == d2->getLayer()) &&
          (d1->getSector() == d2->getSector()));
}

KLMReconstructorModule::KLMReconstructorModule() :
  Module(),
  m_bklmGeoPar(nullptr),
  m_eklmGeoDat(nullptr),
  m_eklmNStrip(0),
  m_eklmTransformData(nullptr),
  m_eklmTimeCalibrationData(nullptr)
{
  setDescription("Create BKLMHit1ds from BKLMDigits and then create BKLMHit2ds from BKLMHit1ds; create EKLMHit2ds from EKLMDigits.");
  setPropertyFlags(c_ParallelProcessingCertified);
  // MC 1 GeV/c muons: 1-sigma width is 0.43 ns
  addParam("CoincidenceWindow", m_bklmCoincidenceWindow,
           "Half-width time coincidence window between adjacent BKLMDigits or orthogonal BKLMHit1ds (ns).",
           double(50.0));
  // MC 1 GeV/c muons: mean prompt time is 0.43 ns
  addParam("PromptTime", m_bklmPromptTime,
           "Nominal time of prompt BKLMHit2ds (ns).",
           double(0.0));
  // MC 1 GeV/c muons: 1-sigma width is 0.15 ns
  // Raw KLM scintillator hit times are in the range from -5000 to -4000 ns
  // approximately. The time window can be readjusted after completion of
  // the implementation of KLM time calibration.
  addParam("PromptWindow", m_bklmPromptWindow,
           "Half-width time window of BKLMHit2ds relative to PrompTime (ns).",
           //double(50.0));
           double(10000.0));
  addParam("IfAlign", m_bklmIfAlign,
           "Perform alignment correction (true) or not (false).",
           bool(true));
  addParam("LoadTimingFromDB", m_bklmLoadTimingFromDB,
           "Load timing window from database (true) or not (false).",
           bool(true));
  addParam("IgnoreScintillators", m_bklmIgnoreScintillators,
           "Ignore scintillators (to debug their electronics mapping).",
           false);
  addParam("CheckSegmentIntersection", m_eklmCheckSegmentIntersection,
           "Check if segments intersect.", true);
}

KLMReconstructorModule::~KLMReconstructorModule()
{
}

void KLMReconstructorModule::initialize()
{
  /* BKLM. */
  m_bklmDigits.isRequired();
  m_bklmHit1ds.registerInDataStore();
  m_bklmHit2ds.registerInDataStore();
  m_bklmHit1ds.registerRelationTo(m_bklmDigits);
  m_bklmHit2ds.registerRelationTo(m_bklmHit1ds);
  m_bklmGeoPar = bklm::GeometryPar::instance();
  /* EKLM. */
  m_eklmDigits.isRequired();
  m_eklmHit2ds.registerInDataStore();
  m_eklmAlignmentHits.registerInDataStore();
  m_eklmHit2ds.registerRelationTo(m_eklmDigits);
  m_eklmAlignmentHits.registerRelationTo(m_eklmHit2ds);
  m_eklmTransformData =
    new EKLM::TransformData(true, EKLM::TransformData::c_Alignment);
  m_eklmGeoDat = &(EKLM::GeometryData::Instance());
  if (m_eklmGeoDat->getNPlanes() != 2)
    B2FATAL("It is not possible to run EKLM reconstruction with 1 plane.");
  m_eklmNStrip = m_eklmGeoDat->getMaximalStripGlobalNumber();
  m_eklmTimeCalibrationData = new const EKLMTimeCalibrationData*[m_eklmNStrip];
}

void KLMReconstructorModule::beginRun()
{
  /* BKLM. */
  if (m_bklmLoadTimingFromDB) {
    if (!m_bklmTiming.isValid())
      B2FATAL("BKLM time window data are not available.");
    m_bklmCoincidenceWindow = m_bklmTiming->getCoincidenceWindow();
    m_bklmPromptTime = m_bklmTiming->getPromptTime();
    /* Not use the promt window value from database
     * until the time calibration is ready. */
    // m_bklmPromptWindow = m_bklmTiming->getPromptWindow();
  }
  /* EKLM. */
  /* cppcheck-suppress variableScope */
  int i;
  if (!m_eklmRecPar.isValid())
    B2FATAL("EKLM digitization parameters are not available.");
  if (!m_eklmTimeCalibration.isValid())
    B2FATAL("EKLM time calibration data is not available.");
  if (m_eklmTimeCalibration.hasChanged()) {
    for (i = 0; i < m_eklmNStrip; i++) {
      m_eklmTimeCalibrationData[i] =
        m_eklmTimeCalibration->getTimeCalibrationData(i + 1);
      if (m_eklmTimeCalibrationData[i] == nullptr) {
        B2FATAL("EKLM time calibration data is missing for strip "
                << i + 1 << ".");
        m_eklmTimeCalibrationData[i] = &m_eklmDefaultTimeCalibrationData;
      }
    }
  }
}

void KLMReconstructorModule::event()
{
  reconstructBKLMHits();
  reconstructEKLMHits();
}

void KLMReconstructorModule::reconstructBKLMHits()
{
  /* Construct BKLMHit1Ds from BKLMDigits. */
  /* Sort BKLMDigits by module and strip number. */
  std::map<uint16_t, int> channelDigitMap;
  for (int index = 0; index < m_bklmDigits.getEntries(); ++index) {
    const BKLMDigit* digit = m_bklmDigits[index];
    if (m_bklmIgnoreScintillators && !digit->inRPC())
      continue;
    if (digit->inRPC() || digit->isAboveThreshold()) {
      int module = digit->getModuleID();
      uint16_t channel = BKLMElementNumbers::getChannelByModule(module);
      channelDigitMap.insert(std::pair<uint16_t, int>(channel, index));
    }
  }
  if (channelDigitMap.empty())
    return;
  std::vector<const BKLMDigit*> digitCluster;
  uint16_t previousChannel = channelDigitMap.begin()->first;
  double averageTime = m_bklmDigits[channelDigitMap.begin()->second]->getTime();
  for (std::map<uint16_t, int>::iterator it = channelDigitMap.begin(); it != channelDigitMap.end(); ++it) {
    const BKLMDigit* digit = m_bklmDigits[it->second];
    double digitTime = digit->getTime();
    if ((it->first > previousChannel + 1) || (std::fabs(digitTime - averageTime) > m_bklmCoincidenceWindow)) {
      m_bklmHit1ds.appendNew(digitCluster); // Also sets relation BKLMHit1d -> BKLMDigit
      digitCluster.clear();
    }
    previousChannel = it->first;
    double n = (double)(digitCluster.size());
    averageTime = (n * averageTime + digitTime) / (n + 1.0);
    digitCluster.push_back(digit);
  }
  m_bklmHit1ds.appendNew(digitCluster); // Also sets relation BKLMHit1d -> BKLMDigit

  /* Construct BKLMHit2Ds from orthogonal same-module BKLMHit1Ds. */
  for (int i = 0; i < m_bklmHit1ds.getEntries(); ++i) {
    int moduleID = m_bklmHit1ds[i]->getModuleID();
    const bklm::Module* m = m_bklmGeoPar->findModule(m_bklmHit1ds[i]->getSection(), m_bklmHit1ds[i]->getSector(),
                                                     m_bklmHit1ds[i]->getLayer());
    bool isPhiReadout = m_bklmHit1ds[i]->isPhiReadout();
    for (int j = i + 1; j < m_bklmHit1ds.getEntries(); ++j) {
      if (!BKLMElementNumbers::hitsFromSameModule(
            moduleID, m_bklmHit1ds[j]->getModuleID()))
        continue;
      if (isPhiReadout == m_bklmHit1ds[j]->isPhiReadout())
        continue;
      int phiIndex = isPhiReadout ? i : j;
      int zIndex   = isPhiReadout ? j : i;
      const BKLMHit1d* phiHit = m_bklmHit1ds[phiIndex];
      const BKLMHit1d* zHit = m_bklmHit1ds[zIndex];
      CLHEP::Hep3Vector local = m->getLocalPosition(phiHit->getStripAve(), zHit->getStripAve());
      CLHEP::Hep3Vector propagationTimes = m->getPropagationTimes(local);
      double phiTime = phiHit->getTime() - propagationTimes.y();
      double zTime = zHit->getTime() - propagationTimes.z();
      if (std::fabs(phiTime - zTime) > m_bklmCoincidenceWindow)
        continue;
      // The second param in localToGlobal is whether do the alignment correction (true) or not (false)
      CLHEP::Hep3Vector global = m->localToGlobal(local + m->getLocalReconstructionShift(), m_bklmIfAlign);
      double time = 0.5 * (phiTime + zTime) - global.mag() / Const::speedOfLight;
      BKLMHit2d* hit2d = m_bklmHit2ds.appendNew(phiHit, zHit, global, time); // Also sets relation BKLMHit2d -> BKLMHit1d
      if (std::fabs(time - m_bklmPromptTime) > m_bklmPromptWindow)
        hit2d->isOutOfTime(true);
    }
  }
}

bool KLMReconstructorModule::fastHit(HepGeom::Point3D<double>& pos,
                                     double time)
{
  return time < pos.mag() / Const::speedOfLight -
         2.0 * m_eklmRecPar->getTimeResolution();
}

double KLMReconstructorModule::getTime(EKLMDigit* d, double dist)
{
  int strip;
  strip = m_eklmGeoDat->stripNumber(d->getSection(), d->getLayer(), d->getSector(),
                                    d->getPlane(), d->getStrip()) - 1;
  return d->getTime() -
         (dist / m_eklmTimeCalibration->getEffectiveLightSpeed() +
          m_eklmTimeCalibrationData[strip]->getTimeShift());
  /**
   * TODO: Subtract time correction given by
   * m_eklmTimeCalibration->getAmplitudeTimeConstant() / sqrt(d->getNPE()).
   * It requires a new firmware version that will be able to extract amplitude.
   */
}

void KLMReconstructorModule::reconstructEKLMHits()
{
  int i, n;
  double d1, d2, t, t1, t2, sd;
  std::vector<EKLMDigit*> digitVector;
  std::vector<EKLMDigit*>::iterator it1, it2, it3, it4, it5, it6, it7, it8, it9;
  n = m_eklmDigits.getEntries();
  for (i = 0; i < n; i++) {
    if (m_eklmDigits[i]->isGood())
      digitVector.push_back(m_eklmDigits[i]);
  }
  /* Sort by sector. */
  sort(digitVector.begin(), digitVector.end(), compareSector);
  it1 = digitVector.begin();
  while (it1 != digitVector.end()) {
    it2 = it1;
    while (1) {
      ++it2;
      if (it2 == digitVector.end())
        break;
      if (!sameSector(*it1, *it2))
        break;
    }
    /* Now it1 .. it2 - hits in a sector. Sort by plane. */
    sort(it1, it2, comparePlane);
    /* If all hits are form the second plane, then continue. */
    if ((*it1)->getPlane() != 1) {
      it1 = it2;
      continue;
    }
    it3 = it1;
    while (1) {
      ++it3;
      if (it3 == it2)
        break;
      if ((*it3)->getPlane() != (*it1)->getPlane())
        break;
    }
    /*
     * Now it1 .. it3 - hits from the first plane, it3 .. it2 - hits from the
     * second plane. If there are no hits from the second plane, then continue.
     */
    if (it3 == it2) {
      it1 = it2;
      continue;
    }
    /* Sort by strip. */
    sort(it1, it3, compareStrip);
    sort(it3, it2, compareStrip);
    it4 = it1;
    while (it4 != it2) {
      it5 = it4;
      while (1) {
        ++it5;
        if (it5 == it2)
          break;
        /* This loop is for both planes so it is necessary to compare planes. */
        if ((*it5)->getStrip() != (*it4)->getStrip() ||
            (*it5)->getPlane() != (*it4)->getPlane())
          break;
      }
      /* Now it4 .. it5 - hits from the same strip. Sort by time. */
      sort(it4, it5, compareTime);
      it4 = it5;
    }
    /* Strip loop. */
    it4 = it1;
    while (it4 != it3) {
      it5 = it4;
      while (1) {
        ++it5;
        if (it5 == it3)
          break;
        if ((*it5)->getStrip() != (*it4)->getStrip())
          break;
      }
      it6 = it3;
      while (it6 != it2) {
        it7 = it6;
        while (1) {
          ++it7;
          if (it7 == it2)
            break;
          if ((*it7)->getStrip() != (*it6)->getStrip())
            break;
        }
        /*
         * Now it4 .. it5 - hits from a single fisrt-plane strip amd
         * it6 .. it7 - hits from a single second-plane strip.
         * If strips do not intersect, then continue.
         */
        HepGeom::Point3D<double> crossPoint(0, 0, 0);
        if (!m_eklmTransformData->intersection(*it4, *it6, &crossPoint,
                                               &d1, &d2, &sd,
                                               m_eklmCheckSegmentIntersection)) {
          it6 = it7;
          continue;
        }
        for (it8 = it4; it8 != it5; ++it8) {
          for (it9 = it6; it9 != it7; ++it9) {
            t1 = getTime(*it8, d1) + 0.5 * sd / Const::speedOfLight;
            t2 = getTime(*it9, d2) - 0.5 * sd / Const::speedOfLight;
            t = (t1 + t2) / 2;
            EKLMHit2d* hit2d = m_eklmHit2ds.appendNew(*it8);
            hit2d->setEDep((*it8)->getEDep() + (*it9)->getEDep());
            hit2d->setPosition(crossPoint.x(), crossPoint.y(), crossPoint.z());
            hit2d->setChiSq((t1 - t2) * (t1 - t2) /
                            m_eklmRecPar->getTimeResolution() /
                            m_eklmRecPar->getTimeResolution());
            hit2d->setTime(t);
            hit2d->setMCTime(((*it8)->getMCTime() + (*it9)->getMCTime()) / 2);
            hit2d->addRelationTo(*it8);
            hit2d->addRelationTo(*it9);
            for (i = 0; i < 2; i++) {
              EKLMAlignmentHit* alignmentHit = m_eklmAlignmentHits.appendNew(i);
              alignmentHit->addRelationTo(hit2d);
            }
            /* Exit the loop. Equivalent to selection of the earliest hit. */
            break;
          }
          /* Exit the loop. Equivalent to selection of the earliest hit. */
          break;
        }
        it6 = it7;
      }
      it4 = it5;
    }
    it1 = it2;
  }
}

void KLMReconstructorModule::endRun()
{
}

void KLMReconstructorModule::terminate()
{
  delete m_eklmTransformData;
  delete[] m_eklmTimeCalibrationData;
}
