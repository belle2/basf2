/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMReconstructor/KLMReconstructorModule.h>

/* KLM headers. */
#include <klm/bklm/geometry/Module.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>

/* Basf2 headers. */
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

/* CLHEP headers. */
#include <CLHEP/Vector/ThreeVector.h>

/* C++ headers. */
#include <utility>

using namespace Belle2;
using namespace Belle2::bklm;

REG_MODULE(KLMReconstructor)

static bool compareSector(KLMDigit* d1, KLMDigit* d2)
{
  int s1, s2;
  static const EKLMElementNumbers& elementNumbers =
    EKLMElementNumbers::Instance();
  s1 = elementNumbers.sectorNumber(d1->getSection(), d1->getLayer(),
                                   d1->getSector());
  s2 = elementNumbers.sectorNumber(d2->getSection(), d2->getLayer(),
                                   d2->getSector());
  return s1 < s2;
}

static bool comparePlane(KLMDigit* d1, KLMDigit* d2)
{
  return d1->getPlane() < d2->getPlane();
}

static bool compareStrip(KLMDigit* d1, KLMDigit* d2)
{
  return d1->getStrip() < d2->getStrip();
}

static bool compareTime(KLMDigit* d1, KLMDigit* d2)
{
  return d1->getTime() < d2->getTime();
}

static bool sameSector(KLMDigit* d1, KLMDigit* d2)
{
  return ((d1->getSection() == d2->getSection()) &&
          (d1->getLayer() == d2->getLayer()) &&
          (d1->getSector() == d2->getSector()));
}

KLMReconstructorModule::KLMReconstructorModule() :
  Module(),
  m_CoincidenceWindow(0),
  m_PromptTime(0),
  m_PromptWindow(0),
  m_EventT0Value(0.),
  m_ElementNumbers(&(KLMElementNumbers::Instance())),
  m_bklmGeoPar(nullptr),
  m_eklmElementNumbers(&(EKLMElementNumbers::Instance())),
  m_eklmGeoDat(nullptr),
  m_eklmNStrip(0),
  m_eklmTransformData{nullptr}
{
  setDescription("Create BKLMHit1ds from KLMDigits and then create KLMHit2ds from BKLMHit1ds; create KLMHit2ds from KLMDigits.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("TimeCableDelayCorrection", m_TimeCableDelayCorrection,
           "Perform cable delay time correction (true) or not (false).", true);
  addParam("EventT0Correction", m_EventT0Correction,
           "Perform EventT0 correction (true) or not (false)", true);
  addParam("IfAlign", m_bklmIfAlign,
           "Perform alignment correction (true) or not (false).",
           bool(true));
  addParam("IgnoreScintillators", m_bklmIgnoreScintillators,
           "Ignore scintillators (to debug their electronics mapping).",
           false);
  addParam("CheckSegmentIntersection", m_eklmCheckSegmentIntersection,
           "Check if segments intersect.", true);
  addParam("IgnoreHotChannels", m_IgnoreHotChannels,
           "Use only Normal and Dead (for debugging) channels during 2d hit reconstruction",
           true);
}

KLMReconstructorModule::~KLMReconstructorModule()
{
}

void KLMReconstructorModule::initialize()
{
  m_Digits.isRequired();
  if (m_EventT0Correction)
    m_EventT0.isRequired();
  // This object is registered by both ECL and KLM packages. Let's be agnostic about the
  // execution order of ecl and klm modules: the first package run registers the module
  m_EventLevelClusteringInfo.isOptional() ?
  m_EventLevelClusteringInfo.isRequired() :
  m_EventLevelClusteringInfo.registerInDataStore();
  m_Hit2ds.registerInDataStore();
  m_Hit2ds.registerRelationTo(m_bklmHit1ds);
  m_Hit2ds.registerRelationTo(m_Digits);
  /* BKLM. */
  m_bklmHit1ds.registerInDataStore();
  m_bklmHit1ds.registerRelationTo(m_Digits);
  m_bklmGeoPar = bklm::GeometryPar::instance();
  /* EKLM. */
  m_eklmAlignmentHits.registerInDataStore();
  m_eklmAlignmentHits.registerRelationTo(m_Hit2ds);
  m_eklmTransformData =
    new EKLM::TransformData(true, EKLM::TransformData::c_Alignment);
  m_eklmGeoDat = &(EKLM::GeometryData::Instance());
  if (m_eklmGeoDat->getNPlanes() != 2)
    B2FATAL("It is not possible to run EKLM reconstruction with 1 plane.");
  m_eklmNStrip = m_eklmElementNumbers->getMaximalStripGlobalNumber();
}

void KLMReconstructorModule::beginRun()
{
  if (m_TimeCableDelayCorrection) {
    if (!m_TimeConstants.isValid())
      B2FATAL("KLM time constants data are not available.");
    if (!m_TimeCableDelay.isValid())
      B2FATAL("KLM time cable decay data are not available.");
    if (!m_TimeResolution.isValid())
      B2ERROR("KLM time resolution data are not available. "
              "The error is non-fatal because the data are only used to set "
              "chi^2 of 2d hit, which is informational only now.");
  }
  if (!m_ChannelStatus.isValid())
    B2FATAL("KLM channel status data are not available.");
  if (!m_TimeWindow.isValid())
    B2FATAL("KLM time window data are not available.");
  m_CoincidenceWindow = m_TimeWindow->getCoincidenceWindow();
  m_PromptTime = m_TimeWindow->getPromptTime();
  m_PromptWindow = m_TimeWindow->getPromptWindow();
  if (m_TimeCableDelayCorrection) {
    m_DelayEKLMScintillators = m_TimeConstants->getDelay(
                                 KLMTimeConstants::c_EKLM);
    m_DelayBKLMScintillators = m_TimeConstants->getDelay(
                                 KLMTimeConstants::c_BKLM);
    m_DelayRPCPhi = m_TimeConstants->getDelay(KLMTimeConstants::c_RPCPhi);
    m_DelayRPCZ = m_TimeConstants->getDelay(KLMTimeConstants::c_RPCZ);
  }
}

void KLMReconstructorModule::event()
{
  m_EventT0Value = 0.;
  if (m_EventT0.isValid())
    if (m_EventT0->hasEventT0())
      m_EventT0Value = m_EventT0->getEventT0();
  if (not m_EventLevelClusteringInfo.isValid())
    m_EventLevelClusteringInfo.construct();
  reconstructBKLMHits();
  reconstructEKLMHits();
}

void KLMReconstructorModule::correctCableDelay(double& ct, const KLMDigit* d)
{
  unsigned int cID = d->getUniqueChannelID();
  ct -= m_TimeCableDelay->getTimeDelay(cID);
}

/*
double KLMReconstructorModule::getTime(KLMDigit* d, double dist)
{
  int strip;
  strip = m_eklmElementNumbers->stripNumber(
            d->getSection(), d->getLayer(), d->getSector(),
            d->getPlane(), d->getStrip()) - 1;
  return d->getTime() -
         (dist / m_eklmTimeCalibration->getEffectiveLightSpeed() +
          m_eklmTimeCalibrationData[strip]->getTimeShift());

   // TODO: Subtract time correction given by
   // m_eklmTimeCalibration->getAmplitudeTimeConstant() / sqrt(d->getNPhotoelectrons()).
   // It requires a new firmware version that will be able to extract amplitude.

}
*/

bool KLMReconstructorModule::isNormal(const KLMDigit* digit) const
{
  int subdetector = digit->getSubdetector();
  int section = digit->getSection();
  int sector = digit->getSector();
  int layer = digit->getLayer();
  int plane = digit->getPlane();
  int strip = digit->getStrip();
  KLMChannelNumber channel = m_ElementNumbers->channelNumber(subdetector, section, sector, layer, plane, strip);
  enum KLMChannelStatus::ChannelStatus status = m_ChannelStatus->getChannelStatus(channel);
  if (status == KLMChannelStatus::c_Unknown)
    B2FATAL("Incomplete KLM channel status data.");
  if (status == KLMChannelStatus::c_Normal || status == KLMChannelStatus::c_Dead)
    return true;
  return false;
}

void KLMReconstructorModule::reconstructBKLMHits()
{
  /* Let's count the multi-strip KLMDigits. */
  uint16_t nKLMDigitsMultiStripBarrel{0};
  /* Construct BKLMHit1Ds from KLMDigits. */
  /* Sort KLMDigits by module and strip number. */
  std::map<KLMChannelNumber, int> channelDigitMap;
  for (int index = 0; index < m_Digits.getEntries(); ++index) {
    const KLMDigit* digit = m_Digits[index];
    if (digit->getSubdetector() != KLMElementNumbers::c_BKLM)
      continue;
    if (digit->isMultiStrip()) {
      nKLMDigitsMultiStripBarrel++;
    }
    if (m_bklmIgnoreScintillators && !digit->inRPC())
      continue;
    if (m_IgnoreHotChannels && !isNormal(digit))
      continue;
    if (digit->inRPC() || digit->isGood()) {
      KLMChannelNumber channel = BKLMElementNumbers::channelNumber(
                                   digit->getSection(), digit->getSector(),
                                   digit->getLayer(), digit->getPlane(),
                                   digit->getStrip());
      channelDigitMap.insert(std::pair<KLMChannelNumber, int>(channel, index));
    }
  }
  if (channelDigitMap.empty())
    return;
  std::vector<std::pair<const KLMDigit*, double>> digitCluster;
  KLMChannelNumber previousChannel = channelDigitMap.begin()->first;
  double averageTime = m_Digits[channelDigitMap.begin()->second]->getTime();
  if (m_TimeCableDelayCorrection)
    correctCableDelay(averageTime, m_Digits[channelDigitMap.begin()->second]);
  for (std::map<KLMChannelNumber, int>::iterator it = channelDigitMap.begin(); it != channelDigitMap.end(); ++it) {
    const KLMDigit* digit = m_Digits[it->second];
    double digitTime = digit->getTime();
    if (m_TimeCableDelayCorrection)
      correctCableDelay(digitTime, digit);
    if ((it->first > previousChannel + 1) || (std::fabs(digitTime - averageTime) > m_CoincidenceWindow)) {
      m_bklmHit1ds.appendNew(digitCluster); // Also sets relation BKLMHit1d -> KLMDigit
      digitCluster.clear();
    }
    previousChannel = it->first;
    double n = (double)(digitCluster.size());
    averageTime = (n * averageTime + digitTime) / (n + 1.0);
    digitCluster.emplace_back(std::make_pair(digit, digitTime));
  }
  m_bklmHit1ds.appendNew(digitCluster); // Also sets relation BKLMHit1d -> KLMDigit

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
      CLHEP::Hep3Vector propagationDist;
      if (m_bklmHit1ds[i]->getLayer() < BKLMElementNumbers::c_FirstRPCLayer) {
        if (isPhiReadout) {
          propagationDist = m->getPropagationDistance(
                              local, m_bklmHit1ds[j]->getStripMin(),
                              m_bklmHit1ds[i]->getStripMin());
        } else {
          propagationDist = m->getPropagationDistance(
                              local, m_bklmHit1ds[i]->getStripMin(),
                              m_bklmHit1ds[j]->getStripMin());
        }
      } else {
        propagationDist = m->getPropagationTimes(local);
      }
      double delayPhi, delayZ;
      if (phiHit->inRPC())
        delayPhi = m_DelayRPCPhi;
      else
        delayPhi = m_DelayBKLMScintillators;
      if (zHit->inRPC())
        delayZ = m_DelayRPCZ;
      else
        delayZ = m_DelayBKLMScintillators;
      double phiTime = phiHit->getTime() - propagationDist.y() * delayPhi;
      double zTime = zHit->getTime() - propagationDist.z() * delayZ;
      if (std::fabs(phiTime - zTime) > m_CoincidenceWindow)
        continue;
      // The second param in localToGlobal is whether do the alignment correction (true) or not (false)
      CLHEP::Hep3Vector global = m->localToGlobal(local + m->getLocalReconstructionShift(), m_bklmIfAlign);
      double time = 0.5 * (phiTime + zTime);
      if (m_EventT0Correction)
        time -= m_EventT0Value;
      KLMHit2d* hit2d = m_Hit2ds.appendNew(phiHit, zHit, global, time); // Also sets relation KLMHit2d -> BKLMHit1d
      if (std::fabs(time - m_PromptTime) > m_PromptWindow)
        hit2d->isOutOfTime(true);
    }
  }
  m_EventLevelClusteringInfo->setNKLMDigitsMultiStripBarrel(nKLMDigitsMultiStripBarrel);
}


void KLMReconstructorModule::reconstructEKLMHits()
{
  /* Let's count the multi-strip KLMDigits. */
  uint16_t nKLMDigitsMultiStripFWD{0};
  uint16_t nKLMDigitsMultiStripBWD{0};
  int i, n;
  double d1, d2, time, t1, t2, sd;
  std::vector<KLMDigit*> digitVector;
  std::vector<KLMDigit*>::iterator it1, it2, it3, it4, it5, it6, it7, it8, it9;
  n = m_Digits.getEntries();
  for (i = 0; i < n; i++) {
    KLMDigit* digit = m_Digits[i];
    if (digit->getSubdetector() != KLMElementNumbers::c_EKLM)
      continue;
    if (digit->isMultiStrip()) {
      digit->getSection() == EKLMElementNumbers::c_BackwardSection ? nKLMDigitsMultiStripBWD++ : nKLMDigitsMultiStripFWD++;
    }
    if (m_IgnoreHotChannels && !isNormal(digit))
      continue;
    if (digit->isGood())
      digitVector.push_back(digit);
  }
  KLMDigit plane1Digit; // to look for geometric intersection of a multi-strip hit
  KLMDigit plane2Digit; // to look for geometric intersection of a multi-strip hit
  HepGeom::Point3D<double> crossPoint(0, 0, 0); // (x,y,z) of geometric intersection
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
         * Now it4 .. it5 - hits from a single first-plane strip and
         * it6 .. it7 - hits from a single second-plane strip.
         */
        for (it8 = it4; it8 != it5; ++it8) {
          for (it9 = it6; it9 != it7; ++it9) {
            /*
             * Check for intersection of the two orthogonal strips. On one strip,
             * one hit might be multi-strip and another single-strip, so the
             * intersection check must be done here rather than before the loop.
             */
            bool intersect = false;
            if ((*it8)->isMultiStrip() || (*it9)->isMultiStrip()) {
              plane1Digit = **it8;
              plane2Digit = **it9;
              int s1First = plane1Digit.getStrip();
              int s1Last = std::max(s1First, plane1Digit.getLastStrip());
              int s2First = plane2Digit.getStrip();
              int s2Last = std::max(s2First, plane2Digit.getLastStrip());
              for (int s1 = s1First; s1 <= s1Last; s1++) {
                plane1Digit.setStrip(s1);
                for (int s2 = s2First; s2 <= s2Last; s2++) {
                  plane2Digit.setStrip(s2);
                  intersect = m_eklmTransformData->intersection(&plane1Digit, &plane2Digit, &crossPoint,
                                                                &d1, &d2, &sd,
                                                                m_eklmCheckSegmentIntersection);
                  if (intersect)
                    break;
                }
                if (intersect)
                  break;
              }
              if (intersect) {
                // use the middle strip in the multi-strip group to get the 2D intersection point
                plane1Digit.setStrip((s1First + s1Last) / 2);
                plane2Digit.setStrip((s2First + s2Last) / 2);
                intersect = m_eklmTransformData->intersection(&plane1Digit, &plane2Digit, &crossPoint,
                                                              &d1, &d2, &sd,
                                                              false); // crossPoint MIGHT be outside fiducial area
              }
            } else {
              intersect = m_eklmTransformData->intersection(*it8, *it9, &crossPoint,
                                                            &d1, &d2, &sd,
                                                            m_eklmCheckSegmentIntersection);
            }
            if (!intersect)
              continue;
            t1 = (*it8)->getTime() - d1 * m_DelayEKLMScintillators
                 + 0.5 * sd / Const::speedOfLight;
            t2 = (*it9)->getTime() - d2 * m_DelayEKLMScintillators
                 - 0.5 * sd / Const::speedOfLight;
            if (m_TimeCableDelayCorrection) {
              correctCableDelay(t1, *it8);
              correctCableDelay(t2, *it9);
            }
            if (std::fabs(t1 - t2) > m_CoincidenceWindow)
              continue;
            time = (t1 + t2) / 2;
            if (m_EventT0Correction)
              time -= m_EventT0Value;
            KLMHit2d* hit2d = m_Hit2ds.appendNew(*it8, *it9);
            hit2d->setEnergyDeposit((*it8)->getEnergyDeposit() +
                                    (*it9)->getEnergyDeposit());
            hit2d->setPosition(crossPoint.x(), crossPoint.y(), crossPoint.z());
            double timeResolution = 1.0;
            if (m_TimeResolution.isValid()) {
              timeResolution *= m_TimeResolution->getTimeResolution(
                                  (*it8)->getUniqueChannelID());
              timeResolution *= m_TimeResolution->getTimeResolution(
                                  (*it9)->getUniqueChannelID());
            }
            hit2d->setChiSq((t1 - t2) * (t1 - t2) / timeResolution);
            hit2d->setTime(time);
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
  m_EventLevelClusteringInfo->setNKLMDigitsMultiStripBWD(nKLMDigitsMultiStripBWD);
  m_EventLevelClusteringInfo->setNKLMDigitsMultiStripFWD(nKLMDigitsMultiStripFWD);
}

void KLMReconstructorModule::endRun()
{
}

void KLMReconstructorModule::terminate()
{
  delete m_eklmTransformData;
}
